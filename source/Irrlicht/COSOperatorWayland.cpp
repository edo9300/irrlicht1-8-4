// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#include "COSOperatorWayland.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "CIrrDeviceWayland.h"

namespace irr
{

COSOperatorWayland::COSOperatorWayland(const core::stringc& osversion, CIrrDeviceWayland* device) :
	COSOperatorPosix(osversion), IrrDeviceWayland(device) {
#ifdef _DEBUG
	setDebugName("COSOperatorWayland");
#endif
}

void COSOperatorWayland::copyToClipboard(const wchar_t* wtext) const {
	if(wtext == nullptr)
		return;
	auto wlen = wcslen(wtext);
	if(wlen == 0)
		return;
	const size_t lenOld = (wlen + 1) * sizeof(wchar_t);
	char* ctext = new char[lenOld];
	core::wcharToUtf8(wtext, ctext, lenOld);
	IrrDeviceWayland->copyToClipboard(ctext);
	delete[] ctext;
}

const wchar_t* COSOperatorWayland::getTextFromClipboard() const {
	const char* cbuffer = IrrDeviceWayland->getTextFromClipboard();
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