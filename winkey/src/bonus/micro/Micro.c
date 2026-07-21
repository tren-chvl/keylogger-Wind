#include "../../winkey.h"


void write_wav_header(FILE *f, WAVEFORMATEX *fmt)
{
	DWORD chunkSize = 36;
	DWORD dataSize  = 0;

	fwrite("RIFF", 1, 4, f);
	fwrite(&chunkSize, 4, 1, f);
	fwrite("WAVE", 1, 4, f);
	fwrite("fmt ", 1, 4, f);
	DWORD subChunk1Size = 16;
	fwrite(&subChunk1Size, 4, 1, f);
	fwrite(&fmt->wFormatTag, 2, 1, f);
	fwrite(&fmt->nChannels, 2, 1, f);
	fwrite(&fmt->nSamplesPerSec, 4, 1, f);
	fwrite(&fmt->nAvgBytesPerSec, 4, 1, f);
	fwrite(&fmt->nBlockAlign, 2, 1, f);
	fwrite(&fmt->wBitsPerSample, 2, 1, f);
	fwrite("data", 1, 4, f);
	fwrite(&dataSize, 4, 1, f);
}


void update_wav_header(FILE *f, WAVEFORMATEX *fmt, DWORD totalBytes)
{
	DWORD chunkSize = 36 + totalBytes;
	fseek(f, 4, SEEK_SET);
	fwrite(&chunkSize, 4, 1, f);
	fseek(f, 40, SEEK_SET);
	fwrite(&totalBytes, 4, 1, f);
}

void write_samples(FILE *f, BYTE *data, UINT32 numFrames, WAVEFORMATEX *fmt, DWORD *totalBytes)
{
	UINT32 bytes = numFrames * fmt->nBlockAlign;
	fwrite(data, 1, bytes, f);
	*totalBytes += bytes;
}

void capture_micro(void)
{
	HRESULT hr;
	CoInitialize(NULL);
	IMMDeviceEnumerator *enumerator = NULL;
	IMMDevice *device = NULL;
	IAudioClient *audioClient = NULL;
	IAudioCaptureClient *captureClient = NULL;
	WAVEFORMATEX *format = NULL;

	if (!create_sound(&hr, &enumerator))
		return;
	if (!get_micro(&hr, enumerator, &device))
		return;
	if (!active_client_audio(&hr, device, &audioClient))
		return;
	if (!take_format_audio(&hr, audioClient, &format))
		return;
	if (!init_wasapi(&hr, audioClient, format))
		return;
	if (!get_service(&hr, audioClient, &captureClient))
		return;
	if (!start_service(&hr, audioClient))	
		return;
	FILE *f = fopen("capture.wav", "wb");
	if (!f) return;
	write_wav_header(f, format);
	DWORD totalBytes = 0;
	printf("Enregistrement... Appuie sur ESC pour arrêter.\n");
	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
	{
		BYTE *data;
		UINT32 numFrames;
		DWORD flags;

		hr = captureClient->lpVtbl->GetBuffer(captureClient,&data,
		&numFrames,&flags,NULL,NULL
		);
		if (SUCCEEDED(hr))
		{
			printf("Frames: %u | Flags: 0x%08X\n", numFrames, flags);
			if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
			{
				printf("→ Son reçu !\n");
				write_samples(f, data, numFrames, format, &totalBytes);
			}
			else
			{
				printf("→ Silence\n");
			}
		captureClient->lpVtbl->ReleaseBuffer(captureClient, numFrames);
		}
		Sleep(5);
	}
	audioClient->lpVtbl->Stop(audioClient);
	update_wav_header(f, format, totalBytes);
	fclose(f);
	printf("Capture terminée. %u octets écrits dans capture.wav\n", totalBytes);
	CoUninitialize();
}


