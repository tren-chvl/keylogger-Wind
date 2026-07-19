#include "../../../winkey.h"


int allow_app(char *exe)
{
	char *blacklist[] = 
	{
		"firefox.exe",
        "notepad.exe"
		NULL
	};
	for (int i = 0; blacklist[i];i++)
	{
		if (strstr(exe, blacklist[i]))
			return 0;
	}
	return 1;
}