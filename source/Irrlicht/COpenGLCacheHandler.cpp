// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "COpenGLCacheHandler.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "COpenGLDriver.h"

namespace irr
{
namespace video
{

/* COpenGLCacheHandler */

COpenGLCacheHandler::COpenGLCacheHandler(COpenGLDriver* driver) :
	COpenGLCoreCacheHandler<COpenGLDriver, COpenGLTexture>(driver), AlphaMode(GL_ALWAYS), AlphaRef(0.f), AlphaTest(false),
	MatrixMode(GL_MODELVIEW), ClientActiveTexture(GL_TEXTURE0), ClientStateVertex(false),
	ClientStateNormal(false), ClientStateColor(false), ClientStateTexCoord0(false)
{
	// Initial OpenGL values from specification.

	Driver->pglAlphaFunc(AlphaMode, AlphaRef);
	Driver->pglDisable(GL_ALPHA_TEST);

	Driver->pglMatrixMode(MatrixMode);

	Driver->irrGlClientActiveTexture(ClientActiveTexture);

	Driver->pglDisableClientState(GL_VERTEX_ARRAY);
	Driver->pglDisableClientState(GL_NORMAL_ARRAY);
	Driver->pglDisableClientState(GL_COLOR_ARRAY);
	Driver->pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

COpenGLCacheHandler::~COpenGLCacheHandler()
{
}

void COpenGLCacheHandler::setAlphaFunc(GLenum mode, GLclampf ref)
{
	if (AlphaMode != mode || AlphaRef != ref)
	{
		Driver->pglAlphaFunc(mode, ref);

		AlphaMode = mode;
		AlphaRef = ref;
	}
}

void COpenGLCacheHandler::setAlphaTest(bool enable)
{
	if (AlphaTest != enable)
	{
		if (enable)
			Driver->pglEnable(GL_ALPHA_TEST);
		else
			Driver->pglDisable(GL_ALPHA_TEST);
		AlphaTest = enable;
	}
}

void COpenGLCacheHandler::setClientState(bool vertex, bool normal, bool color, bool texCoord0)
{
	if (ClientStateVertex != vertex)
	{
		if (vertex)
			Driver->pglEnableClientState(GL_VERTEX_ARRAY);
		else
			Driver->pglDisableClientState(GL_VERTEX_ARRAY);

		ClientStateVertex = vertex;
	}

	if (ClientStateNormal != normal)
	{
		if (normal)
			Driver->pglEnableClientState(GL_NORMAL_ARRAY);
		else
			Driver->pglDisableClientState(GL_NORMAL_ARRAY);

		ClientStateNormal = normal;
	}

	if (ClientStateColor != color)
	{
		if (color)
			Driver->pglEnableClientState(GL_COLOR_ARRAY);
		else
			Driver->pglDisableClientState(GL_COLOR_ARRAY);

		ClientStateColor = color;
	}

	if (ClientStateTexCoord0 != texCoord0)
	{
		setClientActiveTexture(GL_TEXTURE0_ARB);

		if (texCoord0)
			Driver->pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
		else
			Driver->pglDisableClientState(GL_TEXTURE_COORD_ARRAY);

		ClientStateTexCoord0 = texCoord0;
	}
}

void COpenGLCacheHandler::setMatrixMode(GLenum mode)
{
	if (MatrixMode != mode)
	{
		Driver->pglMatrixMode(mode);
		MatrixMode = mode;
	}
}

void COpenGLCacheHandler::setClientActiveTexture(GLenum texture)
{
	if (ClientActiveTexture != texture)
	{
		Driver->irrGlClientActiveTexture(texture);
		ClientActiveTexture = texture;
	}
}

} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_OPENGL_
