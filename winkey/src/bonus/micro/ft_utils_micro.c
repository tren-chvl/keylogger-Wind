#define INITGUID
#include <initguid.h>

// IMMDeviceEnumerator
DEFINE_GUID(IID_IMMDeviceEnumerator,
0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);

// MMDeviceEnumerator
DEFINE_GUID(CLSID_MMDeviceEnumerator,
0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);

// IAudioClient
DEFINE_GUID(IID_IAudioClient,
0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC5, 0x8D, 0xC2, 0x8D, 0xA0, 0x3F);

// IAudioCaptureClient
DEFINE_GUID(IID_IAudioCaptureClient,
0xC8ADBD64, 0xE71E, 0x48a0, 0xA4, 0x0C, 0xFA, 0x0D, 0xC5, 0x8F, 0xE2, 0xC3);


#include <mmdeviceapi.h>
#include <audioclient.h>

#include "../../../winkey.h"
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