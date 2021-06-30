///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "D3DSettings.h"

using namespace D3D;

CSettings::CSettings()
{
	AdapterInfos.SetGrowFactor(2);
	AdapterInfos.SetLength(2);

	ndm[0] = ndm[1] = ndi[0] = ndi[1] = ndc[0] = ndc[1] = 0;

	Windowed = 1;

	nDSFormat = 0;
};

// adapter info wrapper, based on the windowed index

AdapterInfo* CSettings::GetAdapterInfo()
{
	return AdapterInfos[Windowed];
}

// device info wrapper, based on the windowed index and the di index

DeviceInfo*  CSettings::GetDeviceInfo()
{
	return &AdapterInfos[Windowed]->DeviceInfos[ndi[Windowed]];
}

// display mode wrapper, based on the windowed index

D3DDISPLAYMODE CSettings::GetDisplayMode()
{
	return AdapterInfos[Windowed]->DisplayModes[ndm[Windowed]];
}

// VP type wrapper

VPTYPE CSettings::GetVPType()
{
	return (VPTYPE)AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		VPTypes[0];
}

// back buffer format wrapper

D3DFORMAT CSettings::GetBackBufferFormat()
{
	return (D3DFORMAT)AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		BackBufferFormat;
}

// depth/stencil format wrapper

D3DFORMAT CSettings::GetDSFormat()
{
	if (AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		DSFormats.Find(D3DFMT_D24X8) != -1)
		return D3DFMT_D24X8;

	if (AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		DSFormats.Find(D3DFMT_D16) != -1)
		return D3DFMT_D16;

	const uint i = AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		DSFormats.Length();

	return (D3DFORMAT)AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		DSFormats[i - 1];
}

// multisampling wrapper

D3DMULTISAMPLE_TYPE CSettings::GetMSType()
{
#ifndef WORLD_EDITOR
	if (AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].SupportsMultiSampleType(D3DMULTISAMPLE_4_SAMPLES))
		return D3DMULTISAMPLE_4_SAMPLES;
#endif // WORLD_EDITOR

	if (AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].SupportsMultiSampleType(D3DMULTISAMPLE_2_SAMPLES))
		return D3DMULTISAMPLE_2_SAMPLES;

	return (D3DMULTISAMPLE_TYPE)AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		MSTypes[0];
}

// multisampling quality levels wrapper

DWORD CSettings::GetMSQuality()
{
	return AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		MSQualityLevels[0];
}


// presentation interval wrapper

DWORD CSettings::GetPresentInterval()
{
	if (AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		PresentIntervals.Find(D3DPRESENT_INTERVAL_DEFAULT) != -1)
		return D3DPRESENT_INTERVAL_DEFAULT;

	if (AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		PresentIntervals.Find(D3DPRESENT_INTERVAL_ONE) != -1)
		return D3DPRESENT_INTERVAL_ONE;

	return (DWORD)AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		PresentIntervals[0];
}

// Set DS format wrapper

void CSettings::SetDSFormat(UINT nFmt)
{
	if (nFmt < AdapterInfos[Windowed]->
		DeviceInfos[ndi[Windowed]].
		DeviceCombos[ndc[Windowed]].
		DSFormats.Length())
		nDSFormat = nFmt;
}
