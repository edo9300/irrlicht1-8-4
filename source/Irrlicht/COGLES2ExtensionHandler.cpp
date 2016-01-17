// Copyright (C) 2015 Patryk Nadrowski
// Copyright (C) 2009-2010 Amundis
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "COGLES2ExtensionHandler.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "irrString.h"
#include "SMaterial.h"
#include "fast_atof.h"

namespace irr
{
namespace video
{

	static const char* const OGLES2FeatureStrings[COGLES2ExtensionHandler::IRR_OGLES2_Feature_Count] =
	{
		"GL_AMD_compressed_3DC_texture",
		"GL_AMD_compressed_ATC_texture",
		"GL_AMD_performance_monitor",
		"GL_AMD_program_binary_Z400",
		"GL_ANGLE_framebuffer_blit",
		"GL_ANGLE_framebuffer_multisample",
		"GL_ANGLE_instanced_arrays",
		"GL_ANGLE_pack_reverse_row_order",
		"GL_ANGLE_texture_compression_dxt3",
		"GL_ANGLE_texture_compression_dxt5",
		"GL_ANGLE_texture_usage",
		"GL_ANGLE_translated_shader_source",
		"GL_APPLE_copy_texture_levels",
		"GL_APPLE_framebuffer_multisample",
		"GL_APPLE_rgb_422",
		"GL_APPLE_sync",
		"GL_APPLE_texture_2D_limited_npot",
		"GL_APPLE_texture_format_BGRA8888",
		"GL_APPLE_texture_max_level",
		"GL_ARB_texture_env_combine",
		"GL_ARB_texture_env_dot3",
		"GL_ARM_mali_program_binary",
		"GL_ARM_mali_shader_binary",
		"GL_ARM_rgba8",
		"GL_DMP_shader_binary",
		"GL_EXT_blend_minmax",
		"GL_EXT_color_buffer_half_float",
		"GL_EXT_debug_label",
		"GL_EXT_debug_marker",
		"GL_EXT_discard_framebuffer",
		"GL_EXT_frag_depth",
		"GL_EXT_map_buffer_range",
		"GL_EXT_multisampled_render_to_texture",
		"GL_EXT_multiview_draw_buffers",
		"GL_EXT_multi_draw_arrays",
		"GL_EXT_occlusion_query_boolean",
		"GL_EXT_read_format_bgra",
		"GL_EXT_robustness",
		"GL_EXT_separate_shader_objects",
		"GL_EXT_shader_framebuffer_fetch",
		"GL_EXT_shader_texture_lod",
		"GL_EXT_shadow_samplers",
		"GL_EXT_sRGB",
		"GL_EXT_texture_compression_dxt1",
		"GL_EXT_texture_filter_anisotropic",
		"GL_EXT_texture_format_BGRA8888",
		"GL_EXT_texture_lod_bias",
		"GL_EXT_texture_rg",
		"GL_EXT_texture_storage",
		"GL_EXT_texture_type_2_10_10_10_REV",
		"GL_EXT_unpack_subimage",
		"GL_FJ_shader_binary_GCCSO",
		"GL_IMG_multisampled_render_to_texture",
		"GL_IMG_program_binary",
		"GL_IMG_read_format",
		"GL_IMG_shader_binary",
		"GL_IMG_texture_compression_pvrtc",
		"GL_IMG_texture_compression_pvrtc2",
		"GL_IMG_texture_env_enhanced_fixed_function",
		"GL_IMG_texture_format_BGRA8888",
		"GL_IMG_user_clip_plane",
		"GL_IMG_vertex_program",
		"GL_KHR_debug",
		"GL_KHR_texture_compression_astc_ldr",
		"GL_NV_coverage_sample",
		"GL_NV_depth_nonlinear",
		"GL_NV_draw_buffers",
		"GL_NV_EGL_stream_consumer_external",
		"GL_NV_fbo_color_attachments",
		"GL_NV_fence",
		"GL_NV_read_buffer",
		"GL_NV_read_buffer_front",
		"GL_NV_read_depth",
		"GL_NV_read_depth_stencil",
		"GL_NV_read_stencil",
		"GL_NV_texture_compression_s3tc_update",
		"GL_NV_texture_npot_2D_mipmap",
		"GL_OES_blend_equation_separate",
		"GL_OES_blend_func_separate",
		"GL_OES_blend_subtract",
		"GL_OES_byte_coordinates",
		"GL_OES_compressed_ETC1_RGB8_texture",
		"GL_OES_compressed_paletted_texture",
		"GL_OES_depth24",
		"GL_OES_depth32",
		"GL_OES_depth_texture",
		"GL_OES_draw_texture",
		"GL_OES_EGL_image",
		"GL_OES_EGL_image_external",
		"GL_OES_EGL_sync",
		"GL_OES_element_index_uint",
		"GL_OES_extended_matrix_palette",
		"GL_OES_fbo_render_mipmap",
		"GL_OES_fixed_point",
		"GL_OES_fragment_precision_high",
		"GL_OES_framebuffer_object",
		"GL_OES_get_program_binary",
		"GL_OES_mapbuffer",
		"GL_OES_matrix_get",
		"GL_OES_matrix_palette",
		"GL_OES_packed_depth_stencil",
		"GL_OES_point_size_array",
		"GL_OES_point_sprite",
		"GL_OES_query_matrix",
		"GL_OES_read_format",
		"GL_OES_required_internalformat",
		"GL_OES_rgb8_rgba8",
		"GL_OES_single_precision",
		"GL_OES_standard_derivatives",
		"GL_OES_stencil1",
		"GL_OES_stencil4",
		"GL_OES_stencil8",
		"GL_OES_stencil_wrap",
		"GL_OES_surfaceless_context",
		"GL_OES_texture_3D",
		"GL_OES_texture_cube_map",
		"GL_OES_texture_env_crossbar",
		"GL_OES_texture_float",
		"GL_OES_texture_float_linear",
		"GL_OES_texture_half_float",
		"GL_OES_texture_half_float_linear",
		"GL_OES_texture_mirrored_repeat",
		"GL_OES_texture_npot",
		"GL_OES_vertex_array_object",
		"GL_OES_vertex_half_float",
		"GL_OES_vertex_type_10_10_10_2",
		"GL_QCOM_alpha_test",
		"GL_QCOM_binning_control",
		"GL_QCOM_driver_control",
		"GL_QCOM_extended_get",
		"GL_QCOM_extended_get2",
		"GL_QCOM_performance_monitor_global_mode",
		"GL_QCOM_tiled_rendering",
		"GL_QCOM_writeonly_rendering",
		"GL_SUN_multi_draw_arrays",
		"GL_VIV_shader_binary"
	};


	COGLES2ExtensionHandler::COGLES2ExtensionHandler() :
			Version(0), MaxAnisotropy(1), MaxIndices(0xffff),
			MaxTextureSize(1), MaxTextureLODBias(0.f),
			StencilBuffer(false)
	{
		for (u32 i = 0; i < IRR_OGLES2_Feature_Count; ++i)
			FeatureAvailable[i] = false;

		DimAliasedLine[0] = 1.f;
		DimAliasedLine[1] = 1.f;
		DimAliasedPoint[0] = 1.f;
		DimAliasedPoint[1] = 1.f;
	}


	void COGLES2ExtensionHandler::dump() const
	{
		for (u32 i = 0; i < IRR_OGLES2_Feature_Count; ++i)
			os::Printer::log(OGLES2FeatureStrings[i], FeatureAvailable[i] ? " true" : " false");
	}


	void COGLES2ExtensionHandler::initExtensions()
	{
		Version = 0;
		s32 multiplier = 100;

		core::stringc version(glGetString(GL_VERSION));

		for (u32 i = 0; i < version.size(); ++i)
		{
			if (version[i] >= '0' && version[i] <= '9')
			{
				if (multiplier > 1)
				{
					Version += static_cast<u16>(core::floor32(core::fast_atof(&(version[i]))) * multiplier);
					multiplier /= 10;
				}
				else
				{
					break;
				}
			}
		}

		core::stringc extensions = glGetString(GL_EXTENSIONS);
		os::Printer::log(extensions.c_str());

		// typo in the simulator (note the postfixed s)
		if (extensions.find("GL_IMG_user_clip_planes"))
			FeatureAvailable[IRR_IMG_user_clip_plane] = true;
		
		{
			const u32 size = extensions.size() + 1;
			c8* str = new c8[size];
			strncpy(str, extensions.c_str(), extensions.size());
			str[extensions.size()] = ' ';
			c8* p = str;

			for (u32 i=0; i<size; ++i)
			{
				if (str[i] == ' ')
				{
					str[i] = 0;
					if (*p)
						for (u32 j=0; j<IRR_OGLES2_Feature_Count; ++j)
						{
							if (!strcmp(OGLES2FeatureStrings[j], p))
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

		GLint val=0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &val);
		Feature.TextureUnit = static_cast<u8>(val);

	#ifdef GL_EXT_texture_filter_anisotropic
		if (FeatureAvailable[IRR_EXT_texture_filter_anisotropic])
		{
			glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &val);
			MaxAnisotropy = static_cast<u8>(val);
		}
	#endif
	#ifdef GL_MAX_ELEMENTS_INDICES
		glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &val);
		MaxIndices=val;
	#endif
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val);
		MaxTextureSize=static_cast<u32>(val);
	#ifdef GL_EXT_texture_lod_bias
		if (FeatureAvailable[IRR_EXT_texture_lod_bias])
			glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS_EXT, &MaxTextureLODBias);
	#endif
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, DimAliasedLine);
		glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, DimAliasedPoint);

		Feature.TextureUnit = core::min_(Feature.TextureUnit, static_cast<u8>(MATERIAL_MAX_TEXTURES));
		Feature.ColorAttachment = 1;
	}

	const COpenGLCoreFeature& COGLES2ExtensionHandler::getFeature() const
	{
		return Feature;
	}

} // end namespace video
} // end namespace irr


#endif // _IRR_COMPILE_WITH_OGLES2_
