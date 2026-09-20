// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#ifndef C_OS_OPERATOR_POSIX_H_INCLUDED
#define C_OS_OPERATOR_POSIX_H_INCLUDED

#include "COSOperator.h"

namespace irr
{

class COSOperatorPosix : public COSOperator {

public:
	COSOperatorPosix(const core::stringc& osversion);

	//! copies text to the clipboard
	virtual void copyToClipboard(const wchar_t* text) const _IRR_OVERRIDE_ = 0;

	//! gets text from the clipboard
	//! \return Returns 0 if no string is in there.
	virtual const wchar_t* getTextFromClipboard() const _IRR_OVERRIDE_ = 0;

	//! gets the processor speed in megahertz
	//! \param Mhz:
	//! \return Returns true if successful, false if not
	virtual bool getProcessorSpeedMHz(u32* MHz) const _IRR_OVERRIDE_;

	//! gets the total and available system RAM in kB
	//! \param Total: will contain the total system memory
	//! \param Avail: will contain the available memory
	//! \return Returns true if successful, false if not
	virtual bool getSystemMemory(u32* Total, u32* Avail) const _IRR_OVERRIDE_;
};

}

#endif // C_OS_OPERATOR_POSIX_H_INCLUDED