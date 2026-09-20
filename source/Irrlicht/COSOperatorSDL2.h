// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#ifndef C_OS_OPERATOR_LINUX_H_INCLUDED
#define C_OS_OPERATOR_LINUX_H_INCLUDED

#include <IrrCompileConfig.h>

#ifdef _IRR_COMPILE_WITH_SDL2_DEVICE_

#ifdef _IRR_WINDOWS_API_
#include "COSOperatorWindows.h"
#define DEFAULT_OPERATOR COSOperatorWindows
#elif defined(_IRR_POSIX_API_)
#include "COSOperatorPosix.h"
#define DEFAULT_OPERATOR COSOperatorPosix
#else
#include "COSOperator.h"
#define DEFAULT_OPERATOR COSOperator
#endif

namespace irr
{

class COSOperatorSDL2 : public DEFAULT_OPERATOR {

	using base = DEFAULT_OPERATOR;

public:
	COSOperatorSDL2(const core::stringc& osversion);

	//! copies text to the clipboard
	virtual void copyToClipboard(const wchar_t* text) const _IRR_OVERRIDE_;

	//! gets text from the clipboard
	//! \return Returns 0 if no string is in there.
	virtual const wchar_t* getTextFromClipboard() const _IRR_OVERRIDE_;
};

}

#undef DEFAULT_OPERATOR

#endif //_IRR_COMPILE_WITH_X11_DEVICE_

#endif // C_OS_OPERATOR_LINUX_H_INCLUDED
