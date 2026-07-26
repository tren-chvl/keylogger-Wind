/**
 * @file scm.c
 * @brief Implementation of SCM (Service Control Manager) management functions.
 *        These functions allow the installation, starting, stopping, and deletion of the tinky service.
 */

#include "tinky.h"

/**
 * @brief Registers/installs the "tinky" service with the Windows Service Control Manager.
 * 
 * This function retrieves the full path of the current executable, opens a connection to the local
 * SCM with full privileges, and registers "tinky" as a Win32 standalone process service. 
 * Requires administrator privileges.
 */
void InstallService(void)
{
    char szPath[MAX_PATH];
    SC_HANDLE schSCManager, schService;

    /* Get the full path of the current executable module to use as binary path */
    if (!GetModuleFileNameA(NULL, szPath, MAX_PATH)) {
        printf("GetModuleFileName failed (%lu)\n", (unsigned long)GetLastError());
        return;
    }

    /* Open SCM database with all accesses */
    schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager) {
        printf("OpenSCManager failed (%lu). Run as Administrator.\n",
               (unsigned long)GetLastError());
        return;
    }

    /* Create and register the service */
    schService = CreateServiceA(
        schSCManager,
        SERVICE_NAME,
        SERVICE_DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        szPath,
        NULL, NULL, NULL, NULL, NULL);

    if (!schService) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS)
            printf("Service already exists.\n");
        else
            printf("CreateService failed (%lu)\n", (unsigned long)err);
    } else {
        printf("\"%s\" installed successfully.\n", SERVICE_NAME);
        CloseServiceHandle(schService);
    }
    CloseServiceHandle(schSCManager);
}

/**
 * @brief Sends a start signal to the "tinky" service via the SCM.
 * 
 * Connects to SCM and opens the registered "tinky" service to launch it immediately.
 */
void StartServiceByName(void)
{
    SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!schSCManager) { 
        printf("OpenSCManager failed (%lu)\n", (unsigned long)GetLastError()); 
        return; 
    }

    SC_HANDLE schService = OpenServiceA(schSCManager, SERVICE_NAME, SERVICE_START);
    if (!schService) {
        printf("OpenService failed (%lu)\n", (unsigned long)GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!StartServiceA(schService, 0, NULL))
        printf("StartService failed (%lu)\n", (unsigned long)GetLastError());
    else
        printf("Start signal sent to \"%s\".\n", SERVICE_NAME);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

/**
 * @brief Sends a stop signal to the "tinky" service via the SCM.
 * 
 * Commands the SCM to transition the running service into the stopped state.
 */
void StopServiceByName(void)
{
    SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!schSCManager) { 
        printf("OpenSCManager failed (%lu)\n", (unsigned long)GetLastError()); 
        return; 
    }

    SC_HANDLE schService = OpenServiceA(schSCManager, SERVICE_NAME,
                                         SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!schService) {
        printf("OpenService failed (%lu)\n", (unsigned long)GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    SERVICE_STATUS status;
    if (!ControlService(schService, SERVICE_CONTROL_STOP, &status))
        printf("ControlService(stop) failed (%lu)\n", (unsigned long)GetLastError());
    else
        printf("Stop signal sent to \"%s\".\n", SERVICE_NAME);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

/**
 * @brief Stops (if running) and deletes the "tinky" service from SCM registration.
 * 
 * First queries the SCM to check the service state. If running, issues a stop request 
 * and waits up to 10 seconds for it to stop before calling DeleteService.
 */
void DeleteServiceByName(void)
{
    SC_HANDLE schSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager) { 
        printf("OpenSCManager failed (%lu)\n", (unsigned long)GetLastError()); 
        return; 
    }

    SC_HANDLE schService = OpenServiceA(schSCManager, SERVICE_NAME,
                                         DELETE | SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!schService) {
        printf("OpenService failed (%lu)\n", (unsigned long)GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    SERVICE_STATUS status;
    if (QueryServiceStatus(schService, &status) &&
        status.dwCurrentState != SERVICE_STOPPED)
    {
        if (ControlService(schService, SERVICE_CONTROL_STOP, &status)) {
            int i;
            printf("Stopping service before deletion...\n");
            for (i = 0; i < 20; i++) {
                if (!QueryServiceStatus(schService, &status)) break;
                if (status.dwCurrentState == SERVICE_STOPPED)  break;
                Sleep(500);
            }
        }
    }

    if (!DeleteService(schService))
        printf("DeleteService failed (%lu)\n", (unsigned long)GetLastError());
    else
        printf("\"%s\" deleted successfully.\n", SERVICE_NAME);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}
