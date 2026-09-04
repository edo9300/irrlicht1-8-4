#ifndef __WINE_D3DX9CORE_H
#define __WINE_D3DX9CORE_H
/*
 * Copyright 2008 Luis Busquets
 * Copyright 2014 Kai Tietz
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#undef INTERFACE
#include "d3d9.h"
#include "d3d9types.h"

#define D3DX_VERSION 0x0902
#ifndef D3DX_SDK_VERSION
#define D3DX_SDK_VERSION 43
#endif

typedef const char* D3DXHANDLE;
typedef D3DXHANDLE* LPD3DXHANDLE;

typedef enum _D3DXREGISTER_SET {
	D3DXRS_BOOL,
	D3DXRS_INT4,
	D3DXRS_FLOAT4,
	D3DXRS_SAMPLER,
	D3DXRS_FORCE_DWORD = 0x7fffffff
} D3DXREGISTER_SET, * LPD3DXREGISTER_SET;

typedef struct _D3DXCONSTANTTABLE_DESC {
	const char* Creator;
	DWORD Version;
	UINT Constants;
} D3DXCONSTANTTABLE_DESC, * LPD3DXCONSTANTTABLE_DESC;

typedef enum D3DXPARAMETER_CLASS {
	D3DXPC_SCALAR,
	D3DXPC_VECTOR,
	D3DXPC_MATRIX_ROWS,
	D3DXPC_MATRIX_COLUMNS,
	D3DXPC_OBJECT,
	D3DXPC_STRUCT,
	D3DXPC_FORCE_DWORD = 0x7fffffff,
} D3DXPARAMETER_CLASS, * LPD3DXPARAMETER_CLASS;

typedef enum D3DXPARAMETER_TYPE {
	D3DXPT_VOID,
	D3DXPT_BOOL,
	D3DXPT_INT,
	D3DXPT_FLOAT,
	D3DXPT_STRING,
	D3DXPT_TEXTURE,
	D3DXPT_TEXTURE1D,
	D3DXPT_TEXTURE2D,
	D3DXPT_TEXTURE3D,
	D3DXPT_TEXTURECUBE,
	D3DXPT_SAMPLER,
	D3DXPT_SAMPLER1D,
	D3DXPT_SAMPLER2D,
	D3DXPT_SAMPLER3D,
	D3DXPT_SAMPLERCUBE,
	D3DXPT_PIXELSHADER,
	D3DXPT_VERTEXSHADER,
	D3DXPT_PIXELFRAGMENT,
	D3DXPT_VERTEXFRAGMENT,
	D3DXPT_UNSUPPORTED,
	D3DXPT_FORCE_DWORD = 0x7fffffff,
} D3DXPARAMETER_TYPE, * LPD3DXPARAMETER_TYPE;

typedef struct _D3DXCONSTANT_DESC {
	const char* Name;
	D3DXREGISTER_SET RegisterSet;
	UINT RegisterIndex;
	UINT RegisterCount;
	D3DXPARAMETER_CLASS Class;
	D3DXPARAMETER_TYPE Type;
	UINT Rows;
	UINT Columns;
	UINT Elements;
	UINT StructMembers;
	UINT Bytes;
	const void* DefaultValue;
} D3DXCONSTANT_DESC, * LPD3DXCONSTANT_DESC;

typedef struct D3DXVECTOR2 {
#ifdef __cplusplus
	D3DXVECTOR2();
	D3DXVECTOR2(const FLOAT* pf);
	D3DXVECTOR2(FLOAT fx, FLOAT fy);

	operator FLOAT* ();
	operator const FLOAT* () const;

	D3DXVECTOR2& operator += (const D3DXVECTOR2&);
	D3DXVECTOR2& operator -= (const D3DXVECTOR2&);
	D3DXVECTOR2& operator *= (FLOAT);
	D3DXVECTOR2& operator /= (FLOAT);

	D3DXVECTOR2 operator + () const;
	D3DXVECTOR2 operator - () const;

	D3DXVECTOR2 operator + (const D3DXVECTOR2&) const;
	D3DXVECTOR2 operator - (const D3DXVECTOR2&) const;
	D3DXVECTOR2 operator * (FLOAT) const;
	D3DXVECTOR2 operator / (FLOAT) const;

	friend D3DXVECTOR2 operator * (FLOAT, const D3DXVECTOR2&);

	BOOL operator == (const D3DXVECTOR2&) const;
	BOOL operator != (const D3DXVECTOR2&) const;
#endif /* __cplusplus */
	FLOAT x, y;
} D3DXVECTOR2, * LPD3DXVECTOR2;


#ifdef __cplusplus
typedef struct D3DXMATRIX : public D3DMATRIX {
	D3DXMATRIX();
	D3DXMATRIX(const FLOAT* pf);
	D3DXMATRIX(const D3DMATRIX& mat)
	{
		memcpy(&_11, &mat, sizeof(D3DXMATRIX));
	}
	D3DXMATRIX(FLOAT f11, FLOAT f12, FLOAT f13, FLOAT f14,
			   FLOAT f21, FLOAT f22, FLOAT f23, FLOAT f24,
			   FLOAT f31, FLOAT f32, FLOAT f33, FLOAT f34,
			   FLOAT f41, FLOAT f42, FLOAT f43, FLOAT f44);

	FLOAT& operator () (UINT row, UINT col);
	FLOAT operator () (UINT row, UINT col) const;

	operator FLOAT* ();
	operator const FLOAT* () const;

	D3DXMATRIX& operator *= (const D3DXMATRIX&);
	D3DXMATRIX& operator += (const D3DXMATRIX&);
	D3DXMATRIX& operator -= (const D3DXMATRIX&);
	D3DXMATRIX& operator *= (FLOAT);
	D3DXMATRIX& operator /= (FLOAT);

	D3DXMATRIX operator + () const;
	D3DXMATRIX operator - () const;

	D3DXMATRIX operator * (const D3DXMATRIX&) const;
	D3DXMATRIX operator + (const D3DXMATRIX&) const;
	D3DXMATRIX operator - (const D3DXMATRIX&) const;
	D3DXMATRIX operator * (FLOAT) const;
	D3DXMATRIX operator / (FLOAT) const;

	friend D3DXMATRIX operator * (FLOAT, const D3DXMATRIX&);

	BOOL operator == (const D3DXMATRIX&) const;
	BOOL operator != (const D3DXMATRIX&) const;
} D3DXMATRIX, * LPD3DXMATRIX;
#else /* !__cplusplus */
typedef struct _D3DMATRIX D3DXMATRIX, * LPD3DXMATRIX;
#endif /* !__cplusplus */

#ifdef __cplusplus
typedef struct D3DXVECTOR3 : public D3DVECTOR {
	D3DXVECTOR3() {}
	D3DXVECTOR3(const FLOAT* pf);
	D3DXVECTOR3(const D3DVECTOR& v);
	D3DXVECTOR3(FLOAT fx, FLOAT fy, FLOAT fz)
	{
		x = fx;
		y = fy;
		z = fz;
	}

	operator FLOAT* ();
	operator const FLOAT* () const;

	D3DXVECTOR3& operator += (const D3DXVECTOR3&);
	D3DXVECTOR3& operator -= (const D3DXVECTOR3&);
	D3DXVECTOR3& operator *= (FLOAT);
	D3DXVECTOR3& operator /= (FLOAT);

	D3DXVECTOR3 operator + () const;
	D3DXVECTOR3 operator - () const;

	D3DXVECTOR3 operator + (const D3DXVECTOR3&) const;
	D3DXVECTOR3 operator - (const D3DXVECTOR3&) const;
	D3DXVECTOR3 operator * (FLOAT) const;
	D3DXVECTOR3 operator / (FLOAT) const;

	friend D3DXVECTOR3 operator * (FLOAT, const struct D3DXVECTOR3&);

	BOOL operator == (const D3DXVECTOR3&) const;
	BOOL operator != (const D3DXVECTOR3&) const;
} D3DXVECTOR3, * LPD3DXVECTOR3;
#else /* !__cplusplus */
typedef struct _D3DVECTOR D3DXVECTOR3, * LPD3DXVECTOR3;
#endif /* !__cplusplus */


typedef struct D3DXVECTOR4 {
#ifdef __cplusplus
	D3DXVECTOR4();
	D3DXVECTOR4(const FLOAT* pf);
	D3DXVECTOR4(FLOAT fx, FLOAT fy, FLOAT fz, FLOAT fw);

	operator FLOAT* ();
	operator const FLOAT* () const;

	D3DXVECTOR4& operator += (const D3DXVECTOR4&);
	D3DXVECTOR4& operator -= (const D3DXVECTOR4&);
	D3DXVECTOR4& operator *= (FLOAT);
	D3DXVECTOR4& operator /= (FLOAT);

	D3DXVECTOR4 operator + () const;
	D3DXVECTOR4 operator - () const;

	D3DXVECTOR4 operator + (const D3DXVECTOR4&) const;
	D3DXVECTOR4 operator - (const D3DXVECTOR4&) const;
	D3DXVECTOR4 operator * (FLOAT) const;
	D3DXVECTOR4 operator / (FLOAT) const;

	friend D3DXVECTOR4 operator * (FLOAT, const D3DXVECTOR4&);

	BOOL operator == (const D3DXVECTOR4&) const;
	BOOL operator != (const D3DXVECTOR4&) const;
#endif /* __cplusplus */
	FLOAT x, y, z, w;
} D3DXVECTOR4, * LPD3DXVECTOR4;

#undef INTERFACE
#define INTERFACE ID3DXLine

DECLARE_INTERFACE_(ID3DXLine, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // ID3DXLine
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE9* ppDevice) PURE;

    STDMETHOD(Begin)(THIS) PURE;

    STDMETHOD(Draw)(THIS_ CONST D3DXVECTOR2 *pVertexList,
        DWORD dwVertexListCount, D3DCOLOR Color) PURE;

    STDMETHOD(DrawTransform)(THIS_ CONST D3DXVECTOR3 *pVertexList,
        DWORD dwVertexListCount, CONST D3DXMATRIX* pTransform,
        D3DCOLOR Color) PURE;

    STDMETHOD(SetPattern)(THIS_ DWORD dwPattern) PURE;
    STDMETHOD_(DWORD, GetPattern)(THIS) PURE;

    STDMETHOD(SetPatternScale)(THIS_ FLOAT fPatternScale) PURE;
    STDMETHOD_(FLOAT, GetPatternScale)(THIS) PURE;

    STDMETHOD(SetWidth)(THIS_ FLOAT fWidth) PURE;
    STDMETHOD_(FLOAT, GetWidth)(THIS) PURE;

    STDMETHOD(SetAntialias)(THIS_ BOOL bAntialias) PURE;
    STDMETHOD_(BOOL, GetAntialias)(THIS) PURE;

    STDMETHOD(SetGLLines)(THIS_ BOOL bGLLines) PURE;
    STDMETHOD_(BOOL, GetGLLines)(THIS) PURE;

    STDMETHOD(End)(THIS) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};
#undef INTERFACE

#define INTERFACE ID3DXBuffer
DECLARE_INTERFACE_(ID3DXBuffer, IUnknown) {
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** out) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	/*** ID3DXBuffer methods ***/
	STDMETHOD_(void*, GetBufferPointer)(THIS) PURE;
	STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;
};
#undef INTERFACE

#define INTERFACE ID3DXConstantTable

DECLARE_INTERFACE_(ID3DXConstantTable, ID3DXBuffer) {
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, void** out) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	/*** ID3DXBuffer methods ***/
	STDMETHOD_(void*, GetBufferPointer)(THIS) PURE;
	STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;
	/*** ID3DXConstantTable methods ***/
	STDMETHOD(GetDesc)(THIS_ D3DXCONSTANTTABLE_DESC * pDesc) PURE;
	STDMETHOD(GetConstantDesc)(THIS_ D3DXHANDLE hConstant, D3DXCONSTANT_DESC * pConstantDesc, UINT * pCount) PURE;
	STDMETHOD_(UINT, GetSamplerIndex)(THIS_ D3DXHANDLE hConstant) PURE;
	STDMETHOD_(D3DXHANDLE, GetConstant)(THIS_ D3DXHANDLE hConstant, UINT Index) PURE;
	STDMETHOD_(D3DXHANDLE, GetConstantByName)(THIS_ D3DXHANDLE constant, const char* name) PURE;
	STDMETHOD_(D3DXHANDLE, GetConstantElement)(THIS_ D3DXHANDLE hConstant, UINT Index) PURE;
	STDMETHOD(SetDefaults)(THIS_ struct IDirect3DDevice9* device) PURE;
	STDMETHOD(SetValue)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
						const void* data, UINT data_size) PURE;
	STDMETHOD(SetBool)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant, BOOL value) PURE;
	STDMETHOD(SetBoolArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
							const BOOL * values, UINT value_count) PURE;
	STDMETHOD(SetInt)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant, INT value) PURE;
	STDMETHOD(SetIntArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
						   const INT * values, UINT value_count) PURE;
	STDMETHOD(SetFloat)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant, float value) PURE;
	STDMETHOD(SetFloatArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
							 const float* values, UINT value_count) PURE;
	STDMETHOD(SetVector)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant, const D3DXVECTOR4 * value) PURE;
	STDMETHOD(SetVectorArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
							  const D3DXVECTOR4 * values, UINT value_count) PURE;
	STDMETHOD(SetMatrix)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant, const D3DXMATRIX * value) PURE;
	STDMETHOD(SetMatrixArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
							  const D3DXMATRIX * values, UINT value_count) PURE;
	STDMETHOD(SetMatrixPointerArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
									 const D3DXMATRIX * *values, UINT value_count) PURE;
	STDMETHOD(SetMatrixTranspose)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
								  const D3DXMATRIX * value) PURE;
	STDMETHOD(SetMatrixTransposeArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
									   const D3DXMATRIX * values, UINT value_count) PURE;
	STDMETHOD(SetMatrixTransposePointerArray)(THIS_ struct IDirect3DDevice9* device, D3DXHANDLE constant,
											  const D3DXMATRIX * *values, UINT value_count) PURE;
};


typedef struct ID3DXLine* LPD3DXLINE;
typedef struct ID3DXConstantTable* LPD3DXCONSTANTTABLE;

typedef struct _D3DXMACRO {
	const char* Name;
	const char* Definition;
} D3DXMACRO, * LPD3DXMACRO;

typedef struct ID3DXInclude* LPD3DXINCLUDE;
typedef interface ID3DXBuffer* LPD3DXBUFFER;


#endif
