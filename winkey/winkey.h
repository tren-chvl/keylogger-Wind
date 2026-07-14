#ifndef WINKEY_H
#define WINKEY_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>

int vk_to_char(DWORD vkCode, char *out);

#endif