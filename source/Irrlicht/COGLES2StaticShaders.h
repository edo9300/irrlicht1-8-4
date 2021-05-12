#ifndef __C_OGLES2_STATIC_SHADERS_H_INCLUDED__
#define __C_OGLES2_STATIC_SHADERS_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_
#include "irrTypes.h"

namespace irr
{
namespace video
{
extern const irr::c8* COGLES2Solid_vsh;
extern const irr::c8* COGLES2Solid_fsh;

extern const irr::c8* COGLES2Solid2_vsh;
extern const irr::c8* COGLES2Solid2Layer_fsh;
extern const irr::c8* COGLES2LightmapModulate_fsh;
extern const irr::c8* COGLES2LightmapAdd_fsh;
extern const irr::c8* COGLES2DetailMap_fsh;
extern const irr::c8* COGLES2OneTextureBlend_fsh;

extern const irr::c8* COGLES2SphereMap_vsh;
extern const irr::c8* COGLES2SphereMap_fsh;

extern const irr::c8* COGLES2Reflection2Layer_vsh;
extern const irr::c8* COGLES2Reflection2Layer_fsh;

extern const irr::c8* COGLES2TransparentAlphaChannel_fsh;
extern const irr::c8* COGLES2TransparentAlphaChannelRef_fsh;
extern const irr::c8* COGLES2TransparentVertexAlpha_fsh;

extern const irr::c8* COGLES2NormalMap_vsh;
extern const irr::c8* COGLES2NormalMap_fsh;

extern const irr::c8* COGLES2ParallaxMap_vsh;
extern const irr::c8* COGLES2ParallaxMap_fsh;

extern const irr::c8* COGLES2Renderer2D_vsh;
extern const irr::c8* COGLES2Renderer2D_fsh;
extern const irr::c8* COGLES2Renderer2D_noTex_fsh;

} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_OGLES2_

#endif // __C_OGLES2_STATIC_SHADERS_H_INCLUDED__
