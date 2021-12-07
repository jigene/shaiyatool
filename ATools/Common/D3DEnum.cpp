///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "D3DEnum.h"

using namespace D3D;

bool DeviceCombo::SupportsMultiSampleType(D3DMULTISAMPLE_TYPE multisamples)
{
	if (MSTypes.Find(multisamples) == -1)
		return false;

	DSMSConflict c;
	if (DSFormats.Find(D3DFMT_D24X8) != -1)
		c.fmt = D3DFMT_D24X8;
	else if (DSFormats.Find(D3DFMT_D16) != -1)
		c.fmt = D3DFMT_D16;
	else
		c.fmt = (D3DFORMAT)DSFormats[DSFormats.Length() - 1];

	c.mst = multisamples;
	return DSMSConflicts.Find(c) == -1;
}

//-----------------------------------------------------------------------------
// SortModesCallback(): sort callback comparing two D3DDISPLAYMODEs
//-----------------------------------------------------------------------------
static int __cdecl SortModesCallback(const void* arg1, const void* arg2)
{
	D3DDISPLAYMODE* pdm1 = (D3DDISPLAYMODE*)arg1;
	D3DDISPLAYMODE* pdm2 = (D3DDISPLAYMODE*)arg2;

	// the wider display modes sink down, the thinner bubble up
	if (pdm1->Width > pdm2->Width)				return +1;
	if (pdm1->Width < pdm2->Width)				return -1;

	// the taller display modes sink down, the shorter bubble up
	if (pdm1->Height > pdm2->Height)			return +1;
	if (pdm1->Height < pdm2->Height)			return -1;

	// the more colorful display modes sink down
	if (RGBBITS(pdm1->Format) > RGBBITS(pdm2->Format))	return +1;
	if (RGBBITS(pdm1->Format) < RGBBITS(pdm2->Format))	return -1;

	// the faster display modes sink down
	if (pdm1->RefreshRate > pdm2->RefreshRate)	return +1;
	if (pdm1->RefreshRate < pdm2->RefreshRate)	return -1;

	// the two display modes are identical
	return 0;
}

//-----------------------------------------------------------------------------
// CEnum(): constructor, sets up app constraints
//-----------------------------------------------------------------------------
CEnum::CEnum()
{
	AppMinFullscreenWidth = 640;
	AppMinFullscreenHeight = 480;

	AppMinRGBBits = 5;
	AppMinAlphaBits = 0;
	AppMinDepthBits = 15;
	AppMinStencilBits = 0;

	AppUsesDepthBuffer = true;
	AppUsesMixedVP = true;

	// we will allow every possible display mode format by default;
	// they indicate how many bits are dedicated to each channel (Alpha, Red,
	// Green and Blue), with X standing for unused.

	// take care to maintain consistency between the format list and the 
	// 'AppMinRGBBits' and 'AppMinAlphaBits' constraints above.

	// Also notice the 10-bit format is only available in fullscreen modes.

	AppDisplayFormats.Append(D3DFMT_R5G6B5);		// 16-bit, 6 for green
	AppDisplayFormats.Append(D3DFMT_X1R5G5B5);		// 16-bit, 5 per channel
	AppDisplayFormats.Append(D3DFMT_A1R5G5B5);		// 16-bit, 1 for alpha
	AppDisplayFormats.Append(D3DFMT_X8R8G8B8);		// 32-bit, 8 per channel
	AppDisplayFormats.Append(D3DFMT_A8R8G8B8);		// 32-bit, 8 for alpha
	AppDisplayFormats.Append(D3DFMT_A2R10G10B10);	// 32-bit, 2 for alpha

	// we will allow every backbuffer format by default
	AppBackBufferFormats.Append(D3DFMT_R5G6B5);
	AppBackBufferFormats.Append(D3DFMT_X1R5G5B5);
	AppBackBufferFormats.Append(D3DFMT_A1R5G5B5);
	AppBackBufferFormats.Append(D3DFMT_X8R8G8B8);
	AppBackBufferFormats.Append(D3DFMT_A8R8G8B8);
	AppBackBufferFormats.Append(D3DFMT_A2R10G10B10);

	// we will allow every depth/stencil format by default; obviously, D is 
	// for depth S for stencil, and X unused.
	AppDepthStencilFormats.Append(D3DFMT_D16);
	AppDepthStencilFormats.Append(D3DFMT_D15S1);
	AppDepthStencilFormats.Append(D3DFMT_D24X8);
	AppDepthStencilFormats.Append(D3DFMT_D24X4S4);
	AppDepthStencilFormats.Append(D3DFMT_D24S8);
	AppDepthStencilFormats.Append(D3DFMT_D32);

	// we will allow every multisampling type by default, even the nonmaskable,
	// to enable the quality levels
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_NONE);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_NONMASKABLE);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_2_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_3_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_4_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_5_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_6_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_7_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_8_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_9_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_10_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_11_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_12_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_13_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_14_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_15_SAMPLES);
	AppMultiSamplingTypes.Append(D3DMULTISAMPLE_16_SAMPLES);
}

//-----------------------------------------------------------------------------
// EnumerateDSFormats(): add depth/stencil formats compatible with the device
// and the app to the given device combo
//-----------------------------------------------------------------------------
void CEnum::EnumerateDSFormats(DeviceCombo* pdc)
{
	D3DFORMAT fmt;

	// traverse the app defined depth/stencil formats
	for (UINT i = 0; i < AppDepthStencilFormats.Length(); i++)
	{
		fmt = (D3DFORMAT)AppDepthStencilFormats[i];

		// check the format against app requirements
		if (DEPTHBITS(fmt) < AppMinDepthBits)   continue;
		if (STENCILBITS(fmt) < AppMinStencilBits) continue;

		// is the format available for a depth/stencil surface resource on
		// this device?
		if (FAILED(m_pd3d->CheckDeviceFormat(pdc->AdapterOrdinal,
			pdc->DevType,
			pdc->DisplayFormat,
			D3DUSAGE_DEPTHSTENCIL,
			D3DRTYPE_SURFACE,
			fmt)))
			continue;

		// does it match both the display and back buffer formats?
		if (FAILED(m_pd3d->CheckDepthStencilMatch(pdc->AdapterOrdinal,
			pdc->DevType,
			pdc->DisplayFormat,
			pdc->BackBufferFormat,
			fmt)))
			continue;

		// yes, yes!
		pdc->DSFormats.Append(fmt);
	}
}

//-----------------------------------------------------------------------------
// EnumerateMSTypes(): add multisample types that are compatible with the
// device and the app to the given device combo
//-----------------------------------------------------------------------------
void CEnum::EnumerateMSTypes(DeviceCombo* pdc)
{
	D3DMULTISAMPLE_TYPE msType;
	DWORD msQuality;

	// traverse the types and check for support
	for (UINT i = 0; i < AppMultiSamplingTypes.Length(); i++)
	{
		msType = (D3DMULTISAMPLE_TYPE)AppMultiSamplingTypes[i];

		if (FAILED(m_pd3d->CheckDeviceMultiSampleType(pdc->AdapterOrdinal,
			pdc->DevType,
			pdc->BackBufferFormat,
			pdc->Windowed,
			msType,
			&msQuality)))
			continue;

		// supported
		pdc->MSTypes.Append(msType);

		// important! presentation parameters quality levels are zero-based,
		// and the API call returns the number of levels, so we will store
		// the maximum value that can be used in other Direct3D API  calls,
		// i.o., the number of levels. Also notice that both these lists must
		// always be accessed with indices in synch.
		if (msQuality != 0)
			msQuality -= 1;

		pdc->MSQualityLevels.Append(msQuality);
	}
}

//-----------------------------------------------------------------------------
// EnumerateDSMSConflicts(): find any conflicts between the depth/stencil
// formats and multisample types in the given device combo
//-----------------------------------------------------------------------------
void CEnum::EnumerateDSMSConflicts(DeviceCombo* pdc)
{
	DSMSConflict con;

	D3DFORMAT fmt;

	D3DMULTISAMPLE_TYPE mst;

	// traverse formats
	for (UINT i = 0; i < pdc->DSFormats.Length(); i++)
	{
		fmt = (D3DFORMAT)pdc->DSFormats[i];

		// check against multisample types
		for (UINT j = 0; j < pdc->MSTypes.Length(); j++)
		{
			mst = (D3DMULTISAMPLE_TYPE)pdc->MSTypes[j];

			// failure to support the combination indicates a conflict; save it
			if (FAILED(m_pd3d->CheckDeviceMultiSampleType(pdc->AdapterOrdinal,
				pdc->DevType,
				fmt,
				pdc->Windowed,
				mst,
				NULL)))
			{
				con.fmt = fmt;
				con.mst = mst;

				pdc->DSMSConflicts.Append(con);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// EnumerateVPTypes(): add vertex processing types that are compatible with the
// device and the app to the given device combo
//-----------------------------------------------------------------------------
void CEnum::EnumerateVPTypes(DeviceInfo* pdi, DeviceCombo* pdc)
{
	// by default, every VP type is allowed, even the mixed one; your
	// application may have different requirements, i.e. the need for
	// a pure hardware device to test a graphics card...
	if ((pdi->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0
		&& pdi->Caps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
	{
		if ((pdi->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)
			pdc->VPTypes.Append(PURE_VP);

		pdc->VPTypes.Append(HARD_VP);

		if (AppUsesMixedVP)
			pdc->VPTypes.Append(MIXD_VP);
	}

	pdc->VPTypes.Append(SOFT_VP);
}

//-----------------------------------------------------------------------------
// EnumeratePIntervals(): query device caps to add the presentation intervals
// that may deal with flicker or other artifacts.
//-----------------------------------------------------------------------------
void CEnum::EnumeratePIntervals(DeviceInfo* pdi, DeviceCombo* pdc)
{
	// the default interval (the same as D3DPRESENT_INTERVAL_ONE) is always
	// available; we will put it at the top, to avoid mouse flicker in windowed
	//apps
	pdc->PresentIntervals.Append(D3DPRESENT_INTERVAL_DEFAULT);

	// the immediate interval is always available, but is worth checking...
	if ((pdi->Caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE) != 0)
		pdc->PresentIntervals.Append(D3DPRESENT_INTERVAL_IMMEDIATE);

	// the rest are truly hardware-dependant
	if ((pdi->Caps.PresentationIntervals & D3DPRESENT_INTERVAL_TWO) != 0)
		pdc->PresentIntervals.Append(D3DPRESENT_INTERVAL_TWO);

	if ((pdi->Caps.PresentationIntervals & D3DPRESENT_INTERVAL_THREE) != 0)
		pdc->PresentIntervals.Append(D3DPRESENT_INTERVAL_THREE);

	if ((pdi->Caps.PresentationIntervals & D3DPRESENT_INTERVAL_FOUR) != 0)
		pdc->PresentIntervals.Append(D3DPRESENT_INTERVAL_FOUR);
}

//-----------------------------------------------------------------------------
// EnumerateDeviceCombos(): for a particular device
//-----------------------------------------------------------------------------
HRESULT CEnum::EnumerateDeviceCombos(DeviceInfo* pdi,
	DWORDARRAY* pDisplayFormats)
{
	D3DFORMAT fmt;
	D3DFORMAT bbfmt;

	// traverse the passed-in display formats
	for (UINT i = 0; i < pDisplayFormats->Length(); i++)
	{
		fmt = (D3DFORMAT)(*pDisplayFormats)[i];

		// traverse the app allowed backbuffer formats
		for (UINT j = 0; j < AppBackBufferFormats.Length(); j++)
		{
			bbfmt = (D3DFORMAT)AppBackBufferFormats[j];

			// check each against the app constraint
			if (ALPHABITS(bbfmt) < AppMinAlphaBits)
				continue;

			// we'll check if the device supports a display mode-backbuffer
			// formats combination, once for windowed display modes,
			// once for fullscreen modes
			for (UINT k = 0; k < 2; k++)
			{
				// check for system support
				if (FAILED(m_pd3d->CheckDeviceType(pdi->AdapterOrdinal,
					pdi->DevType,
					fmt,
					bbfmt,
					k % 2 == 0)))
					continue;

				// at this point we have a DeviceCombo supported by the system,
				// but we still need to confirm that it is compatible with
				// other app constraints; we'll fill in a device combo with
				// what we know so far

				DeviceCombo dc;

				dc.Windowed = k % 2 == 0;
				dc.AdapterOrdinal = pdi->AdapterOrdinal;
				dc.DevType = pdi->DevType;
				dc.DisplayFormat = fmt;
				dc.BackBufferFormat = bbfmt;

				// enumerate VP types (software VP should always be available)
				EnumerateVPTypes(pdi, &dc);

				// enumerate presentation intervals (the default should always
				// be available)
				EnumeratePIntervals(pdi, &dc);

				// check for multisampling requirements
				EnumerateMSTypes(&dc);

				if (dc.MSTypes.Length() == 0)
					continue;

				// check the depth/stencil requirements
				if (AppUsesDepthBuffer)
				{
					EnumerateDSFormats(&dc);

					if (dc.DSFormats.Length() == 0)
						continue;

					// gather depth/stecil-multisampling conflicts
					EnumerateDSMSConflicts(&dc);
				}

				// met every requirement!
				pdi->DeviceCombos.Append(dc);
			}
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// EnumerateDevices(): Enumerates D3D devices for a particular adapter
//-----------------------------------------------------------------------------
HRESULT CEnum::EnumerateDevices(AdapterInfo* pai, DWORDARRAY* pDisplayFormats)
{
	HRESULT hr;

	DeviceInfo di;

	// traverse the device types, there are only three, namely, a HAL device, a
	// reference device and a software device, defined by the D3DDEVTYPE enum
	// with values 1, 2 and 3 respectively
	for (UINT i = 1; i < 4; i++)
	{
		// save members of this device info
		di.AdapterOrdinal = pai->AdapterOrdinal;
		di.DevType = (D3DDEVTYPE)i;

		// retrieve and store device capabilities in this device
		// info for inspection later
		if (FAILED(m_pd3d->GetDeviceCaps(di.AdapterOrdinal,
			di.DevType,
			&di.Caps)))
			continue;

		// get info for each device combo on this device info
		if (FAILED(hr = EnumerateDeviceCombos(&di, pDisplayFormats)))
			return hr;

		if (di.DeviceCombos.Length() == 0)
			continue;

		// if at least one device combo for this device was found,
		// add it to the corresponding list
		pai->DeviceInfos.Append(di);
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Enumerate(): available D3D adapters, devices, modes, etc. for the passed-in
// D3D object, a reference to which is mantained by the class.
//-----------------------------------------------------------------------------
bool CEnum::Enumerate(LPDIRECT3D9 pD3D)
{
	// we need a valid D3D object to continue
	if (pD3D == NULL)
		return false;

	// keep a local reference to it
	m_pd3d = pD3D;

	HRESULT hr;

	AdapterInfo ai;

	DWORDARRAY formats;

	// traverse adapters (usually just one)
	for (UINT i = 0; i < m_pd3d->GetAdapterCount(); i++)
	{
		// identify this adapter (retrieve and store a description)
		ai.AdapterOrdinal = i;
		m_pd3d->GetAdapterIdentifier(i, 0, &ai.AdapterIdentifier);

		D3DFORMAT fmt;
		D3DDISPLAYMODE dm;

		// we will check adapter modes for compatibility with each of the
		// app defined display formats, resolution and color depth,
		// setup in the constructor
		for (UINT j = 0; j < AppDisplayFormats.Length(); j++)
		{
			// get one of the application defined formats
			fmt = (D3DFORMAT)AppDisplayFormats[j];

			// get a list of modes for this adapter that support it
			for (UINT k = 0; k < m_pd3d->GetAdapterModeCount(i, fmt); k++)
			{
				//  retrieve a display mode with an enumeration call
				m_pd3d->EnumAdapterModes(i, fmt, k, &dm);

				// check the display mode for resolution, color and 
				// alpha bit depth
				if (dm.Width < AppMinFullscreenWidth ||
					dm.Height < AppMinFullscreenHeight ||
					RGBBITS(dm.Format) < AppMinRGBBits ||
					ALPHABITS(dm.Format) < AppMinAlphaBits)
					continue;

				// it meets the requirements, so the current adapter info
				// inherits it, for it is compatible with the app
				ai.DisplayModes.Append(dm);

				// append the format to the temp list that we'll use to
				// enumerate devices, if not already there
				if (formats.Find(dm.Format) == -1)
					formats.Append(dm.Format);
			}
		}

		// sort the display modes list so that the smallest and fastest
		// gets to the top of the list (see SortModesCallback)
		ai.DisplayModes.Sort(SortModesCallback);

		// get info for each device on this adapter, providing the formats
		// that met the application requirements
		if (FAILED(hr = EnumerateDevices(&ai, &formats)))
		{
			qCritical("Can't enumerate Direct3D devices.");
			return false;
		}

		// if at least one device on this adapter is available and compatible
		// with the app, add the adapterInfo to the list
		if (ai.DeviceInfos.Length() != 0)
			AdapterInfos.Append(ai);

		// clear the format list for the next adapter
		formats.Clear();
	}

	return true;
}
