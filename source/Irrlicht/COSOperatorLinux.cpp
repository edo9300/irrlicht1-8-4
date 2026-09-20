// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#include "COSOperatorLinux.h"

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_

#include "CIrrDeviceLinux.h"

namespace irr
{

COSOperatorLinux::COSOperatorLinux(const core::stringc& osversion, CIrrDeviceLinux* device) :
	base(osversion), IrrDeviceLinux(device) {
#ifdef _DEBUG
	setDebugName("COSOperatorLinux");
#endif
}

void COSOperatorLinux::copyToClipboard(const wchar_t* wtext) const {
	if(wtext == nullptr)
		return;
	auto wlen = wcslen(wtext);
	if(wlen == 0)
		return;
	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);
	char* ctext = new char[lenOld];
	core::wcharToUtf8(wtext, ctext, lenOld);
	IrrDeviceLinux->copyToClipboard(ctext);
	delete[] ctext;
}

const wchar_t* COSOperatorLinux::getTextFromClipboard() const {
	const char* cbuffer = IrrDeviceLinux->getTextFromClipboard();
	if(!cbuffer)
		return nullptr;
	if(cbuffer) {
		size_t lenOld = strlen(cbuffer);
		wchar_t* ws = new wchar_t[lenOld + 1];
		core::utf8ToWchar(cbuffer, ws, (lenOld + 1) * sizeof(wchar_t));
		ClipboardString = ws;
		delete[] ws;
	}
	return ClipboardString.data();
}

}

#endif