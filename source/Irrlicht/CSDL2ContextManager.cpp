// Copyright (C) 2013 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CSDL2ContextManager.h"

#ifdef _IRR_COMPILE_WITH_SDL2_DEVICE_

#include <SDL2/sdl.h>
#include "os.h"

namespace irr
{
namespace video
{

CSDL2ContextManager::CSDL2ContextManager(const SExposedVideoData& videodata) : Params(), PrimaryContext(videodata), CurrentContext(), GLLibraryLoaded(false)
{
	#ifdef _DEBUG
	setDebugName("CSDL2ContextManager");
	#endif
	GLLibraryLoaded = SDL_GL_LoadLibrary(nullptr) == 0;
	CurrentContext.OGLSDL2.Window = GetWindow(PrimaryContext);
	CurrentContext.OGLSDL2.Context = nullptr;
}

CSDL2ContextManager::~CSDL2ContextManager()
{
	if(GetContext(PrimaryContext) != nullptr)
		SDL_GL_DeleteContext(GetContext(PrimaryContext));
	if(GLLibraryLoaded)
		SDL_GL_UnloadLibrary();
}

static void logAttributes() {
	core::stringc sdl_attr("SDL attribs:");
	int value = 0;
	if(SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value) == 0)
		sdl_attr += core::stringc(" r:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value) == 0)
		sdl_attr += core::stringc(" g:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value) == 0)
		sdl_attr += core::stringc(" b:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &value) == 0)
		sdl_attr += core::stringc(" a:") + core::stringc(value);

	if(SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value) == 0)
		sdl_attr += core::stringc(" depth:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value) == 0)
		sdl_attr += core::stringc(" stencil:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value) == 0)
		sdl_attr += core::stringc(" doublebuf:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &value) == 0)
		sdl_attr += core::stringc(" aa:") + core::stringc(value);
	if(SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &value) == 0)
		sdl_attr += core::stringc(" aa-samples:") + core::stringc(value);

	os::Printer::log(sdl_attr.c_str());
}

void CSDL2ContextManager::SetWindowOGLProperties(const SIrrlichtCreationParameters& CreationParams) {
	if(CreationParams.Bits == 16) {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 4);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 4);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, CreationParams.WithAlphaChannel ? 1 : 0);
	} else {
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, CreationParams.WithAlphaChannel ? 8 : 0);
	}
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, CreationParams.ZBufferBits);
	if(CreationParams.Doublebuffer)
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	if(CreationParams.Stereobuffer)
		SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
	if(CreationParams.AntiAlias > 1) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, CreationParams.AntiAlias);
	} else {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}
	switch(CreationParams.DriverType) {
	case irr::video::E_DRIVER_TYPE::EDT_OGLES1:
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		break;
	}
	case irr::video::E_DRIVER_TYPE::EDT_WEBGL1:
		logAttributes();
		//
	case irr::video::E_DRIVER_TYPE::EDT_OGLES2:
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		break;
	}
	default:
		break;
	}
}

bool CSDL2ContextManager::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& videodata)
{
	if(!GLLibraryLoaded)
		return false;
	Params = params;

	CurrentContext.OGLSDL2.Window = GetWindow(videodata);

	return true;
}

void CSDL2ContextManager::terminate()
{
	if(GetWindow(PrimaryContext) != nullptr && GetWindow(CurrentContext) == GetWindow(PrimaryContext))
		memset(&PrimaryContext, 0, sizeof(PrimaryContext));
	memset(&CurrentContext, 0, sizeof(CurrentContext));
}

bool CSDL2ContextManager::generateSurface()
{
	return true;
}

void CSDL2ContextManager::destroySurface()
{
}

bool CSDL2ContextManager::generateContext()
{
	auto Context = SDL_GL_CreateContext(GetWindow(CurrentContext));
	CurrentContext.OGLSDL2.Context = Context;
	if(PrimaryContext.OGLSDL2.Context == nullptr)
		PrimaryContext.OGLSDL2.Context = Context;
	return Context != nullptr;
}

const SExposedVideoData& CSDL2ContextManager::getContext() const
{
	return CurrentContext;
}

bool CSDL2ContextManager::activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero)
{
	if(GetWindow(videoData) != nullptr && GetContext(videoData) != nullptr) {
		if(SDL_GL_MakeCurrent(GetWindow(videoData), GetContext(videoData)) != 0) {
			os::Printer::log("Render Context switch failed.");
			return false;
		}
	}
	// set back to main context
	else if(GetWindow(videoData) == nullptr && GetContext(CurrentContext) != GetContext(PrimaryContext)) {
		if(SDL_GL_MakeCurrent(GetWindow(PrimaryContext), GetContext(PrimaryContext)) != 0) {
			os::Printer::log("Render Context switch failed.");
			return false;
		}
		CurrentContext = PrimaryContext;
	}
	return true;
}

void CSDL2ContextManager::destroyContext()
{
	if(GetContext(CurrentContext) != nullptr) {
		if(SDL_GL_MakeCurrent(GetWindow(CurrentContext), nullptr) != 0)
			os::Printer::log("Release of render context failed.", ELL_WARNING);

		SDL_GL_DeleteContext(GetContext(CurrentContext));

		if(GetContext(CurrentContext) == GetContext(PrimaryContext))
			PrimaryContext.OGLSDL2.Context = nullptr;
		CurrentContext.OGLSDL2.Context = nullptr;
	}
}

bool CSDL2ContextManager::swapBuffers()
{
	SDL_GL_SwapWindow(GetWindow(CurrentContext));
	return true;
}

void CSDL2ContextManager::swapInterval(int interval)
{
	SDL_GL_SetSwapInterval(interval);
}

void* CSDL2ContextManager::loadFunction(const char* function_name)
{
	return SDL_GL_GetProcAddress(function_name);
}

}
}

#endif
