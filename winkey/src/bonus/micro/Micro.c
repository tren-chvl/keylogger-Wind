#include "../../winkey.h"


void capture_micro_demo(void)
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
	for (int i = 0; i < 10; i++)
	{
		get_buffer(&hr, captureClient);
		Sleep(50);
	}
	audioClient->lpVtbl->Stop(audioClient);
	CoUninitialize();
}


int get_buffer(HRESULT *hr, IAudioCaptureClient *captureClient)
{
	BYTE *data = NULL;
	UINT32 numFrames = 0;
	DWORD flags = 0;

	*hr = captureClient->lpVtbl->GetBuffer(
		captureClient,
		&data,
		&numFrames,
		&flags,
		NULL,
		NULL
	);

	if (!SUCCEEDED(*hr))
		return 0;
	printf("Buffer audio reçu (DEMO) : %u frames\n", numFrames);
	captureClient->lpVtbl->ReleaseBuffer(captureClient, numFrames);
	return 1;
}
