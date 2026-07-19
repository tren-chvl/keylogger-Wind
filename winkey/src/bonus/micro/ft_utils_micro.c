#include "../../winkey.h"


int create_sound(HRESULT *hr, IMMDeviceEnumerator **enumerator)
{
	*hr = CoCreateInstance(
		&CLSID_MMDeviceEnumerator,
		NULL,
		CLSCTX_ALL,
		&IID_IMMDeviceEnumerator,
		(void**)enumerator
	);
	return SUCCEEDED(*hr);
}

int get_micro(HRESULT *hr, IMMDeviceEnumerator *enumerator, IMMDevice **device)
{
	*hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(
		enumerator,
		eCapture,
		eCommunications,
		device
	);

	return SUCCEEDED(*hr);
}

int active_client_audio(HRESULT *hr, IMMDevice *device, IAudioClient **audioClient)
{
	*hr = device->lpVtbl->Activate(
		device,
		&IID_IAudioClient,
		CLSCTX_ALL,
		NULL,
		(void**)audioClient
	);

	return SUCCEEDED(*hr);
}

int take_format_audio(HRESULT *hr, IAudioClient *audioClient, WAVEFORMATEX **format)
{
	*hr = audioClient->lpVtbl->GetMixFormat(audioClient, format);

	return SUCCEEDED(*hr);
}


int init_wasapi(HRESULT *hr, IAudioClient *audioClient, WAVEFORMATEX *format)
{
	*hr = audioClient->lpVtbl->Initialize(
		audioClient,
		AUDCLNT_SHAREMODE_SHARED,
		0,
		0,
		0,
		format,
		NULL
	);

	return SUCCEEDED(*hr);
}

int get_service(HRESULT *hr, IAudioClient *audioClient, IAudioCaptureClient **captureClient)
{
	*hr = audioClient->lpVtbl->GetService(
		audioClient,
		&IID_IAudioCaptureClient,
		(void**)captureClient
	);

	return SUCCEEDED(*hr);
}

int start_service(HRESULT *hr, IAudioClient *audioClient)
{
	*hr = audioClient->lpVtbl->Start(audioClient);

	return SUCCEEDED(*hr);
}