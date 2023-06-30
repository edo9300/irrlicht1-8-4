// Copyright (C) 2013 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CEGLManager.h"

#ifdef _IRR_COMPILE_WITH_EGL_MANAGER_

#include "irrString.h"
#include "irrArray.h"
#include "os.h"

template<typename T, typename T2>
inline T function_cast(T2 ptr) {
	using generic_function_ptr = void (*)(void);
	return reinterpret_cast<T>(reinterpret_cast<generic_function_ptr>(ptr));
}

#if defined(_IRR_DYNAMIC_OPENGL_ES_1_) || defined(_IRR_DYNAMIC_OPENGL_ES_2_) || defined(_IRR_DYNAMIC_OPENGL_)
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#endif

#if defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
#include <android/native_activity.h>
#endif

#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
#include "LibX11Loader.h"
#endif

namespace irr
{
namespace video
{

CEGLManager::CEGLManager() : IContextManager(), EglWindow(0), EglDisplay(EGL_NO_DISPLAY),
    EglSurface(EGL_NO_SURFACE), EglContext(EGL_NO_CONTEXT), EglConfig(0), MajorVersion(0), MinorVersion(0)
#if defined(_IRR_DYNAMIC_OPENGL_ES_1_) || defined(_IRR_DYNAMIC_OPENGL_ES_2_) || defined(_IRR_DYNAMIC_OPENGL_)
	, LibEGL(0)
	, LibGLES(0)
#define EGL_FUNC(name, ...) ,p##name(0)
#include "CEGLFunctions.inl"
#undef EGL_FUNC
#endif
#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
	, WaylandDevice(nullptr)
#endif
#ifdef _IRR_COMPILE_WITH_X11_DEVICE_
	, Initialized(true)
#endif
{
	#ifdef _DEBUG
	setDebugName("CEGLManager");
	#endif
}

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
CEGLManager::CEGLManager(CIrrDeviceWayland* wayland_device) : CEGLManager() {
	WaylandDevice = wayland_device;
}
#endif

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_
CEGLManager::CEGLManager(const SIrrlichtCreationParameters& params, const SExposedVideoData& data) : CEGLManager() {
	Initialized = initialize(params, data);
	if(!Initialized)
		return;
	GenerateConfig();
	Initialized = EglConfig != 0;
	if(!Initialized)
		return;
	EGLint vid;
	Initialized = peglGetConfigAttrib(EglDisplay, EglConfig, EGL_NATIVE_VISUAL_ID, &vid);
	if(!Initialized)
		return;
	XVisualInfo vis_tmpl;
	int num_visuals;
	vis_tmpl.visualid = vid;
	VisualInfo = X11Loader::XGetVisualInfo((Display*)data.OpenGLLinux.X11Display, VisualIDMask, &vis_tmpl, &num_visuals);
	Initialized = VisualInfo != nullptr;
	if(!Initialized)
		return;
}
#endif

CEGLManager::~CEGLManager()
{
    destroyContext();
    destroySurface();
    terminate();
#if defined(_IRR_DYNAMIC_OPENGL_ES_1_) || defined(_IRR_DYNAMIC_OPENGL_ES_2_) || defined(_IRR_DYNAMIC_OPENGL_)
#ifdef _WIN32
	if(LibEGL)
		FreeLibrary((HMODULE)LibEGL);
	if(LibGLES)
		FreeLibrary((HMODULE)LibGLES);
#else
	if(LibEGL)
		dlclose(LibEGL);
	if(LibGLES)
		dlclose(LibGLES);
#endif
#endif
}

bool CEGLManager::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data)
{
#ifdef _IRR_COMPILE_WITH_X11_DEVICE_
	if(!Initialized) {
		terminate();
		return false;
	}
#endif
	// store new data
	Params=params;
	Data=data;

	if (EglWindow != 0 && EglDisplay != EGL_NO_DISPLAY)
        return true;
	if(!LoadEGL())
		return false;

	// Window depends on the platform.
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	EglWindow = (NativeWindowType)Data.OpenGLWin32.HWnd;
	Data.OpenGLWin32.HDc = GetDC((HWND)EglWindow);
	EglDisplay = peglGetDisplay((NativeDisplayType)Data.OpenGLWin32.HDc);
#elif defined(_IRR_EMSCRIPTEN_PLATFORM_)
	EglWindow = 0;
	EglDisplay = peglGetDisplay(EGL_DEFAULT_DISPLAY);
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_) || defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_)
#if defined(_IRR_COMPILE_WITH_WAYLAND_DEVICE_)
	if(WaylandDevice != nullptr) {
		if(setenv("EGL_PLATFORM", "wayland", 1) != 0) {
			os::Printer::log("Could not set \"EGL_PLATFORM\" environment variable to \"wayland\".");
			return false;
		}
		EglWindow = (NativeWindowType)Data.OpenGLWayland.EGLWindow;
		EglDisplay = peglGetDisplay((NativeDisplayType)Data.OpenGLWayland.EGLDisplay);
	}
#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	else
#endif
#endif
#if defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	{
		if(EglDisplay != EGL_NO_DISPLAY) {
			EglWindow = (NativeWindowType)Data.OpenGLLinux.X11Window;
			return false;
		}
		if(setenv("EGL_PLATFORM", "x11", 1) != 0) {
			os::Printer::log("Could not set \"EGL_PLATFORM\" environment variable to \"x11\".");
			return false;
		}
		EglWindow = (NativeWindowType)Data.OpenGLLinux.X11Window;
		EglDisplay = peglGetDisplay((NativeDisplayType)Data.OpenGLLinux.X11Display);
	}
#endif
#elif defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
	EglWindow =	(ANativeWindow*)Data.OGLESAndroid.Window;
	EglDisplay = peglGetDisplay(EGL_DEFAULT_DISPLAY);
#elif defined(_IRR_COMPILE_WITH_FB_DEVICE_)
	EglWindow = (NativeWindowType)Data.OpenGLFB.Window;
	EglDisplay = peglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif

	// We must check if EGL display is valid.
	if (EglDisplay == EGL_NO_DISPLAY)
    {
		os::Printer::log("Could not get EGL display.");
		terminate();
        return false;
    }

	// Initialize EGL here.
	if (!peglInitialize(EglDisplay, &MajorVersion, &MinorVersion))
    {
		os::Printer::log("Could not initialize EGL display.");

        EglDisplay = EGL_NO_DISPLAY;
		terminate();
        return false;
    }
	else
		os::Printer::log("EGL version", core::stringc(MajorVersion+(MinorVersion*0.1f)).c_str());

	if(Params.DriverType == EDT_OPENGL && MinorVersion < 4) {
		os::Printer::log("Current EGL version doesn't support OpenGL Context creation.");
		terminate();
		return false;
	}

    return true;
}

void CEGLManager::terminate()
{
	if (EglWindow == 0 && EglDisplay == EGL_NO_DISPLAY)
		return;

	if (EglDisplay != EGL_NO_DISPLAY)
	{
		// We should unbind current EGL context before terminate EGL.
		peglMakeCurrent(EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		peglTerminate(EglDisplay);
		EglDisplay = EGL_NO_DISPLAY;
	}

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	if (Data.OpenGLWin32.HDc)
    {
		ReleaseDC((HWND)EglWindow, (HDC)Data.OpenGLWin32.HDc);
        Data.OpenGLWin32.HDc = 0;
    }
#endif

    MajorVersion = 0;
    MinorVersion = 0;
}

bool CEGLManager::generateSurface()
{
	if(EglDisplay == EGL_NO_DISPLAY)
		return false;

	if(EglSurface != EGL_NO_SURFACE)
		return true;

	// We should assign new WindowID on platforms, where WindowID may change at runtime,
	// at this time only Android support this feature.
	// this needs an update method instead!

	GenerateConfig();

#if defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
	EglWindow = (ANativeWindow*)Data.OGLESAndroid.Window;
#endif

	if ( EglConfig == 0 )
	{
		os::Printer::log("Could not get config for EGL display.");
		return false;
	}


#if defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
    EGLint Format = 0;
    peglGetConfigAttrib(EglDisplay, EglConfig, EGL_NATIVE_VISUAL_ID, &Format);

    ANativeWindow_setBuffersGeometry(EglWindow, 0, 0, Format);
#endif

	// Now we are able to create EGL surface.
	EglSurface = peglCreateWindowSurface(EglDisplay, EglConfig, EglWindow, 0);

	if (EGL_NO_SURFACE == EglSurface)
		EglSurface = peglCreateWindowSurface(EglDisplay, EglConfig, 0, 0);

	if (EGL_NO_SURFACE == EglSurface)
		os::Printer::log("Could not create EGL surface.");

#ifdef EGL_VERSION_1_2
	if (MinorVersion > 1)
		peglBindAPI(Params.DriverType == EDT_OPENGL ? EGL_OPENGL_API : EGL_OPENGL_ES_API);
#endif

    return true;
}

EGLConfig CEGLManager::chooseConfig(EConfigStyle confStyle)
{
	EGLConfig configResult = 0;

	// Find proper OpenGL BIT.
	EGLint eglOpenGLBIT = 0;
	switch (Params.DriverType)
	{
	case EDT_OGLES1:
		eglOpenGLBIT = EGL_OPENGL_ES_BIT;
		break;
	case EDT_OGLES2:
	case EDT_WEBGL1:
		eglOpenGLBIT = EGL_OPENGL_ES2_BIT;
		break;
	case EDT_OPENGL:
		eglOpenGLBIT = EGL_OPENGL_BIT;
		break;
	default:
		break;
	}

	if ( confStyle == ECS_EGL_CHOOSE_FIRST_LOWER_EXPECTATIONS )
	{
		EGLint Attribs[] =
		{
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, Params.WithAlphaChannel ? 1:0,
			EGL_BUFFER_SIZE, Params.Bits,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_DEPTH_SIZE, Params.ZBufferBits,
			EGL_STENCIL_SIZE, Params.Stencilbuffer,
			EGL_SAMPLE_BUFFERS, Params.AntiAlias ? 1:0,
			EGL_SAMPLES, Params.AntiAlias,
	#ifdef EGL_VERSION_1_3
			EGL_RENDERABLE_TYPE, eglOpenGLBIT,
	#endif
			EGL_NONE, 0
		};

		EGLint numConfigs = 0;
		u32 steps = 5;

		// Choose the best EGL config.
			// TODO: We should also have a confStyle ECS_EGL_CHOOSE_CLOSEST
			//       which doesn't take first result of eglChooseConfigs,
			//       but the closest to requested parameters. eglChooseConfigs
			//       can return more than 1 result and first one might have
			//       "better" values than requested (more bits per pixel etc).
			//       So this returns the config which can do most, not the
			//       config which is closest to the requested parameters.
		//
		while (!peglChooseConfig(EglDisplay, Attribs, &configResult, 1, &numConfigs) || !numConfigs)
		{
			switch (steps)
			{
			case 5: // samples
				if (Attribs[19] > 2)	// Params.AntiAlias
					--Attribs[19];
				else
				{
					Attribs[17] = 0;	// Params.Stencilbuffer
					Attribs[19] = 0;	// Params.AntiAlias
					--steps;
				}
				break;
			case 4: // alpha
				if (Attribs[7])	// Params.WithAlphaChannel
				{
					Attribs[7] = 0;

					if (Params.AntiAlias)
					{
						Attribs[17] = 1;
						Attribs[19] = Params.AntiAlias;
						steps = 5;
					}
				}
				else
					--steps;
				break;
			case 3: // stencil
				if (Attribs[15])	// Params.Stencilbuffer
				{
					Attribs[15] = 0;

					if (Params.AntiAlias)
					{
						Attribs[17] = 1;
						Attribs[19] = Params.AntiAlias;
						steps = 5;
					}
				}
				else
					--steps;
				break;
			case 2: // depth size
				if (Attribs[13] > 16)	// Params.ZBufferBits
				{
					Attribs[13] -= 8;
				}
				else
					--steps;
				break;
			case 1: // buffer size
				if (Attribs[9] > 16)	// Params.Bits
				{
					Attribs[9] -= 8;
				}
				else
					--steps;
				break;
			default:
				return 0;
			}
		}

		if (Params.AntiAlias && !Attribs[17])
			os::Printer::log("No multisampling.");

		if (Params.WithAlphaChannel && !Attribs[7])
			os::Printer::log("No alpha.");

		if (Params.Stencilbuffer && !Attribs[15])
			os::Printer::log("No stencil buffer.");

		if (Params.ZBufferBits > Attribs[13])
			os::Printer::log("No full depth buffer.");

		if (Params.Bits > Attribs[9])
			os::Printer::log("No full color buffer.");
	}
	else if ( confStyle == ECS_IRR_CHOOSE )
	{
		// find number of available configs
		EGLint numConfigs;
		if ( peglGetConfigs( EglDisplay, NULL, 0, &numConfigs) == EGL_FALSE )
		{
			testEGLError();
			return 0;
		}

		if ( numConfigs <= 0 )
			return 0;

		// Get all available configs.
		EGLConfig * configs = new EGLConfig[numConfigs];
		if ( peglGetConfigs( EglDisplay, configs, numConfigs, &numConfigs) == EGL_FALSE )
		{
			testEGLError();
			return 0;
		}

		// Find the best one.
		core::array<SConfigRating> ratings((u32)numConfigs);
		for ( u32 i=0; i < (u32)numConfigs; ++i )
		{
			SConfigRating r;
			r.config = configs[i];
			r.rating = rateConfig(r.config, eglOpenGLBIT);

			if ( r.rating >= 0 )
				ratings.push_back(r);
		}

		if ( ratings.size() > 0 )
		{
			ratings.sort();
			configResult = ratings[0].config;

			if ( ratings[0].rating != 0 )
			{
				// This is just to print some log info (it also rates again while doing that, but rating is cheap enough, so that doesn't matter here).
				rateConfig(ratings[0].config, eglOpenGLBIT, true);
			}
		}

		delete[] configs;
	}

	return configResult;
}

irr::s32 CEGLManager::rateConfig(EGLConfig config, EGLint eglOpenGLBIT, bool log)
{
	// some values must be there or we ignore the config
#ifdef EGL_VERSION_1_3
	EGLint attribRenderableType = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_RENDERABLE_TYPE, &attribRenderableType);
	if  ( !(attribRenderableType & eglOpenGLBIT) )
	{
		if ( log )
			os::Printer::log("!(EGL_RENDERABLE_TYPE & eglOpenGLBIT)");
		return -1;
	}
#endif
	EGLint attribSurfaceType = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_SURFACE_TYPE, &attribSurfaceType);
	if ( !(attribSurfaceType & EGL_WINDOW_BIT) )
	{
		if ( log )
			os::Printer::log("EGL_SURFACE_TYPE != EGL_WINDOW_BIT");
		return -1;
	}

	// Generally we give a really bad rating if attributes are worse than requested
	// We give a slight worse rating if attributes are not exact as requested
	// And we use some priorities which might make sense (but not really fine-tuned,
	// so if you think other priorities would be better don't worry about changing the values.
	int rating = 0;

	EGLint attribBufferSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_BUFFER_SIZE, &attribBufferSize);
	if ( attribBufferSize < Params.Bits )
	{
		if ( log )
			os::Printer::log("No full color buffer.");
		rating += 100;
	}
	if ( attribBufferSize > Params.Bits )
	{
		if ( log )
			os::Printer::log("Larger color buffer.", ELL_DEBUG);
		++rating;
	}

	EGLint attribRedSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_RED_SIZE, &attribRedSize);
	if ( attribRedSize < 5 && Params.Bits >= 4 )
		rating += 100;
	else if ( attribRedSize < 8 && Params.Bits >= 24)
		rating += 10;
	else if ( attribRedSize >= 8 && Params.Bits < 24 )
		rating ++;
	EGLint attribGreenSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_GREEN_SIZE, &attribGreenSize);
	if ( attribGreenSize < 5 && Params.Bits >= 4 )
		rating += 100;
	else if ( attribGreenSize < 8 && Params.Bits >= 24)
		rating += 10;
	else if ( attribGreenSize >= 8 && Params.Bits < 24 )
		rating ++;
	EGLint attribBlueSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_BLUE_SIZE, &attribBlueSize);
	if ( attribBlueSize < 5 && Params.Bits >= 4 )
		rating += 100;
	else if ( attribBlueSize < 8 && Params.Bits >= 24)
		rating += 10;
	else if ( attribBlueSize >= 8 && Params.Bits < 24 )
		rating ++;

	EGLint attribAlphaSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_ALPHA_SIZE, &attribAlphaSize);
	if ( Params.WithAlphaChannel && attribAlphaSize == 0 )
	{
		if ( log )
			os::Printer::log("No alpha.");
		rating += 10;
	}
	else if ( !Params.WithAlphaChannel && attribAlphaSize > 0 )
	{
		if ( log )
			os::Printer::log("Got alpha (unrequested).", ELL_DEBUG);
		rating += 100;
	}

	EGLint attribStencilSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_STENCIL_SIZE, &attribStencilSize);
	if ( Params.Stencilbuffer && attribStencilSize == 0 )
	{
		if ( log )
			os::Printer::log("No stencil buffer.");
		rating += 10;
	}
	else if ( !Params.Stencilbuffer && attribStencilSize > 0 )
	{
		if ( log )
			os::Printer::log("Got a stencil buffer (unrequested).", ELL_DEBUG);
		rating ++;
	}

	EGLint attribDepthSize = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_DEPTH_SIZE, &attribDepthSize);
	if ( attribDepthSize < Params.ZBufferBits )
	{
		if ( log )
		{
			if (attribDepthSize > 0)
				os::Printer::log("No full depth buffer.");
			else
				os::Printer::log("No depth buffer.");
		}
		rating += 50;
	}
	else if ( attribDepthSize != Params.ZBufferBits )
	{
		if ( log )
		{
			if ( Params.ZBufferBits == 0 )
				os::Printer::log("Got a depth buffer (unrequested).", ELL_DEBUG);
			else
				os::Printer::log("Got a larger depth buffer.", ELL_DEBUG);
		}
		rating ++;
	}

	EGLint attribSampleBuffers=0, attribSamples = 0;
	peglGetConfigAttrib( EglDisplay, config, EGL_SAMPLE_BUFFERS, &attribSampleBuffers);
	peglGetConfigAttrib( EglDisplay, config, EGL_SAMPLES, &attribSamples);
	if ( Params.AntiAlias && attribSampleBuffers == 0 )
	{
		if ( log )
			os::Printer::log("No multisampling.");
		rating += 20;
	}
	else if ( Params.AntiAlias && attribSampleBuffers && attribSamples < Params.AntiAlias )
	{
		if ( log )
			os::Printer::log("Multisampling with less samples than requested.", ELL_DEBUG);
		rating += 10;
	}
	else if ( Params.AntiAlias && attribSampleBuffers && attribSamples > Params.AntiAlias )
	{
		if ( log )
			os::Printer::log("Multisampling with more samples than requested.", ELL_DEBUG);
		rating += 5;
	}
	else if ( !Params.AntiAlias && attribSampleBuffers > 0 )
	{
		if ( log )
			os::Printer::log("Got multisampling (unrequested).", ELL_DEBUG);
		rating += 3;
	}

	return rating;
}

void CEGLManager::destroySurface()
{
	if (EglSurface == EGL_NO_SURFACE)
		return;

	// We should unbind current EGL context before destroy EGL surface.
	peglMakeCurrent(EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    peglDestroySurface(EglDisplay, EglSurface);
    EglSurface = EGL_NO_SURFACE;
}

bool CEGLManager::generateContext()
{
	if (EglDisplay == EGL_NO_DISPLAY || EglSurface == EGL_NO_SURFACE)
		return false;

	if (EglContext != EGL_NO_CONTEXT)
		return true;

	EGLint OpenGLESVersion = 0;

	switch (Params.DriverType)
	{
	case EDT_OPENGL:
	case EDT_OGLES1:
		OpenGLESVersion = 1;
		break;
	case EDT_OGLES2:
	case EDT_WEBGL1:
		OpenGLESVersion = 2;
		break;
	default:
		break;
	}

    EGLint ContextAttrib[] =
	{
#ifdef EGL_VERSION_1_3
		EGL_CONTEXT_CLIENT_VERSION, OpenGLESVersion,
#endif
		EGL_NONE, 0
	};

	EglContext = peglCreateContext(EglDisplay, EglConfig, EGL_NO_CONTEXT, ContextAttrib);

	if (testEGLError())
	{
		os::Printer::log("Could not create EGL context.", ELL_ERROR);
		return false;
	}

	os::Printer::log("EGL context created with OpenGLESVersion: ", core::stringc((int)OpenGLESVersion), ELL_DEBUG);

    return true;
}

void CEGLManager::destroyContext()
{
	if (EglContext == EGL_NO_CONTEXT)
		return;

	// We must unbind current EGL context before destroy it.
	peglMakeCurrent(EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	peglDestroyContext(EglDisplay, EglContext);

    EglContext = EGL_NO_CONTEXT;
}

bool CEGLManager::activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero)
{
	peglMakeCurrent(EglDisplay, EglSurface, EglSurface, EglContext);

	if (testEGLError())
	{
		os::Printer::log("Could not make EGL context current.");
		return false;
	}
	return true;
}

const SExposedVideoData& CEGLManager::getContext() const
{
	return Data;
}

bool CEGLManager::swapBuffers()
{
	auto result = peglSwapBuffers(EglDisplay, EglSurface) == EGL_TRUE;
#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
	if(WaylandDevice)
		WaylandDevice->checkPendingResizes();
#endif
	return result;
}

void CEGLManager::swapInterval(int interval)
{
	peglSwapInterval(EglDisplay, interval);
}

bool CEGLManager::testEGLError()
{
#if defined(EGL_VERSION_1_0) && defined(_DEBUG)
	EGLint status = peglGetError();

	switch (status)
	{
		case EGL_SUCCESS:
            return false;
		case EGL_NOT_INITIALIZED :
			os::Printer::log("Not Initialized", ELL_ERROR);
            break;
		case EGL_BAD_ACCESS:
			os::Printer::log("Bad Access", ELL_ERROR);
            break;
		case EGL_BAD_ALLOC:
			os::Printer::log("Bad Alloc", ELL_ERROR);
            break;
		case EGL_BAD_ATTRIBUTE:
			os::Printer::log("Bad Attribute", ELL_ERROR);
            break;
		case EGL_BAD_CONTEXT:
			os::Printer::log("Bad Context", ELL_ERROR);
            break;
		case EGL_BAD_CONFIG:
			os::Printer::log("Bad Config", ELL_ERROR);
            break;
		case EGL_BAD_CURRENT_SURFACE:
			os::Printer::log("Bad Current Surface", ELL_ERROR);
            break;
		case EGL_BAD_DISPLAY:
			os::Printer::log("Bad Display", ELL_ERROR);
            break;
		case EGL_BAD_SURFACE:
			os::Printer::log("Bad Surface", ELL_ERROR);
            break;
		case EGL_BAD_MATCH:
			os::Printer::log("Bad Match", ELL_ERROR);
            break;
		case EGL_BAD_PARAMETER:
			os::Printer::log("Bad Parameter", ELL_ERROR);
            break;
		case EGL_BAD_NATIVE_PIXMAP:
			os::Printer::log("Bad Native Pixmap", ELL_ERROR);
            break;
		case EGL_BAD_NATIVE_WINDOW:
			os::Printer::log("Bad Native Window", ELL_ERROR);
            break;
		case EGL_CONTEXT_LOST:
			os::Printer::log("Context Lost", ELL_ERROR);
            break;
        default:
            break;
	};

	return true;
#else
	return false;
#endif
}

#if defined(_IRR_DYNAMIC_OPENGL_ES_1_) || defined(_IRR_DYNAMIC_OPENGL_ES_2_) || defined(_IRR_DYNAMIC_OPENGL_)
static const fschar_t* GetGLLibName(E_DRIVER_TYPE driverType) {
	switch(driverType) {
	case EDT_OPENGL:
#ifdef _WIN32
		return _IRR_TEXT("Opengl32.dll");
#else
		return _IRR_TEXT("libGL.so.1");
#endif
	case EDT_OGLES1:
#ifdef _WIN32
		return _IRR_TEXT("libGLESv1_CM.dll");
#else
		return _IRR_TEXT("libGLESv1_CM.so.1");
#endif
	case EDT_OGLES2:
#ifdef _WIN32
		return _IRR_TEXT("libGLESv2.dll");
#else
		return _IRR_TEXT("libGLESv2.so.2");
#endif
	default:
		return nullptr;
	}
}
#endif

#if defined(_IRR_DYNAMIC_OPENGL_ES_1_) || defined(_IRR_DYNAMIC_OPENGL_ES_2_) || defined(_IRR_DYNAMIC_OPENGL_)
#ifdef _WIN32
#define LoadFunction(lib, name) GetProcAddress((HMODULE)lib, name)
static auto LoadEglLib() {
	auto ret = LoadLibrary(TEXT("libEGL.dll"));
	if(ret)
		return ret;
	return LoadLibrary(TEXT("atioglxx.dll"));
}
#else
#define LoadFunction(lib, name) dlsym(lib, name)
#define LoadEglLib() dlopen("libEGL.so.1", RTLD_LAZY)
#define LoadLibrary(name) dlopen(name, RTLD_LAZY)
#define FreeLibrary(lib) dlclose(lib)
#endif
#define EGL_FUNC(name, ret_type, ...) p##name = function_cast<ret_type(EGLAPIENTRY *)(__VA_ARGS__)>(LoadFunction(EGLLib, #name)); if(!p##name) break;

bool CEGLManager::LoadEGL() {
	if(LibEGL)
		return true;
	auto EGLLib = LoadEglLib();
	if(!EGLLib)
		return false;
	const auto* name = GetGLLibName(Params.DriverType);
	auto GLESLib = LoadLibrary(name);
	if(!GLESLib)
		os::Printer::log("Couldn't load", name, ELL_WARNING);
	do {
#include "CEGLFunctions.inl"
		LibEGL = EGLLib;
		LibGLES = GLESLib;
		return true;
	} while(0);
	FreeLibrary(EGLLib);
	FreeLibrary(GLESLib);
	os::Printer::log("Failed to load all required egl functions", ELL_ERROR);
	return false;
}
#undef EGL_FUNC
#else
bool CEGLManager::LoadEGL() {
	return true;
}
#endif

void* CEGLManager::loadFunction(const char* function_name) {
	void* ret = (void*)peglGetProcAddress(function_name);
#if defined(_IRR_DYNAMIC_OPENGL_ES_1_) || defined(_IRR_DYNAMIC_OPENGL_ES_2_) || defined(_IRR_DYNAMIC_OPENGL_)
	if(!ret && LibGLES)
		ret = (void*)LoadFunction(LibGLES, function_name);
	if(!ret && LibEGL)
		ret = (void*)LoadFunction(LibEGL, function_name);
#endif
	return ret;
}

void CEGLManager::GenerateConfig() {
#if defined(_IRR_EMSCRIPTEN_PLATFORM_) || (defined(__linux__) && !defined(__ANDROID__))
	// eglChooseConfig is currently only implemented as stub in emscripten (version 1.37.22 at point of writing)
	// But the other solution would also be fine as it also only generates a single context so there is not much to choose from.
	// We also need to manually choose the config under wayland devices, as some compositors might ignore the opacity hints, thus
	// we need to be 100% sure the selected configuration doesn't have an alpha channel if not wanted
	EglConfig = chooseConfig(ECS_IRR_CHOOSE);
#else
	EglConfig = chooseConfig(ECS_EGL_CHOOSE_FIRST_LOWER_EXPECTATIONS);
#endif
}

}
}

#endif
