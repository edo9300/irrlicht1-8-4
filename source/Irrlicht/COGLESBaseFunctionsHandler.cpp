// Copyright (C) 2021 Edoardo Lolletti
// This file is part of the "Irrlicht Engine".
// Licensed under agpl3

#include "COGLESBaseFunctionsHandler.h"

#ifdef _IRR_COMPILE_WITH_OGLES1_

#ifdef _IRR_DYNAMIC_OPENGL_ES_1_

#include "IcontextManager.h"

namespace irr
{
namespace video
{

COGLES1BaseFunctionsHandler::COGLES1BaseFunctionsHandler(IContextManager* contextManager) :
#define GL_FUNC(name, ret_type, ...) p##name(0), 
#include "COGLESBaseFunctions.inl"
#undef GL_FUNC
	ContextManager(contextManager)
{
}

#define GL_FUNC(name, ret_type, ...) p##name = (ret_type(GL_APIENTRY*)(__VA_ARGS__))ContextManager->loadFunction(#name); if(!p##name) return false;
bool COGLES1BaseFunctionsHandler::initBaseFunctions() {
#include "COGLESBaseFunctions.inl"
	return true;
}
#undef GL_FUNC


}
}

#endif

#endif
