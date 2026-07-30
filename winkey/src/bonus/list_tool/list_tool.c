#include "winkey.h"


static DWORD find_pid(void)
{
	PROCESSENTRY32 pe = {0};
	pe.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return 0;
	if (!Process32First(snap, &pe)) 
	{
		CloseHandle(snap);
		return 0;
	}
	while (1)
	{
		wchar_t wExeFile[MAX_PATH];
		MultiByteToWideChar(CP_UTF8, 0, pe.szExeFile, -1, wExeFile, MAX_PATH);
		if (_wcsicmp(wExeFile, L"explorer.exe") == 0)
		{
			DWORD pid = pe.th32ProcessID;
			CloseHandle(snap);
			return pid;
		}
		if (!Process32Next(snap, &pe))
			break;
	}
	CloseHandle(snap);
	return 0;
}


int inject_into_explorer(void)
{
	DWORD pid = find_pid();
	if (!pid)
		return 0;
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
		return 0;

	wchar_t *dllPath = L"C:\\path\\to\\your\\hook.dll";
	SIZE_T size = (wcslen(dllPath) + 1) * sizeof(wchar_t);
	LPVOID remoteMem = VirtualAllocEx(hProc, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!remoteMem)
	{
		CloseHandle(hProc);
		return 0;
	}
	if (!WriteProcessMemory(hProc, remoteMem, dllPath, size, NULL))
	{
		VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
		CloseHandle(hProc);
		return 0;
	}
	HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
	FARPROC fp = GetProcAddress(hKernel32, "LoadLibraryW");
	LPTHREAD_START_ROUTINE pLoadLibraryW = (LPTHREAD_START_ROUTINE)(void*)fp;
	HANDLE hThread = CreateRemoteThread( hProc, NULL, 0, pLoadLibraryW, remoteMem, 0, NULL);
	if (!hThread)
	{
		VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
		CloseHandle(hProc);
		return 0;
	}
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
	CloseHandle(hProc);
	return 1;
}


