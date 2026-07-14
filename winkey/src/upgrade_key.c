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
