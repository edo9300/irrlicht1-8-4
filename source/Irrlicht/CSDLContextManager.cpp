// Copyright (C) 2013 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CSDLContextManager.h"

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_

#include <SDL/SDL.h>

namespace irr
{
namespace video
{

CSDLContextManager::CSDLContextManager() : Params(), ExposedData(), pglFrontFace(nullptr)
{
	#ifdef _DEBUG
	setDebugName("CSDLContextManager");
	#endif
}

CSDLContextManager::~CSDLContextManager()
{
}

bool CSDLContextManager::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& videodata)
{
	Params = params;
	pglFrontFace = (void*)loadFunction("glFrontFace");
	return pglFrontFace;
}

void CSDLContextManager::terminate()
{
}

bool CSDLContextManager::generateSurface()
{
	return true;
}

void CSDLContextManager::destroySurface()
{
}

bool CSDLContextManager::generateContext()
{
	return true;
}

const SExposedVideoData& CSDLContextManager::getContext() const
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

bool CSDLContextManager::activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero)
{
	((PGLFRONTFACEPROC)pglFrontFace)(GL_CW);
	return true;
}

void CSDLContextManager::destroyContext()
{
}

bool CSDLContextManager::swapBuffers()
{
	SDL_GL_SwapBuffers();
	return true;
}

void CSDLContextManager::swapInterval(int interval)
{

}

void* CSDLContextManager::loadFunction(const char* function_name)
{
	return SDL_GL_GetProcAddress(function_name);
}

}
}

#endif
