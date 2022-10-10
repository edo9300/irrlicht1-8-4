#define _WIN32_DCOM

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_

#include "CIrrDeviceWin32.h"
#include <Wbemidl.h>

namespace irr {
#ifndef PROCESSOR_ARCHITECTURE_ARM
#define PROCESSOR_ARCHITECTURE_ARM 5
#endif
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif

bool CIrrDeviceWin32::GetWindowsVersionViaWMI(core::stringc& out, DWORD& majorVersion, DWORD& minorVersion, DWORD& buildNumber) {
	int systemMajor, systemMinor, systemBuild, servicePackMajor, servicePackMinor;
	core::stringw wSystemCaption;
	{
		class my_bstr_t {
			BSTR my_bstr;
			public:
			my_bstr_t(const wchar_t* str) : my_bstr(SysAllocString(str)) {}
			~my_bstr_t() { SysFreeString(my_bstr); }
			operator BSTR const() { return my_bstr; }
		};

		// Step 1: --------------------------------------------------
		// Initialize COM. ------------------------------------------

		if(FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
			return false;


		IWbemLocator* pLoc = nullptr;
		IWbemServices* pSvc = nullptr;
		IEnumWbemClassObject* pEnumerator = nullptr;
		IWbemClassObject* pclsObj = nullptr;
		VARIANT vtProp;
		VariantInit(&vtProp);

		auto FreeObjects = [&] {
			if(pLoc)
				pLoc->Release();
			if(pSvc)
				pSvc->Release();
			if(pEnumerator)
				pEnumerator->Release();
			if(pclsObj)
				pclsObj->Release();
			VariantClear(&vtProp);
			CoUninitialize();
		};

		auto Error = [&] {
			FreeObjects();
			return false;
		};
#define CHECK(...) do {if(FAILED(__VA_ARGS__)) { return Error();} } while(0)

		// Step 2: --------------------------------------------------
		// Set general COM security levels --------------------------

		CHECK(CoInitializeSecurity(
			nullptr,
			-1,						  // COM authentication
			nullptr,						// Authentication services
			nullptr,						// Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
			nullptr,						// Authentication info
			EOAC_NONE,				   // Additional capabilities 
			nullptr						 // Reserved
		));

		// Step 3: ---------------------------------------------------
		// Obtain the initial locator to WMI -------------------------

		CHECK(CoCreateInstance(
			CLSID_WbemLocator,
			0,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID*)&pLoc
		));

		// Step 4: -----------------------------------------------------
		// Connect to WMI through the IWbemLocator::ConnectServer method

		// Connect to the root\cimv2 namespace with
		// the current user and obtain pointer pSvc
		// to make IWbemServices calls.
		CHECK(pLoc->ConnectServer(
			my_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
			nullptr,				 // User name. nullptr = current user
			nullptr,				 // User password. nullptr = current
			nullptr,				 // Locale. nullptr indicates current
			0,						 // Security flags.
			0,						 // Authority (for example, Kerberos)
			0,						 // Context object 
			&pSvc					 // pointer to IWbemServices proxy
		));


		// Step 5: --------------------------------------------------
		// Set security levels on the proxy -------------------------

		CHECK(CoSetProxyBlanket(
			pSvc,						// Indicates the proxy to set
			RPC_C_AUTHN_WINNT,			// RPC_C_AUTHN_xxx
			RPC_C_AUTHZ_NONE,			// RPC_C_AUTHZ_xxx
			nullptr,					// Server principal name 
			RPC_C_AUTHN_LEVEL_CALL,		// RPC_C_AUTHN_LEVEL_xxx 
			RPC_C_IMP_LEVEL_IMPERSONATE,// RPC_C_IMP_LEVEL_xxx
			nullptr,					// client identity
			EOAC_NONE					// proxy capabilities 
		));

		// Step 6: --------------------------------------------------
		// Use the IWbemServices pointer to make requests of WMI ----

		// For example, get the name of the operating system
		CHECK(pSvc->ExecQuery(
			my_bstr_t(L"WQL"),
			my_bstr_t(L"SELECT BuildNumber, Name, Version, ServicePackMajorVersion, ServicePackMinorVersion FROM Win32_OperatingSystem"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			nullptr,
			&pEnumerator
		));

		// Step 7: -------------------------------------------------
		// Get the data from the query in step 6 -------------------
		ULONG uReturn = 0;
		HRESULT hr = pEnumerator->Next(500, 1, &pclsObj, &uReturn);
		if(FAILED(hr) || uReturn == 0)
			return Error();

		auto Get = [&pclsObj, &vtProp](wchar_t* name, CIMTYPE type) -> HRESULT {
			CIMTYPE retType;
			auto res = pclsObj->Get(name, 0, &vtProp, &retType, 0);
			if(!FAILED(res) && retType != type)
				return -1;
			return res;
		};

#define GET(name,type,...) do {VariantInit(&vtProp); CHECK(Get(name, type)); __VA_ARGS__ VariantClear(&vtProp);} while(0)

		// Get the value of the Name property
		GET(my_bstr_t(L"Name"), CIM_STRING, {
			wSystemCaption = vtProp.bstrVal;
		});

		GET(my_bstr_t(L"Version"), CIM_STRING, {
			if(swscanf(vtProp.bstrVal, L"%d.%d.%d", &systemMajor, &systemMinor, &systemBuild) != 3)
				return Error();
		});

		GET(my_bstr_t(L"ServicePackMajorVersion"), CIM_UINT16, {
			servicePackMajor = vtProp.uiVal;
		});

		GET(L"ServicePackMinorVersion", CIM_UINT16, {
			servicePackMinor = vtProp.uiVal;
		});

		FreeObjects();
	}

	auto pos = -1;
	if((pos = wSystemCaption.find(L"x64")) != -1 || // Server 2003 and XP x64 have an "x64 Edition" in their name, remove it
	   (pos = wSystemCaption.find(L"|")) != -1)     // The Name field contains the version name and separated by a pipe | the installation volume, remove it
		wSystemCaption = wSystemCaption.subString(0, pos).trim();

	wSystemCaption.remove(L'\xae'); //U+00AE Registered Sign
	wSystemCaption.remove(L'\x2122'); //U+2122 Trade Mark Sign
	wSystemCaption.remove(L"(R)");
	if((pos = wSystemCaption.find(L"Windows")) != -1) // Make sure the string always starts with "Microsoft" windows
		wSystemCaption = core::stringw(L"Microsoft ") + wSystemCaption.subString(pos, wSystemCaption.size());
	char systemCaption[512];
	{
		const size_t lenOld = (wSystemCaption.size() + 1) * sizeof(wchar_t);
		if(sizeof(systemCaption) < lenOld)
			return false;
		core::wcharToUtf8(wSystemCaption.data(), systemCaption, lenOld);
	}

	char servicePackString[255];
	if(servicePackMajor != 0) {
		snprintf_irr(servicePackString, sizeof(servicePackString), servicePackMinor != 0 ? " Service Pack %d.%d" : " Service Pack %d", servicePackMajor, servicePackMinor);
	} else
		servicePackString[0] = 0;

	char productBuildString[255];
	auto GetWin10ProductInfo = [&productBuildString, systemBuild]() -> bool {
		auto GetRegEntry = [](const TCHAR* path, const TCHAR* name, DWORD flag, void* buff, DWORD size) {
			HKEY hKey;
			DWORD dwRetFlag;
			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
				return false;
			auto ret = RegQueryValueEx(hKey, name, nullptr, &dwRetFlag, (LPBYTE)buff, &size);
			RegCloseKey(hKey);
			return ret == ERROR_SUCCESS && (dwRetFlag & flag) != 0;
		};

		const auto* path = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
		TCHAR szReleaseId[80];
		if(!GetRegEntry(path, TEXT("DisplayVersion"), REG_SZ, szReleaseId, sizeof(szReleaseId)) &&
		   !GetRegEntry(path, TEXT("ReleaseId"), REG_SZ, szReleaseId, sizeof(szReleaseId))) {
			return false;
		}
		DWORD UBR;
		if(!GetRegEntry(path, TEXT("UBR"), REG_DWORD, &UBR, sizeof(UBR)))
			return false;
		sprintf(productBuildString, " (Version %s, Build %ld.%ld)", irr::core::stringc(szReleaseId).c_str(), systemBuild, UBR);
		return true;
	};

	majorVersion = systemMajor;
	minorVersion = systemMinor;
	buildNumber = systemBuild;

	if(systemMajor < 10 || !GetWin10ProductInfo()) {
		sprintf(productBuildString, " (Build %ld)", systemBuild);
	}

	out.append(systemCaption).append(servicePackString).append(productBuildString);
	if(systemMajor >= 6 || systemMinor >= 1) {
		using GetNativeSystemInfo_t = VOID(WINAPI*)(LPSYSTEM_INFO);
		auto pGetNativeSystemInfo = (GetNativeSystemInfo_t)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
		if(pGetNativeSystemInfo) {
			SYSTEM_INFO info;
			pGetNativeSystemInfo(&info);
			switch(info.wProcessorArchitecture) {
			case PROCESSOR_ARCHITECTURE_AMD64:
				out.append(" x64");
				break;
			case PROCESSOR_ARCHITECTURE_ARM:
				out.append(" ARM");
				break;
			case PROCESSOR_ARCHITECTURE_ARM64:
				out.append(" ARM64");
				break;
			case PROCESSOR_ARCHITECTURE_IA64:
				out.append(" Itanium");
				break;
			case PROCESSOR_ARCHITECTURE_INTEL:
				out.append(" x86");
				break;
			default:
				break;
			}
		}
	}
	return true;
}
}

#endif