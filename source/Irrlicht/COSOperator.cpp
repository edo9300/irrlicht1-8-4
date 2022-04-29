// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COSOperator.h"

#ifdef _IRR_WINDOWS_API_
#ifndef _IRR_XBOX_PLATFORM_
#include <windows.h>
#endif
#else
#include <string.h>
#include <unistd.h>
#ifndef _IRR_ANDROID_PLATFORM_
#include <sys/types.h>
#ifdef _IRR_OSX_PLATFORM_
#include <sys/sysctl.h>
#endif
#endif
#endif

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#include "CIrrDeviceLinux.h"
#endif
#if defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_)
#include "CIrrDeviceWayland.h"
#endif
#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
#import <Cocoa/Cocoa.h>
#endif
#if defined(_IRR_COMPILE_WITH_SDL2_DEVICE_)
#include <SDL2/SDL_clipboard.h>
#endif

#include "fast_atof.h"

namespace irr
{

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
// constructor  linux
COSOperator::COSOperator(const core::stringc& osVersion, CIrrDeviceLinux* device)
	: OperatingSystem(osVersion), IrrDeviceLinux(device), DeviceType(EIDT_X11)
{
	#ifdef _DEBUG
	setDebugName("COSOperator");
	#endif
}
#endif

#if defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_)
// constructor  linux
COSOperator::COSOperator(const core::stringc& osVersion, CIrrDeviceWayland* device)
	: OperatingSystem(osVersion), IrrDeviceWayland(device), DeviceType(EIDT_WAYLAND)
{
	#ifdef _DEBUG
	setDebugName("COSOperator");
	#endif
}
#endif

// constructor
COSOperator::COSOperator(const core::stringc& osVersion, E_DEVICE_TYPE deviceType) : OperatingSystem(osVersion), DeviceType(deviceType)
{
	#ifdef _DEBUG
	setDebugName("COSOperator");
	#endif
}


//! returns the current operating system version as string.
const core::stringc& COSOperator::getOperatingSystemVersion() const
{
	return OperatingSystem;
}

#if defined(_IRR_WINDOWS_API_)
static void copyClipboardWindows(const wchar_t* wtext, size_t wlen) {
	if(!OpenClipboard(NULL))
		return;
	EmptyClipboard();

	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);

	HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, lenOld);
	char* cbuffer = (char*)GlobalLock(clipbuffer);
	core::wcharToUtf8(wtext, cbuffer, lenOld);
	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_TEXT, clipbuffer);

	clipbuffer = GlobalAlloc(GMEM_DDESHARE, lenOld);
	wchar_t* wbuffer = (wchar_t*)GlobalLock(clipbuffer);
	memcpy(wbuffer, wtext, lenOld);
	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_UNICODETEXT, clipbuffer);

	CloseClipboard();
}
#endif

#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
static void copyClipboardOSX(const char* ctext) {
	NSString* str;
	NSPasteboard* board;
	str = [NSString stringWithUTF8String : ctext];
	board = [NSPasteboard generalPasteboard];
	[board declareTypes : [NSArray arrayWithObject : NSStringPboardType] owner : NSApp] ;
	[board setString : str forType : NSStringPboardType] ;
}
#endif

//! copies text to the clipboard
void COSOperator::copyToClipboard(const wchar_t* wtext) const
{
	if(wtext == nullptr)
		return;
	auto wlen = wcslen(wtext);
	if(wlen == 0)
		return;

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	if(DeviceType == EIDT_WIN32) {
		copyClipboardWindows(wtext, wlen);
		return;
	}
#endif

	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);
	char* ctext = new char[lenOld];
	core::wcharToUtf8(wtext, ctext, lenOld);

	switch(DeviceType) {
#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	case EIDT_OSX:
		copyClipboardOSX(ctext);
		break;
#endif
#if defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
	case EIDT_SDL:
		SDL_SetClipboardText(ctext);
		break;
#endif
#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	case EIDT_X11:
		if(IrrDeviceLinux)
			IrrDeviceLinux->copyToClipboard(ctext);
		break;
#endif
#if defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_)
	case EIDT_WAYLAND:
		if(IrrDeviceWayland)
			IrrDeviceWayland->copyToClipboard(ctext);
		break;
#endif
	case EIDT_BEST: //we need at least 1 valid case to not generate compiler warnings about switch without case
	default:
		break;
	}
	delete[] ctext;
}


#if defined(_IRR_WINDOWS_API_)
static void* RetrieveBuffer(HANDLE& hData, UINT type) {
	if(hData = GetClipboardData(type)) {
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

static void* getClipboardWindows(const char*& cbuffer, const wchar_t*& wbuffer) {
	if(!OpenClipboard(NULL))
		return nullptr;

	HANDLE hData = nullptr;

	if((wbuffer = (wchar_t*)RetrieveBuffer(hData, CF_UNICODETEXT)) == nullptr)
		cbuffer = (char*)RetrieveBuffer(hData, CF_TEXT);

	return (void*)hData;
}

static void closeClipboardWindows(void* hData) {
	GlobalUnlock((HANDLE)hData);
	CloseClipboard();
}
#endif

#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
static const char* getClipboardOSX() {
	NSString* str = nil;
	NSPasteboard* board = nil;
	board = [NSPasteboard generalPasteboard];
	str = [board stringForType : NSStringPboardType];
	if(str != nil)
		return (char*)[str UTF8String];
	return nullptr;
}
#endif



//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const wchar_t* COSOperator::getTextFromClipboard() const {
	static core::stringw wstring;
	const char* cbuffer = nullptr;
	const wchar_t* wbuffer = nullptr;
	void* hData = nullptr;
	(void)wbuffer;
	(void)hData;
	switch(DeviceType) {
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	case EIDT_WIN32:
		hData = getClipboardWindows(cbuffer, wbuffer);
		break;
#endif
#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	case EIDT_OSX:
		cbuffer = getClipboardOSX();
		break;
#endif
#if defined(_IRR_COMPILE_WITH_SDL2_DEVICE_)
	case EIDT_SDL2:
		if(SDL_HasClipboardText())
			cbuffer = SDL_GetClipboardText();
		break;
#endif
#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	case EIDT_X11:
		if(IrrDeviceLinux)
			cbuffer = IrrDeviceLinux->getTextFromClipboard();
		break;
#endif
#if defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_)
	case EIDT_WAYLAND:
		if(IrrDeviceWayland)
			cbuffer = IrrDeviceWayland->getTextFromClipboard();
		break;
#endif
	case EIDT_BEST: //we need at least 1 valid case to not generate compiler warnings about switch without case
	default:
		break;
	}
	if(!wbuffer && !cbuffer)
		return nullptr;
	if(cbuffer) {
		size_t lenOld = strlen(cbuffer);
		wchar_t* ws = new wchar_t[lenOld + 1];
		core::utf8ToWchar(cbuffer, ws, (lenOld + 1)*sizeof(wchar_t));
		wstring = ws;
		delete[] ws;
	} else
		wstring = wbuffer;
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	if(hData)
		closeClipboardWindows(hData);
#endif
#if defined(_IRR_COMPILE_WITH_SDL2_DEVICE_)
	if(DeviceType == EIDT_SDL2)
		SDL_free(const_cast<char*>(cbuffer));
#endif
	return wstring.c_str();
}


bool COSOperator::getProcessorSpeedMHz(u32* MHz) const
{
	if (MHz)
		*MHz=0;
#if defined(_IRR_WINDOWS_API_) && !defined(_WIN32_WCE ) && !defined (_IRR_XBOX_PLATFORM_)
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

#elif defined(_IRR_OSX_PLATFORM_)
	struct clockinfo CpuClock;
	size_t Size = sizeof(clockinfo);

	if (!sysctlbyname("kern.clockrate", &CpuClock, &Size, NULL, 0))
		return false;
	else if (MHz)
		*MHz = CpuClock.hz;
	return true;
#else
	// read from "/proc/cpuinfo"
	FILE* file = fopen("/proc/cpuinfo", "r");
	if (file)
	{
		char buffer[1024];
		size_t r = fread(buffer, 1, 1023, file);
		buffer[r] = 0;
		buffer[1023]=0;
		core::stringc str(buffer);
		s32 pos = str.find("cpu MHz");
		if (pos != -1)
		{
			pos = str.findNext(':', pos);
			if (pos != -1)
			{
				while ( str[++pos] == ' ' );
				*MHz = core::fast_atof(str.c_str()+pos);
			}
		}
		fclose(file);
	}
	return (MHz && *MHz != 0);
#endif
}

bool COSOperator::getSystemMemory(u32* Total, u32* Avail) const
{
#if defined(_IRR_WINDOWS_API_) && !defined (_IRR_XBOX_PLATFORM_)

    #if (_WIN32_WINNT >= 0x0500)
	MEMORYSTATUSEX MemoryStatusEx;
 	MemoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);

	// cannot fail
	GlobalMemoryStatusEx(&MemoryStatusEx);

	if (Total)
		*Total = (u32)(MemoryStatusEx.ullTotalPhys>>10);
	if (Avail)
		*Avail = (u32)(MemoryStatusEx.ullAvailPhys>>10);
	return true;
	#else
	MEMORYSTATUS MemoryStatus;
	MemoryStatus.dwLength = sizeof(MEMORYSTATUS);

 	// cannot fail
	GlobalMemoryStatus(&MemoryStatus);

 	if (Total)
		*Total = (u32)(MemoryStatus.dwTotalPhys>>10);
 	if (Avail)
		*Avail = (u32)(MemoryStatus.dwAvailPhys>>10);
    return true;
	#endif

#elif defined(_IRR_POSIX_API_) && !defined(__FreeBSD__)
#if defined(_SC_PHYS_PAGES) && defined(_SC_AVPHYS_PAGES)
        long ps = sysconf(_SC_PAGESIZE);
        long pp = sysconf(_SC_PHYS_PAGES);
        long ap = sysconf(_SC_AVPHYS_PAGES);

	if ((ps==-1)||(pp==-1)||(ap==-1))
		return false;

	if (Total)
		*Total = (u32)((ps*(long long)pp)>>10);
	if (Avail)
		*Avail = (u32)((ps*(long long)ap)>>10);
	return true;
#else
	// TODO: implement for non-availability of symbols/features
	return false;
#endif
#elif defined(_IRR_OSX_PLATFORM_)
	int mib[2];
	int64_t physical_memory;
	size_t length;

	// Get the Physical memory size
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	length = sizeof(int64_t);
	sysctl(mib, 2, &physical_memory, &length, NULL, 0);
	return true;
#else
	// TODO: implement for others
	return false;
#endif
}


} // end namespace

