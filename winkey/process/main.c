#include <stdio.h>
#include <windows.h>
#include <winternl.h>

int main(void)
{
	if (!MoveFileW(
		L"C:\\Users\\marcc\\keylogger-Wind\\winkey\\winkey.exe",
		L"C:\\Users\\marcc\\keylogger-Wind\\winkey\\key.exe"
	)) 
	{
		wprintf(L"Rename failed: %lu\n", GetLastError());
		return 1;
	}
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);
	if (!CreateProcessW(L"C:\\Users\\marcc\\keylogger-Wind\\winkey\\key.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	)) 
	{
		wprintf(L"CreateProcess failed: %lu\n", GetLastError());
		return 1;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}
