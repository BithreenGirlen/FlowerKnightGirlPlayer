

#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>

#include "win_support.h"

namespace win_support
{
	static bool GetWindowVersion(DWORD* dwMajorVersion, DWORD* dwBuildNumber)
	{
		if (dwMajorVersion == nullptr || dwBuildNumber == nullptr)return false;

		HMODULE hModule = ::GetModuleHandleW(L"ntdll.dll");
		if (hModule == nullptr)return false;

		NTSTATUS(*pRtlGetVersion)(PRTL_OSVERSIONINFOW) = nullptr;
		pRtlGetVersion = reinterpret_cast<NTSTATUS(*)(PRTL_OSVERSIONINFOW)>(::GetProcAddress(hModule, "RtlGetVersion"));
		if (pRtlGetVersion == nullptr)return false;

		RTL_OSVERSIONINFOW sRtlOsVersionInfoEx{};
		sRtlOsVersionInfoEx.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

		BOOL iRet = pRtlGetVersion(&sRtlOsVersionInfoEx);
		if (iRet == STATUS_SUCCESS)
		{
			*dwMajorVersion = sRtlOsVersionInfoEx.dwMajorVersion;
			*dwBuildNumber = sRtlOsVersionInfoEx.dwBuildNumber;

			return true;
		}

		return false;
	}
}

bool win_support::IsUtf8ForCRuntimeSupported()
{
	DWORD dwMajorVersion = 0;
	DWORD dwBuildNumber = 0;
	if (GetWindowVersion(&dwMajorVersion, &dwBuildNumber))
	{
		return dwMajorVersion >= 10 && dwBuildNumber >= 17134;
	}

	return false;
}

bool win_support::IsWebpSupported()
{
	DWORD dwMajorVersion = 0;
	DWORD dwBuildNumber = 0;
	if (GetWindowVersion(&dwMajorVersion, &dwBuildNumber))
	{
		return dwMajorVersion >= 10 && dwBuildNumber >= 17763;
	}

	return false;
}
