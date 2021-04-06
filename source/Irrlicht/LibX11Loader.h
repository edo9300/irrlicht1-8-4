// Copyright (C) 2021 Edoardo Lolletti
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef __LIB_X11_LOADER_H_INCLUDED__
#define __LIB_X11_LOADER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_

#ifdef _IRR_COMPILE_WITH_X11_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#ifdef _IRR_LINUX_X11_VIDMODE_
#include <X11/extensions/xf86vmode.h>
#endif

namespace irr
{

#ifdef _IRR_X11_DYNAMIC_LOAD_
	struct X11Loader {
#define X(name) static decltype(::name) *name;
#include "LibX11Loader.inl"
#ifdef _IRR_LINUX_X11_VIDMODE_
#include "LibXxf86vmLoader.inl"
#endif
#undef X
		// XIC(*XCreateIC)( XIM, ...){ nullptr };
		X11Loader()=default;
		~X11Loader();
		bool Init();
	private:
		static void* LibX11;
		static void* LibXxf86vm;
		static int amt;
		void Load();
		void Unload();
		bool inited{ false };
	};
#else
	struct X11Loader {
#define X(name) constexpr static auto name = ::name;
#include "LibX11Loader.inl"
#ifdef _IRR_LINUX_X11_VIDMODE_
#include "LibXxf86vmLoader.inl"
#endif
#undef X
		// XIC(*XCreateIC)( XIM, ...){ nullptr };
		X11Loader()=default;
		~X11Loader()=default;
	};
#endif 	//_IRR_X11_DYNAMIC_LOAD_
} // end namespace irr

#endif //_IRR_COMPILE_WITH_X11_

#endif // _IRR_COMPILE_WITH_X11_DEVICE_
#endif // __LIB_X11_LOADER_H_INCLUDED__

