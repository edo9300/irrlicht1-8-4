// Copyright (c) 2026 Edoardo Lolletti <edoardo762@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later
// Refer to the COPYING file included.

#include "COSOperatorPosix.h"

#ifdef _IRR_POSIX_API_

#include "fast_atof.h"

#include <unistd.h>

namespace irr
{

COSOperatorPosix::COSOperatorPosix(const core::stringc& osversion) :
	COSOperator(osversion) {
#ifdef _DEBUG
	setDebugName("COSOperatorPosix");
#endif
}

bool COSOperatorPosix::getProcessorSpeedMHz(u32* MHz) const {
	FILE* file = fopen("/proc/cpuinfo", "r");
	if (file)
	{
		char buffer[1024];
		size_t r = fread(buffer, 1, 1023, file);
		buffer[r] = 0;
		buffer[1023]=0;
		core::stringc str(buffer);
		s32 pos = str.find("cpu MHz");
		if (pos != -1)
		{
			pos = str.findNext(':', pos);
			if (pos != -1)
			{
				while ( str[++pos] == ' ' );
				*MHz = core::fast_atof(str.c_str()+pos);
			}
		}
		fclose(file);
	}
	return (MHz && *MHz != 0);
}

bool COSOperatorPosix::getSystemMemory(u32* Total, u32* Avail) const {
#if !defined(__FreeBSD__)
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
	// TODO: implement for non-availability of symbols/features
	return false;
#endif
#else
	// TODO: implement for others
	return false;
#endif
}

}

#endif