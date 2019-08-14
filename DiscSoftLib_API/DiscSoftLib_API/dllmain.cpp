#include <stdio.h>
#include <Windows.h>
#include "DiscSoftLib.h"

#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include "detours.h"
#pragma comment(lib, "detours.lib")

typedef BOOL(*detour_CryptQueryObject)(DWORD, const void*, DWORD, DWORD,
	DWORD, DWORD*, DWORD*, DWORD*, HCERTSTORE*, HCRYPTMSG*, const void**);
detour_CryptQueryObject fpCryptQueryObject = CryptQueryObject;

typedef HANDLE(*detour_CreateFileW)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
detour_CreateFileW fpCreateFileW = CreateFileW;

BOOL Hooked_CryptQueryObject(
	DWORD      dwObjectType,
	const void *pvObject,
	DWORD      dwExpectedContentTypeFlags,
	DWORD      dwExpectedFormatTypeFlags,
	DWORD      dwFlags,
	DWORD      *pdwMsgAndCertEncodingType,
	DWORD      *pdwContentType,
	DWORD      *pdwFormatType,
	HCERTSTORE *phCertStore,
	HCRYPTMSG  *phMsg,
	const void **ppvContext) {
	auto lpFileName = StrStrIW((LPWSTR)pvObject, L"DiscSoftLib.dll");
	if (lpFileName != NULL)
		StrCpyW(lpFileName, DiscSoftLib);
	return fpCryptQueryObject(dwObjectType, pvObject, dwExpectedContentTypeFlags, dwExpectedFormatTypeFlags,
		dwFlags, pdwMsgAndCertEncodingType, pdwContentType, pdwFormatType, phCertStore, phMsg, ppvContext);
}

HANDLE Hooked_CreateFileW(
	LPCWSTR               lpFileName,
	DWORD                 dwDesiredAccess,
	DWORD                 dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD                 dwCreationDisposition,
	DWORD                 dwFlagsAndAttributes,
	HANDLE                hTemplateFile) {
	auto lpLibName = StrStrIW(lpFileName, L"DiscSoftLib.dll");
	if (lpLibName != NULL)
		StrCpyW(lpLibName, DiscSoftLib);
	return fpCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
		lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DiscSoftLib_Init();
		DetourTransactionBegin();
		DetourAttach(&(LPVOID&)fpCryptQueryObject, Hooked_CryptQueryObject);
		DetourAttach(&(LPVOID&)fpCreateFileW, Hooked_CreateFileW);
		DetourTransactionCommit();
	}
	return TRUE;
}