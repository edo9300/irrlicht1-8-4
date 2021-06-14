// Copyright (C) 2013 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CGLXManager.h"

#ifdef _IRR_COMPILE_WITH_GLX_MANAGER_

#include "os.h"

#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
	#define GL_GLEXT_LEGACY 1
	#define GLX_GLXEXT_LEGACY 1
#else
	#define GL_GLEXT_PROTOTYPES 1
	#define GLX_GLXEXT_PROTOTYPES 1
#endif
#include <GL/gl.h>
#include <GL/glx.h>
#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
#include "glext.h"
#undef GLX_ARB_get_proc_address // avoid problems with local glxext.h
#include "glxext.h"
#endif

#include "LibX11Loader.h"
#ifdef _IRR_X11_DYNAMIC_LOAD_
#include <dlfcn.h>
#define CALL_GLX_FUNC(name,...) ((decltype(&name))(p##name))(__VA_ARGS__)
#else
#define CALL_GLX_FUNC(name,...) name(__VA_ARGS__)
#endif

namespace irr
{
namespace video
{

CGLXManager::CGLXManager(const SIrrlichtCreationParameters& params, const SExposedVideoData& videodata, int screennr)
	: Params(params), PrimaryContext(videodata), VisualInfo(0), glxFBConfig(0), GlxWin(0)
#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
	,pGlxSwapIntervalSGI(0)
	,pGlxSwapIntervalEXT(0)
	,pGlxSwapIntervalMESA(0)
#endif // _IRR_OPENGL_USE_EXTPOINTER_
	,pglXGetProcAddress(0)
#ifdef _IRR_X11_DYNAMIC_LOAD_
	,LibGLX(0)
	,LibGL(0)
#endif
{
	#ifdef _DEBUG
	setDebugName("CGLXManager");
	#endif
	
#ifdef _IRR_X11_DYNAMIC_LOAD_
	LibGL = dlopen("libGL.so", RTLD_LAZY);
	if(!LibGL)
		LibGL = dlopen("libGL.so.1", RTLD_LAZY);
	if(!LibGL)
		os::Printer::log("Failed to load LibGL.so", ELL_ERROR);
	LibGLX = dlopen("libGLX.so", RTLD_LAZY);
	if(!LibGLX)
		LibGLX = dlopen("libGLX.so.0", RTLD_LAZY);
	if(!LibGLX) {
		os::Printer::log("Failed to load libGLX.so", ELL_ERROR);
		return;
	}
	#define GLX_FUNC(name) p##name = (void*)dlsym(LibGLX, #name); if(!p##name) { dlclose(LibGLX); LibGLX=0; return; }
		#include "CGLXFunctions.inl"
	#undef GLX_FUNC
#endif

	CurrentContext.OpenGLLinux.X11Display=PrimaryContext.OpenGLLinux.X11Display;

	int major, minor;
	Display* display = (Display*)PrimaryContext.OpenGLLinux.X11Display;
	const bool isAvailableGLX=CALL_GLX_FUNC(glXQueryExtension,display,&major,&minor);

	if (isAvailableGLX && CALL_GLX_FUNC(glXQueryVersion,display, &major, &minor))
	{
#if defined(GLX_VERSION_1_3)
		typedef GLXFBConfig * ( * PFNGLXCHOOSEFBCONFIGPROC) (Display *dpy, int screen, const int *attrib_list, int *nelements);

#ifdef _IRR_OPENGL_USE_EXTPOINTER_
		PFNGLXCHOOSEFBCONFIGPROC glxChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)loadFunction("glXChooseFBConfig");
#else
		PFNGLXCHOOSEFBCONFIGPROC glxChooseFBConfig=glXChooseFBConfig;
#endif
		if (major==1 && minor>2 && glxChooseFBConfig)
		{
os::Printer::log("GLX >= 1.3", ELL_DEBUG);
			// attribute array for the draw buffer
			int visualAttrBuffer[] =
			{
				GLX_RENDER_TYPE, GLX_RGBA_BIT,
				GLX_RED_SIZE, 4,
				GLX_GREEN_SIZE, 4,
				GLX_BLUE_SIZE, 4,
				GLX_ALPHA_SIZE, Params.WithAlphaChannel?1:0,
				GLX_DEPTH_SIZE, Params.ZBufferBits, //10,11
				GLX_DOUBLEBUFFER, Params.Doublebuffer?True:False,
				GLX_STENCIL_SIZE, Params.Stencilbuffer?1:0,
#if defined(GLX_VERSION_1_4) && defined(GLX_SAMPLE_BUFFERS) // we need to check the extension string!
				GLX_SAMPLE_BUFFERS, 1,
				GLX_SAMPLES, Params.AntiAlias, // 18,19
#elif defined(GLX_ARB_multisample)
				GLX_SAMPLE_BUFFERS_ARB, 1,
				GLX_SAMPLES_ARB, Params.AntiAlias, // 18,19
#elif defined(GLX_SGIS_multisample)
				GLX_SAMPLE_BUFFERS_SGIS, 1,
				GLX_SAMPLES_SGIS, Params.AntiAlias, // 18,19
#endif
//#ifdef GL_ARB_framebuffer_sRGB
//					GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, Params.HandleSRGB,
//#elif defined(GL_EXT_framebuffer_sRGB)
//					GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, Params.HandleSRGB,
//#endif
				GLX_STEREO, Params.Stereobuffer?True:False,
				None
			};

			GLXFBConfig *configList=0;
			int nitems=0;
			if (Params.AntiAlias<2)
			{
				visualAttrBuffer[17] = 0;
				visualAttrBuffer[19] = 0;
			}
			// first round with unchanged values
			{
				configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
				if (!configList && Params.AntiAlias)
				{
					while (!configList && (visualAttrBuffer[19]>1))
					{
						visualAttrBuffer[19] -= 1;
						configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
					}
					if (!configList)
					{
						visualAttrBuffer[17] = 0;
						visualAttrBuffer[19] = 0;
						configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
						if (configList)
						{
							os::Printer::log("No FSAA available.", ELL_WARNING);
							Params.AntiAlias=0;
						}
						else
						{
							//reenable multisampling
							visualAttrBuffer[17] = 1;
							visualAttrBuffer[19] = Params.AntiAlias;
						}
					}
				}
			}
			// Next try with flipped stencil buffer value
			// If the first round was with stencil flag it's now without
			// Other way round also makes sense because some configs
			// only have depth buffer combined with stencil buffer
			if (!configList)
			{
				if (Params.Stencilbuffer)
					os::Printer::log("No stencilbuffer available, disabling stencil shadows.", ELL_WARNING);
				Params.Stencilbuffer = !Params.Stencilbuffer;
				visualAttrBuffer[15]=Params.Stencilbuffer?1:0;

				configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
				if (!configList && Params.AntiAlias)
				{
					while (!configList && (visualAttrBuffer[19]>1))
					{
						visualAttrBuffer[19] -= 1;
						configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
					}
					if (!configList)
					{
						visualAttrBuffer[17] = 0;
						visualAttrBuffer[19] = 0;
						configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
						if (configList)
						{
							os::Printer::log("No FSAA available.", ELL_WARNING);
							Params.AntiAlias=0;
						}
						else
						{
							//reenable multisampling
							visualAttrBuffer[17] = 1;
							visualAttrBuffer[19] = Params.AntiAlias;
						}
					}
				}
			}
			// Next try without double buffer
			if (!configList && Params.Doublebuffer)
			{
				os::Printer::log("No doublebuffering available.", ELL_WARNING);
				Params.Doublebuffer=false;
				visualAttrBuffer[13] = GLX_DONT_CARE;
				Params.Stencilbuffer = false;
				visualAttrBuffer[15]=0;
				configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
				if (!configList && Params.AntiAlias)
				{
					while (!configList && (visualAttrBuffer[19]>1))
					{
						visualAttrBuffer[19] -= 1;
						configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
					}
					if (!configList)
					{
						visualAttrBuffer[17] = 0;
						visualAttrBuffer[19] = 0;
						configList=glxChooseFBConfig(display, screennr, visualAttrBuffer,&nitems);
						if (configList)
						{
							os::Printer::log("No FSAA available.", ELL_WARNING);
							Params.AntiAlias=0;
						}
						else
						{
							//reenable multisampling
							visualAttrBuffer[17] = 1;
							visualAttrBuffer[19] = Params.AntiAlias;
						}
					}
				}
			}
			if (configList)
			{
				glxFBConfig=configList[0];
				X11Loader::XFree(configList);
#ifdef _IRR_OPENGL_USE_EXTPOINTER_
				typedef XVisualInfo * ( * PFNGLXGETVISUALFROMFBCONFIGPROC) (Display *dpy, GLXFBConfig config);
				PFNGLXGETVISUALFROMFBCONFIGPROC glxGetVisualFromFBConfig= (PFNGLXGETVISUALFROMFBCONFIGPROC)loadFunction("glXGetVisualFromFBConfig");
				if (glxGetVisualFromFBConfig)
					VisualInfo = glxGetVisualFromFBConfig(display,(GLXFBConfig)glxFBConfig);
#else
					VisualInfo = glXGetVisualFromFBConfig(display,(GLXFBConfig)glxFBConfig);
#endif
			}
		}
		else
#endif
		{
			// attribute array for the draw buffer
			int visualAttrBuffer[] =
			{
				GLX_RGBA, GLX_USE_GL,
				GLX_RED_SIZE, 4,
				GLX_GREEN_SIZE, 4,
				GLX_BLUE_SIZE, 4,
				GLX_ALPHA_SIZE, Params.WithAlphaChannel?1:0,
				GLX_DEPTH_SIZE, Params.ZBufferBits,
				GLX_STENCIL_SIZE, Params.Stencilbuffer?1:0, // 12,13
				// The following attributes have no flags, but are
				// either present or not. As a no-op we use
				// GLX_USE_GL, which is silently ignored by glXChooseVisual
				Params.Doublebuffer?GLX_DOUBLEBUFFER:GLX_USE_GL, // 14
				Params.Stereobuffer?GLX_STEREO:GLX_USE_GL, // 15
//#ifdef GL_ARB_framebuffer_sRGB
//					Params.HandleSRGB?GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB:GLX_USE_GL,
//#elif defined(GL_EXT_framebuffer_sRGB)
//					Params.HandleSRGB?GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT:GLX_USE_GL,
//#endif
				None
			};

			VisualInfo=CALL_GLX_FUNC(glXChooseVisual,display, screennr, visualAttrBuffer);
			if (!VisualInfo)
			{
				if (Params.Stencilbuffer)
					os::Printer::log("No stencilbuffer available, disabling.", ELL_WARNING);
				Params.Stencilbuffer = !Params.Stencilbuffer;
				visualAttrBuffer[13]=Params.Stencilbuffer?1:0;

				VisualInfo=CALL_GLX_FUNC(glXChooseVisual,display, screennr, visualAttrBuffer);
				if (!VisualInfo && Params.Doublebuffer)
				{
					os::Printer::log("No doublebuffering available.", ELL_WARNING);
					Params.Doublebuffer=false;
					visualAttrBuffer[14] = GLX_USE_GL;
					VisualInfo=CALL_GLX_FUNC(glXChooseVisual,display, screennr, visualAttrBuffer);
				}
			}
		}
		// Accessing the correct function is quite complex
		// All libraries should support the ARB version, however
		// since GLX 1.4 the non-ARB version is the official one
		// So we have to check the runtime environment and
		// choose the proper symbol
		// In case you still have problems please enable the
		// next line by uncommenting it
		// #define _IRR_GETPROCADDRESS_WORKAROUND_
#ifdef _IRR_X11_DYNAMIC_LOAD_
#define LOAD(name) dlsym(LibGLX, #name)
#else
#define LOAD(name) name
#endif
#if !defined(_IRR_GETPROCADDRESS_WORKAROUND_) && defined(GLX_VERSION_1_4)
		if((major > 1) || (minor > 3))
			pglXGetProcAddress = (void*)LOAD(glXGetProcAddress);
		else
#endif // workaround
			pglXGetProcAddress = (void*)LOAD(glXGetProcAddressARB);

#ifdef _IRR_OPENGL_USE_EXTPOINTER_
		#if defined(GLX_SGI_swap_control)
			pGlxSwapIntervalSGI = loadFunction("glXSwapIntervalSGI");
		#endif
		#if defined(GLX_EXT_swap_control)
			pGlxSwapIntervalEXT = loadFunction("glXSwapIntervalEXT");
		#endif
		#if defined(GLX_MESA_swap_control)
			pGlxSwapIntervalMESA = loadFunction("glXSwapIntervalMESA");
		#endif
		#undef IRR_OGL_LOAD_EXTENSION
#endif
	}
	else
		os::Printer::log("No GLX support available. OpenGL driver will not work.", ELL_WARNING);
}

CGLXManager::~CGLXManager()
{
#ifdef _IRR_X11_DYNAMIC_LOAD_
	if(LibGLX)
		dlclose(LibGLX);
#endif
}

bool CGLXManager::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& videodata)
{
#ifdef _IRR_X11_DYNAMIC_LOAD_
	if(LibGLX == 0)
		return false;
#endif
	// store params
	Params=params;

	// set display
	CurrentContext.OpenGLLinux.X11Display=videodata.OpenGLLinux.X11Display;

	// now get new window
	CurrentContext.OpenGLLinux.X11Window=videodata.OpenGLLinux.X11Window;
	if (!PrimaryContext.OpenGLLinux.X11Window)
	{
		PrimaryContext.OpenGLLinux.X11Window=CurrentContext.OpenGLLinux.X11Window;
	}

	return true;
}

void CGLXManager::terminate()
{
	memset(&CurrentContext, 0, sizeof(CurrentContext));
}

bool CGLXManager::generateSurface()
{
	if (glxFBConfig)
	{
		GlxWin=CALL_GLX_FUNC(glXCreateWindow,(Display*)CurrentContext.OpenGLLinux.X11Display,(GLXFBConfig)glxFBConfig,CurrentContext.OpenGLLinux.X11Window,NULL);
		if (!GlxWin)
		{
			os::Printer::log("Could not create GLX window.", ELL_WARNING);
			return false;
		}

		CurrentContext.OpenGLLinux.X11Window=GlxWin;
	}
	return true;
}

void CGLXManager::destroySurface()
{
	if (GlxWin)
		CALL_GLX_FUNC(glXDestroyWindow,(Display*)CurrentContext.OpenGLLinux.X11Display, GlxWin);
}

bool CGLXManager::generateContext()
{
	GLXContext context;

	if (glxFBConfig)
	{
		if (GlxWin)
		{
			// create glx context
			context = CALL_GLX_FUNC(glXCreateNewContext,(Display*)CurrentContext.OpenGLLinux.X11Display, (GLXFBConfig)glxFBConfig, GLX_RGBA_TYPE, NULL, True);
			if (!context)
			{
				os::Printer::log("Could not create GLX rendering context.", ELL_WARNING);
				return false;
			}
		}
		else
		{
			os::Printer::log("GLX window was not properly created.", ELL_WARNING);
			return false;
		}
	}
	else
	{
		context = CALL_GLX_FUNC(glXCreateContext,(Display*)CurrentContext.OpenGLLinux.X11Display, VisualInfo, NULL, True);
		if (!context)
		{
			os::Printer::log("Could not create GLX rendering context.", ELL_WARNING);
			return false;
		}
	}
	CurrentContext.OpenGLLinux.X11Context=context;
	return true;
}

const SExposedVideoData& CGLXManager::getContext() const
{
	return CurrentContext;
}

bool CGLXManager::activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero)
{
	//TODO: handle restorePrimaryOnZero

	if (videoData.OpenGLLinux.X11Window)
	{
		if (videoData.OpenGLLinux.X11Display && videoData.OpenGLLinux.X11Context)
		{
			if (!CALL_GLX_FUNC(glXMakeCurrent,(Display*)videoData.OpenGLLinux.X11Display, videoData.OpenGLLinux.X11Window, (GLXContext)videoData.OpenGLLinux.X11Context))
			{
				os::Printer::log("Context activation failed.");
				return false;
			}
			else
			{
				CurrentContext.OpenGLLinux.X11Window = videoData.OpenGLLinux.X11Window;
				CurrentContext.OpenGLLinux.X11Display = videoData.OpenGLLinux.X11Display;
			}
		}
		else
		{
			// in case we only got a window ID, try with the existing values for display and context
			if (!CALL_GLX_FUNC(glXMakeCurrent,(Display*)PrimaryContext.OpenGLLinux.X11Display, videoData.OpenGLLinux.X11Window, (GLXContext)PrimaryContext.OpenGLLinux.X11Context))
			{
				os::Printer::log("Context activation failed.");
				return false;
			}
			else
			{
				CurrentContext.OpenGLLinux.X11Window = videoData.OpenGLLinux.X11Window;
				CurrentContext.OpenGLLinux.X11Display = PrimaryContext.OpenGLLinux.X11Display;
			}
		}
	}
	else if (!restorePrimaryOnZero && !videoData.OpenGLLinux.X11Window && !videoData.OpenGLLinux.X11Display)
	{
		if (!CALL_GLX_FUNC(glXMakeCurrent,(Display*)PrimaryContext.OpenGLLinux.X11Display, None, NULL))
		{
			os::Printer::log("Render Context reset failed.");
			return false;
		}
		CurrentContext.OpenGLLinux.X11Window = 0;
		CurrentContext.OpenGLLinux.X11Display = 0;
	}
	// set back to main context
	else if (CurrentContext.OpenGLLinux.X11Display != PrimaryContext.OpenGLLinux.X11Display)
	{
		if (!CALL_GLX_FUNC(glXMakeCurrent,(Display*)PrimaryContext.OpenGLLinux.X11Display, PrimaryContext.OpenGLLinux.X11Window, (GLXContext)PrimaryContext.OpenGLLinux.X11Context))
		{
			os::Printer::log("Context activation failed.");
			return false;
		}
		else
		{
			CurrentContext = PrimaryContext;
		}
	}
	return true;
}

void CGLXManager::destroyContext()
{
	if (CurrentContext.OpenGLLinux.X11Context)
	{
		if (GlxWin)
		{
			if (!CALL_GLX_FUNC(glXMakeContextCurrent,(Display*)CurrentContext.OpenGLLinux.X11Display, None, None, NULL))
				os::Printer::log("Could not release glx context.", ELL_WARNING);
		}
		else
		{
			if (!CALL_GLX_FUNC(glXMakeCurrent,(Display*)CurrentContext.OpenGLLinux.X11Display, None, NULL))
				os::Printer::log("Could not release glx context.", ELL_WARNING);
		}
		CALL_GLX_FUNC(glXDestroyContext,(Display*)CurrentContext.OpenGLLinux.X11Display, (GLXContext)CurrentContext.OpenGLLinux.X11Context);
	}
}

bool CGLXManager::swapBuffers()
{
	CALL_GLX_FUNC(glXSwapBuffers,(Display*)CurrentContext.OpenGLLinux.X11Display, CurrentContext.OpenGLLinux.X11Window);
	return true;
}

void CGLXManager::swapInterval(int interval)
{
#ifdef GLX_SGI_swap_control
#ifdef _IRR_OPENGL_USE_EXTPOINTER_
	if (pGlxSwapIntervalSGI)
		((PFNGLXSWAPINTERVALSGIPROC)pGlxSwapIntervalSGI)(interval);
#else
	glXSwapIntervalSGI(interval);
#endif
#elif defined(GLX_EXT_swap_control)
	Display *dpy = glXGetCurrentDisplay();
	GLXDrawable drawable = glXGetCurrentDrawable();

#ifdef _IRR_OPENGL_USE_EXTPOINTER_
	if (pGlxSwapIntervalEXT)
		((PFNGLXSWAPINTERVALEXTPROC)pGlxSwapIntervalEXT)(dpy, drawable, interval);
#else
	pGlXSwapIntervalEXT(dpy, drawable, interval);
#endif
#elif defined(GLX_MESA_swap_control)
#ifdef _IRR_OPENGL_USE_EXTPOINTER_
	if (pGlxSwapIntervalMESA)
		((PFNGLXSWAPINTERVALMESAPROC)pGlxSwapIntervalMESA)(interval);
#else
	pGlXSwapIntervalMESA(interval);
#endif
#endif
}

void* CGLXManager::loadFunction(const char* function_name)
{
	void* ret = nullptr;
	if(pglXGetProcAddress)
		ret = (void*)(reinterpret_cast<__GLXextFuncPtr(*)(const GLubyte*)>(pglXGetProcAddress))((const GLubyte*)function_name);
	if(!ret && LibGLX)
		ret = dlsym(LibGLX, function_name);
	if(!ret && LibGL)
		ret = dlsym(LibGL, function_name);
	return ret;
}

}
}

#endif

