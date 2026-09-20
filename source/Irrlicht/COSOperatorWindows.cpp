// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#include <IrrCompileConfig.h>

#ifdef _IRR_WINDOWS_API_

#include "COSOperatorWindows.h"

#include <windows.h>
#include "IrrFunctionCast.h"

namespace irr
{

COSOperatorWindows::COSOperatorWindows(const core::stringc& osVersion) : COSOperator(osVersion)
{
#ifdef _DEBUG
	setDebugName("COSOperatorWindows");
#endif
}


void COSOperatorWindows::copyToClipboard(const wchar_t* wtext) const
{
	if(wtext == nullptr)
		return;
	auto wlen = wcslen(wtext);
	if(wlen == 0)
		return;

	if(!OpenClipboard(nullptr))
		return;

	EmptyClipboard();

	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);

	HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, lenOld);
	wchar_t* wbuffer = (wchar_t*)GlobalLock(clipbuffer);
	memcpy(wbuffer, wtext, lenOld);
	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_UNICODETEXT, clipbuffer);

	clipbuffer = GlobalAlloc(GMEM_DDESHARE, lenOld);
	char* cbuffer = (char*)GlobalLock(clipbuffer);
	core::wcharToUtf8(wtext, cbuffer, lenOld);
	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_TEXT, clipbuffer);

	CloseClipboard();
}


static void* RetrieveBuffer(HANDLE& hData, UINT type) {
	if((hData = GetClipboardData(type))) {
		void* buffer = GlobalLock(hData);
		if(!buffer) {
			GlobalUnlock(hData);
			hData = nullptr;
			return nullptr;
		}
		return buffer;
	}
	return nullptr;
}

const wchar_t* COSOperatorWindows::getTextFromClipboard() const {
	if(!OpenClipboard(nullptr))
		return nullptr;

	HANDLE hData = nullptr;

	if(auto* wbuffer = static_cast<const wchar_t*>(RetrieveBuffer(hData, CF_UNICODETEXT));
		wbuffer != nullptr) {
		ClipboardString = wbuffer;

	} else if(auto* cbuffer = static_cast<const char*>(RetrieveBuffer(hData, CF_TEXT));
			   cbuffer != nullptr) {
		size_t lenOld = strlen(cbuffer);
		wchar_t* ws = new wchar_t[lenOld + 1];
		core::utf8ToWchar(cbuffer, ws, (lenOld + 1)*sizeof(wchar_t));
		ClipboardString = ws;
	} else
		return nullptr;

	GlobalUnlock(hData);
	CloseClipboard();
	return ClipboardString.c_str();
}


bool COSOperatorWindows::getProcessorSpeedMHz(u32* MHz) const
{
	if (MHz)
		*MHz=0;
	LONG Error;

	HKEY Key;
	Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						 __TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
						 0, KEY_READ, &Key);

	if(Error != ERROR_SUCCESS)
		return false;

	DWORD Speed = 0;
	DWORD Size = sizeof(Speed);
	Error = RegQueryValueEx(Key, __TEXT("~MHz"), NULL, NULL, (LPBYTE)&Speed, &Size);

	RegCloseKey(Key);

	if (Error != ERROR_SUCCESS)
		return false;
	else if (MHz)
		*MHz = Speed;
	return true;
}

struct memstatusex {
	DWORD     dwLength;
	DWORD     dwMemoryLoad;
	DWORDLONG ullTotalPhys;
	DWORDLONG ullAvailPhys;
	DWORDLONG ullTotalPageFile;
	DWORDLONG ullAvailPageFile;
	DWORDLONG ullTotalVirtual;
	DWORDLONG ullAvailVirtual;
	DWORDLONG ullAvailExtendedVirtual;
};
using GlobalMemoryStatusExPtr = BOOL(__stdcall*)(memstatusex* lpBuffer);

bool COSOperatorWindows::getSystemMemory(u32* Total, u32* Avail) const
{
	HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
	GlobalMemoryStatusExPtr globalMemoryStatusExPtr = function_cast<GlobalMemoryStatusExPtr>(GetProcAddress(kernel32, "GlobalMemoryStatusEx"));
	if(globalMemoryStatusExPtr)
	{
		memstatusex MemoryStatusEx;
		MemoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);

		// cannot fail
		globalMemoryStatusExPtr(&MemoryStatusEx);

		if (Total)
			*Total = (u32)(MemoryStatusEx.ullTotalPhys>>10);
		if (Avail)
			*Avail = (u32)(MemoryStatusEx.ullAvailPhys>>10);
		return true;
	}
	MEMORYSTATUS MemoryStatus;
	MemoryStatus.dwLength = sizeof(MEMORYSTATUS);

	// cannot fail
	GlobalMemoryStatus(&MemoryStatus);

	if (Total)
		*Total = (u32)(MemoryStatus.dwTotalPhys>>10);
	if (Avail)
		*Avail = (u32)(MemoryStatus.dwAvailPhys>>10);
	return true;
}


} // end namespace

#endif // _IRR_WINDOWS_API_
