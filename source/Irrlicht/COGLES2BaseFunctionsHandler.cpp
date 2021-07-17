// Copyright (C) 2021 Edoardo Lolletti
// This file is part of the "Irrlicht Engine".
// Licensed under agpl3

#include "COGLES2BaseFunctionsHandler.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#ifdef _IRR_DYNAMIC_OPENGL_ES_2_

#include "IContextManager.h"

namespace irr
{
namespace video
{

COGLES2BaseFunctionsHandler::COGLES2BaseFunctionsHandler(IContextManager* contextManager) :
#define GL_FUNC(name, ret_type, ...) p##name(0), 
#include "COGLES2BaseFunctions.inl"
#undef GL_FUNC
	ContextManager(contextManager)
{
}

#define GL_FUNC(name, ret_type, ...) p##name = (ret_type(GL_APIENTRY*)(__VA_ARGS__))ContextManager->loadFunction(#name); if(!p##name) return false;
bool COGLES2BaseFunctionsHandler::initBaseFunctions() {
#include "COGLES2BaseFunctions.inl"
	return true;
}
#undef GL_FUNC


}
}

#endif

#endif
