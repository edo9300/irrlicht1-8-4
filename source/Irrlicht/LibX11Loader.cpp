// Copyright (C) 2021 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "LibX11Loader.h"

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_) && defined(_IRR_X11_DYNAMIC_LOAD_)

#include <dlfcn.h>
#include "os.h"

namespace irr
{
#define X(name) decltype(::name)* X11Loader::name{ nullptr };
#include "LibX11Loader.inl"
#ifdef _IRR_LINUX_X11_VIDMODE_
#include "LibXxf86vmLoader.inl"
#endif
#undef X

void* X11Loader::LibX11{ nullptr };
#ifdef _IRR_LINUX_X11_VIDMODE_
void* X11Loader::LibXxf86vm{ nullptr };
#endif
int X11Loader::amt{ 0 };

void X11Loader::Load() {
	LibX11 = dlopen("libX11.so.6", RTLD_LAZY); //libx11 has almost 2000 exported symbols
											   //there's no point in using RTDL_NOW
	if(LibX11) {
#define X(name) name=(decltype(name))dlsym(LibX11, #name);\
	if(!name) {\
		os::Printer::log("Failed to load " #name " from libX11.so.6", ELL_ERROR);\
		Unload();\
		return;\
	}
#include "LibX11Loader.inl"
#undef X
	} else {
		os::Printer::log("Failed to load libX11.so.6", ELL_ERROR);
		return;
	}
#ifdef _IRR_LINUX_X11_VIDMODE_
	LibXxf86vm = dlopen("libXxf86vm.so.1", RTLD_NOW);
	if(LibXxf86vm) {
#define X(name) name=(decltype(name))dlsym(LibXxf86vm, #name);\
	if(!name) {\
		os::Printer::log("Failed to load " #name " from libXxf86vm.so.1", ELL_ERROR);\
		Unload();\
		return;\
	}
#include "LibXxf86vmLoader.inl"
#undef X
	} else {
		os::Printer::log("Failed to load libXxf86vm.so.1", ELL_ERROR);
		Unload();
		return;
	}
#endif
}

void X11Loader::Unload() {
	if(LibX11) {
		dlclose(LibX11);
#define X(name) name=nullptr;
#include "LibX11Loader.inl"
#undef X
		LibX11 = nullptr;
	}
#ifdef _IRR_LINUX_X11_VIDMODE_
	if(LibXxf86vm) {
		dlclose(LibXxf86vm);
#define X(name) name=nullptr;
#include "LibXxf86vmLoader.inl"
#undef X
		LibXxf86vm = nullptr;
	}
#endif
}

bool X11Loader::Init()
{
	if(amt==0 && !LibX11)
		Load();
	inited = true;
	amt++;
	return LibX11;
}


//! destructor
X11Loader::~X11Loader()
{
	if(!inited)
		return;
	if(--amt <= 0 && LibX11) {
		Unload();
	}
}

} // end namespace

#endif // _IRR_COMPILE_WITH_X11_DEVICE_

