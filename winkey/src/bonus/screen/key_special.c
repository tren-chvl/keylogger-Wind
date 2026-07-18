#include "../../../winkey.h"

int is_sensitive_key(DWORD vk)
{
	switch (vk)
	{
		case VK_RETURN:     // Enter
		case VK_TAB:        // Navigation
		case VK_ESCAPE:     // Cancel
		case VK_BACK:       // Effacer
		case VK_DELETE:     // Supprimer
		case VK_F12:        // DevTools
		case VK_F5:         // Refresh
		case VK_SNAPSHOT:   // PrintScreen
		case VK_LWIN:       // Windows key
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


