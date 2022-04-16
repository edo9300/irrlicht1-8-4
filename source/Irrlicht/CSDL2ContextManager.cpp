// Copyright (C) 2013 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CSDL2ContextManager.h"

#ifdef _IRR_COMPILE_WITH_SDL2_DEVICE_

#include <SDL2/sdl.h>

namespace irr
{
namespace video
{

CSDL2ContextManager::CSDL2ContextManager() : Params(), ExposedData(), pglFrontFace(nullptr), window(nullptr), Context(nullptr)
{
	#ifdef _DEBUG
	setDebugName("CSDL2ContextManager");
	#endif
}

CSDL2ContextManager::~CSDL2ContextManager()
{
	SDL_GL_DeleteContext(Context);
	Context = nullptr;
}

bool CSDL2ContextManager::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& videodata)
{
	Params = params;
	window = static_cast<SDL_Window*>(videodata.OGLSDL2.Window);
	pglFrontFace = (void*)loadFunction("glFrontFace");

	if(!pglFrontFace)
		return false;

	return SDL_GL_LoadLibrary(nullptr) == 0;
}

void CSDL2ContextManager::terminate()
{
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
	Context = SDL_GL_CreateContext(window);
	return Context != nullptr;
}

const SExposedVideoData& CSDL2ContextManager::getContext() const
{
	return ExposedData;
}

#define GL_CW 0x0900
#ifdef _WIN32
#define APICALL _stdcall
#else
#define APICALL
#endif

typedef void(APICALL *PGLFRONTFACEPROC)(unsigned int mode);

bool CSDL2ContextManager::activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero)
{
	((PGLFRONTFACEPROC)pglFrontFace)(GL_CW);
	return true;
}

void CSDL2ContextManager::destroyContext()
{
}

bool CSDL2ContextManager::swapBuffers()
{
	SDL_GL_SwapWindow(window);
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
