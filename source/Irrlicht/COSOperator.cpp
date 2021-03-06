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
#ifndef _IRR_SOLARIS_PLATFORM_
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#endif

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#include "CIrrDeviceLinux.h"
#endif
#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
#include "MacOSX/OSXClipboard.h"
#endif

namespace irr
{

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
// constructor  linux
	COSOperator::COSOperator(const core::stringc& osVersion, CIrrDeviceLinux* device)
: OperatingSystem(osVersion), IrrDeviceLinux(device)
{
}
#endif

// constructor
COSOperator::COSOperator(const core::stringc& osVersion) : OperatingSystem(osVersion)
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

// UTF-16/UTF-32 to UTF-8
static int EncodeUTF8(char * dest, const wchar_t * src, int size) {
	char* pstr = dest;
	while(*src != 0 && (dest - pstr < size)) {
		if(*src < 0x80) {
			*dest = static_cast<char>(*src);
			++dest;
		} else if(*src < 0x800) {
			dest[0] = ((*src >> 6) & 0x1f) | 0xc0;
			dest[1] = ((*src) & 0x3f) | 0x80;
			dest += 2;
		} else if(*src < 0x10000 && (*src < 0xd800 || *src > 0xdfff)) {
			dest[0] = ((*src >> 12) & 0xf) | 0xe0;
			dest[1] = ((*src >> 6) & 0x3f) | 0x80;
			dest[2] = ((*src) & 0x3f) | 0x80;
			dest += 3;
		} else {
#if	WCHAR_MAX == 0xffff
				unsigned unicode = 0;
				unicode |= (*src++ & 0x3ff) << 10;
				unicode |= *src & 0x3ff;
				unicode += 0x10000;
				dest[0] = ((unicode >> 18) & 0x7) | 0xf0;
				dest[1] = ((unicode >> 12) & 0x3f) | 0x80;
				dest[2] = ((unicode >> 6) & 0x3f) | 0x80;
				dest[3] = ((unicode) & 0x3f) | 0x80;
#else
				dest[0] = ((*src >> 18) & 0x7) | 0xf0;
				dest[1] = ((*src >> 12) & 0x3f) | 0x80;
				dest[2] = ((*src >> 6) & 0x3f) | 0x80;
				dest[3] = ((*src) & 0x3f) | 0x80;
#endif
			dest += 4;
		}
		src++;
	}
	*dest = 0;
	return dest - pstr;
}
// UTF-8 to UTF-16/UTF-32
static int DecodeUTF8(wchar_t * dest, const char * src, int size) {
	const char* p = src;
	wchar_t* wp = dest;
	while(*p != 0 && (wp - dest < size)) {
		if((*p & 0x80) == 0) {
			*wp = *p;
			p++;
		} else if((*p & 0xe0) == 0xc0) {
			*wp = (((unsigned)p[0] & 0x1f) << 6) | ((unsigned)p[1] & 0x3f);
			p += 2;
		} else if((*p & 0xf0) == 0xe0) {
			*wp = (((unsigned)p[0] & 0xf) << 12) | (((unsigned)p[1] & 0x3f) << 6) | ((unsigned)p[2] & 0x3f);
			p += 3;
		} else if((*p & 0xf8) == 0xf0) {
#if	WCHAR_MAX == 0xffff
				unsigned unicode = (((unsigned)p[0] & 0x7) << 18) | (((unsigned)p[1] & 0x3f) << 12) | (((unsigned)p[2] & 0x3f) << 6) | ((unsigned)p[3] & 0x3f);
				unicode -= 0x10000;
				*wp++ = (unicode >> 10) | 0xd800;
				*wp = (unicode & 0x3ff) | 0xdc00;
#else
				*wp = (((unsigned)p[0] & 0x7) << 18) | (((unsigned)p[1] & 0x3f) << 12) | (((unsigned)p[2] & 0x3f) << 6) | ((unsigned)p[3] & 0x3f);
#endif
			p += 4;
		} else
			p++;
		wp++;
	}
	*wp = 0;
	return wp - dest;
}


//! copies text to the clipboard
void COSOperator::copyToClipboard(const wchar_t* _text) const
{
	if(wcslen(_text) == 0)
		return;
#if !defined(_IRR_WCHAR_FILESYSTEM) && !defined(_IRR_WINDOWS_API_)
	size_t lenOld = wcslen(_text) * sizeof(wchar_t);
	char* text = new char[lenOld + 1];
	size_t len = EncodeUTF8(text, _text, lenOld);
	text[len] = 0;
#else
	const wchar_t* text = _text;
#endif

// Windows version
#if defined(_IRR_XBOX_PLATFORM_)
#elif defined(_IRR_WINDOWS_API_)
	if(!OpenClipboard(NULL) || _text == 0)
		return;

	EmptyClipboard();

	HGLOBAL clipbuffer;
	wchar_t * buffer;

	clipbuffer = GlobalAlloc(GMEM_DDESHARE, sizeof(wchar_t) * (wcslen(text) + 1));
	buffer = (wchar_t*)GlobalLock(clipbuffer);

	wcscpy(buffer, text);

	GlobalUnlock(clipbuffer);
	SetClipboardData(CF_UNICODETEXT, clipbuffer);
	CloseClipboard();
	return;

// MacOSX version
#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)

	OSXCopyToClipboard(text);

#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	if(IrrDeviceLinux)
		IrrDeviceLinux->copyToClipboard(text);
#endif
#if !defined(_IRR_WCHAR_FILESYSTEM) && !defined(_IRR_WINDOWS_API_)
	if(text)
		delete[] text;
#endif
}


//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const wchar_t* COSOperator::getTextFromClipboard() const {
	static core::stringw wstring;
	const char* cbuffer = 0;
	wchar_t* buffer = 0;
#if defined(_IRR_XBOX_PLATFORM_)
	return 0;
#elif defined(_IRR_WINDOWS_API_)
	if(!OpenClipboard(NULL))
		return 0;

	HANDLE hData = nullptr;

	auto RetrieveBuffer = [](HANDLE& hData, UINT type)->void* {
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
	};

	if((buffer = (wchar_t*)RetrieveBuffer(hData, CF_UNICODETEXT)) == nullptr &&
		(cbuffer = (char*)RetrieveBuffer(hData, CF_TEXT)) == nullptr)
		return 0;

#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	cbuffer = OSXCopyFromClipboard();

#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	if(IrrDeviceLinux)
		cbuffer = IrrDeviceLinux->getTextFromClipboard();
#else
	return 0;
#endif
	if(cbuffer) {
		size_t lenOld = strlen(cbuffer);
		wchar_t *ws = new wchar_t[lenOld + 1];
		size_t len = DecodeUTF8(ws, cbuffer, lenOld);
		ws[len] = 0;
		wstring = ws;
		delete[] ws;
	} else if(buffer)
		wstring = buffer;
#if defined(_IRR_WINDOWS_API_)
	GlobalUnlock(hData);
	CloseClipboard();
#endif
	return wstring.c_str();
}


bool COSOperator::getProcessorSpeedMHz(u32* MHz) const
{
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
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
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
	// could probably be read from "/proc/cpuinfo" or "/proc/cpufreq"

	return false;
#endif
}

bool COSOperator::getSystemMemory(u32* Total, u32* Avail) const
{
#if defined(_IRR_WINDOWS_API_) && !defined (_IRR_XBOX_PLATFORM_)
	MEMORYSTATUS MemoryStatus;
	MemoryStatus.dwLength = sizeof(MEMORYSTATUS);

	// cannot fail
	GlobalMemoryStatus(&MemoryStatus);

	if (Total)
		*Total = (u32)(MemoryStatus.dwTotalPhys>>10);
	if (Avail)
		*Avail = (u32)(MemoryStatus.dwAvailPhys>>10);

	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return true;

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
	// TODO: implement for non-availablity of symbols/features
	return false;
#endif
#else
	// TODO: implement for OSX
	return false;
#endif
}


} // end namespace

