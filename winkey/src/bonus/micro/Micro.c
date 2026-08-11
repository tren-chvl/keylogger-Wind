#include "winkey.h"
#include <stdio.h>
#include <stdlib.h>


static void write_wav_header(FILE *f, DWORD sampleRate, WORD channels,WORD bitsPerSample)
{
    DWORD byteRate = sampleRate * channels * bitsPerSample / 8;
    WORD blockAlign = (WORD)(channels * bitsPerSample / 8);
    DWORD dataSize = 0;
    DWORD chunkSize = 36;

    fwrite("RIFF", 1, 4, f);
    fwrite(&chunkSize, sizeof(DWORD), 1, f);
    fwrite("WAVE", 1, 4, f);

    fwrite("fmt ", 1, 4, f);

    {
        DWORD subChunk1Size = 16;
        fwrite(&subChunk1Size, sizeof(DWORD), 1, f);
    }

    {
        WORD audioFormat = 1; /* PCM */
        fwrite(&audioFormat, sizeof(WORD), 1, f);
    }

    fwrite(&channels, sizeof(WORD), 1, f);
    fwrite(&sampleRate, sizeof(DWORD), 1, f);
    fwrite(&byteRate, sizeof(DWORD), 1, f);
    fwrite(&blockAlign, sizeof(WORD), 1, f);
    fwrite(&bitsPerSample, sizeof(WORD), 1, f);

    fwrite("data", 1, 4, f);
    fwrite(&dataSize, sizeof(DWORD), 1, f);
}


static void update_wav_header(FILE *f, DWORD totalBytes)
{
    DWORD chunkSize = 36 + totalBytes;

    /* RIFF chunk size */
    fseek(f, 4, SEEK_SET);
    fwrite(&chunkSize, sizeof(DWORD), 1, f);

    /* data chunk size */
    fseek(f, 40, SEEK_SET);
    fwrite(&totalBytes, sizeof(DWORD), 1, f);
}


static void release_devices(IMFActivate **devices, UINT32 count)
{
    UINT32 i;

    if (!devices)
        return;

    for (i = 0; i < count; i++) {
        if (devices[i])
            devices[i]->lpVtbl->Release(devices[i]);
    }

    CoTaskMemFree(devices);
}


int capture_micro(void)
{
    HRESULT hr;

    IMFAttributes *attr = NULL;
    IMFActivate **devices = NULL;
    UINT32 count = 0;

    IMFMediaSource *source = NULL;
    IMFSourceReader *reader = NULL;

    IMFMediaType *outputType = NULL;
    IMFMediaType *currentType = NULL;

    FILE *f = NULL;

    UINT32 sampleRate = 0;
    UINT32 channels = 0;
    UINT32 bits = 0;

    DWORD totalBytes = 0;

    printf("[MF] Initialisation...\n");

    /*
     * Initialisation de Media Foundation
     */
    hr = MFStartup((ULONG)MF_VERSION, MFSTARTUP_FULL);

    if (FAILED(hr)) {
        printf("[MF] MFStartup FAILED: 0x%08lX\n",
               (unsigned long)hr);
        return 0;
    }

    /*
     * Création des attributs utilisés pour rechercher
     * les périphériques audio de capture.
     */
    hr = MFCreateAttributes(&attr, 1);

    if (FAILED(hr)) {
        printf("[MF] MFCreateAttributes FAILED: 0x%08lX\n",
               (unsigned long)hr);
        MFShutdown();
        return 0;
    }

    /*
     * On demande uniquement les périphériques
     * de capture audio.
     */
    hr = attr->lpVtbl->SetGUID(
        attr,
        &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID
    );

    if (FAILED(hr)) {
        printf("[MF] SetGUID FAILED: 0x%08lX\n",
               (unsigned long)hr);

        attr->lpVtbl->Release(attr);
        MFShutdown();
        return 0;
    }

    /*
     * Énumération des microphones.
     */
    hr = MFEnumDeviceSources(
        attr,
        &devices,
        &count
    );

    attr->lpVtbl->Release(attr);
    attr = NULL;

    printf("Devices count = %u\n", (unsigned)count);

    if (FAILED(hr) || count == 0) {
        printf("[MF] Aucun micro trouvé.\n");

        if (devices)
            CoTaskMemFree(devices);

        MFShutdown();
        return 0;
    }

    printf("[MF] Micro trouvé.\n");

    /*
     * On utilise le premier périphérique.
     */
    hr = devices[0]->lpVtbl->ActivateObject(
        devices[0],
        &IID_IMFMediaSource,
        (void **)&source
    );

    if (FAILED(hr)) {
        printf("[MF] ActivateObject FAILED: 0x%08lX\n",
               (unsigned long)hr);

        release_devices(devices, count);
        MFShutdown();
        return 0;
    }

    /*
     * Le tableau IMFActivate n'est plus nécessaire
     * après activation du périphérique.
     */
    release_devices(devices, count);
    devices = NULL;
    count = 0;

    /*
     * Création du Source Reader.
     */
    hr = MFCreateSourceReaderFromMediaSource(
        source,
        NULL,
        &reader
    );

    if (FAILED(hr)) {
        printf("[MF] MFCreateSourceReaderFromMediaSource FAILED: 0x%08lX\n",
               (unsigned long)hr);

        source->lpVtbl->Release(source);
        MFShutdown();
        return 0;
    }

    /*
     * On demande explicitement du PCM.
     *
     * 16 bits
     * même fréquence que le micro
     * même nombre de canaux que le micro
     */

    /*
     * Récupération du format actuel du stream audio.
     */
    hr = reader->lpVtbl->GetCurrentMediaType(
        reader,
        (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        &currentType
    );

    if (FAILED(hr)) {
        printf("[MF] GetCurrentMediaType FAILED: 0x%08lX\n",
               (unsigned long)hr);

        reader->lpVtbl->Release(reader);
        source->lpVtbl->Release(source);
        MFShutdown();
        return 0;
    }

    /*
     * Fréquence d'échantillonnage.
     */
    hr = currentType->lpVtbl->GetUINT32(
        currentType,
        &MF_MT_AUDIO_SAMPLES_PER_SECOND,
        &sampleRate
    );

    if (FAILED(hr)) {
        printf("[MF] Impossible de récupérer le sample rate.\n");

        currentType->lpVtbl->Release(currentType);
        reader->lpVtbl->Release(reader);
        source->lpVtbl->Release(source);
        MFShutdown();
        return 0;
    }

    /*
     * Nombre de canaux.
     */
    hr = currentType->lpVtbl->GetUINT32(
        currentType,
        &MF_MT_AUDIO_NUM_CHANNELS,
        &channels
    );

    if (FAILED(hr)) {
        printf("[MF] Impossible de récupérer le nombre de canaux.\n");

        currentType->lpVtbl->Release(currentType);
        reader->lpVtbl->Release(reader);
        source->lpVtbl->Release(source);
        MFShutdown();
        return 0;
    }

    currentType->lpVtbl->Release(currentType);
    currentType = NULL;

    /*
     * On impose du PCM 16 bits.
     */
    bits = 16;

    printf(
        "[MF] Format : %lu Hz / %u canaux / %u bits PCM\n",
        (unsigned long)sampleRate,
        (unsigned)channels,
        (unsigned)bits
    );

    /*
     * Création du type de sortie.
     */
    hr = MFCreateMediaType(&outputType);

    if (FAILED(hr)) {
        printf("[MF] MFCreateMediaType FAILED: 0x%08lX\n",
               (unsigned long)hr);

        reader->lpVtbl->Release(reader);
        source->lpVtbl->Release(source);
        MFShutdown();
        return 0;
    }

    /*
     * Type principal : Audio.
     */
    hr = outputType->lpVtbl->SetGUID(
        outputType,
        &MF_MT_MAJOR_TYPE,
        &MFMediaType_Audio
    );

    if (FAILED(hr))
        goto cleanup;

    /*
     * Sous-type : PCM.
     */
    hr = outputType->lpVtbl->SetGUID(
        outputType,
        &MF_MT_SUBTYPE,
        &MFAudioFormat_PCM
    );

    if (FAILED(hr))
        goto cleanup;

    /*
     * Sample rate.
     */
    hr = outputType->lpVtbl->SetUINT32(
        outputType,
        &MF_MT_AUDIO_SAMPLES_PER_SECOND,
        sampleRate
    );

    if (FAILED(hr))
        goto cleanup;

    /*
     * Nombre de canaux.
     */
    hr = outputType->lpVtbl->SetUINT32(
        outputType,
        &MF_MT_AUDIO_NUM_CHANNELS,
        channels
    );

    if (FAILED(hr))
        goto cleanup;

    /*
     * 16 bits.
     */
    hr = outputType->lpVtbl->SetUINT32(
        outputType,
        &MF_MT_AUDIO_BITS_PER_SAMPLE,
        bits
    );

    if (FAILED(hr))
        goto cleanup;

    /*
     * Configuration du format de sortie du Source Reader.
     */
    hr = reader->lpVtbl->SetCurrentMediaType(
        reader,
        (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        NULL,
        outputType
    );

    if (FAILED(hr)) {
        printf(
            "[MF] SetCurrentMediaType FAILED: 0x%08lX\n",
            (unsigned long)hr
        );
        goto cleanup;
    }

    printf("[MF] PCM configuré.\n");

    /*
     * Création du fichier WAV.
     */
    f = fopen("capture/capture.wav", "wb");

    if (!f) {
        printf("[MF] Impossible de créer capture.wav\n");
        goto cleanup;
    }

    /*
     * Écriture de l'en-tête WAV.
     */
    write_wav_header(
        f,
        sampleRate,
        (WORD)channels,
        (WORD)bits
    );

    printf("[MF] Capture en cours... ESC pour arreter.\n");

    /*
     * Boucle de capture.
     */
    while (g_micro_run) {

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
    		g_micro_run = 0;
    		break;
		}
        IMFSample *sample = NULL;
        IMFMediaBuffer *buffer = NULL;

        DWORD flags = 0;

        BYTE *data = NULL;

        DWORD maxLen = 0;
        DWORD curLen = 0;

        /*
         * Lecture d'un échantillon audio.
         */
        hr = reader->lpVtbl->ReadSample(
            reader,
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0,
            NULL,
            &flags,
            NULL,
            &sample
        );

        if (FAILED(hr)) {
            printf(
                "[MF] ReadSample FAILED: 0x%08lX\n",
                (unsigned long)hr
            );

            if (sample)
                sample->lpVtbl->Release(sample);

            break;
        }

        /*
         * Fin du stream.
         */
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            if (sample)
                sample->lpVtbl->Release(sample);

            break;
        }

        /*
         * Il peut arriver qu'un ReadSample ne retourne
         * pas immédiatement de sample.
         */
        if (!sample) {
            Sleep(1);
            continue;
        }

        /*
         * Transformation en buffer contigu.
         */
        hr = sample->lpVtbl->ConvertToContiguousBuffer(
            sample,
            &buffer
        );

        if (SUCCEEDED(hr)) {

            hr = buffer->lpVtbl->Lock(
                buffer,
                &data,
                &maxLen,
                &curLen
            );

            if (SUCCEEDED(hr)) {

                if (curLen > 0) {
                    fwrite(data, 1, curLen, f);
                    totalBytes += curLen;
                }

                buffer->lpVtbl->Unlock(buffer);
            }

            buffer->lpVtbl->Release(buffer);
        }

        sample->lpVtbl->Release(sample);

        /*
         * Petite pause pour éviter de monopoliser le CPU.
         */
        Sleep(1);
    }

    /*
     * Mise à jour de l'en-tête WAV avec la taille réelle.
     */
    update_wav_header(f, totalBytes);

    fclose(f);
    f = NULL;

    printf(
        "[MF] Capture terminée. %lu octets enregistrés.\n",
        (unsigned long)totalBytes
    );

cleanup:

    if (f)
        fclose(f);

    if (outputType)
        outputType->lpVtbl->Release(outputType);

    if (reader)
        reader->lpVtbl->Release(reader);

    if (source)
        source->lpVtbl->Release(source);

    MFShutdown();

    if (FAILED(hr))
        return 0;

    return 1;
}
