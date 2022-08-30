// Copyright (C) 2021 Edoardo Lolletti
// This file is part of the "Irrlicht Engine".
// Licensed under agpl3

#ifndef __C_OPEN_GL_BASE_FUNCTIONS_HANDLER_H_INCLUDED__
#define __C_OPEN_GL_BASE_FUNCTIONS_HANDLER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "EDriverFeatures.h"
#include "irrTypes.h"
#include "IContextManager.h"
#include "os.h"

#include "COpenGLCommon.h"

#include "COpenGLCoreFeature.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define GL_APIENTRY APIENTRY
#else
#define GL_APIENTRY
#endif

namespace irr {
namespace video {

#ifdef _IRR_DYNAMIC_OPENGL_

class COpenGLBaseFunctionsHandler {
public:
	// constructor
	COpenGLBaseFunctionsHandler(IContextManager* contextManager);

	// deferred initialization
	bool initBaseFunctions();

#define GL_FUNC(name, ret_type, ...) ret_type(GL_APIENTRY * p##name)(__VA_ARGS__);
#include "COpenGLBaseFunctions.inl"
#undef GL_FUNC

private:
	IContextManager* ContextManager;
};

#else

struct COpenGLBaseFunctionsHandler {
public:
	// constructor
	COpenGLBaseFunctionsHandler(IContextManager* contextManager) { (void)contextManager; };

	// deferred initialization
	bool initBaseFunctions() { return true; };

#define GL_FUNC(name, ret_type, ...) ret_type(GL_APIENTRY * p##name)(__VA_ARGS__) = &name;
#include "COpenGLBaseFunctions.inl"
#undef GL_FUNC
};

#endif


}

}

#endif

#endif

