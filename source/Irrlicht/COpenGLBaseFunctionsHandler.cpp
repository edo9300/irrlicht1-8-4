// Copyright (C) 2021 Edoardo Lolletti
// This file is part of the "Irrlicht Engine".
// Licensed under agpl3

#include "COpenGLBaseFunctionsHandler.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#ifdef _IRR_DYNAMIC_OPENGL_

#include "IContextManager.h"

namespace irr
{
namespace video
{

COpenGLBaseFunctionsHandler::COpenGLBaseFunctionsHandler(IContextManager* contextManager) :
#define GL_FUNC(name, ret_type, ...) p##name(0), 
#include "COpenGLBaseFunctions.inl"
#undef GL_FUNC
	ContextManager(contextManager)
{
}

#define GL_FUNC(name, ret_type, ...) p##name = (ret_type(GL_APIENTRY*)(__VA_ARGS__))ContextManager->loadFunction(#name);\
									if(!p##name) {\
										os::Printer::log("Failed to load base OpenGL function: "#name".", ELL_ERROR);\
										return false;\
									}
bool COpenGLBaseFunctionsHandler::initBaseFunctions() {
#include "COpenGLBaseFunctions.inl"
	return true;
}
#undef GL_FUNC


}
}

#endif

#endif
