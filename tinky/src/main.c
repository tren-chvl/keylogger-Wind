/**
 * @file main.c
 * @brief Main entry point and global variable definitions for the tinky service.
 * 
 * Usage:
 *   svc.exe install   -> registers "tinky" with SCM
 *   svc.exe start     -> starts the service
 *   svc.exe stop      -> stops the service
 *   svc.exe delete    -> stops (if running) and removes the service
 * 
 * If run without arguments by the SCM, starts the service control dispatcher.
 */

#pragma comment(linker, "/ENTRY:mainCRTStartup")
#include "tinky.h"

/* ---------- global variable definitions ---------- */

SERVICE_STATUS        g_ServiceStatus    = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle     = 0;
HANDLE                g_ServiceStopEvent = NULL;

PROCESS_INFORMATION   g_WinkeyProcInfo   = { 0 };
HANDLE                g_SingleInstanceMutex = NULL;
CRITICAL_SECTION      g_ProcLock;

/**
 * @brief Standard main entry point of the executable.
 * 
 * Directs execution to SCM setup commands if arguments are supplied, or
 * connects to SCM if executed as a service by Windows.
 * 
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @return int 0 on success, or non-zero error code.
 */
int main(int argc, char *argv[])
{
    if (argc > 1) {
        if      (strcmp(argv[1], "install") == 0) { InstallService();     return 0; }
        else if (strcmp(argv[1], "start")   == 0) { StartServiceByName(); return 0; }
        else if (strcmp(argv[1], "stop")    == 0) { StopServiceByName();  return 0; }
        else if (strcmp(argv[1], "delete")  == 0) { DeleteServiceByName(); return 0; }
        else {
            printf("Unknown option: %s\n", argv[1]);
            printf("Usage: %s [install|start|stop|delete]\n", argv[0]);
            return 1;
        }
    }

    /* Start the service controller dispatcher to link with ServiceMain */
    {
        SERVICE_TABLE_ENTRYA ServiceTable[] = {
            { (LPSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTIONA)ServiceMain },
            { NULL, NULL }
        };

        if (!StartServiceCtrlDispatcherA(ServiceTable)) {
            DWORD err = GetLastError();
            if (err == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
                printf("Usage: svc.exe [install|start|stop|delete]\n");
            else
                printf("StartServiceCtrlDispatcher failed (%lu)\n", (unsigned long)err);
            return 1;
        }
    }

    return 0;
}
