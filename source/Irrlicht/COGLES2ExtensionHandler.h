// Copyright (C) 2015 Patryk Nadrowski
// Copyright (C) 2009-2010 Amundis
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OGLES2_EXTENSION_HANDLER_H_INCLUDED__
#define __C_OGLES2_EXTENSION_HANDLER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "EDriverFeatures.h"
#include "irrTypes.h"
#include "IContextManager.h"
#include "os.h"

#include "COGLES2Common.h"
#include "COGLES2BaseFunctionsHandler.h"

#include "COGLESCoreExtensionHandler.h"

namespace irr
{
namespace video
{

	class COGLES2ExtensionHandler : public COGLESCoreExtensionHandler<COGLES2BaseFunctionsHandler>, public COGLES2BaseFunctionsHandler
	{
	public:
		COGLES2ExtensionHandler(IContextManager* contextManager) : COGLESCoreExtensionHandler(this), COGLES2BaseFunctionsHandler(contextManager){}

		void initExtensions();

		bool queryFeature(video::E_VIDEO_DRIVER_FEATURE feature) const
		{
			switch (feature)
			{
			case EVDF_RENDER_TO_TARGET:
			case EVDF_HARDWARE_TL:
			case EVDF_MULTITEXTURE:
			case EVDF_BILINEAR_FILTER:
			case EVDF_MIP_MAP:
			case EVDF_MIP_MAP_AUTO_UPDATE:
			case EVDF_VERTEX_SHADER_1_1:
			case EVDF_PIXEL_SHADER_1_1:
			case EVDF_PIXEL_SHADER_1_2:
			case EVDF_PIXEL_SHADER_2_0:
			case EVDF_VERTEX_SHADER_2_0:
			case EVDF_ARB_GLSL:
			case EVDF_TEXTURE_NSQUARE:
#ifndef _IRR_COMPILE_WITH_IOS_DEVICE_ //always treat ios devices as not supporting power of 2 textures
			case EVDF_TEXTURE_NPOT:
#endif
			case EVDF_FRAMEBUFFER_OBJECT:
			case EVDF_VERTEX_BUFFER_OBJECT:
			case EVDF_COLOR_MASK:
			case EVDF_ALPHA_TO_COVERAGE:
			case EVDF_POLYGON_OFFSET:
			case EVDF_BLEND_OPERATIONS:
			case EVDF_BLEND_SEPARATE:
			case EVDF_TEXTURE_MATRIX:
			case EVDF_TEXTURE_CUBEMAP:
				return true;
#ifdef _IRR_COMPILE_WITH_IOS_DEVICE_
			case EVDF_TEXTURE_NPOT:
#endif
			case EVDF_ARB_VERTEX_PROGRAM_1:
			case EVDF_ARB_FRAGMENT_PROGRAM_1:
			case EVDF_GEOMETRY_SHADER:
			case EVDF_MULTIPLE_RENDER_TARGETS:
			case EVDF_MRT_BLEND:
			case EVDF_MRT_COLOR_MASK:
			case EVDF_MRT_BLEND_FUNC:
			case EVDF_OCCLUSION_QUERY:
				return false;
			case EVDF_TEXTURE_COMPRESSED_DXT:
				return false; // NV Tegra need improvements here
			case EVDF_TEXTURE_COMPRESSED_PVRTC:
				return FeatureAvailable[IRR_GL_IMG_texture_compression_pvrtc];
			case EVDF_TEXTURE_COMPRESSED_PVRTC2:
				return FeatureAvailable[IRR_GL_IMG_texture_compression_pvrtc2];
			case EVDF_TEXTURE_COMPRESSED_ETC1:
				return FeatureAvailable[IRR_GL_OES_compressed_ETC1_RGB8_texture];
			case EVDF_TEXTURE_COMPRESSED_ETC2:
				return false;
			case EVDF_STENCIL_BUFFER:
				return StencilBuffer;
			default:
				return false;
			};
		}

		inline void irrGlActiveTexture(GLenum texture)
		{
			pglActiveTexture(texture);
		}

		inline void irrGlCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border,
			GLsizei imageSize, const void* data)
		{
			pglCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
		}

		inline void irrGlCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
			GLenum format, GLsizei imageSize, const void* data)
		{
			pglCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
		}

		inline void irrGlUseProgram(GLuint prog)
		{
			pglUseProgram(prog);
		}

		inline void irrGlBindFramebuffer(GLenum target, GLuint framebuffer)
		{
			pglBindFramebuffer(target, framebuffer);
		}

		inline void irrGlDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
		{
			pglDeleteFramebuffers(n, framebuffers);
		}

		inline void irrGlGenFramebuffers(GLsizei n, GLuint *framebuffers)
		{
			pglGenFramebuffers(n, framebuffers);
		}

		inline GLenum irrGlCheckFramebufferStatus(GLenum target)
		{
			return pglCheckFramebufferStatus(target);
		}

		inline void irrGlFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
		{
			pglFramebufferTexture2D(target, attachment, textarget, texture, level);
		}

		inline void irrGlGenerateMipmap(GLenum target)
		{
			pglGenerateMipmap(target);
		}

		inline void irrGlActiveStencilFace(GLenum face)
		{
		}

		inline void irrGlDrawBuffer(GLenum mode)
		{
		}

		inline void irrGlDrawBuffers(GLsizei n, const GLenum *bufs)
		{
		}

		inline void irrGlBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
		{
			pglBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
		}

		inline void irrGlBlendEquation(GLenum mode)
		{
			pglBlendEquation(mode);
		}

		inline void irrGlEnableIndexed(GLenum target, GLuint index)
		{
		}

		inline void irrGlDisableIndexed(GLenum target, GLuint index)
		{
		}

		inline void irrGlColorMaskIndexed(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
		{
		}

		inline void irrGlBlendFuncIndexed(GLuint buf, GLenum src, GLenum dst)
		{
		}

		inline void irrGlBlendFuncSeparateIndexed(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
		{
		}

		inline void irrGlBlendEquationIndexed(GLuint buf, GLenum mode)
		{
		}

		inline void irrGlBlendEquationSeparateIndexed(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
		{
		}
	};

}
}

#endif
#endif
