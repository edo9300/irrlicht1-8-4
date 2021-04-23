// Copyright (C) 2021 Edoardo Lolletti
// This file is part of the "Irrlicht Engine".
// Licensed under agpl3

#ifndef __C_OPEN_GL_BASE_FUNCTIONS_HANDLER_H_INCLUDED__
#define __C_OPEN_GL_BASE_FUNCTIONS_HANDLER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES1_

#include "EDriverFeatures.h"
#include "irrTypes.h"
#include "IContextManager.h"
#include "os.h"

#include "COGLESCommon.h"
#include "GLES/glplatform.h"

namespace irr {
namespace video {

#ifdef _IRR_DYNAMIC_OPENGL_ES_1_

class COGLES1BaseFunctionsHandler {
public:
	// constructor
	COGLES1BaseFunctionsHandler(IContextManager* contextManager);

	// deferred initialization
	bool initBaseFunctions();

#define GL_FUNC(name, ret_type, ...) ret_type(GL_APIENTRY * p##name)(__VA_ARGS__);
#include "COGLESBaseFunctions.inl"
#undef GL_FUNC

private:
	IContextManager* ContextManager;
};

#else

struct COGLES1BaseFunctionsHandler {
public:
	// constructor
	COGLES1BaseFunctionsHandler(IContextManager* contextManager) { (void)contextManager; };

	// deferred initialization
	bool initBaseFunctions() { return true; };

#define GL_FUNC(name, ret_type, ...) ret_type(GL_APIENTRY * p##name)(__VA_ARGS__) = &name;
#include "COGLESBaseFunctions.inl"
#undef GL_FUNC
};

#endif


}

}

#endif

#endif

