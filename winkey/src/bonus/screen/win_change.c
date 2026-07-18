#include "../../../winkey.h"
static HWND last_window = NULL;

void screenshot_on_window_change(void)
{
    HWND current = GetForegroundWindow();
    if (current && current != last_window)
    {
        capture_screen(L"C:\\winkey_screens\\window_change.bmp");
        last_window = current;
    }
}
