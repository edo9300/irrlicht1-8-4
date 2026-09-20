// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#ifndef C_OS_OPERATOR_WINDOWS_H_INCLUDED
#define C_OS_OPERATOR_WINDOWS_H_INCLUDED

#include "COSOperator.h"

namespace irr
{

//! The Operating system operator provides operation system specific methods and information.
class COSOperatorWindows : public COSOperator
{
public:

	COSOperatorWindows(const core::stringc& osversion);

	virtual void copyToClipboard(const wchar_t* text) const _IRR_OVERRIDE_;

	virtual const wchar_t* getTextFromClipboard() const _IRR_OVERRIDE_;

	virtual bool getProcessorSpeedMHz(u32* MHz) const _IRR_OVERRIDE_;

	virtual bool getSystemMemory(u32* Total, u32* Avail) const _IRR_OVERRIDE_;
};

} // end namespace

#endif //C_OS_OPERATOR_WINDOWS_H_INCLUDED
