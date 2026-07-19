#include "../../../winkey.h"

int is_sensitive_key(DWORD vk)
{
	switch (vk)
	{
		case VK_RETURN:
		case VK_TAB:
		case VK_ESCAPE:
		case VK_BACK:
		case VK_DELETE:
		case VK_F12:
		case VK_F5:
		case VK_SNAPSHOT:
		case VK_LWIN:
		case VK_RWIN:
			return 1;
		case 'X':
		case 'C':
		case 'V':
		case 'S':
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				return 1;
			break;
	}

	return 0;
}


