#pragma once
#include <stdio.h>
#include <Windows.h>

enum LicenseType {
	ltNotSelected,
	ltFree,
	ltTrial,
	ltPaid,
	ltSubscription
};

enum LicenseCheckingResultEnum {
	lcrSuccess,
	lcrFailed,
	lcrInvalidLicense,
	lcrRequestEmail,
	lcrTryLater,
	lcrTrialExpired,
	lcrConnectionError,
	lcrUncheckedWarning,
	lcrUncheckedTimeout,
	lcrSystemIdChanged,
	lcrOnAnotherSystemId,
	lcrNetServiceUnsupportedVersion,
	lcrApplicationIsCorrupted,
	lcrNetServiceConnectionError,
	lcrTrialWarning,
	lcrNotActivated,
	lcrUpdatePeriodRanOut,
	lcrDataFileSavingError,
	lcrSerialIsOld,
	lcrEmptyMultilicenseInstances,
	lcrSubscriptionAuthError,
	lcrUserAlreadyExists,
	lcrSubscriptionExpired,
	lcrSubscriptionDifferentOwner,
	lcrBlockedUser
};

enum ServerMessageType {
	smtError,
	smtWarning,
	smtInfo
};

enum SubscriptionType {
	ltsFree,
	ltsPaid
};

enum FeatureLicenseStatus {
	flsNotActivated = 1,
	flsTrial,
	flsPaid
};

struct FeatureState {
	WCHAR pszFeatureId[64];
	FeatureLicenseStatus licenseStatus;
	int TrialDaysLeft;
};

struct ActivationLicenseInfo
{
	LicenseType licenseType;
	WCHAR serial[33];
	WCHAR email[256];
};

struct LicenseCheckResult {
	LicenseCheckingResultEnum Result;
	int IsSuccessResult;
	WCHAR Message[2048];
	WCHAR URL[256];
	int NewVersionAvailable;
	WCHAR UpdateURL[256];
	WCHAR UpdateNotes[8192];
	WCHAR PaidSerial[33];
	int PaidMajorVersion;
	int TrialDaysLeft;
	ServerMessageType MessageType;
	WCHAR MessageOpenLinkBtnText[32];
	WCHAR MessageCloseBtnText[32];
	int MessageHideCloseBtn;
	WCHAR OfferText[8192];
	WCHAR OfferLink[1024];
	WCHAR OfferButtonText[256];
	WCHAR AdditionalURL[256];
	WCHAR AdditionalURLText[256];
	int ConfigurationInstances;
	int IsInstanceActivated;
	int IsInstanceMajorUpgraded;
	int IsProlonged;
	WCHAR Email[256];
	WCHAR Expires[11];
	SubscriptionType Subscription;
	int SubscriptionDaysLeft;
	int IsEmailConfirmed;
	int FeaturesCount;
	FeatureState FeaturesStates[50];
};

struct LicenseInfo {
	WCHAR Serial[33];
	WCHAR InstallationName[129];
	LicenseType License;
	LicenseType InitialLicense;
	WCHAR HardwareId[65];
	int TrialDaysLeft;
	int VerifiedLicense;
	WCHAR Email[256];
	WCHAR Expires[11];
	SubscriptionType Subscription;
	int SubscriptionDaysLeft;
	int IsEmailConfirmed;
	int IsLifetime;
	int FeaturesCount;
	FeatureState FeaturesStates[50];
};

FARPROC fpActivateTrialFeature;
FARPROC fpCheckForUpdate;
FARPROC fpClearHTMLOffer;
FARPROC fpGetHTMLOffer;
FARPROC fpGetUserId;
FARPROC fpIsHTMLOfferExist;
FARPROC fpSetFileUsersFullAccess;

typedef bool(*detour_ActivateLicense)(ActivationLicenseInfo &licenseInfo, LicenseCheckResult &licenseCheckResult);
detour_ActivateLicense fpActivateLicense;

typedef bool(*detour_CheckLicense)(bool forceOnlineCheck, LicenseCheckResult &licenseCheckResult);
detour_CheckLicense fpCheckLicense;

typedef void(*detour_GetLicenseInfo)(LicenseInfo &pInstanceInfo);
detour_GetLicenseInfo fpGetLicenseInfo;

#define DiscSoftLib L"_DiscSoftLib.dll"

void DiscSoftLib_Init()
{
	WCHAR lpLibPath[260];
	GetModuleHandle(lpLibPath);
	for (int i = lstrlen(lpLibPath) - 1; i >= 0; i++) {
		if (lpLibPath[i] == L'\\') {
			lpLibPath[i + 1] = 0;
			break;
		}
	}
	lstrcat(lpLibPath, DiscSoftLib);
	GetFileAttributes(lpLibPath);
	if (GetLastError() == ERROR_FILE_NOT_FOUND) {
		MessageBox(NULL, L"找不到 _DiscSoftLib.dll 檔案。", L"reWASD", MB_ICONERROR);
		ExitProcess(0);
	}
	HMODULE hModule = LoadLibrary(lpLibPath);
	if (hModule == NULL) {
		MessageBox(NULL, L"無法載入 _DiscSoftLib.dll 檔案。", L"reWASD", MB_ICONERROR);
		ExitProcess(0);
	}
	fpActivateLicense = (detour_ActivateLicense)GetProcAddress(hModule, "ActivateLicense");
	fpActivateTrialFeature = GetProcAddress(hModule, "ActivateTrialFeature");
	fpCheckForUpdate = GetProcAddress(hModule, "CheckForUpdate");
	fpCheckLicense = (detour_CheckLicense)GetProcAddress(hModule, "CheckLicense");
	fpClearHTMLOffer = GetProcAddress(hModule, "ClearHTMLOffer");
	fpGetHTMLOffer = GetProcAddress(hModule, "GetHTMLOffer");
	fpGetLicenseInfo = (detour_GetLicenseInfo)GetProcAddress(hModule, "GetLicenseInfo");
	fpGetUserId = GetProcAddress(hModule, "GetUserId");
	fpIsHTMLOfferExist = GetProcAddress(hModule, "IsHTMLOfferExist");
	fpSetFileUsersFullAccess = GetProcAddress(hModule, "SetFileUsersFullAccess");
}

extern "C" __declspec(dllexport) bool ActivateLicense(ActivationLicenseInfo &licenseInfo, LicenseCheckResult &licenseCheckResult)
{
	return fpActivateLicense(licenseInfo, licenseCheckResult);
}
extern "C" __declspec(dllexport) void ActivateTrialFeature() { fpActivateTrialFeature(); }
extern "C" __declspec(dllexport) void CheckForUpdate() { fpCheckForUpdate(); }
extern "C" __declspec(dllexport) bool CheckLicense(bool forceOnlineCheck, LicenseCheckResult &licenseCheckResult)
{
	bool bStat = fpCheckLicense(forceOnlineCheck, licenseCheckResult);
	if (licenseCheckResult.FeaturesCount == 0) {
		LicenseCheckResult _licenseCheckResult;
		fpCheckLicense(true, _licenseCheckResult);
		ActivationLicenseInfo _activationLicenseInfo;
		_activationLicenseInfo.licenseType = LicenseType::ltTrial;
		fpActivateLicense(_activationLicenseInfo, _licenseCheckResult);
	}
	licenseCheckResult.Result = LicenseCheckingResultEnum::lcrSuccess;
	for (int i = 0; i < licenseCheckResult.FeaturesCount; i++) {
		licenseCheckResult.FeaturesStates[i].licenseStatus = FeatureLicenseStatus::flsPaid;
	}
	return bStat;
}
extern "C" __declspec(dllexport) void ClearHTMLOffer() { fpClearHTMLOffer(); }
extern "C" __declspec(dllexport) void GetHTMLOffer() { fpGetHTMLOffer(); }
extern "C" __declspec(dllexport) void GetLicenseInfo(LicenseInfo &pInstanceInfo)
{
	fpGetLicenseInfo(pInstanceInfo);
	pInstanceInfo.License = LicenseType::ltPaid;
	lstrcpy(pInstanceInfo.Serial, L"Crack By 阿皇仔");
}
extern "C" __declspec(dllexport) void GetUserId() { fpGetUserId(); }
extern "C" __declspec(dllexport) bool IsHTMLOfferExist(bool &exists)
{
	exists = false;
	return true;
}
extern "C" __declspec(dllexport) void SetFileUsersFullAccess() { fpSetFileUsersFullAccess(); }