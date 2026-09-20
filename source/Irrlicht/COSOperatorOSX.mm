// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#include "COSOperatorOSX.h"

#ifdef _IRR_OSX_PLATFORM_

#include <unistd.h>
#include <sys/sysctl.h>
#import <Cocoa/Cocoa.h>

#include <AvailabilityMacros.h>
#if !defined(MAC_OS_X_VERSION_10_14) || MAC_OS_X_VERSION_MIN_ALLOWED < MAC_OS_X_VERSION_10_14
#define NSPasteboardTypeString NSStringPboardType
#endif

namespace irr
{

COSOperatorOSX::COSOperatorOSX(const core::stringc& osversion) :
	COSOperator(osversion) {
#ifdef _DEBUG
	setDebugName("COSOperatorOSX");
#endif
}

void COSOperatorOSX::copyToClipboard(const wchar_t* wtext) const {
	if(wtext == nullptr)
		return;
	auto wlen = wcslen(wtext);
	if(wlen == 0)
		return;

	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);
	char* ctext = new char[lenOld];
	core::wcharToUtf8(wtext, ctext, lenOld);

	NSString* str;
	NSPasteboard* board;
	str = [NSString stringWithUTF8String : ctext];
	board = [NSPasteboard generalPasteboard];
	[board declareTypes : [NSArray arrayWithObject : NSPasteboardTypeString] owner : NSApp] ;
	[board setString : str forType : NSPasteboardTypeString] ;

	delete[] ctext;
}

//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const wchar_t* COSOperatorOSX::getTextFromClipboard() const {
	NSPasteboard* board = nil;
	board = [NSPasteboard generalPasteboard];
	NSString* str = [board stringForType : NSPasteboardTypeString];
	if(str == nil)
		return nullptr;
	const char* cbuffer = (const char*)[str UTF8String];
	size_t lenOld = strlen(cbuffer);
	wchar_t* ws = new wchar_t[lenOld + 1];
	core::utf8ToWchar(cbuffer, ws, (lenOld + 1)*sizeof(wchar_t));
	ClipboardString = ws;
	delete[] ws;
	return ClipboardString.c_str();
}

bool COSOperatorOSX::getProcessorSpeedMHz(u32* MHz) const {
	struct clockinfo CpuClock;
	size_t Size = sizeof(clockinfo);

	if (!sysctlbyname("kern.clockrate", &CpuClock, &Size, NULL, 0))
		return false;
	else if (MHz)
		*MHz = CpuClock.hz;
	return true;
}

bool COSOperatorOSX::getSystemMemory(u32* Total, u32* Avail) const {
	int mib[2];
	int64_t physical_memory;
	size_t length;

	// Get the Physical memory size
	mib[0] = CTL_HW;
	mib[1] = HW_MEMSIZE;
	length = sizeof(int64_t);
	sysctl(mib, 2, &physical_memory, &length, NULL, 0);
	return true;
}

}

#endif