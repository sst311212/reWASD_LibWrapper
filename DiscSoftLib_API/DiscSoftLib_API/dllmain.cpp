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

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DiscSoftLib_Init();
		DetourTransactionBegin();
		DetourAttach(&(LPVOID&)fpCryptQueryObject, Hooked_CryptQueryObject);
		DetourTransactionCommit();
	}
	return TRUE;
}