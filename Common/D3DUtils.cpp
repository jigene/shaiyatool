///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "D3DUtils.h"

using namespace D3D;


//-----------------------------------------------------------------------------
// Name: D3DFormatToString
// Desc: Returns the string for the given D3DFORMAT.
//-----------------------------------------------------------------------------
TCHAR* D3D::D3DUtil_D3DFormatToString(D3DFORMAT format, bool bWithPrefix)
{
	TCHAR* pstr = NULL;
	switch (format)
	{
	case D3DFMT_UNKNOWN:         pstr = TEXT("D3DFMT_UNKNOWN"); break;
	case D3DFMT_R8G8B8:          pstr = TEXT("D3DFMT_R8G8B8"); break;
	case D3DFMT_A8R8G8B8:        pstr = TEXT("D3DFMT_A8R8G8B8"); break;
	case D3DFMT_X8R8G8B8:        pstr = TEXT("D3DFMT_X8R8G8B8"); break;
	case D3DFMT_R5G6B5:          pstr = TEXT("D3DFMT_R5G6B5"); break;
	case D3DFMT_X1R5G5B5:        pstr = TEXT("D3DFMT_X1R5G5B5"); break;
	case D3DFMT_A1R5G5B5:        pstr = TEXT("D3DFMT_A1R5G5B5"); break;
	case D3DFMT_A4R4G4B4:        pstr = TEXT("D3DFMT_A4R4G4B4"); break;
	case D3DFMT_R3G3B2:          pstr = TEXT("D3DFMT_R3G3B2"); break;
	case D3DFMT_A8:              pstr = TEXT("D3DFMT_A8"); break;
	case D3DFMT_A8R3G3B2:        pstr = TEXT("D3DFMT_A8R3G3B2"); break;
	case D3DFMT_X4R4G4B4:        pstr = TEXT("D3DFMT_X4R4G4B4"); break;
	case D3DFMT_A2B10G10R10:     pstr = TEXT("D3DFMT_A2B10G10R10"); break;
	case D3DFMT_A8B8G8R8:        pstr = TEXT("D3DFMT_A8B8G8R8"); break;
	case D3DFMT_X8B8G8R8:        pstr = TEXT("D3DFMT_X8B8G8R8"); break;
	case D3DFMT_G16R16:          pstr = TEXT("D3DFMT_G16R16"); break;
	case D3DFMT_A2R10G10B10:     pstr = TEXT("D3DFMT_A2R10G10B10"); break;
	case D3DFMT_A16B16G16R16:    pstr = TEXT("D3DFMT_A16B16G16R16"); break;
	case D3DFMT_A8P8:            pstr = TEXT("D3DFMT_A8P8"); break;
	case D3DFMT_P8:              pstr = TEXT("D3DFMT_P8"); break;
	case D3DFMT_L8:              pstr = TEXT("D3DFMT_L8"); break;
	case D3DFMT_A8L8:            pstr = TEXT("D3DFMT_A8L8"); break;
	case D3DFMT_A4L4:            pstr = TEXT("D3DFMT_A4L4"); break;
	case D3DFMT_V8U8:            pstr = TEXT("D3DFMT_V8U8"); break;
	case D3DFMT_L6V5U5:          pstr = TEXT("D3DFMT_L6V5U5"); break;
	case D3DFMT_X8L8V8U8:        pstr = TEXT("D3DFMT_X8L8V8U8"); break;
	case D3DFMT_Q8W8V8U8:        pstr = TEXT("D3DFMT_Q8W8V8U8"); break;
	case D3DFMT_V16U16:          pstr = TEXT("D3DFMT_V16U16"); break;
	case D3DFMT_A2W10V10U10:     pstr = TEXT("D3DFMT_A2W10V10U10"); break;
	case D3DFMT_UYVY:            pstr = TEXT("D3DFMT_UYVY"); break;
	case D3DFMT_YUY2:            pstr = TEXT("D3DFMT_YUY2"); break;
	case D3DFMT_DXT1:            pstr = TEXT("D3DFMT_DXT1"); break;
	case D3DFMT_DXT2:            pstr = TEXT("D3DFMT_DXT2"); break;
	case D3DFMT_DXT3:            pstr = TEXT("D3DFMT_DXT3"); break;
	case D3DFMT_DXT4:            pstr = TEXT("D3DFMT_DXT4"); break;
	case D3DFMT_DXT5:            pstr = TEXT("D3DFMT_DXT5"); break;
	case D3DFMT_D16_LOCKABLE:    pstr = TEXT("D3DFMT_D16_LOCKABLE"); break;
	case D3DFMT_D32:             pstr = TEXT("D3DFMT_D32"); break;
	case D3DFMT_D15S1:           pstr = TEXT("D3DFMT_D15S1"); break;
	case D3DFMT_D24S8:           pstr = TEXT("D3DFMT_D24S8"); break;
	case D3DFMT_D24X8:           pstr = TEXT("D3DFMT_D24X8"); break;
	case D3DFMT_D24X4S4:         pstr = TEXT("D3DFMT_D24X4S4"); break;
	case D3DFMT_D16:             pstr = TEXT("D3DFMT_D16"); break;
	case D3DFMT_L16:             pstr = TEXT("D3DFMT_L16"); break;
	case D3DFMT_VERTEXDATA:      pstr = TEXT("D3DFMT_VERTEXDATA"); break;
	case D3DFMT_INDEX16:         pstr = TEXT("D3DFMT_INDEX16"); break;
	case D3DFMT_INDEX32:         pstr = TEXT("D3DFMT_INDEX32"); break;
	case D3DFMT_Q16W16V16U16:    pstr = TEXT("D3DFMT_Q16W16V16U16"); break;
	case D3DFMT_MULTI2_ARGB8:    pstr = TEXT("D3DFMT_MULTI2_ARGB8"); break;
	case D3DFMT_R16F:            pstr = TEXT("D3DFMT_R16F"); break;
	case D3DFMT_G16R16F:         pstr = TEXT("D3DFMT_G16R16F"); break;
	case D3DFMT_A16B16G16R16F:   pstr = TEXT("D3DFMT_A16B16G16R16F"); break;
	case D3DFMT_R32F:            pstr = TEXT("D3DFMT_R32F"); break;
	case D3DFMT_G32R32F:         pstr = TEXT("D3DFMT_G32R32F"); break;
	case D3DFMT_A32B32G32R32F:   pstr = TEXT("D3DFMT_A32B32G32R32F"); break;
	case D3DFMT_CxV8U8:          pstr = TEXT("D3DFMT_CxV8U8"); break;
	default:                     pstr = TEXT("Unknown format"); break;
	}
	if (bWithPrefix || wcsstr(pstr, TEXT("D3DFMT_")) == NULL)
		return pstr;
	else
		return pstr + lstrlen(TEXT("D3DFMT_"));
}

// color bits in a D3D format

UINT D3D::RGBBITS(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_A2B10G10R10:	return 10;
	case D3DFMT_A2R10G10B10:	return 10;
	case D3DFMT_R8G8B8:			return 8;
	case D3DFMT_A8R8G8B8:		return 8;
	case D3DFMT_X8R8G8B8:		return 8;
	case D3DFMT_R5G6B5:			return 5;
	case D3DFMT_X1R5G5B5:		return 5;
	case D3DFMT_A1R5G5B5:		return 5;
	case D3DFMT_A4R4G4B4:		return 4;
	case D3DFMT_X4R4G4B4:		return 4;
	case D3DFMT_R3G3B2:			return 2;
	case D3DFMT_A8R3G3B2:		return 2;
	default:					return 0;
	}
}

// alpha bits in a D3D format

UINT D3D::ALPHABITS(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_A8R8G8B8:		return 8;
	case D3DFMT_A8R3G3B2:		return 8;
	case D3DFMT_A4R4G4B4:		return 4;
	case D3DFMT_A2B10G10R10:	return 2;
	case D3DFMT_A2R10G10B10:	return 2;
	case D3DFMT_A1R5G5B5:		return 1;
	case D3DFMT_X8R8G8B8:		return 0;
	case D3DFMT_X4R4G4B4:		return 0;
	case D3DFMT_X1R5G5B5:		return 0;
	case D3DFMT_R8G8B8:			return 0;
	case D3DFMT_R5G6B5:			return 0;
	case D3DFMT_R3G3B2:			return 0;
	default:					return 0;
	}
}

// depth bits in a D3D format

UINT D3D::DEPTHBITS(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_D32:		return 32;
	case D3DFMT_D24X8:		return 24;
	case D3DFMT_D24S8:		return 24;
	case D3DFMT_D24X4S4:	return 24;
	case D3DFMT_D16:		return 16;
	case D3DFMT_D15S1:		return 15;
	default:				return 0;
	}
}

// stencil bits in a D3D format

UINT D3D::STENCILBITS(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_D24S8:		return 8;
	case D3DFMT_D24X4S4:	return 4;
	case D3DFMT_D15S1:		return 1;
	case D3DFMT_D16:		return 0;
	case D3DFMT_D24X8:		return 0;
	case D3DFMT_D32:		return 0;
	default:				return 0;
	}
}

// VP type to string UNUSED

TCHAR* D3D::VPTYPESTRING(VPTYPE vpt)
{
	switch (vpt)
	{
	case SOFT_VP: return TEXT("SOFTWARE_VP");
	case MIXD_VP: return TEXT("MIXED_VP");
	case HARD_VP: return TEXT("HARDWARE_VP");
	case PURE_VP: return TEXT("PURE_HARDWARE_VP");
	default:	  return TEXT("Unknown VP Type");
	}
}

// D3D device type to string, with optional prefix removal

TCHAR* D3D::DEVICETYPESTRING(D3DDEVTYPE dt, bool bPrefix)
{
	TCHAR* pRet = NULL;

	switch (dt)
	{
	case D3DDEVTYPE_HAL: pRet = TEXT("D3DDEVTYPE_HAL"); break;
	case D3DDEVTYPE_SW:  pRet = TEXT("D3DDEVTYPE_SOF"); break;
	case D3DDEVTYPE_REF: pRet = TEXT("D3DDEVTYPE_REF"); break;
	default:             pRet = TEXT("Unknown Device Type"); break;
	}

	if (bPrefix || wcsstr(pRet, TEXT("D3DDEVTYPE_")) == NULL)
		return pRet;

	return pRet + lstrlen(TEXT("D3DDEVTYPE_"));
}

// Multisample type to string converter, with optional prefix removal

TCHAR* D3D::MULTISAMPLESTRING(D3DMULTISAMPLE_TYPE mst, bool bPrefix)
{
	TCHAR* pRet = NULL;

	// notice we'll skip the D3DMULTISAMPLE_NONE case
	switch (mst)
	{
	case D3DMULTISAMPLE_NONMASKABLE:	pRet = TEXT("D3DMULTISAMPLE_NONMASKABLE"); break;
	case D3DMULTISAMPLE_2_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_2_SAMPLES");   break;
	case D3DMULTISAMPLE_3_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_3_SAMPLES");   break;
	case D3DMULTISAMPLE_4_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_4_SAMPLES");   break;
	case D3DMULTISAMPLE_5_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_5_SAMPLES");   break;
	case D3DMULTISAMPLE_6_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_6_SAMPLES");   break;
	case D3DMULTISAMPLE_7_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_7_SAMPLES");   break;
	case D3DMULTISAMPLE_8_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_8_SAMPLES");   break;
	case D3DMULTISAMPLE_9_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_9_SAMPLES");   break;
	case D3DMULTISAMPLE_10_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_10_SAMPLES");  break;
	case D3DMULTISAMPLE_11_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_11_SAMPLES");  break;
	case D3DMULTISAMPLE_12_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_12_SAMPLES");  break;
	case D3DMULTISAMPLE_13_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_13_SAMPLES");  break;
	case D3DMULTISAMPLE_14_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_14_SAMPLES");  break;
	case D3DMULTISAMPLE_15_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_15_SAMPLES");  break;
	case D3DMULTISAMPLE_16_SAMPLES:		pRet = TEXT("D3DMULTISAMPLE_16_SAMPLES");  break;
	default:							pRet = TEXT("");
	}

	if (bPrefix || wcsstr(pRet, TEXT("D3DMULTISAMPLE_")) == NULL)
		return pRet;

	return pRet + lstrlen(TEXT("D3DMULTISAMPLE_"));
}

// Presentation interval to string, with optional prefix removal

TCHAR* D3D::PRESENTINTERVALSTRING(UINT pi, bool bPrefix)
{
	TCHAR* pRet = NULL;

	switch (pi)
	{
	case D3DPRESENT_INTERVAL_IMMEDIATE: pRet = TEXT("D3DPRESENT_INTERVAL_IMMEDIATE"); break;
	case D3DPRESENT_INTERVAL_DEFAULT:   pRet = TEXT("D3DPRESENT_INTERVAL_DEFAULT"); break;
	case D3DPRESENT_INTERVAL_ONE:       pRet = TEXT("D3DPRESENT_INTERVAL_ONE"); break;
	case D3DPRESENT_INTERVAL_TWO:       pRet = TEXT("D3DPRESENT_INTERVAL_TWO"); break;
	case D3DPRESENT_INTERVAL_THREE:     pRet = TEXT("D3DPRESENT_INTERVAL_THREE"); break;
	case D3DPRESENT_INTERVAL_FOUR:      pRet = TEXT("D3DPRESENT_INTERVAL_FOUR"); break;
	default:                            pRet = TEXT("Unknown Present Interval"); break;
	}

	if (bPrefix || wcsstr(pRet, TEXT("D3DPRESENT_INTERVAL_")) == NULL)
		return pRet;

	return pRet + lstrlen(TEXT("D3DPRESENT_INTERVAL_"));
}