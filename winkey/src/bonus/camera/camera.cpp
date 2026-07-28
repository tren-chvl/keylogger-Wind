extern "C" 
{
	#include "../../../winkey.h"
}
IMFSourceReader *pReader = NULL;

extern "C" DWORD WINAPI camera_run(LPVOID lp)
{
	HRESULT hr = S_OK;
	IMFSample *pSample = NULL;

	while (1)
	{
		if (!g_camera_run)
		{
			Sleep(10);
			continue;
		}

		DWORD streamIndex = 0;
		DWORD flags = 0;
		LONGLONG timestamp = 0;

		hr = pReader->ReadSample(
			MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0,
			&streamIndex,
			&flags,
			&timestamp,
			&pSample
		);

		if (SUCCEEDED(hr) && pSample)
		{
			IMFMediaBuffer *pBuffer = NULL;
			BYTE *pData = NULL;
			DWORD maxLen = 0;
			DWORD curLen = 0;

			hr = pSample->GetBufferByIndex(0, &pBuffer);
			if (SUCCEEDED(hr))
			{
				hr = pBuffer->Lock(&pData, &maxLen, &curLen);
				if (SUCCEEDED(hr))
				{
					printf("Frame reçue : %lu bytes\n", curLen);
					pBuffer->Unlock();
				}
				pBuffer->Release();
			}
			pSample->Release();
		}

		Sleep(1);
	}

	return 0;
}

extern "C" int start_camera(void)
{
	HRESULT hr = S_OK;

	hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
	if (FAILED(hr))
		return 0;

	IMFAttributes *pAttributes = NULL;
	hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr))
		return 0;

	hr = pAttributes->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
	);

	IMFActivate **ppDevices = NULL;
	UINT32 count = 0;

	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
	if (FAILED(hr) || count == 0)
	{
		printf("No cameras found ...\n");
		return 0;
	}

	IMFMediaSource *pSource = NULL;
	hr = ppDevices[0]->ActivateObject(IID_IMFMediaSource, (void**)&pSource);
	if (FAILED(hr))
	{
		printf("Error: can't activate camera.\n");
		return 0;
	}

	hr = MFCreateSourceReaderFromMediaSource(pSource, NULL, &pReader);
	if (FAILED(hr))
	{
		printf("Error can't create the SourceReader.\n");
		return 0;
	}

	CreateThread(NULL, 0, camera_run, NULL, 0, NULL);
	printf("CAMERA ACTIVED CHEF ;)\n");

	return 1;
}
