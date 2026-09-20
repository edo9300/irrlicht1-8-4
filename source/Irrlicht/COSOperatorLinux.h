// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#ifndef C_OS_OPERATOR_LINUX_H_INCLUDED
#define C_OS_OPERATOR_LINUX_H_INCLUDED

#include "IrrCompileConfig.h"

#ifdef _IRR_WINDOWS_API_
#include "COSOperatorWindows.h"
#define DEFAULT_OPERATOR COSOperatorWindows
#else
#include "COSOperatorPosix.h"
#define DEFAULT_OPERATOR COSOperatorPosix
#endif

namespace irr
{

class CIrrDeviceLinux;

class COSOperatorLinux : public DEFAULT_OPERATOR {

public:
	COSOperatorLinux(const core::stringc& osversion, CIrrDeviceLinux* device);

	//! copies text to the clipboard
	virtual void copyToClipboard(const wchar_t* text) const _IRR_OVERRIDE_;

	//! gets text from the clipboard
	//! \return Returns 0 if no string is in there.
	virtual const wchar_t* getTextFromClipboard() const _IRR_OVERRIDE_;

private:
	CIrrDeviceLinux* IrrDeviceLinux;
	mutable core::stringw ClipboardString;
};

}

#undef DEFAULT_OPERATOR

#endif // C_OS_OPERATOR_LINUX_H_INCLUDED