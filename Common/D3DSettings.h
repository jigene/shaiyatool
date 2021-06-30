///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef D3DSETTINGS_H
#define D3DSETTINGS_H

#include "D3DEnum.h"

namespace D3D
{
	typedef CTArray<AdapterInfo*> AdapterInfoPtrArray;

	//-----------------------------------------------------------------------------
	// CSettings: Current D3D settings: adapter, device, mode, formats, etc.
	//-----------------------------------------------------------------------------
	class CSettings
	{
	public:
		AdapterInfoPtrArray	AdapterInfos;	// fullscreen [0] and windowed [1]

		UINT Windowed;						// index to the previous

		UINT ndm[2];	// indices to the best display mode within each AdapterInfo
		UINT ndi[2];	// indices to the best DeviceInfo within each AdapterInfo
		UINT ndc[2];	// indices to the best DeviceCombo within each DeviceInfo

		// indices into DeviceCombo properties that can be changed
		UINT nDSFormat;

		// constructor
		CSettings();

		// 'Get' wrappers
		AdapterInfo*		GetAdapterInfo();
		DeviceInfo*			GetDeviceInfo();
		D3DDISPLAYMODE		GetDisplayMode();

		VPTYPE				GetVPType();
		D3DFORMAT			GetBackBufferFormat();
		D3DFORMAT			GetDSFormat();
		D3DMULTISAMPLE_TYPE GetMSType();
		DWORD				GetMSQuality();
		DWORD				GetPresentInterval();

		// 'Set' wrappers
		void SetDSFormat(UINT nFmt);
	};
}

#endif // D3DSETTINGS_H