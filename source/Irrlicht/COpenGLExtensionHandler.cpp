// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "COpenGLExtensionHandler.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#include "irrString.h"
#include "SMaterial.h"
#include "fast_atof.h"

namespace irr
{
namespace video
{

bool COpenGLExtensionHandler::needsDSAFramebufferHack = true;

COpenGLExtensionHandler::COpenGLExtensionHandler(IContextManager* contextManager) :
		COpenGLBaseFunctionsHandler(contextManager),
		StencilBuffer(false), TextureCompressionExtension(false), MaxLights(1),
		MaxAnisotropy(1), MaxUserClipPlanes(0), MaxAuxBuffers(0), MaxIndices(65535),
		MaxTextureSize(1), MaxGeometryVerticesOut(0),
		MaxTextureLODBias(0.f), Version(0), ShaderLanguageVersion(0),
		OcclusionQuerySupport(false), ContextManager(contextManager)
#ifdef _IRR_OPENGL_USE_EXTPOINTER_
	,pGlActiveTexture(0)
	,pGlActiveTextureARB(0), pGlClientActiveTextureARB(0),
	pGlGenProgramsARB(0), pGlGenProgramsNV(0),
	pGlBindProgramARB(0), pGlBindProgramNV(0),
	pGlDeleteProgramsARB(0), pGlDeleteProgramsNV(0),
	pGlProgramStringARB(0), pGlLoadProgramNV(0),
	pGlProgramLocalParameter4fvARB(0),
	pGlCreateShaderObjectARB(0), pGlShaderSourceARB(0),
	pGlCompileShaderARB(0), pGlCreateProgramObjectARB(0), pGlAttachObjectARB(0),
	pGlLinkProgramARB(0), pGlUseProgramObjectARB(0), pGlDeleteObjectARB(0),
	pGlCreateProgram(0), pGlUseProgram(0),
	pGlDeleteProgram(0), pGlDeleteShader(0),
	pGlGetAttachedObjectsARB(0), pGlGetAttachedShaders(0),
	pGlCreateShader(0), pGlShaderSource(0), pGlCompileShader(0),
	pGlAttachShader(0), pGlLinkProgram(0),
	pGlGetInfoLogARB(0), pGlGetShaderInfoLog(0), pGlGetProgramInfoLog(0),
	pGlGetObjectParameterivARB(0), pGlGetShaderiv(0), pGlGetProgramiv(0),
	pGlGetUniformLocationARB(0), pGlGetUniformLocation(0),
	pGlUniform1fvARB(0), pGlUniform2fvARB(0), pGlUniform3fvARB(0), pGlUniform4fvARB(0),
	pGlUniform1ivARB(0), pGlUniform2ivARB(0), pGlUniform3ivARB(0), pGlUniform4ivARB(0),
	pGlUniform1uiv(0), pGlUniform2uiv(0), pGlUniform3uiv(0), pGlUniform4uiv(0),
	pGlUniformMatrix2fvARB(0), pGlUniformMatrix2x3fv(0), pGlUniformMatrix2x4fv(0),
	pGlUniformMatrix3x2fv(0), pGlUniformMatrix3fvARB(0), pGlUniformMatrix3x4fv(0),
	pGlUniformMatrix4x2fv(0), pGlUniformMatrix4x3fv(0), pGlUniformMatrix4fvARB(0),
	pGlGetActiveUniformARB(0), pGlGetActiveUniform(0),
	pGlPointParameterfARB(0), pGlPointParameterfvARB(0),
	pGlStencilFuncSeparate(0), pGlStencilOpSeparate(0),
	pGlStencilFuncSeparateATI(0), pGlStencilOpSeparateATI(0),
	pGlCompressedTexImage2D(0), pGlCompressedTexSubImage2D(0),
	// ARB framebuffer object
	pGlBindFramebuffer(0), pGlDeleteFramebuffers(0), pGlGenFramebuffers(0),
	pGlCheckFramebufferStatus(0), pGlFramebufferTexture2D(0),
	pGlBindRenderbuffer(0), pGlDeleteRenderbuffers(0), pGlGenRenderbuffers(0),
	pGlRenderbufferStorage(0), pGlFramebufferRenderbuffer(0), pGlGenerateMipmap(0),
	// EXT framebuffer object
	pGlBindFramebufferEXT(0), pGlDeleteFramebuffersEXT(0), pGlGenFramebuffersEXT(0),
	pGlCheckFramebufferStatusEXT(0), pGlFramebufferTexture2DEXT(0),
	pGlBindRenderbufferEXT(0), pGlDeleteRenderbuffersEXT(0), pGlGenRenderbuffersEXT(0),
	pGlRenderbufferStorageEXT(0), pGlFramebufferRenderbufferEXT(0), pGlGenerateMipmapEXT(0),
	pGlActiveStencilFaceEXT(0),
	// MRTs
	pGlDrawBuffersARB(0), pGlDrawBuffersATI(0),
	pGlGenBuffersARB(0), pGlBindBufferARB(0), pGlBufferDataARB(0), pGlDeleteBuffersARB(0),
	pGlBufferSubDataARB(0), pGlGetBufferSubDataARB(0), pGlMapBufferARB(0), pGlUnmapBufferARB(0),
	pGlIsBufferARB(0), pGlGetBufferParameterivARB(0), pGlGetBufferPointervARB(0),
	pGlProvokingVertexARB(0), pGlProvokingVertexEXT(0),
	pGlProgramParameteriARB(0), pGlProgramParameteriEXT(0),
	pGlGenQueriesARB(0), pGlDeleteQueriesARB(0), pGlIsQueryARB(0),
	pGlBeginQueryARB(0), pGlEndQueryARB(0), pGlGetQueryivARB(0),
	pGlGetQueryObjectivARB(0), pGlGetQueryObjectuivARB(0),
	pGlGenOcclusionQueriesNV(0), pGlDeleteOcclusionQueriesNV(0),
	pGlIsOcclusionQueryNV(0), pGlBeginOcclusionQueryNV(0),
	pGlEndOcclusionQueryNV(0), pGlGetOcclusionQueryivNV(0),
	pGlGetOcclusionQueryuivNV(0),
	// Blend
	pGlBlendFuncSeparateEXT(0), pGlBlendFuncSeparate(0),
	pGlBlendEquationEXT(0), pGlBlendEquation(0), pGlBlendEquationSeparateEXT(0), pGlBlendEquationSeparate(0),
	// Indexed
	pGlEnableIndexedEXT(0), pGlDisableIndexedEXT(0),
	pGlColorMaskIndexedEXT(0),
	pGlBlendFuncIndexedAMD(0), pGlBlendFunciARB(0), pGlBlendFuncSeparateIndexedAMD(0), pGlBlendFuncSeparateiARB(0),
	pGlBlendEquationIndexedAMD(0), pGlBlendEquationiARB(0), pGlBlendEquationSeparateIndexedAMD(0), pGlBlendEquationSeparateiARB(0),
	// DSA
    pGlTextureStorage2D(0), pGlTextureStorage3D(0), pGlTextureSubImage2D(0), pGlGetTextureImage(0), pGlNamedFramebufferTexture(0),
    pGlTextureParameteri(0), pGlTextureParameterf(0), pGlTextureParameteriv(0), pGlTextureParameterfv(0),
	pGlCreateTextures(0), pGlCreateFramebuffers(0), pGlBindTextures(0), pGlGenerateTextureMipmap(0),
    // DSA with EXT or functions to simulate it
	pGlTextureStorage2DEXT(0), pGlTexStorage2D(0), pGlTextureStorage3DEXT(0), pGlTexStorage3D(0), pGlTextureSubImage2DEXT(0), pGlGetTextureImageEXT(0),
	pGlNamedFramebufferTextureEXT(0), pGlFramebufferTexture(0), pGlGenerateTextureMipmapEXT(0)
#endif // _IRR_OPENGL_USE_EXTPOINTER_
{
	for (u32 i=0; i<IRR_OpenGL_Feature_Count; ++i)
		FeatureAvailable[i]=false;
	DimAliasedLine[0]=1.f;
	DimAliasedLine[1]=1.f;
	DimAliasedPoint[0]=1.f;
	DimAliasedPoint[1]=1.f;
	DimSmoothedLine[0]=1.f;
	DimSmoothedLine[1]=1.f;
	DimSmoothedPoint[0]=1.f;
	DimSmoothedPoint[1]=1.f;
}


void COpenGLExtensionHandler::dump(ELOG_LEVEL logLevel) const
{
	for (u32 i=0; i<IRR_OpenGL_Feature_Count; ++i)
		os::Printer::log(OpenGLFeatureStrings[i], FeatureAvailable[i]?" true":" false", logLevel);
}


void COpenGLExtensionHandler::dumpFramebufferFormats() const
{
#ifdef _IRR_WINDOWS_API_
	auto pwglGetCurrentDC = (decltype(&wglGetCurrentDC))ContextManager->loadFunction("wglGetCurrentDC");
	if(!pwglGetCurrentDC)
		return;
	HDC hdc = pwglGetCurrentDC();
	core::stringc wglExtensions;
#ifdef WGL_ARB_extensions_string
	PFNWGLGETEXTENSIONSSTRINGARBPROC irrGetExtensionsString = (PFNWGLGETEXTENSIONSSTRINGARBPROC)ContextManager->loadFunction("wglGetExtensionsStringARB");
	if (irrGetExtensionsString)
		wglExtensions = irrGetExtensionsString(hdc);
#elif defined(WGL_EXT_extensions_string)
	PFNWGLGETEXTENSIONSSTRINGEXTPROC irrGetExtensionsString = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)ContextManager->loadFunction("wglGetExtensionsStringEXT");
	if (irrGetExtensionsString)
		wglExtensions = irrGetExtensionsString(hdc);
#endif
	const bool pixel_format_supported = (wglExtensions.find("WGL_ARB_pixel_format") != -1);
	const bool multi_sample_supported = ((wglExtensions.find("WGL_ARB_multisample") != -1) ||
		(wglExtensions.find("WGL_EXT_multisample") != -1) || (wglExtensions.find("WGL_3DFX_multisample") != -1) );
#ifdef _DEBUG
	os::Printer::log("WGL_extensions", wglExtensions);
#endif

#ifdef WGL_ARB_pixel_format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat_ARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)ContextManager->loadFunction("wglChoosePixelFormatARB");
	if (pixel_format_supported && wglChoosePixelFormat_ARB)
	{
		// This value determines the number of samples used for antialiasing
		// My experience is that 8 does not show a big
		// improvement over 4, but 4 shows a big improvement
		// over 2.

		PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribiv_ARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)ContextManager->loadFunction("wglGetPixelFormatAttribivARB");
		if (wglGetPixelFormatAttribiv_ARB)
		{
			int vals[128];
			int atts[] = {
				WGL_NUMBER_PIXEL_FORMATS_ARB,
				WGL_DRAW_TO_BITMAP_ARB,
				WGL_ACCELERATION_ARB,
				WGL_NEED_PALETTE_ARB,
				WGL_NEED_SYSTEM_PALETTE_ARB,
				WGL_SWAP_LAYER_BUFFERS_ARB,
				WGL_SWAP_METHOD_ARB,
				WGL_NUMBER_OVERLAYS_ARB,
				WGL_NUMBER_UNDERLAYS_ARB,
				WGL_TRANSPARENT_ARB,
				WGL_TRANSPARENT_RED_VALUE_ARB,
				WGL_TRANSPARENT_GREEN_VALUE_ARB,
				WGL_TRANSPARENT_BLUE_VALUE_ARB,
				WGL_TRANSPARENT_ALPHA_VALUE_ARB,
				WGL_TRANSPARENT_INDEX_VALUE_ARB,
				WGL_SHARE_DEPTH_ARB,
				WGL_SHARE_STENCIL_ARB,
				WGL_SHARE_ACCUM_ARB,
				WGL_SUPPORT_GDI_ARB,
				WGL_SUPPORT_OPENGL_ARB,
				WGL_DOUBLE_BUFFER_ARB,
				WGL_STEREO_ARB,
				WGL_PIXEL_TYPE_ARB,
				WGL_COLOR_BITS_ARB,
				WGL_RED_BITS_ARB,
				WGL_RED_SHIFT_ARB,
				WGL_GREEN_BITS_ARB,
				WGL_GREEN_SHIFT_ARB,
				WGL_BLUE_BITS_ARB,
				WGL_BLUE_SHIFT_ARB,
				WGL_ALPHA_BITS_ARB,
				WGL_ALPHA_SHIFT_ARB,
				WGL_ACCUM_BITS_ARB,
				WGL_ACCUM_RED_BITS_ARB,
				WGL_ACCUM_GREEN_BITS_ARB,
				WGL_ACCUM_BLUE_BITS_ARB,
				WGL_ACCUM_ALPHA_BITS_ARB,
				WGL_DEPTH_BITS_ARB,
				WGL_STENCIL_BITS_ARB,
				WGL_AUX_BUFFERS_ARB
#ifdef WGL_ARB_render_texture
				,WGL_BIND_TO_TEXTURE_RGB_ARB //40
				,WGL_BIND_TO_TEXTURE_RGBA_ARB
#endif
#ifdef WGL_ARB_pbuffer
				,WGL_DRAW_TO_PBUFFER_ARB //42
				,WGL_MAX_PBUFFER_PIXELS_ARB
				,WGL_MAX_PBUFFER_WIDTH_ARB
				,WGL_MAX_PBUFFER_HEIGHT_ARB
#endif
#ifdef WGL_ARB_framebuffer_sRGB
				,WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB //46
#endif
#ifdef WGL_ARB_multisample
				,WGL_SAMPLES_ARB //47
				,WGL_SAMPLE_BUFFERS_ARB
#endif
#ifdef WGL_EXT_depth_float
				,WGL_DEPTH_FLOAT_EXT //49
#endif
				,0,0,0,0
			};
			size_t nums = sizeof(atts)/sizeof(int);
			const bool depth_float_supported= (wglExtensions.find("WGL_EXT_depth_float") != -1);
			if (!depth_float_supported)
			{
				memmove(&atts[49], &atts[50], (nums-50)*sizeof(int));
				nums -= 1;
			}
			if (!multi_sample_supported)
			{
				memmove(&atts[47], &atts[49], (nums-49)*sizeof(int));
				nums -= 2;
			}
			const bool framebuffer_sRGB_supported= (wglExtensions.find("WGL_ARB_framebuffer_sRGB") != -1);
			if (!framebuffer_sRGB_supported)
			{
				memmove(&atts[46], &atts[47], (nums-47)*sizeof(int));
				nums -= 1;
			}
			const bool pbuffer_supported = (wglExtensions.find("WGL_ARB_pbuffer") != -1);
			if (!pbuffer_supported)
			{
				memmove(&atts[42], &atts[46], (nums-46)*sizeof(int));
				nums -= 4;
			}
			const bool render_texture_supported = (wglExtensions.find("WGL_ARB_render_texture") != -1);
			if (!render_texture_supported)
			{
				memmove(&atts[40], &atts[42], (nums-42)*sizeof(int));
				nums -= 2;
			}
			wglGetPixelFormatAttribiv_ARB(hdc,0,0,1,atts,vals);
			const int count = vals[0];
			atts[0]=WGL_DRAW_TO_WINDOW_ARB;
			for (int i=1; i<count; ++i)
			{
				memset(vals,0,sizeof(vals));
#define tmplog(x,y) os::Printer::log(x, core::stringc(y).c_str())
				const BOOL res = wglGetPixelFormatAttribiv_ARB(hdc,i,0,(UINT)nums,atts,vals);
				if (FALSE==res)
					continue;
				tmplog("Pixel format ",i);
				u32 j=0;
				tmplog("Draw to window " , vals[j]);
				tmplog("Draw to bitmap " , vals[++j]);
				++j;
				tmplog("Acceleration " , (vals[j]==WGL_NO_ACCELERATION_ARB?"No":
					vals[j]==WGL_GENERIC_ACCELERATION_ARB?"Generic":vals[j]==WGL_FULL_ACCELERATION_ARB?"Full":"ERROR"));
				tmplog("Need palette " , vals[++j]);
				tmplog("Need system palette " , vals[++j]);
				tmplog("Swap layer buffers " , vals[++j]);
				++j;
				tmplog("Swap method " , (vals[j]==WGL_SWAP_EXCHANGE_ARB?"Exchange":
					vals[j]==WGL_SWAP_COPY_ARB?"Copy":vals[j]==WGL_SWAP_UNDEFINED_ARB?"Undefined":"ERROR"));
				tmplog("Number of overlays " , vals[++j]);
				tmplog("Number of underlays " , vals[++j]);
				tmplog("Transparent " , vals[++j]);
				tmplog("Transparent red value " , vals[++j]);
				tmplog("Transparent green value " , vals[++j]);
				tmplog("Transparent blue value " , vals[++j]);
				tmplog("Transparent alpha value " , vals[++j]);
				tmplog("Transparent index value " , vals[++j]);
				tmplog("Share depth " , vals[++j]);
				tmplog("Share stencil " , vals[++j]);
				tmplog("Share accum " , vals[++j]);
				tmplog("Support GDI " , vals[++j]);
				tmplog("Support OpenGL " , vals[++j]);
				tmplog("Double Buffer " , vals[++j]);
				tmplog("Stereo Buffer " , vals[++j]);
				tmplog("Pixel type " , vals[++j]);
				tmplog("Color bits" , vals[++j]);
				tmplog("Red bits " , vals[++j]);
				tmplog("Red shift " , vals[++j]);
				tmplog("Green bits " , vals[++j]);
				tmplog("Green shift " , vals[++j]);
				tmplog("Blue bits " , vals[++j]);
				tmplog("Blue shift " , vals[++j]);
				tmplog("Alpha bits " , vals[++j]);
				tmplog("Alpha Shift " , vals[++j]);
				tmplog("Accum bits " , vals[++j]);
				tmplog("Accum red bits " , vals[++j]);
				tmplog("Accum green bits " , vals[++j]);
				tmplog("Accum blue bits " , vals[++j]);
				tmplog("Accum alpha bits " , vals[++j]);
				tmplog("Depth bits " , vals[++j]);
				tmplog("Stencil bits " , vals[++j]);
				tmplog("Aux buffers " , vals[++j]);
				if (render_texture_supported)
				{
					tmplog("Bind to texture RGB" , vals[++j]);
					tmplog("Bind to texture RGBA" , vals[++j]);
				}
				if (pbuffer_supported)
				{
					tmplog("Draw to pbuffer" , vals[++j]);
					tmplog("Max pbuffer pixels " , vals[++j]);
					tmplog("Max pbuffer width" , vals[++j]);
					tmplog("Max pbuffer height" , vals[++j]);
				}
				if (framebuffer_sRGB_supported)
					tmplog("Framebuffer sRBG capable" , vals[++j]);
				if (multi_sample_supported)
				{
					tmplog("Samples " , vals[++j]);
					tmplog("Sample buffers " , vals[++j]);
				}
				if (depth_float_supported)
					tmplog("Depth float" , vals[++j]);
#undef tmplog
			}
		}
	}
#endif
#elif defined(IRR_LINUX_DEVICE)
#endif
}


void COpenGLExtensionHandler::initExtensions(bool stencilBuffer)
{
	const f32 ogl_ver = core::fast_atof(reinterpret_cast<const c8*>(pglGetString(GL_VERSION)));
	Version = static_cast<u16>(core::floor32(ogl_ver)*100+core::round32(core::fract(ogl_ver)*10.0f));
	if ( Version >= 102)
		os::Printer::log("OpenGL driver version is 1.2 or better.", ELL_INFORMATION);
	else
		os::Printer::log("OpenGL driver version is not 1.2 or better.", ELL_WARNING);

	{
		const char* t = reinterpret_cast<const char*>(pglGetString(GL_EXTENSIONS));
		size_t len = 0;
		c8 *str = 0;
		if (t)
		{
			len = strlen(t);
			str = new c8[len+1];
		}
		c8* p = str;

		for (size_t i=0; i<len; ++i)
		{
			str[i] = static_cast<char>(t[i]);

			if (str[i] == ' ')
			{
				str[i] = 0;
				for (u32 j=0; j<IRR_OpenGL_Feature_Count; ++j)
				{
					if (!strcmp(OpenGLFeatureStrings[j], p))
					{
						FeatureAvailable[j] = true;
						break;
					}
				}

				p = p + strlen(p) + 1;
			}
		}

		delete [] str;
	}

	TextureCompressionExtension = FeatureAvailable[IRR_ARB_texture_compression];
	StencilBuffer=stencilBuffer;

#ifdef _IRR_OPENGL_USE_EXTPOINTER_

	// get multitexturing function pointers
	pGlActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) ContextManager->loadFunction("glActiveTextureARB");
	pGlClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) ContextManager->loadFunction("glClientActiveTextureARB");

	// get fragment and vertex program function pointers
	pGlGenProgramsARB = (PFNGLGENPROGRAMSARBPROC) ContextManager->loadFunction("glGenProgramsARB");
	pGlGenProgramsNV = (PFNGLGENPROGRAMSNVPROC) ContextManager->loadFunction("glGenProgramsNV");
	pGlBindProgramARB = (PFNGLBINDPROGRAMARBPROC) ContextManager->loadFunction("glBindProgramARB");
	pGlBindProgramNV = (PFNGLBINDPROGRAMNVPROC) ContextManager->loadFunction("glBindProgramNV");
	pGlProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC) ContextManager->loadFunction("glProgramStringARB");
	pGlLoadProgramNV = (PFNGLLOADPROGRAMNVPROC) ContextManager->loadFunction("glLoadProgramNV");
	pGlDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC) ContextManager->loadFunction("glDeleteProgramsARB");
	pGlDeleteProgramsNV = (PFNGLDELETEPROGRAMSNVPROC) ContextManager->loadFunction("glDeleteProgramsNV");
	pGlProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) ContextManager->loadFunction("glProgramLocalParameter4fvARB");
	pGlCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC) ContextManager->loadFunction("glCreateShaderObjectARB");
	pGlCreateShader = (PFNGLCREATESHADERPROC) ContextManager->loadFunction("glCreateShader");
	pGlShaderSourceARB = (PFNGLSHADERSOURCEARBPROC) ContextManager->loadFunction("glShaderSourceARB");
	pGlShaderSource = (PFNGLSHADERSOURCEPROC) ContextManager->loadFunction("glShaderSource");
	pGlCompileShaderARB = (PFNGLCOMPILESHADERARBPROC) ContextManager->loadFunction("glCompileShaderARB");
	pGlCompileShader = (PFNGLCOMPILESHADERPROC) ContextManager->loadFunction("glCompileShader");
	pGlCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC) ContextManager->loadFunction("glCreateProgramObjectARB");
	pGlCreateProgram = (PFNGLCREATEPROGRAMPROC) ContextManager->loadFunction("glCreateProgram");
	pGlAttachObjectARB = (PFNGLATTACHOBJECTARBPROC) ContextManager->loadFunction("glAttachObjectARB");
	pGlAttachShader = (PFNGLATTACHSHADERPROC) ContextManager->loadFunction("glAttachShader");
	pGlLinkProgramARB = (PFNGLLINKPROGRAMARBPROC) ContextManager->loadFunction("glLinkProgramARB");
	pGlLinkProgram = (PFNGLLINKPROGRAMPROC) ContextManager->loadFunction("glLinkProgram");
	pGlUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC) ContextManager->loadFunction("glUseProgramObjectARB");
	pGlUseProgram = (PFNGLUSEPROGRAMPROC) ContextManager->loadFunction("glUseProgram");
	pGlDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC) ContextManager->loadFunction("glDeleteObjectARB");
	pGlDeleteProgram = (PFNGLDELETEPROGRAMPROC) ContextManager->loadFunction("glDeleteProgram");
	pGlDeleteShader = (PFNGLDELETESHADERPROC) ContextManager->loadFunction("glDeleteShader");
	pGlGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) ContextManager->loadFunction("glGetAttachedShaders");
	pGlGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC) ContextManager->loadFunction("glGetAttachedObjectsARB");
	pGlGetInfoLogARB = (PFNGLGETINFOLOGARBPROC) ContextManager->loadFunction("glGetInfoLogARB");
	pGlGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) ContextManager->loadFunction("glGetShaderInfoLog");
	pGlGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) ContextManager->loadFunction("glGetProgramInfoLog");
	pGlGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) ContextManager->loadFunction("glGetObjectParameterivARB");
	pGlGetShaderiv = (PFNGLGETSHADERIVPROC) ContextManager->loadFunction("glGetShaderiv");
	pGlGetProgramiv = (PFNGLGETPROGRAMIVPROC) ContextManager->loadFunction("glGetProgramiv");
	pGlGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC) ContextManager->loadFunction("glGetUniformLocationARB");
	pGlGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) ContextManager->loadFunction("glGetUniformLocation");
	pGlUniform1fvARB = (PFNGLUNIFORM1FVARBPROC) ContextManager->loadFunction("glUniform1fvARB");
	pGlUniform2fvARB = (PFNGLUNIFORM2FVARBPROC) ContextManager->loadFunction("glUniform2fvARB");
	pGlUniform3fvARB = (PFNGLUNIFORM3FVARBPROC) ContextManager->loadFunction("glUniform3fvARB");
	pGlUniform4fvARB = (PFNGLUNIFORM4FVARBPROC) ContextManager->loadFunction("glUniform4fvARB");
	pGlUniform1ivARB = (PFNGLUNIFORM1IVARBPROC) ContextManager->loadFunction("glUniform1ivARB");
	pGlUniform2ivARB = (PFNGLUNIFORM2IVARBPROC) ContextManager->loadFunction("glUniform2ivARB");
	pGlUniform3ivARB = (PFNGLUNIFORM3IVARBPROC) ContextManager->loadFunction("glUniform3ivARB");
	pGlUniform4ivARB = (PFNGLUNIFORM4IVARBPROC) ContextManager->loadFunction("glUniform4ivARB");
	pGlUniform1uiv = (PFNGLUNIFORM1UIVPROC) ContextManager->loadFunction("glUniform1uiv");
	pGlUniform2uiv = (PFNGLUNIFORM2UIVPROC) ContextManager->loadFunction("glUniform2uiv");
	pGlUniform3uiv = (PFNGLUNIFORM3UIVPROC) ContextManager->loadFunction("glUniform3uiv");
	pGlUniform4uiv = (PFNGLUNIFORM4UIVPROC) ContextManager->loadFunction("glUniform4uiv");
	pGlUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC) ContextManager->loadFunction("glUniformMatrix2fvARB");
	pGlUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) ContextManager->loadFunction("glUniformMatrix2x3fv");
	pGlUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)ContextManager->loadFunction("glUniformMatrix2x4fv");
	pGlUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)ContextManager->loadFunction("glUniformMatrix3x2fv");
	pGlUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC) ContextManager->loadFunction("glUniformMatrix3fvARB");
	pGlUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)ContextManager->loadFunction("glUniformMatrix3x4fv");
	pGlUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)ContextManager->loadFunction("glUniformMatrix4x2fv");
	pGlUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)ContextManager->loadFunction("glUniformMatrix4x3fv");
	pGlUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC) ContextManager->loadFunction("glUniformMatrix4fvARB");
	pGlGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC) ContextManager->loadFunction("glGetActiveUniformARB");
	pGlGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) ContextManager->loadFunction("glGetActiveUniform");

	// get point parameter extension
	pGlPointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC) ContextManager->loadFunction("glPointParameterfARB");
	pGlPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC) ContextManager->loadFunction("glPointParameterfvARB");

	// get stencil extension
	pGlStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) ContextManager->loadFunction("glStencilFuncSeparate");
	pGlStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) ContextManager->loadFunction("glStencilOpSeparate");
	pGlStencilFuncSeparateATI = (PFNGLSTENCILFUNCSEPARATEATIPROC) ContextManager->loadFunction("glStencilFuncSeparateATI");
	pGlStencilOpSeparateATI = (PFNGLSTENCILOPSEPARATEATIPROC) ContextManager->loadFunction("glStencilOpSeparateATI");

	// compressed textures
	pGlCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) ContextManager->loadFunction("glCompressedTexImage2D");
	pGlCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) ContextManager->loadFunction("glCompressedTexSubImage2D");

	// ARB FrameBufferObjects
	pGlBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) ContextManager->loadFunction("glBindFramebuffer");
	pGlDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) ContextManager->loadFunction("glDeleteFramebuffers");
	pGlGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) ContextManager->loadFunction("glGenFramebuffers");
	pGlCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) ContextManager->loadFunction("glCheckFramebufferStatus");
	pGlFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) ContextManager->loadFunction("glFramebufferTexture2D");
	pGlBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) ContextManager->loadFunction("glBindRenderbuffer");
	pGlDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) ContextManager->loadFunction("glDeleteRenderbuffers");
	pGlGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) ContextManager->loadFunction("glGenRenderbuffers");
	pGlRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) ContextManager->loadFunction("glRenderbufferStorage");
	pGlFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) ContextManager->loadFunction("glFramebufferRenderbuffer");
	pGlGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) ContextManager->loadFunction("glGenerateMipmap");

	// EXT FrameBufferObjects
	pGlBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) ContextManager->loadFunction("glBindFramebufferEXT");
	pGlDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) ContextManager->loadFunction("glDeleteFramebuffersEXT");
	pGlGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) ContextManager->loadFunction("glGenFramebuffersEXT");
	pGlCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) ContextManager->loadFunction("glCheckFramebufferStatusEXT");
	pGlFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) ContextManager->loadFunction("glFramebufferTexture2DEXT");
	pGlBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) ContextManager->loadFunction("glBindRenderbufferEXT");
	pGlDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) ContextManager->loadFunction("glDeleteRenderbuffersEXT");
	pGlGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) ContextManager->loadFunction("glGenRenderbuffersEXT");
	pGlRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) ContextManager->loadFunction("glRenderbufferStorageEXT");
	pGlFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) ContextManager->loadFunction("glFramebufferRenderbufferEXT");
	pGlGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) ContextManager->loadFunction("glGenerateMipmapEXT");
	pGlDrawBuffersARB = (PFNGLDRAWBUFFERSARBPROC) ContextManager->loadFunction("glDrawBuffersARB");
	pGlDrawBuffersATI = (PFNGLDRAWBUFFERSATIPROC) ContextManager->loadFunction("glDrawBuffersATI");

	// get vertex buffer extension
	pGlGenBuffersARB = (PFNGLGENBUFFERSARBPROC) ContextManager->loadFunction("glGenBuffersARB");
	pGlBindBufferARB = (PFNGLBINDBUFFERARBPROC) ContextManager->loadFunction("glBindBufferARB");
	pGlBufferDataARB = (PFNGLBUFFERDATAARBPROC) ContextManager->loadFunction("glBufferDataARB");
	pGlDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) ContextManager->loadFunction("glDeleteBuffersARB");
	pGlBufferSubDataARB= (PFNGLBUFFERSUBDATAARBPROC) ContextManager->loadFunction("glBufferSubDataARB");
	pGlGetBufferSubDataARB= (PFNGLGETBUFFERSUBDATAARBPROC)ContextManager->loadFunction("glGetBufferSubDataARB");
	pGlMapBufferARB= (PFNGLMAPBUFFERARBPROC) ContextManager->loadFunction("glMapBufferARB");
	pGlUnmapBufferARB= (PFNGLUNMAPBUFFERARBPROC) ContextManager->loadFunction("glUnmapBufferARB");
	pGlIsBufferARB= (PFNGLISBUFFERARBPROC) ContextManager->loadFunction("glIsBufferARB");
	pGlGetBufferParameterivARB= (PFNGLGETBUFFERPARAMETERIVARBPROC) ContextManager->loadFunction("glGetBufferParameterivARB");
	pGlGetBufferPointervARB= (PFNGLGETBUFFERPOINTERVARBPROC) ContextManager->loadFunction("glGetBufferPointervARB");
	pGlProvokingVertexARB= (PFNGLPROVOKINGVERTEXPROC) ContextManager->loadFunction("glProvokingVertex");
	pGlProvokingVertexEXT= (PFNGLPROVOKINGVERTEXEXTPROC) ContextManager->loadFunction("glProvokingVertexEXT");
	pGlProgramParameteriARB= (PFNGLPROGRAMPARAMETERIARBPROC) ContextManager->loadFunction("glProgramParameteriARB");
	pGlProgramParameteriEXT= (PFNGLPROGRAMPARAMETERIEXTPROC) ContextManager->loadFunction("glProgramParameteriEXT");

	// occlusion query
	pGlGenQueriesARB = (PFNGLGENQUERIESARBPROC) ContextManager->loadFunction("glGenQueriesARB");
	pGlDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC) ContextManager->loadFunction("glDeleteQueriesARB");
	pGlIsQueryARB = (PFNGLISQUERYARBPROC) ContextManager->loadFunction("glIsQueryARB");
	pGlBeginQueryARB = (PFNGLBEGINQUERYARBPROC) ContextManager->loadFunction("glBeginQueryARB");
	pGlEndQueryARB = (PFNGLENDQUERYARBPROC) ContextManager->loadFunction("glEndQueryARB");
	pGlGetQueryivARB = (PFNGLGETQUERYIVARBPROC) ContextManager->loadFunction("glGetQueryivARB");
	pGlGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC) ContextManager->loadFunction("glGetQueryObjectivARB");
	pGlGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC) ContextManager->loadFunction("glGetQueryObjectuivARB");
	pGlGenOcclusionQueriesNV = (PFNGLGENOCCLUSIONQUERIESNVPROC) ContextManager->loadFunction("glGenOcclusionQueriesNV");
	pGlDeleteOcclusionQueriesNV = (PFNGLDELETEOCCLUSIONQUERIESNVPROC) ContextManager->loadFunction("glDeleteOcclusionQueriesNV");
	pGlIsOcclusionQueryNV = (PFNGLISOCCLUSIONQUERYNVPROC) ContextManager->loadFunction("glIsOcclusionQueryNV");
	pGlBeginOcclusionQueryNV = (PFNGLBEGINOCCLUSIONQUERYNVPROC) ContextManager->loadFunction("glBeginOcclusionQueryNV");
	pGlEndOcclusionQueryNV = (PFNGLENDOCCLUSIONQUERYNVPROC) ContextManager->loadFunction("glEndOcclusionQueryNV");
	pGlGetOcclusionQueryivNV = (PFNGLGETOCCLUSIONQUERYIVNVPROC) ContextManager->loadFunction("glGetOcclusionQueryivNV");
	pGlGetOcclusionQueryuivNV = (PFNGLGETOCCLUSIONQUERYUIVNVPROC) ContextManager->loadFunction("glGetOcclusionQueryuivNV");

	// blend
	pGlBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC) ContextManager->loadFunction("glBlendFuncSeparateEXT");
	pGlBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) ContextManager->loadFunction("glBlendFuncSeparate");
	pGlBlendEquationEXT = (PFNGLBLENDEQUATIONEXTPROC) ContextManager->loadFunction("glBlendEquationEXT");
	pGlBlendEquation = (PFNGLBLENDEQUATIONPROC) ContextManager->loadFunction("glBlendEquation");
	pGlBlendEquationSeparateEXT = (PFNGLBLENDEQUATIONSEPARATEEXTPROC) ContextManager->loadFunction("glBlendEquationSeparateEXT");
	pGlBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) ContextManager->loadFunction("glBlendEquationSeparate");

	// indexed
	pGlEnableIndexedEXT = (PFNGLENABLEINDEXEDEXTPROC) ContextManager->loadFunction("glEnableIndexedEXT");
	pGlDisableIndexedEXT = (PFNGLDISABLEINDEXEDEXTPROC) ContextManager->loadFunction("glDisableIndexedEXT");
	pGlColorMaskIndexedEXT = (PFNGLCOLORMASKINDEXEDEXTPROC) ContextManager->loadFunction("glColorMaskIndexedEXT");
	pGlBlendFuncIndexedAMD = (PFNGLBLENDFUNCINDEXEDAMDPROC) ContextManager->loadFunction("glBlendFuncIndexedAMD");
	pGlBlendFunciARB = (PFNGLBLENDFUNCIPROC) ContextManager->loadFunction("glBlendFunciARB");
	pGlBlendFuncSeparateIndexedAMD = (PFNGLBLENDFUNCSEPARATEINDEXEDAMDPROC) ContextManager->loadFunction("glBlendFuncSeparateIndexedAMD");
	pGlBlendFuncSeparateiARB = (PFNGLBLENDFUNCSEPARATEIPROC) ContextManager->loadFunction("glBlendFuncSeparateiARB");
	pGlBlendEquationIndexedAMD = (PFNGLBLENDEQUATIONINDEXEDAMDPROC) ContextManager->loadFunction("glBlendEquationIndexedAMD");
	pGlBlendEquationiARB = (PFNGLBLENDEQUATIONIPROC) ContextManager->loadFunction("glBlendEquationiARB");
	pGlBlendEquationSeparateIndexedAMD = (PFNGLBLENDEQUATIONSEPARATEINDEXEDAMDPROC) ContextManager->loadFunction("glBlendEquationSeparateIndexedAMD");
	pGlBlendEquationSeparateiARB = (PFNGLBLENDEQUATIONSEPARATEIPROC) ContextManager->loadFunction("glBlendEquationSeparateiARB");

    pGlTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC) ContextManager->loadFunction("glTextureStorage2D");
    pGlTextureStorage3D = (PFNGLTEXTURESTORAGE3DPROC) ContextManager->loadFunction("glTextureStorage3D");
	pGlTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)ContextManager->loadFunction("glTextureSubImage2D");
	pGlGetTextureImage = (PFNGLGETTEXTUREIMAGEPROC)ContextManager->loadFunction("glGetTextureImage");
    pGlNamedFramebufferTexture = (PFNGLNAMEDFRAMEBUFFERTEXTUREPROC) ContextManager->loadFunction("glNamedFramebufferTexture");
    pGlTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC) ContextManager->loadFunction("glTextureParameteri");
	pGlTextureParameterf = (PFNGLTEXTUREPARAMETERFPROC)ContextManager->loadFunction("glTextureParameterf");
	pGlTextureParameteriv = (PFNGLTEXTUREPARAMETERIVPROC)ContextManager->loadFunction("glTextureParameteriv");
	pGlTextureParameterfv = (PFNGLTEXTUREPARAMETERFVPROC)ContextManager->loadFunction("glTextureParameterfv");

    pGlCreateTextures = (PFNGLCREATETEXTURESPROC) ContextManager->loadFunction("glCreateTextures");
    pGlCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERSPROC) ContextManager->loadFunction("glCreateFramebuffers");
    pGlBindTextures = (PFNGLBINDTEXTURESPROC) ContextManager->loadFunction("glBindTextures");
    pGlGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC) ContextManager->loadFunction("glGenerateTextureMipmap");
    //==============================
    pGlTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)ContextManager->loadFunction("glTextureStorage2DEXT");
    pGlTexStorage2D = (PFNGLTEXSTORAGE2DPROC)ContextManager->loadFunction("glTexStorage2D");
    pGlTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)ContextManager->loadFunction("glTextureStorage3DEXT");
    pGlTexStorage3D = (PFNGLTEXSTORAGE3DPROC)ContextManager->loadFunction("glTexStorage3D");
	pGlTextureSubImage2DEXT = (PFNGLTEXTURESUBIMAGE2DEXTPROC)ContextManager->loadFunction("glTextureSubImage2DEXT");
	pGlGetTextureImageEXT = (PFNGLGETTEXTUREIMAGEEXTPROC)ContextManager->loadFunction("glGetTextureImageEXT");
    pGlNamedFramebufferTextureEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC)ContextManager->loadFunction("glNamedFramebufferTextureEXT");
    pGlFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)ContextManager->loadFunction("glFramebufferTexture");
    pGlActiveTexture = (PFNGLACTIVETEXTUREPROC)ContextManager->loadFunction("glActiveTexture");
    pGlGenerateTextureMipmapEXT = (PFNGLGENERATETEXTUREMIPMAPEXTPROC) ContextManager->loadFunction("glGenerateTextureMipmapEXT");
#endif // use extension pointer

	GLint num=0;
	// set some properties
#if defined(GL_ARB_multitexture) || defined(GL_VERSION_1_3)
	if (Version>102 || FeatureAvailable[IRR_ARB_multitexture])
	{
#if defined(GL_MAX_TEXTURE_UNITS)
		pglGetIntegerv(GL_MAX_TEXTURE_UNITS, &num);
#elif defined(GL_MAX_TEXTURE_UNITS_ARB)
		pglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &num);
#endif
		Feature.MaxTextureUnits=static_cast<u8>(num);	// MULTITEXTURING (fixed function pipeline texture units)
	}
#endif
#if defined(GL_ARB_vertex_shader) || defined(GL_VERSION_2_0)
	if (Version>=200 || FeatureAvailable[IRR_ARB_vertex_shader])
	{
		num=0;
#if defined(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
		pglGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &num);
#elif defined(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB)
		pglGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, &num);
#endif
		Feature.MaxTextureUnits =core::max_(Feature.MaxTextureUnits,static_cast<u8>(num));
	}
#endif
	pglGetIntegerv(GL_MAX_LIGHTS, &num);
	MaxLights=static_cast<u8>(num);
#ifdef GL_EXT_texture_filter_anisotropic
	if (FeatureAvailable[IRR_EXT_texture_filter_anisotropic])
	{
		pglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &num);
		MaxAnisotropy=static_cast<u8>(num);
	}
#endif
#ifdef GL_VERSION_1_2
	if (Version>101)
	{
		pglGetIntegerv(GL_MAX_ELEMENTS_INDICES, &num);
		MaxIndices=num;
	}
#endif
	pglGetIntegerv(GL_MAX_TEXTURE_SIZE, &num);
	MaxTextureSize=static_cast<u32>(num);
	if (queryFeature(EVDF_GEOMETRY_SHADER))
	{
#if defined(GL_ARB_geometry_shader4) || defined(GL_EXT_geometry_shader4) || defined(GL_NV_geometry_shader4)
		pglGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &num);
		MaxGeometryVerticesOut=static_cast<u32>(num);
#elif defined(GL_NV_geometry_program4)
		extGlGetProgramiv(GEOMETRY_PROGRAM_NV, GL_MAX_PROGRAM_OUTPUT_VERTICES_NV, &num);
		MaxGeometryVerticesOut=static_cast<u32>(num);
#endif
	}
#ifdef GL_EXT_texture_lod_bias
	if (FeatureAvailable[IRR_EXT_texture_lod_bias])
		pglGetFloatv(GL_MAX_TEXTURE_LOD_BIAS_EXT, &MaxTextureLODBias);
#endif
	pglGetIntegerv(GL_MAX_CLIP_PLANES, &num);
	MaxUserClipPlanes=static_cast<u8>(num);
	pglGetIntegerv(GL_AUX_BUFFERS, &num);
	MaxAuxBuffers=static_cast<u8>(num);
#ifdef GL_ARB_draw_buffers
	if (FeatureAvailable[IRR_ARB_draw_buffers])
	{
		pglGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &num);
		Feature.MultipleRenderTarget = static_cast<u8>(num);
	}
#endif
#if defined(GL_ATI_draw_buffers)
#ifdef GL_ARB_draw_buffers
	else
#endif
	if (FeatureAvailable[IRR_ATI_draw_buffers])
	{
		pglGetIntegerv(GL_MAX_DRAW_BUFFERS_ATI, &num);
		Feature.MultipleRenderTarget = static_cast<u8>(num);
	}
#endif
#ifdef GL_ARB_framebuffer_object
	if (FeatureAvailable[IRR_ARB_framebuffer_object])
	{
		pglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &num);
		Feature.ColorAttachment = static_cast<u8>(num);
	}
#endif
#if defined(GL_EXT_framebuffer_object)
#ifdef GL_ARB_framebuffer_object
	else
#endif
		if (FeatureAvailable[IRR_EXT_framebuffer_object])
		{
			pglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &num);
			Feature.ColorAttachment = static_cast<u8>(num);
		}
#endif

	pglGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, DimAliasedLine);
	pglGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, DimAliasedPoint);
	pglGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, DimSmoothedLine);
	pglGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, DimSmoothedPoint);
#if defined(GL_ARB_shading_language_100) || defined (GL_VERSION_2_0)
	if (FeatureAvailable[IRR_ARB_shading_language_100] || Version>=200)
	{
		pglGetError(); // clean error buffer
#ifdef GL_SHADING_LANGUAGE_VERSION
		const GLubyte* shaderVersion = pglGetString(GL_SHADING_LANGUAGE_VERSION);
#else
		const GLubyte* shaderVersion = pglGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
#endif
		if (pglGetError() == GL_INVALID_ENUM)
			ShaderLanguageVersion = 100;
		else
		{
			const f32 sl_ver = core::fast_atof(reinterpret_cast<const c8*>(shaderVersion));
			ShaderLanguageVersion = static_cast<u16>(core::floor32(sl_ver)*100+core::round32(core::fract(sl_ver)*10.0f));
		}
	}
#endif

#ifdef _IRR_OPENGL_USE_EXTPOINTER_
	if (!pGlActiveTextureARB || !pGlClientActiveTextureARB)
	{
		Feature.MaxTextureUnits = 1;
		os::Printer::log("Failed to load OpenGL's multitexture extension, proceeding without.", ELL_WARNING);
	}
	else
#endif
	Feature.MaxTextureUnits = core::min_(Feature.MaxTextureUnits, static_cast<u8>(MATERIAL_MAX_TEXTURES));
	Feature.MaxTextureUnits = core::min_(Feature.MaxTextureUnits, static_cast<u8>(MATERIAL_MAX_TEXTURES_USED));

#ifdef GL_ARB_occlusion_query
	if (FeatureAvailable[IRR_ARB_occlusion_query])
	{
		extGlGetQueryiv(GL_SAMPLES_PASSED_ARB,GL_QUERY_COUNTER_BITS_ARB,
						&num);
		OcclusionQuerySupport=(num>0);
	}
	else
#endif
#ifdef GL_NV_occlusion_query
	if (FeatureAvailable[IRR_NV_occlusion_query])
	{
		pglGetIntegerv(GL_PIXEL_COUNTER_BITS_NV, &num);
		OcclusionQuerySupport=(num>0);
	}
	else
#endif
		OcclusionQuerySupport=false;

    Feature.BlendOperation = (Version >= 104) ||
            FeatureAvailable[IRR_EXT_blend_minmax] ||
            FeatureAvailable[IRR_EXT_blend_subtract] ||
            FeatureAvailable[IRR_EXT_blend_logic_op];

#ifdef _DEBUG
	if (FeatureAvailable[IRR_NVX_gpu_memory_info])
	{
		// undocumented flags, so use the RAW values
		GLint val;
		pglGetIntegerv(0x9047, &val);
		os::Printer::log("Dedicated video memory (kB)", core::stringc(val));
		pglGetIntegerv(0x9048, &val);
		os::Printer::log("Total video memory (kB)", core::stringc(val));
		pglGetIntegerv(0x9049, &val);
		os::Printer::log("Available video memory (kB)", core::stringc(val));
	}
#ifdef GL_ATI_meminfo
	if (FeatureAvailable[IRR_ATI_meminfo])
	{
		GLint val[4];
		pglGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, val);
		os::Printer::log("Free texture memory (kB)", core::stringc(val[0]));
		pglGetIntegerv(GL_VBO_FREE_MEMORY_ATI, val);
		os::Printer::log("Free VBO memory (kB)", core::stringc(val[0]));
		pglGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, val);
		os::Printer::log("Free render buffer memory (kB)", core::stringc(val[0]));
	}
#endif

	if (queryFeature(EVDF_TEXTURE_CUBEMAP_SEAMLESS))
		pglEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#endif
}

const COpenGLCoreFeature& COpenGLExtensionHandler::getFeature() const
{
	return Feature;
}

bool COpenGLExtensionHandler::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
{
	switch (feature)
	{
	case EVDF_RENDER_TO_TARGET:
		return true;
	case EVDF_HARDWARE_TL:
		return true; // we cannot tell other things
	case EVDF_MULTITEXTURE:
		return Feature.MaxTextureUnits > 1;
	case EVDF_BILINEAR_FILTER:
		return true;
	case EVDF_MIP_MAP:
		return true;
	case EVDF_MIP_MAP_AUTO_UPDATE:
		return FeatureAvailable[IRR_SGIS_generate_mipmap] || FeatureAvailable[IRR_EXT_framebuffer_object] || FeatureAvailable[IRR_ARB_framebuffer_object];
	case EVDF_STENCIL_BUFFER:
		return StencilBuffer;
	case EVDF_VERTEX_SHADER_1_1:
	case EVDF_ARB_VERTEX_PROGRAM_1:
		return FeatureAvailable[IRR_ARB_vertex_program] || FeatureAvailable[IRR_NV_vertex_program1_1];
	case EVDF_PIXEL_SHADER_1_1:
	case EVDF_PIXEL_SHADER_1_2:
	case EVDF_ARB_FRAGMENT_PROGRAM_1:
		return FeatureAvailable[IRR_ARB_fragment_program] || FeatureAvailable[IRR_NV_fragment_program];
	case EVDF_PIXEL_SHADER_2_0:
	case EVDF_VERTEX_SHADER_2_0:
	case EVDF_ARB_GLSL:
		return (FeatureAvailable[IRR_ARB_shading_language_100]||Version>=200);
	case EVDF_TEXTURE_NSQUARE:
		return true; // non-square is always supported
	case EVDF_TEXTURE_NPOT:
		// Some ATI cards seem to have only SW support in OpenGL 2.0
		// drivers if the extension is not exposed, so we skip this
		// extra test for now!
		// return (FeatureAvailable[IRR_ARB_texture_non_power_of_two]||Version>=200);
		return (FeatureAvailable[IRR_ARB_texture_non_power_of_two]);
	case EVDF_FRAMEBUFFER_OBJECT:
		return FeatureAvailable[IRR_EXT_framebuffer_object] || FeatureAvailable[IRR_ARB_framebuffer_object];
	case EVDF_VERTEX_BUFFER_OBJECT:
		return FeatureAvailable[IRR_ARB_vertex_buffer_object];
	case EVDF_COLOR_MASK:
		return true;
	case EVDF_ALPHA_TO_COVERAGE:
		return FeatureAvailable[IRR_ARB_multisample];
	case EVDF_GEOMETRY_SHADER:
		return FeatureAvailable[IRR_ARB_geometry_shader4] || FeatureAvailable[IRR_EXT_geometry_shader4] || FeatureAvailable[IRR_NV_geometry_program4] || FeatureAvailable[IRR_NV_geometry_shader4];
	case EVDF_MULTIPLE_RENDER_TARGETS:
		return FeatureAvailable[IRR_ARB_draw_buffers] || FeatureAvailable[IRR_ATI_draw_buffers];
	case EVDF_MRT_BLEND:
	case EVDF_MRT_COLOR_MASK:
		return FeatureAvailable[IRR_EXT_draw_buffers2];
	case EVDF_MRT_BLEND_FUNC:
		return FeatureAvailable[IRR_ARB_draw_buffers_blend] || FeatureAvailable[IRR_AMD_draw_buffers_blend];
	case EVDF_OCCLUSION_QUERY:
		return FeatureAvailable[IRR_ARB_occlusion_query] && OcclusionQuerySupport;
	case EVDF_POLYGON_OFFSET:
		// both features supported with OpenGL 1.1
		return Version>=101;
	case EVDF_BLEND_OPERATIONS:
		return Feature.BlendOperation;
	case EVDF_BLEND_SEPARATE:
		return (Version>=104) || FeatureAvailable[IRR_EXT_blend_func_separate];
	case EVDF_TEXTURE_MATRIX:
		return true;
	case EVDF_TEXTURE_COMPRESSED_DXT:
		return FeatureAvailable[IRR_EXT_texture_compression_s3tc];
	case EVDF_TEXTURE_CUBEMAP:
		return (Version >= 103) || FeatureAvailable[IRR_ARB_texture_cube_map] || FeatureAvailable[IRR_EXT_texture_cube_map];
	case EVDF_TEXTURE_CUBEMAP_SEAMLESS:
		return FeatureAvailable[IRR_ARB_seamless_cube_map];
	case EVDF_DEPTH_CLAMP:
		return FeatureAvailable[IRR_NV_depth_clamp] || FeatureAvailable[IRR_ARB_depth_clamp];

	default:
		return false;
	};
}


}
}

#endif
