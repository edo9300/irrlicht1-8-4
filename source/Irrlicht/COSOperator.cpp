// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COSOperator.h"

#ifdef _IRR_WINDOWS_API_
#ifndef _IRR_XBOX_PLATFORM_
#include <windows.h>
#include "IrrFunctionCast.h"
#endif
#else
#include <string.h>
#include <unistd.h>
#ifndef _IRR_ANDROID_PLATFORM_
#include <sys/types.h>
#endif
#endif

#include "fast_atof.h"


namespace irr
{

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

//! copies text to the clipboard
void COSOperator::copyToClipboard(const wchar_t* wtext) const
{
	return;
}


//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const wchar_t* COSOperator::getTextFromClipboard() const {
	return nullptr;
}


bool COSOperator::getProcessorSpeedMHz(u32* MHz) const
{
	return false;
}

bool COSOperator::getSystemMemory(u32* Total, u32* Avail) const
{
	return false;
}


} // end namespace

