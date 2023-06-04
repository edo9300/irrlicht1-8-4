#define _WIN32_DCOM

#include "IrrCompileConfig.h"

#ifdef _WIN32

template<typename T, typename T2>
inline T function_cast(T2 ptr) {
	using generic_function_ptr = void (*)(void);
	return reinterpret_cast<T>(reinterpret_cast<generic_function_ptr>(ptr));
}

#include "CIrrDeviceWin32WindowsVersionWMI.h"
#include <Wbemidl.h>
#include <tchar.h>

namespace irr {
#ifndef PROCESSOR_ARCHITECTURE_ARM
#define PROCESSOR_ARCHITECTURE_ARM 5
#endif
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif
typedef BOOL(WINAPI* PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
// Needed for old windows apis
// depending on the SDK version and compilers some defines might be available
// or not
#ifndef PRODUCT_ULTIMATE
#define PRODUCT_ULTIMATE	0x00000001
#define PRODUCT_HOME_BASIC	0x00000002
#define PRODUCT_HOME_PREMIUM	0x00000003
#define PRODUCT_ENTERPRISE	0x00000004
#define PRODUCT_HOME_BASIC_N	0x00000005
#define PRODUCT_BUSINESS	0x00000006
#define PRODUCT_STARTER		0x0000000B
#endif
#ifndef PRODUCT_ULTIMATE_N
#define PRODUCT_BUSINESS_N	0x00000010
#define PRODUCT_HOME_PREMIUM_N	0x0000001A
#define PRODUCT_ENTERPRISE_N	0x0000001B
#define PRODUCT_ULTIMATE_N	0x0000001C
#endif
#ifndef PRODUCT_STARTER_N
#define PRODUCT_STARTER_N	0x0000002F
#endif
#ifndef PRODUCT_PROFESSIONAL
#define PRODUCT_PROFESSIONAL	0x00000030
#define PRODUCT_PROFESSIONAL_N	0x00000031
#endif
#ifndef PRODUCT_ULTIMATE_E
#define PRODUCT_STARTER_E	0x00000042
#define PRODUCT_HOME_BASIC_E	0x00000043
#define PRODUCT_HOME_PREMIUM_E	0x00000044
#define PRODUCT_PROFESSIONAL_E	0x00000045
#define PRODUCT_ENTERPRISE_E	0x00000046
#define PRODUCT_ULTIMATE_E	0x00000047
#endif
#ifndef SM_SERVERR2
#define SM_SERVERR2 89
#endif
#ifndef PROCESSOR_ARCHITECTURE_ARM
#define PROCESSOR_ARCHITECTURE_ARM 5
#endif
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif

bool GetWindowsVersionViaWMI(core::stringc& out, DWORD& majorVersion, DWORD& minorVersion, DWORD& buildNumber);

void GetWindowsVersion(core::stringc& out, core::stringc& compatModeVersion) {
	auto GetWineVersion = [&out] {
		auto lib = GetModuleHandle(TEXT("ntdll.dll"));
		auto wine_get_build_id = function_cast<const char* (*)(void)>(GetProcAddress(lib, "wine_get_build_id"));
		if(wine_get_build_id == nullptr)
			return false;
		auto wine_get_host_version = function_cast<void (*)(const char **, const char **)>(GetProcAddress(lib, "wine_get_host_version"));
		if(wine_get_host_version) {
			const char *sysname, *release;
			wine_get_host_version(&sysname, &release);
			out.append(sysname).append(" ").append(release).append(" ");
		}
		out.append(wine_get_build_id()).append(" ");
		return true;
	};

	auto GetRegEntry = [](const TCHAR* path, const TCHAR* name, DWORD flag, void* buff, DWORD size) {
		HKEY hKey;
		DWORD dwRetFlag;
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
			return false;
		auto ret = RegQueryValueEx(hKey, name, NULL, &dwRetFlag, (LPBYTE)buff, &size);
		RegCloseKey(hKey);
		return ret == ERROR_SUCCESS && (dwRetFlag & flag) != 0;
	};

	if(GetWineVersion())
		return;

	OSVERSIONINFOEX osvi{ sizeof(OSVERSIONINFOEX) };

	if(!GetVersionEx((OSVERSIONINFO*)&osvi) &&
	   (osvi = { sizeof(OSVERSIONINFO) }, !GetVersionEx((OSVERSIONINFO*)&osvi)))
		return;

	bool compatMode = false;

	{
		DWORD actualMajorVersion;
		DWORD actualMinorVersion;
		DWORD actualBuildNumber;
		if(GetWindowsVersionViaWMI(out, actualMajorVersion, actualMinorVersion, actualBuildNumber)) {
			compatMode = osvi.dwMajorVersion != actualMajorVersion || osvi.dwMinorVersion != actualMinorVersion || osvi.dwBuildNumber != actualBuildNumber;
			if(!compatMode)
				return;
		}
	}

	auto& realVersionOut = compatMode ? compatModeVersion : out;

	auto ServerOrWorkstation = [workstation = osvi.wProductType == VER_NT_WORKSTATION,&realVersionOut](const char* str1, const char* str2) {
		realVersionOut.append(workstation ? str1 : str2);
	};

	switch(osvi.dwPlatformId) {
		case VER_PLATFORM_WIN32_NT:
			realVersionOut.append("Microsoft Windows ");
			switch(osvi.dwMajorVersion) {
				case 1:
				case 2:
				case 3:
				case 4:
					realVersionOut.append("NT ");
					break;
				case 5:
					switch(osvi.dwMinorVersion) {
						case 0:
							realVersionOut.append("2000 ");
							break;
						case 1:
							realVersionOut.append("XP ");
							break;
						case 2:
							if(osvi.wProductType == VER_NT_WORKSTATION) {
								realVersionOut.append("XP ");
								break;
							}
							realVersionOut.append("Server 2003 ");
							if(GetSystemMetrics(SM_SERVERR2))
								realVersionOut.append("R2 ");
							break;
						default:
							break;
					}
					break;
				case 6:
					switch(osvi.dwMinorVersion) {
						case 0:
							ServerOrWorkstation("Vista ", "Server 2008");
							break;
						case 1:
							ServerOrWorkstation("7 ", "Server 2008 R2 ");
							break;
						case 2:
							ServerOrWorkstation("8 ", "Server 2012 ");
							break;
						case 3:
							ServerOrWorkstation("8.1 ", "Server 2012 R2 ");
							break;
					}
					break;

				case 10:
					switch(osvi.dwMinorVersion) {
						case 0:
							if(osvi.wProductType == VER_NT_WORKSTATION) {
								if(osvi.dwBuildNumber >= 22000)
									realVersionOut.append("11 ");
								else
									realVersionOut.append("10 ");
							} else {
								realVersionOut.append("Server ");
								switch(osvi.dwBuildNumber) {
								case 14393:
									realVersionOut.append("2016 ");
									break;
								case 17763:
									realVersionOut.append("2019 ");
									break;
								case 20348:
									realVersionOut.append("2022 ");
									break;
								default:
									break;
								}
							}
							break;
					}
					break;
				default:
					break;
			}

		if (osvi.dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX))
		{
			if (osvi.dwMajorVersion >= 6)
			{
				DWORD dwType;
				auto pGPI = function_cast<PGPI>(GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo"));
				if(pGPI && pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType)) {
					switch(dwType) {
						case PRODUCT_ULTIMATE:
						case PRODUCT_ULTIMATE_E:
						case PRODUCT_ULTIMATE_N:
							realVersionOut.append("Ultimate Edition ");
							break;
						case PRODUCT_PROFESSIONAL:
						case PRODUCT_PROFESSIONAL_E:
						case PRODUCT_PROFESSIONAL_N:
							realVersionOut.append("Professional Edition ");
							break;
						case PRODUCT_HOME_BASIC:
						case PRODUCT_HOME_BASIC_E:
						case PRODUCT_HOME_BASIC_N:
							realVersionOut.append("Home Basic Edition ");
							break;
						case PRODUCT_HOME_PREMIUM:
						case PRODUCT_HOME_PREMIUM_E:
						case PRODUCT_HOME_PREMIUM_N:
							realVersionOut.append("Home Premium Edition ");
							break;
						case PRODUCT_ENTERPRISE:
						case PRODUCT_ENTERPRISE_E:
						case PRODUCT_ENTERPRISE_N:
							realVersionOut.append("Enterprise Edition ");
							break;
						case PRODUCT_BUSINESS:
						case PRODUCT_BUSINESS_N:
							realVersionOut.append("Business Edition ");
							break;
						case PRODUCT_STARTER:
						case PRODUCT_STARTER_E:
						case PRODUCT_STARTER_N:
							realVersionOut.append("Starter Edition ");
							break;
					}
				}
			}
#ifdef VER_SUITE_ENTERPRISE
			else
			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
#ifndef __BORLANDC__
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
					realVersionOut.append("Personal ");
				else
					realVersionOut.append("Professional ");
#endif
			}
			else if (osvi.wProductType == VER_NT_SERVER)
			{
				if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
					realVersionOut.append("DataCenter Server ");
				else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
					realVersionOut.append("Advanced Server ");
				else
					realVersionOut.append("Server ");
			}
#endif
		}
		else
		{
			const LPCTSTR path = __TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions");

			TCHAR szProductType[80];
			if(GetRegEntry(path, __TEXT("ProductType"), REG_SZ, szProductType, sizeof(szProductType))) {
				if(_tcsicmp(__TEXT("WINNT"), szProductType) == 0)
					realVersionOut.append("Professional ");
				if(_tcsicmp(__TEXT("LANMANNT"), szProductType) == 0)
					realVersionOut.append("Server ");
				if(_tcsicmp(__TEXT("SERVERNT"), szProductType) == 0)
					realVersionOut.append("Advanced Server ");
			}
		}

		// Display version, service pack (if any), and build number.

		char tmp[255];

		if (osvi.dwMajorVersion <= 4)
		{
			sprintf(tmp, "version %ld.%ld %s (Build %ld)",
					osvi.dwMajorVersion,
					osvi.dwMinorVersion,
					irr::core::stringc(osvi.szCSDVersion).c_str(),
					osvi.dwBuildNumber & 0xFFFF);
		}
		else
		{
			auto GetWin10ProductInfo = [&]()->bool {
				const auto* path = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
				TCHAR szReleaseId[80];
				if(!GetRegEntry(path, TEXT("DisplayVersion"), REG_SZ, szReleaseId, sizeof(szReleaseId)) &&
				   !GetRegEntry(path, TEXT("ReleaseId"), REG_SZ, szReleaseId, sizeof(szReleaseId))) {
					return false;
				}
				DWORD UBR;
				if(!GetRegEntry(path, TEXT("UBR"), REG_DWORD, &UBR, sizeof(UBR)))
					return false;
				sprintf(tmp, "(Version %s, Build %ld.%ld)", irr::core::stringc(szReleaseId).c_str(), osvi.dwBuildNumber & 0xFFFF, UBR);
				return true;
			};
			if(osvi.dwMajorVersion < 10 || !GetWin10ProductInfo()) {
				sprintf(tmp, "%s (Build %ld)", irr::core::stringc(osvi.szCSDVersion).c_str(),
						osvi.dwBuildNumber & 0xFFFF);
			}
		}

		realVersionOut.append(tmp);
		{
			using GetNativeSystemInfo_t = VOID(WINAPI*)(LPSYSTEM_INFO);
			auto pGetNativeSystemInfo = function_cast<GetNativeSystemInfo_t>(GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo"));
			if(pGetNativeSystemInfo) {
				SYSTEM_INFO info;
				pGetNativeSystemInfo(&info);
				switch(info.wProcessorArchitecture) {
					case PROCESSOR_ARCHITECTURE_AMD64:
						realVersionOut.append(" x64");
						break;
					case PROCESSOR_ARCHITECTURE_ARM:
						realVersionOut.append(" ARM");
						break;
					case PROCESSOR_ARCHITECTURE_ARM64:
						realVersionOut.append(" ARM64");
						break;
					case PROCESSOR_ARCHITECTURE_IA64:
						realVersionOut.append(" Itanium");
						break;
					case PROCESSOR_ARCHITECTURE_INTEL:
						realVersionOut.append(" x86");
						break;
					default:
						break;
				}
			}
		}
		break;

		case VER_PLATFORM_WIN32_WINDOWS:

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			{
				realVersionOut.append("Microsoft Windows 95 ");
				if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
					realVersionOut.append("OSR2 " );
			}

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			{
				realVersionOut.append("Microsoft Windows 98 ");
				if ( osvi.szCSDVersion[1] == 'A' )
					realVersionOut.append( "SE " );
			}

			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
				realVersionOut.append("Microsoft Windows Me ");

			break;

		case VER_PLATFORM_WIN32s:
			realVersionOut.append("Microsoft Win32s ");
			break;
	}
}

bool GetWindowsVersionViaWMI(core::stringc& out, DWORD& majorVersion, DWORD& minorVersion, DWORD& buildNumber) {
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

		auto Get = [&pclsObj, &vtProp](auto name, CIMTYPE type) -> HRESULT {
			CIMTYPE retType;
			auto res = pclsObj->Get(my_bstr_t(name), 0, &vtProp, &retType, 0);
			if(!FAILED(res) && retType != type)
				return -1;
			return res;
		};

#define GET(name,type,...) do {VariantInit(&vtProp); CHECK(Get(name, type)); __VA_ARGS__ VariantClear(&vtProp);} while(0)

		// Get the value of the Name property
		GET(L"Name", CIM_STRING, {
			wSystemCaption = vtProp.bstrVal;
		});

		GET(L"Version", CIM_STRING, {
			if(swscanf(vtProp.bstrVal, L"%d.%d.%d", &systemMajor, &systemMinor, &systemBuild) != 3)
				return Error();
		});

		GET(L"ServicePackMajorVersion", CIM_UINT16, {
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
		sprintf(productBuildString, " (Version %s, Build %d.%ld)", irr::core::stringc(szReleaseId).c_str(), systemBuild, UBR);
		return true;
	};

	majorVersion = systemMajor;
	minorVersion = systemMinor;
	buildNumber = systemBuild;

	if(systemMajor < 10 || !GetWin10ProductInfo()) {
		sprintf(productBuildString, " (Build %d)", systemBuild);
	}

	out.append(systemCaption).append(servicePackString).append(productBuildString);
	if(systemMajor >= 6 || systemMinor >= 1) {
		using GetNativeSystemInfo_t = VOID(WINAPI*)(LPSYSTEM_INFO);
		auto pGetNativeSystemInfo = function_cast<GetNativeSystemInfo_t>(GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo"));
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
