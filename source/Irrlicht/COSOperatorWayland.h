// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#ifndef C_OS_OPERATOR_WAYLAND_H_INCLUDED
#define C_OS_OPERATOR_WAYLAND_H_INCLUDED

#include <IrrCompileConfig.h>

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "COSOperatorPosix.h"

namespace irr
{

class CIrrDeviceWayland;

class COSOperatorWayland : public COSOperatorPosix {

public:
	COSOperatorWayland(const core::stringc& osversion, CIrrDeviceWayland* device);

	//! copies text to the clipboard
	virtual void copyToClipboard(const wchar_t* text) const _IRR_OVERRIDE_;

	//! gets text from the clipboard
	//! \return Returns 0 if no string is in there.
	virtual const wchar_t* getTextFromClipboard() const _IRR_OVERRIDE_;

private:
	CIrrDeviceWayland* IrrDeviceWayland;
	mutable core::stringw ClipboardString;
};

}

#endif // _IRR_COMPILE_WITH_WAYLAND_DEVICE_


#endif // C_OS_OPERATOR_WAYLAND_H_INCLUDED