#ifndef CIRR_DEVICE_WIN32_WINDOWS_VERSION_WMI_H
#define CIRR_DEVICE_WIN32_WINDOWS_VERSION_WMI_H
#ifdef _WIN32
#include <irrString.h>

namespace irr {

void GetWindowsVersion(core::stringc& out, core::stringc& compatModeVersion);

}

#endif //_WIN32
#endif //CIRR_DEVICE_WIN32_WINDOWS_VERSION_WMI_H