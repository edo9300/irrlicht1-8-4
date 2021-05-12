#include "COGLES2StaticShaders.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

namespace irr
{
namespace video
{
const irr::c8* COGLES2Solid_vsh =
R"(#define MAX_LIGHTS 8

/* Attributes */

attribute vec3 inVertexPosition;
attribute vec3 inVertexNormal;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;

/* Uniforms */

uniform mat4 uWVPMatrix;
uniform mat4 uWVMatrix;
uniform mat4 uNMatrix;
uniform mat4 uTMatrix0;

uniform vec4 uGlobalAmbient;
uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialDiffuse;
uniform vec4 uMaterialEmissive;
uniform vec4 uMaterialSpecular;
uniform float uMaterialShininess;

uniform int uLightCount;
uniform int uLightType[MAX_LIGHTS];
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec3 uLightDirection[MAX_LIGHTS];
uniform vec3 uLightAttenuation[MAX_LIGHTS];
uniform vec4 uLightAmbient[MAX_LIGHTS];
uniform vec4 uLightDiffuse[MAX_LIGHTS];
uniform vec4 uLightSpecular[MAX_LIGHTS];

uniform float uThickness;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

void dirLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = normalize(-(uNMatrix * vec4(uLightDirection[index], 0.0)).xyz);

	ambient += uLightAmbient[index];

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor;
	}
}

void pointLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = uLightPosition[index] - position;
	float D = length(L);
	L = normalize(L);

	float Attenuation = 1.0 / (uLightAttenuation[index].x + uLightAttenuation[index].y * D +
		uLightAttenuation[index].z * D * D);

	ambient += uLightAmbient[index] * Attenuation;

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL * Attenuation;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor * Attenuation;
	}
}

void spotLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	// TO-DO
}

void main()
{
	gl_Position = uWVPMatrix * vec4(inVertexPosition, 1.0);
	gl_PointSize = uThickness;

	vec4 TextureCoord0 = vec4(inTexCoord0.x, inTexCoord0.y, 1.0, 1.0);
	vTextureCoord0 = vec4(uTMatrix0 * TextureCoord0).xy;

	vVertexColor = inVertexColor.bgra;
	vSpecularColor = vec4(0.0, 0.0, 0.0, 0.0);

	vec3 Position = (uWVMatrix * vec4(inVertexPosition, 1.0)).xyz;

	if (uLightCount > 0)
	{
		vec3 Normal = normalize((uNMatrix * vec4(inVertexNormal, 0.0)).xyz);

		vec4 Ambient = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 Diffuse = vec4(0.0, 0.0, 0.0, 0.0);

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )	// can't use uniform as loop-counter directly in glsl 
				break;
			if (uLightType[i] == 0)
				pointLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )	
				break;
			if (uLightType[i] == 1)
				spotLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )	
				break;
			if (uLightType[i] == 2)
				dirLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		vec4 LightColor = Ambient * uMaterialAmbient + Diffuse * uMaterialDiffuse;
		LightColor = clamp(LightColor, 0.0, 1.0);
		LightColor.w = 1.0;

		vVertexColor *= LightColor;
		vVertexColor += uMaterialEmissive;
		vVertexColor += uGlobalAmbient * uMaterialAmbient;
		vVertexColor = clamp(vVertexColor, 0.0, 1.0);
		
		vSpecularColor *= uMaterialSpecular;
	}

	vFogCoord = length(Position);
}
)";

const irr::c8* COGLES2Solid_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color = vVertexColor;

	if (bool(uTextureUsage0))
		Color *= texture2D(uTextureUnit0, vTextureCoord0);
	Color += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		Color = mix(FogColor, Color, FogFactor);
	}

	gl_FragColor = Color;
}
)";

const irr::c8* COGLES2Solid2_vsh =
R"(#define MAX_LIGHTS 8

/* Attributes */

attribute vec3 inVertexPosition;
attribute vec3 inVertexNormal;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;
attribute vec2 inTexCoord1;

/* Uniforms */

uniform mat4 uWVPMatrix;
uniform mat4 uWVMatrix;
uniform mat4 uNMatrix;
uniform mat4 uTMatrix0;
uniform mat4 uTMatrix1;

uniform vec4 uGlobalAmbient;
uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialDiffuse;
uniform vec4 uMaterialEmissive;
uniform vec4 uMaterialSpecular;
uniform float uMaterialShininess;

uniform int uLightCount;
uniform int uLightType[MAX_LIGHTS];
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec3 uLightDirection[MAX_LIGHTS];
uniform vec3 uLightAttenuation[MAX_LIGHTS];
uniform vec4 uLightAmbient[MAX_LIGHTS];
uniform vec4 uLightDiffuse[MAX_LIGHTS];
uniform vec4 uLightSpecular[MAX_LIGHTS];

uniform float uThickness;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

void dirLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = normalize(-(uNMatrix * vec4(uLightDirection[index], 0.0)).xyz);

	ambient += uLightAmbient[index];

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor;
	}
}

void pointLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = uLightPosition[index] - position;
	float D = length(L);
	L = normalize(L);

	float Attenuation = 1.0 / (uLightAttenuation[index].x + uLightAttenuation[index].y * D +
		uLightAttenuation[index].z * D * D);

	ambient += uLightAmbient[index] * Attenuation;

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL * Attenuation;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor * Attenuation;
	}
}

void spotLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	// TO-DO
}

void main()
{
	gl_Position = uWVPMatrix * vec4(inVertexPosition, 1.0);
	gl_PointSize = uThickness;

	vec4 TextureCoord0 = vec4(inTexCoord0.x, inTexCoord0.y, 1.0, 1.0);
	vTextureCoord0 = vec4(uTMatrix0 * TextureCoord0).xy;

	vec4 TextureCoord1 = vec4(inTexCoord1.x, inTexCoord1.y, 1.0, 1.0);
	vTextureCoord1 = vec4(uTMatrix1 * TextureCoord1).xy;

	vVertexColor = inVertexColor.bgra;
	vSpecularColor = vec4(0.0, 0.0, 0.0, 0.0);

	vec3 Position = (uWVMatrix * vec4(inVertexPosition, 1.0)).xyz;

	if (uLightCount > 0)
	{
		vec3 Normal = normalize((uNMatrix * vec4(inVertexNormal, 0.0)).xyz);

		vec4 Ambient = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 Diffuse = vec4(0.0, 0.0, 0.0, 0.0);

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )	// can't use uniform as loop-counter directly in glsl 
				break;
			if (uLightType[i] == 0)
				pointLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )
				break;
			if (uLightType[i] == 1)
				spotLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )
				break;
			if (uLightType[i] == 2)
				dirLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		vec4 LightColor = Ambient * uMaterialAmbient + Diffuse * uMaterialDiffuse;
		LightColor = clamp(LightColor, 0.0, 1.0);
		LightColor.w = 1.0;

		vVertexColor *= LightColor;
		vVertexColor += uMaterialEmissive;
		vVertexColor += uGlobalAmbient * uMaterialAmbient;
		vVertexColor = clamp(vVertexColor, 0.0, 1.0);
		
		vSpecularColor *= uMaterialSpecular;
	}

	vFogCoord = length(Position);
}
)";

const irr::c8* COGLES2Solid2Layer_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform int uTextureUsage1;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color0 = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 Color1 = vec4(1.0, 1.0, 1.0, 1.0);

	if (bool(uTextureUsage0))
		Color0 = texture2D(uTextureUnit0, vTextureCoord0);

	if (bool(uTextureUsage1))
		Color1 = texture2D(uTextureUnit1, vTextureCoord1);

	vec4 FinalColor = (Color0 * vVertexColor.a + Color1 * (1.0 - vVertexColor.a)) * vVertexColor;
	FinalColor += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}

	gl_FragColor = FinalColor;

}
)";

const irr::c8* COGLES2LightmapModulate_fsh =
R"(precision mediump float;

/* Uniforms */

uniform float uModulate;
uniform int uTextureUsage0;
uniform int uTextureUsage1;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color0 = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 Color1 = vec4(1.0, 1.0, 1.0, 1.0);

	if (bool(uTextureUsage0))
		Color0 = texture2D(uTextureUnit0, vTextureCoord0);

	if (bool(uTextureUsage1))
		Color1 = texture2D(uTextureUnit1, vTextureCoord1);

	vec4 FinalColor = (Color0 * Color1 * uModulate) * vVertexColor;
	FinalColor += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}
	
	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2LightmapAdd_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform int uTextureUsage1;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color0 = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 Color1 = vec4(1.0, 1.0, 1.0, 1.0);

	if (bool(uTextureUsage0))
		Color0 = texture2D(uTextureUnit0, vTextureCoord0);

	if (bool(uTextureUsage1))
		Color1 = texture2D(uTextureUnit1, vTextureCoord1);

	vec4 FinalColor = (Color0 + Color1) * vVertexColor + vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}
	
	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2DetailMap_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform int uTextureUsage1;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color0 = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 Color1 = vec4(1.0, 1.0, 1.0, 1.0);

	if (bool(uTextureUsage0))
		Color0 = texture2D(uTextureUnit0, vTextureCoord0);

	if (bool(uTextureUsage1))
		Color1 = texture2D(uTextureUnit1, vTextureCoord1);

	vec4 FinalColor = vec4(Color0 + (Color1 - 0.5)) * vVertexColor + vSpecularColor;
	
	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}
	
	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2OneTextureBlend_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform int uBlendType;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color0 = vVertexColor;
	vec4 Color1 = vec4(1.0, 1.0, 1.0, 1.0);

	if (bool(uTextureUsage0))
		Color1 = texture2D(uTextureUnit0, vTextureCoord0);

	vec4 FinalColor = Color0 * Color1;
	FinalColor += vSpecularColor;

	if (uBlendType == 1)
	{
		FinalColor.w = Color0.w;
	}
	else if (uBlendType == 2)
	{
		FinalColor.w = Color1.w;
	}

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}

	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2SphereMap_vsh =
R"(#define MAX_LIGHTS 8

/* Attributes */

attribute vec3 inVertexPosition;
attribute vec3 inVertexNormal;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;
attribute vec2 inTexCoord1;

/* Uniforms */

uniform mat4 uWVPMatrix;
uniform mat4 uWVMatrix;
uniform mat4 uNMatrix;

uniform vec4 uGlobalAmbient;
uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialDiffuse;
uniform vec4 uMaterialEmissive;
uniform vec4 uMaterialSpecular;
uniform float uMaterialShininess;

uniform int uLightCount;
uniform int uLightType[MAX_LIGHTS];
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec3 uLightDirection[MAX_LIGHTS];
uniform vec3 uLightAttenuation[MAX_LIGHTS];
uniform vec4 uLightAmbient[MAX_LIGHTS];
uniform vec4 uLightDiffuse[MAX_LIGHTS];
uniform vec4 uLightSpecular[MAX_LIGHTS];

uniform float uThickness;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

void dirLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = normalize(-(uNMatrix * vec4(uLightDirection[index], 0.0)).xyz);

	ambient += uLightAmbient[index];

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor;
	}
}

void pointLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = uLightPosition[index] - position;
	float D = length(L);
	L = normalize(L);

	float Attenuation = 1.0 / (uLightAttenuation[index].x + uLightAttenuation[index].y * D +
		uLightAttenuation[index].z * D * D);

	ambient += uLightAmbient[index] * Attenuation;

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL * Attenuation;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor * Attenuation;
	}
}

void spotLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	// TO-DO
}

void main()
{
	gl_Position = uWVPMatrix * vec4(inVertexPosition, 1.0);
	gl_PointSize = uThickness;

	vec3 Position = (uWVMatrix * vec4(inVertexPosition, 1.0)).xyz;
	vec3 P = normalize(Position);
	vec3 N = normalize(vec4(uNMatrix * vec4(inVertexNormal, 0.0)).xyz);
	vec3 R = reflect(P, N);

	float V = 2.0 * sqrt(R.x*R.x + R.y*R.y + (R.z+1.0)*(R.z+1.0));
	vTextureCoord0 = vec2(R.x/V + 0.5, R.y/V + 0.5);

	vVertexColor = inVertexColor.bgra;
	vSpecularColor = vec4(0.0, 0.0, 0.0, 0.0);

	if (uLightCount > 0)
	{
		vec3 Normal = normalize((uNMatrix * vec4(inVertexNormal, 0.0)).xyz);

		vec4 Ambient = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 Diffuse = vec4(0.0, 0.0, 0.0, 0.0);

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )	// can't use uniform as loop-counter directly in glsl 
				break;
			if (uLightType[i] == 0)
				pointLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )
				break;
			if (uLightType[i] == 1)
				spotLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )
				break;
			if (uLightType[i] == 2)
				dirLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		vec4 LightColor = Ambient * uMaterialAmbient + Diffuse * uMaterialDiffuse;
		LightColor = clamp(LightColor, 0.0, 1.0);
		LightColor.w = 1.0;

		vVertexColor *= LightColor;
		vVertexColor += uMaterialEmissive;
		vVertexColor += uGlobalAmbient * uMaterialAmbient;
		vVertexColor = clamp(vVertexColor, 0.0, 1.0);
		
		vSpecularColor *= uMaterialSpecular;
	}

	vFogCoord = length(Position);
}
)";

const irr::c8* COGLES2SphereMap_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color = vVertexColor;

	if (bool(uTextureUsage0))
		Color *= texture2D(uTextureUnit0, vTextureCoord0);
	Color += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		Color = mix(FogColor, Color, FogFactor);
	}

	gl_FragColor = Color;
}
)";

const irr::c8* COGLES2Reflection2Layer_vsh =
R"(#define MAX_LIGHTS 8

/* Attributes */

attribute vec3 inVertexPosition;
attribute vec3 inVertexNormal;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;
attribute vec2 inTexCoord1;

/* Uniforms */

uniform mat4 uWVPMatrix;
uniform mat4 uWVMatrix;
uniform mat4 uNMatrix;
uniform mat4 uTMatrix0;

uniform vec4 uGlobalAmbient;
uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialDiffuse;
uniform vec4 uMaterialEmissive;
uniform vec4 uMaterialSpecular;
uniform float uMaterialShininess;

uniform int uLightCount;
uniform int uLightType[MAX_LIGHTS];
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec3 uLightDirection[MAX_LIGHTS];
uniform vec3 uLightAttenuation[MAX_LIGHTS];
uniform vec4 uLightAmbient[MAX_LIGHTS];
uniform vec4 uLightDiffuse[MAX_LIGHTS];
uniform vec4 uLightSpecular[MAX_LIGHTS];

uniform float uThickness;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

void dirLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = normalize(-(uNMatrix * vec4(uLightDirection[index], 0.0)).xyz);

	ambient += uLightAmbient[index];

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor;
	}
}

void pointLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	vec3 L = uLightPosition[index] - position;
	float D = length(L);
	L = normalize(L);

	float Attenuation = 1.0 / (uLightAttenuation[index].x + uLightAttenuation[index].y * D +
		uLightAttenuation[index].z * D * D);

	ambient += uLightAmbient[index] * Attenuation;

	float NdotL = dot(normal, L);

	if (NdotL > 0.0)
	{
		diffuse += uLightDiffuse[index] * NdotL * Attenuation;

		vec3 E = normalize(-position); 
		vec3 HalfVector = normalize(L + E);
		float NdotH = max(0.0, dot(normal, HalfVector));

		float SpecularFactor = pow(NdotH, uMaterialShininess);
		specular += uLightSpecular[index] * SpecularFactor * Attenuation;
	}
}

void spotLight(in int index, in vec3 position, in vec3 normal, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
	// TO-DO
}

void main()
{
	gl_Position = uWVPMatrix * vec4(inVertexPosition, 1.0);
	gl_PointSize = uThickness;

	vec4 TextureCoord0 = vec4(inTexCoord0.x, inTexCoord0.y, 1.0, 1.0);
	vTextureCoord0 = vec4(uTMatrix0 * TextureCoord0).xy;

	vec3 Position = (uWVMatrix * vec4(inVertexPosition, 1.0)).xyz;
	vec3 P = normalize(Position);
	vec3 N = normalize(vec4(uNMatrix * vec4(inVertexNormal, 0.0)).xyz);
	vec3 R = reflect(P, N);

	float V = 2.0 * sqrt(R.x*R.x + R.y*R.y + (R.z+1.0)*(R.z+1.0));
	vTextureCoord1 = vec2(R.x/V + 0.5, R.y/V + 0.5);

	vVertexColor = inVertexColor.bgra;
	vSpecularColor = vec4(0.0, 0.0, 0.0, 0.0);

	if (uLightCount > 0)
	{
		vec3 Normal = normalize((uNMatrix * vec4(inVertexNormal, 0.0)).xyz);

		vec4 Ambient = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 Diffuse = vec4(0.0, 0.0, 0.0, 0.0);

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )	// can't use uniform as loop-counter directly in glsl 
				break;
			if (uLightType[i] == 0)
				pointLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )
				break;
			if (uLightType[i] == 1)
				spotLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		for (int i = 0; i < int(MAX_LIGHTS); i++)
		{
			if( i >= uLightCount )
				break;
			if (uLightType[i] == 2)
				dirLight(i, Position, Normal, Ambient, Diffuse, vSpecularColor);
		}

		vec4 LightColor = Ambient * uMaterialAmbient + Diffuse * uMaterialDiffuse;
		LightColor = clamp(LightColor, 0.0, 1.0);
		LightColor.w = 1.0;

		vVertexColor *= LightColor;
		vVertexColor += uMaterialEmissive;
		vVertexColor += uGlobalAmbient * uMaterialAmbient;
		vVertexColor = clamp(vVertexColor, 0.0, 1.0);
		
		vSpecularColor *= uMaterialSpecular;
	}

	vFogCoord = length(Position);
}
)";

const irr::c8* COGLES2Reflection2Layer_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform int uTextureUsage1;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec2 vTextureCoord1;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color0 = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 Color1 = vec4(1.0, 1.0, 1.0, 1.0);

	if (bool(uTextureUsage0))
		Color0 = texture2D(uTextureUnit0, vTextureCoord0);

	if (bool(uTextureUsage1))
		Color1 = texture2D(uTextureUnit1, vTextureCoord1);

	vec4 FinalColor = (Color0 * Color1) * vVertexColor + vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}

	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2TransparentAlphaChannel_fsh =
R"(precision mediump float;

/* Uniforms */

uniform float uAlphaRef;
uniform int uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color = vVertexColor;

	if (bool(uTextureUsage0))
	{
		Color *= texture2D(uTextureUnit0, vTextureCoord0);
		
		// TODO: uAlphaRef should rather control sharpness of alpha, don't know how to do that right now and this works in most cases.
		if (Color.a < uAlphaRef)
			discard;
	}
	Color += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		Color = mix(FogColor, Color, FogFactor);
	}

	gl_FragColor = Color;
}
)";

const irr::c8* COGLES2TransparentAlphaChannelRef_fsh =
R"(precision mediump float;

/* Uniforms */

uniform float uAlphaRef;
uniform int uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color = vVertexColor;

	if (bool(uTextureUsage0))
		Color *= texture2D(uTextureUnit0, vTextureCoord0);

	if (Color.a < uAlphaRef)
		discard;
		
	Color += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		Color = mix(FogColor, Color, FogFactor);
	}

	gl_FragColor = Color;
}
)";

		const irr::c8* COGLES2TransparentVertexAlpha_fsh =
			R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTextureCoord0;
varying vec4 vVertexColor;
varying vec4 vSpecularColor;
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color = vVertexColor;

	if (bool(uTextureUsage0))
		Color *= texture2D(uTextureUnit0, vTextureCoord0);
	Color += vSpecularColor;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		Color = mix(FogColor, Color, FogFactor);
	}

	gl_FragColor = Color;
}
)";

const irr::c8* COGLES2NormalMap_vsh =
R"(#define MAX_LIGHTS 2

/* Attributes */

attribute vec3 inVertexPosition;
attribute vec3 inVertexNormal;
attribute vec3 inVertexTangent;
attribute vec3 inVertexBinormal;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;

/* Uniforms */

uniform mat4 uWVPMatrix;
uniform mat4 uWVMatrix;
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS];

/* Varyings */

varying vec2 vTexCoord;
varying vec3 vLightVector[MAX_LIGHTS];
varying vec4 vLightColor[MAX_LIGHTS];
varying float vFogCoord;

void main()
{
	gl_Position = uWVPMatrix * vec4(inVertexPosition, 1.0);

	vTexCoord = inTexCoord0;

	for (int i = 0; i < int(MAX_LIGHTS); i++)
	{
		vec3 LightVector = uLightPosition[i] - inVertexPosition;

		vLightVector[i].x = dot(inVertexTangent, LightVector);
		vLightVector[i].y = dot(inVertexBinormal, LightVector);
		vLightVector[i].z = dot(inVertexNormal, LightVector);

		vLightColor[i].x = dot(LightVector, LightVector);
		vLightColor[i].x *= uLightColor[i].a;
		vLightColor[i] = vec4(inversesqrt(vLightColor[i].x));
		vLightColor[i] *= uLightColor[i];
		vLightColor[i].a = inVertexColor.a;

		vLightColor[i].x = clamp(vLightColor[i].x, 0.0, 1.0);
		vLightColor[i].y = clamp(vLightColor[i].y, 0.0, 1.0);
		vLightColor[i].z = clamp(vLightColor[i].z, 0.0, 1.0);
	}

	vFogCoord = length((uWVMatrix * vec4(inVertexPosition, 1.0)).xyz);
}
)";

const irr::c8* COGLES2NormalMap_fsh =
R"(#define MAX_LIGHTS 2

precision mediump float;

/* Uniforms */

uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTexCoord;
varying vec3 vLightVector[MAX_LIGHTS];
varying vec4 vLightColor[MAX_LIGHTS];
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 Color  = texture2D(uTextureUnit0, vTexCoord);
	vec3 Normal = texture2D(uTextureUnit1, vTexCoord).xyz *  2.0 - 1.0;

	vec4 FinalColor = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < int(MAX_LIGHTS); i++)
	{
		vec3 LightVector = normalize(vLightVector[i]);

		float Lambert = max(dot(LightVector, Normal), 0.0);
		FinalColor += vec4(Lambert) * vLightColor[i];
	}

	FinalColor *= Color;
	FinalColor.w = vLightColor[0].w;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}
	
	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2ParallaxMap_vsh =
R"(#define MAX_LIGHTS 2

/* Attributes */

attribute vec3 inVertexPosition;
attribute vec3 inVertexNormal;
attribute vec3 inVertexTangent;
attribute vec3 inVertexBinormal;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;

/* Uniforms */

uniform mat4 uWVPMatrix;
uniform mat4 uWVMatrix;
uniform vec3 uEyePosition;
uniform vec3 uLightPosition[MAX_LIGHTS];
uniform vec4 uLightColor[MAX_LIGHTS];

/* Varyings */

varying vec2 vTexCoord;
varying vec3 vEyeVector;
varying vec3 vLightVector[MAX_LIGHTS];
varying vec4 vLightColor[MAX_LIGHTS];
varying float vFogCoord;

void main()
{
	gl_Position = uWVPMatrix * vec4(inVertexPosition, 1.0);

	vTexCoord = inTexCoord0;

	vec3 EyeVector = uEyePosition - inVertexPosition;

	vEyeVector.x = dot(inVertexTangent, EyeVector);
	vEyeVector.y = dot(inVertexBinormal, EyeVector);
	vEyeVector.z = dot(inVertexNormal, EyeVector);
	vEyeVector *= vec3(1.0, -1.0, -1.0);

	for (int i = 0; i < int(MAX_LIGHTS); i++)
	{
		vec3 LightVector = uLightPosition[i] - inVertexPosition;

		vLightVector[i].x = dot(inVertexTangent, LightVector);
		vLightVector[i].y = dot(inVertexBinormal, LightVector);
		vLightVector[i].z = dot(inVertexNormal, LightVector);

		vLightColor[i].x = dot(LightVector, LightVector);
		vLightColor[i].x *= uLightColor[i].a;
		vLightColor[i] = vec4(inversesqrt(vLightColor[i].x));
		vLightColor[i] *= uLightColor[i];
		vLightColor[i].a = inVertexColor.a;

		vLightColor[i].x = clamp(vLightColor[i].x, 0.0, 1.0);
		vLightColor[i].y = clamp(vLightColor[i].y, 0.0, 1.0);
		vLightColor[i].z = clamp(vLightColor[i].z, 0.0, 1.0);
	}

	vFogCoord = length((uWVMatrix * vec4(inVertexPosition, 1.0)).xyz);
}
)";

const irr::c8* COGLES2ParallaxMap_fsh =
R"(#define MAX_LIGHTS 2

precision mediump float;

/* Uniforms */

uniform float uFactor;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
uniform int uFogEnable;
uniform int uFogType;
uniform vec4 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

/* Varyings */

varying vec2 vTexCoord;
varying vec3 vEyeVector;
varying vec3 vLightVector[MAX_LIGHTS];
varying vec4 vLightColor[MAX_LIGHTS];
varying float vFogCoord;

float computeFog()
{
	const float LOG2 = 1.442695;
	float FogFactor = 0.0;

	if (uFogType == 0) // Exp
	{
		FogFactor = exp2(-uFogDensity * vFogCoord * LOG2);
	}
	else if (uFogType == 1) // Linear
	{
		float Scale = 1.0 / (uFogEnd - uFogStart);
		FogFactor = (uFogEnd - vFogCoord) * Scale;
	}
	else if (uFogType == 2) // Exp2
	{
		FogFactor = exp2(-uFogDensity * uFogDensity * vFogCoord * vFogCoord * LOG2);
	}

	FogFactor = clamp(FogFactor, 0.0, 1.0);

	return FogFactor;
}

void main()
{
	vec4 TempFetch = texture2D(uTextureUnit1, vTexCoord) *  2.0 - 1.0;
	TempFetch *= uFactor;

	vec3 EyeVector = normalize(vEyeVector);
	vec2 TexCoord = EyeVector.xy * TempFetch.w + vTexCoord;

	vec4 Color  = texture2D(uTextureUnit0, TexCoord);
	vec3 Normal = texture2D(uTextureUnit1, TexCoord).xyz *  2.0 - 1.0;

	vec4 FinalColor = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = 0; i < int(MAX_LIGHTS); i++)
	{
		vec3 LightVector = normalize(vLightVector[i]);

		float Lambert = max(dot(LightVector, Normal), 0.0);
		FinalColor += vec4(Lambert) * vLightColor[i];
	}

	FinalColor *= Color;
	FinalColor.w = vLightColor[0].w;

	if (bool(uFogEnable))
	{
		float FogFactor = computeFog();
		vec4 FogColor = uFogColor;
		FogColor.a = 1.0;
		FinalColor = mix(FogColor, FinalColor, FogFactor);
	}
	
	gl_FragColor = FinalColor;
}
)";

const irr::c8* COGLES2Renderer2D_vsh =
R"(/* Attributes */

attribute vec4 inVertexPosition;
attribute vec4 inVertexColor;
attribute vec2 inTexCoord0;

/* Uniforms */

uniform float uThickness;

/* Varyings */

varying vec2 vTextureCoord;
varying vec4 vVertexColor;

void main()
{
	gl_Position = inVertexPosition;
	gl_PointSize = uThickness;
	vTextureCoord = inTexCoord0;
	vVertexColor = inVertexColor.bgra;
}
)";

const irr::c8* COGLES2Renderer2D_fsh =
R"(precision mediump float;

/* Uniforms */

uniform int uTextureUsage;
uniform sampler2D uTextureUnit;

/* Varyings */

varying vec2 vTextureCoord;
varying vec4 vVertexColor;

void main()
{
	vec4 Color = vVertexColor;

	if (bool(uTextureUsage))
		Color *= texture2D(uTextureUnit, vTextureCoord);

	gl_FragColor = Color;
}
)";

const irr::c8* COGLES2Renderer2D_noTex_fsh =
R"(precision mediump float;

/* Varyings */
varying vec4 vVertexColor;

void main()
{
	gl_FragColor = vVertexColor;
}
)";

} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_OGLES2_
