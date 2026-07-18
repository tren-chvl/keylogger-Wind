#include "../../../winkey.h"


int save_bitmap(LPCWSTR path, HBITMAP hbm, HDC hdc)
{
    BITMAP bm;
    GetObject(hbm, sizeof(bm), &bm);
    BITMAPFILEHEADER bmfHeader = {0};
    BITMAPINFOHEADER bi = {0};
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth       = bm.bmWidth;
    bi.biHeight      = bm.bmHeight;
    bi.biPlanes      = 1;
    bi.biBitCount    = 32;
    bi.biCompression = BI_RGB;
    DWORD dwBmpSize = ((bm.bmWidth * bi.biBitCount + 31) / 32) * 4 * bm.bmHeight;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (!hDIB)
        return 0;
    char *lpbitmap = (char *)GlobalLock(hDIB);
    GetDIBits(hdc, hbm, 0, (UINT)bm.bmHeight,
              lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        return 0;
    }
    DWORD dwSizeOfHeaders = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    DWORD dwSizeOfImage   = dwBmpSize;
    DWORD dwFileSize      = dwSizeOfHeaders + dwSizeOfImage;
    bmfHeader.bfType  = 0x4D42; // 'BM'
    bmfHeader.bfSize  = dwFileSize;
    bmfHeader.bfOffBits = dwSizeOfHeaders;
    DWORD written = 0;
    WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &written, NULL);
    WriteFile(hFile, &bi, sizeof(bi), &written, NULL);
    WriteFile(hFile, lpbitmap, dwSizeOfImage, &written, NULL);
    CloseHandle(hFile);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    return 1;
}

int capture_screen(LPCWSTR path)
{
    HDC hScreen = GetDC(NULL);
    if (!hScreen)
        return 0;
    int width  = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HDC hMemDC = CreateCompatibleDC(hScreen);
    if (!hMemDC)
    {
        ReleaseDC(NULL, hScreen);
        return 0;
    }
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    if (!hBitmap)
    {
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hScreen);
        return 0;
    }
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, width, height,
           hScreen, 0, 0, SRCCOPY);
    int ok = save_bitmap(path, hBitmap, hMemDC);
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreen);
    return ok;
}
