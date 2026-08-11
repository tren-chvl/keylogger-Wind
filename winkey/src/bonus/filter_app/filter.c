#include "winkey.h"

int is_user_blacklisted(HANDLE hProcess)
{
	HANDLE hToken = NULL;
	if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
		return 0;
	DWORD size = 0;
	GetTokenInformation(hToken, TokenUser, NULL, 0, &size);
	TOKEN_USER *user = (TOKEN_USER *)malloc(size);
	if (!GetTokenInformation(hToken, TokenUser, user, size, &size))
	{
		free(user);
		CloseHandle(hToken);
		return 0;
	}
	LPWSTR sidString = NULL;
	ConvertSidToStringSidW(user->User.Sid, &sidString);
	WCHAR name[256];
	WCHAR domain[256];
	DWORD nameLen = 256;
	DWORD domainLen = 256;
	SID_NAME_USE use;
	if (!LookupAccountSidW(NULL, user->User.Sid, name, &nameLen, domain, &domainLen, &use))
	{
		LocalFree(sidString);
		free(user);
		CloseHandle(hToken);
		return 0;
	}
	char username[256];
	WideCharToMultiByte(CP_UTF8, 0, name, -1, username, 256, NULL, NULL);
	char *user_blacklist[] =
	{
		"admin",
		NULL
	};
	for (int i = 0; user_blacklist[i]; i++) 
	{
		if (_stricmp(username, user_blacklist[i]) == 0)
		{
			printf("USER BLOCKED: %s\n", username);
			LocalFree(sidString);
			free(user);
			CloseHandle(hToken);
			return 1;
		}
	}
	LocalFree(sidString);
	free(user);
	CloseHandle(hToken);
	return 0;
}


int allow_app(char *exe, DWORD pid)
{
	char *blacklist[] = 
	{
		"firefox.exe",
		"notepad.exe",
		"powershell.exe",
		NULL
	};
	for (int i = 0; blacklist[i]; i++)
	{
		if (strstr(exe, blacklist[i]))
			return 0;
	}
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (!hProc)
		return 1;
	if (is_user_blacklisted(hProc))
	{
		CloseHandle(hProc);
		return 0;
	}
	CloseHandle(hProc);
	return 1;
}
