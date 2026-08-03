#include "hide_process.h"

LPVOID load_pe_from_file(LPCWSTR path, DWORD *outSize)
{
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;
    DWORD size = GetFileSize(hFile, NULL);
    if (size == INVALID_FILE_SIZE || size == 0) 
    {
        CloseHandle(hFile);
        return NULL;
    }
    LPVOID buf = HeapAlloc(GetProcessHeap(), 0, size);
    if (!buf) 
    {
        CloseHandle(hFile);
        return NULL;
    }
    DWORD read = 0;
    if (!ReadFile(hFile, buf, size, &read, NULL) || read != size)
    {
        HeapFree(GetProcessHeap(), 0, buf);
        CloseHandle(hFile);
        return NULL;
    }
    CloseHandle(hFile);
    *outSize = size;
    return buf;
}
