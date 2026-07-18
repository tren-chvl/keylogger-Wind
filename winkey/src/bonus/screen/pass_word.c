#include "../../../winkey.h"

int is_password_field(HWND hwnd)
{
	if (!hwnd)
		return 0;

	wchar_t className[64];
	GetClassNameW(hwnd, className, 64);
	if (_wcsicmp(className, L"Edit") != 0)
		return 0;
	LONG style = GetWindowLongW(hwnd, GWL_STYLE);
	if (style & ES_PASSWORD)
		return 1;
	return 0;
}
