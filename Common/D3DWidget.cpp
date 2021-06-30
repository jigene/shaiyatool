///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "D3DWidget.h"

using namespace D3D;

CD3DWidget::CD3DWidget(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_d3d(nullptr),
	m_isFullscreen(false),
	m_device(nullptr),
	m_deviceObjectsInitied(false),
	m_deviceLost(false),
	m_autoRefresh(false),
	m_deviceObjectsRestored(false),
	m_skipFrame(true)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoFillBackground(false);
	setMouseTracking(true);
}

void CD3DWidget::Destroy()
{
	CleanupEnvironment();
	Release(m_d3d);
}

bool CD3DWidget::CreateEnvironment()
{
	m_d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!m_d3d)
	{
		qCritical("Could not initialize Direct3D.\n" \
			"Check that the latest version of DirectX\n" \
			"is correctly installed on your system.");
		return false;
	}

	if (!m_enum.Enumerate(m_d3d))
		return false;

	if (!_chooseInitialSettings())
		return false;

	if (!_initializeEnvironment())
		return false;

	return true;
}

void CD3DWidget::CleanupEnvironment()
{
	if (m_device != nullptr)
	{
		if (m_deviceObjectsRestored)
		{
			m_deviceObjectsRestored = false;
			InvalidateDeviceObjects();
		}

		if (m_deviceObjectsInitied)
		{
			m_deviceObjectsInitied = false;
			DeleteDeviceObjects();
		}

		if (m_device->Release() > 0)
		{
			qCritical("A D3D object has a non-zero reference count\n" \
				"(meaning things were not properly cleaned up).");
		}

		m_device = nullptr;
	}
}

LPDIRECT3DDEVICE9 CD3DWidget::GetDevice() const
{
	return m_device;
}

string CD3DWidget::GetDeviceStats() const
{
	return string::fromWCharArray(m_deviceStats);
}

QPaintEngine* CD3DWidget::paintEngine() const
{
	return null;
}

bool CD3DWidget::InitDeviceObjects()
{
	return true;
}

void CD3DWidget::DeleteDeviceObjects()
{
}

bool CD3DWidget::RestoreDeviceObjects()
{
	return true;
}

void CD3DWidget::InvalidateDeviceObjects()
{
}

bool CD3DWidget::Render()
{
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 50, 50, 50), 1.0f, 0L);

	if (FAILED(m_device->BeginScene()))
		return false;

	m_device->EndScene();
	return true;
}

bool CD3DWidget::MoveFrame()
{
	return true;
}

bool CD3DWidget::RenderEnvironment()
{
	if (!m_device)
		return false;

	if (m_deviceLost)
	{
		HRESULT hr;

		if (FAILED(hr = m_device->TestCooperativeLevel()))
		{
			if (hr == D3DERR_DEVICELOST)
				return true;

			if (hr == D3DERR_DEVICENOTRESET)
			{
				if (!m_isFullscreen)
				{
					m_d3d->GetAdapterDisplayMode(m_settings.GetAdapterInfo()->AdapterOrdinal,
						&m_settings.GetDisplayMode());

					m_presentParameters.BackBufferFormat = m_settings.GetDisplayMode().Format;
				}

				if (m_deviceObjectsRestored)
				{
					m_deviceObjectsRestored = false;
					InvalidateDeviceObjects();
				}

				if (FAILED(hr = m_device->Reset(&m_presentParameters)))
					return false;

				if (!RestoreDeviceObjects())
				{
					InvalidateDeviceObjects();
					return false;
				}
				else
					m_deviceObjectsRestored = true;
			}

			return SUCCEEDED(hr);
		}

		m_deviceLost = false;
	}

	if (!MoveFrame())
		return false;

	if (m_autoRefresh && m_skipFrame)
	{
		if (m_elapsedTimer.elapsed() > 16)
		{
			if (!Render())
				return false;

			if (m_device->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
				m_deviceLost = true;

			m_elapsedTimer.start();
		}
	}
	else
	{
		if (!Render())
			return false;

		if (m_device->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST)
			m_deviceLost = true;
	}

	return true;
}

void CD3DWidget::resizeEvent(QResizeEvent* event)
{
	if (!m_device)
		return;

	if (m_presentParameters.BackBufferWidth != width()
		|| m_presentParameters.BackBufferHeight != height())
	{
		m_presentParameters.BackBufferWidth = width();
		m_presentParameters.BackBufferHeight = height();

		if (m_device != nullptr)
		{
			if (m_deviceObjectsRestored)
			{
				m_deviceObjectsRestored = false;
				InvalidateDeviceObjects();
			}

			if (FAILED(m_device->Reset(&m_presentParameters)))
				return;

			if (!RestoreDeviceObjects())
			{
				InvalidateDeviceObjects();
				return;
			}
			else
				m_deviceObjectsRestored = true;
		}
	}

	if (!IsAutoRefresh())
		RenderEnvironment();
}

bool CD3DWidget::IsAutoRefresh() const
{
	return m_autoRefresh;
}

void CD3DWidget::SetAutoRefresh(bool autoRefresh)
{
	if (m_autoRefresh == autoRefresh)
		return;

	m_autoRefresh = autoRefresh;

	if (autoRefresh)
	{
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(RenderEnvironment()));
		m_timer.start(0);
		if (m_skipFrame)
			m_elapsedTimer.start();
	}
	else
	{
		disconnect(&m_timer, SIGNAL(timeout()), this, SLOT(RenderEnvironment()));
		m_timer.stop();
	}
}

bool CD3DWidget::_initializeEnvironment()
{
	AdapterInfo* pai = m_settings.GetAdapterInfo();
	DeviceInfo*  pdi = m_settings.GetDeviceInfo();

	// Warn user about a null REF device that cannot render anything
	if (pdi->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE)
	{
		qCritical("Warning: Nothing will be rendered.\n" \
			"The reference rendering device was selected, but\n" \
			"your computer only has a reduced-functionality\n" \
			"reference device installed. Install the DirectX\n" \
			"SDK to get the full reference device.\n");
	}

	DWORD dwCreateFlags;

	// translate the VP type to a device creation behavior
	switch (m_settings.GetVPType())
	{
	case PURE_VP: dwCreateFlags = D3DCREATE_PUREDEVICE;
	case HARD_VP: dwCreateFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING; break;
	case MIXD_VP: dwCreateFlags = D3DCREATE_MIXED_VERTEXPROCESSING;    break;
	case SOFT_VP: dwCreateFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING; break;
	default:	  return false;
	}

	// setup the creation presentation parameters
	_buildPresentParamsFromSettings();

	// create the device
	HRESULT hr;
	hr = m_d3d->CreateDevice(pdi->AdapterOrdinal,
		pdi->DevType,
		(HWND)winId(),
		dwCreateFlags,
		&m_presentParameters,
		&m_device);

	if (SUCCEEDED(hr))
	{
		lstrcpy(m_deviceStats, TEXT("<u>Direct3D 9.0c device stats :</u><br/>"));

		// store the device's description, beginning with type;
		lstrcat(m_deviceStats, DEVICETYPESTRING(pdi->DevType, true));

		// then VP type, including  non-HAL devices simulating hardware VP and
		// the pure hardware VP variant...
		if ((dwCreateFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0 &&
			pdi->DevType != D3DDEVTYPE_HAL)
			lstrcat(m_deviceStats, TEXT(" Simulated"));

		if (dwCreateFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
		{
			if (dwCreateFlags & D3DCREATE_PUREDEVICE)
				lstrcat(m_deviceStats, TEXT(" Pure"));

			lstrcat(m_deviceStats, TEXT(" Hardware"));
		}
		else if (dwCreateFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
			lstrcat(m_deviceStats, TEXT(" Mixed"));
		else
			lstrcat(m_deviceStats, TEXT(" Software"));

		lstrcat(m_deviceStats, TEXT(" VP"));

		// ...and the adapter's description for HAL devices
		if (pdi->DevType == D3DDEVTYPE_HAL)
		{
			lstrcat(m_deviceStats, TEXT("<br/>"));

			// be sure not to overflow m_deviceStats when appending
			const int nDescription = sizeof(pai->AdapterIdentifier.Description);

			TCHAR szDescription[nDescription];

			int nResult = MultiByteToWideChar(CP_ACP, 0, pai->AdapterIdentifier.Description, -1,
				szDescription, nDescription);
			szDescription[nDescription - 1] = 0;

			lstrcat(szDescription, TEXT("<br/>"));

			// append as many characters as space is left on the stats
			wcsncat(m_deviceStats,
				szDescription,
				(sizeof(m_deviceStats) / sizeof((m_deviceStats)[0])) - lstrlen(m_deviceStats) - 1);

			TCHAR  szDims[100];
			TCHAR  szFmt[100];
			TCHAR  szDepthFmt[100];
			TCHAR* szMS;

			_snwprintf(szDims, 100, TEXT("%dx%d "),
				m_presentParameters.BackBufferWidth,
				m_presentParameters.BackBufferHeight);

			szDims[99] = TEXT('\0');

			if (m_isFullscreen)
			{
				// append as many characters as space is left on the stats
				wcsncat(m_deviceStats, szDims,
					(sizeof(m_deviceStats) / sizeof((m_deviceStats)[0])) - lstrlen(m_deviceStats) - 1);
			}

			D3DFORMAT fmt = m_settings.GetDisplayMode().Format;

			// display format (including the back buffer format if they do not match)
			if (fmt == m_presentParameters.BackBufferFormat)
				lstrcpyn(szFmt, D3DUtil_D3DFormatToString(fmt, true), 100);
			else
				_snwprintf(szFmt, 100, TEXT("%s back, %s front"),
				D3DUtil_D3DFormatToString(m_presentParameters.BackBufferFormat, true),
				D3DUtil_D3DFormatToString(fmt, true));

			szFmt[99] = TEXT('\0');

			// append as many characters as space is left on the stats
			wcsncat(m_deviceStats,
				szFmt,
				(sizeof(m_deviceStats) / sizeof((m_deviceStats)[0])) - lstrlen(m_deviceStats) - 1);

			lstrcat(m_deviceStats, TEXT("<br/>"));

			// depth/stencil buffer format
			if (m_enum.AppUsesDepthBuffer)
			{
				_snwprintf(szDepthFmt, 100, TEXT("%s "),
					D3DUtil_D3DFormatToString(m_settings.GetDSFormat(), true));

				szDepthFmt[99] = TEXT('\0');

				// append as many characters as space is left on the stats
				wcsncat(m_deviceStats,
					szDepthFmt,
					(sizeof(m_deviceStats) / sizeof((m_deviceStats)[0])) - lstrlen(m_deviceStats) - 1);
			}

			lstrcat(m_deviceStats, TEXT("<br/>"));

			// multisampling type (no. of samples or nonmaskable)
			szMS = MULTISAMPLESTRING(m_settings.GetMSType(), true);

			// append as many characters as space is left on the stats
			wcsncat(m_deviceStats,
				szMS,
				(sizeof(m_deviceStats) / sizeof((m_deviceStats)[0])) - lstrlen(m_deviceStats) - 1);
		}

		switch (m_presentParameters.PresentationInterval)
		{
		case D3DPRESENT_INTERVAL_DEFAULT:
		case D3DPRESENT_INTERVAL_ONE:
		case D3DPRESENT_INTERVAL_TWO:
		case D3DPRESENT_INTERVAL_THREE:
		case D3DPRESENT_INTERVAL_FOUR:
			m_skipFrame = false;
			break;
		default:
			m_skipFrame = true;
			break;
		}

		if (!InitDeviceObjects())
			DeleteDeviceObjects();
		else
		{
			m_deviceObjectsInitied = true;

			if (!RestoreDeviceObjects())
				InvalidateDeviceObjects();
			else
			{
				m_deviceObjectsRestored = true;
				return true;
			}
		}

		CleanupEnvironment();
		return false;
	}
	else
	{
		if (_findBestWindowedMode(false, true))
		{
			qCritical("Switching to the reference rasterizer, a\n" \
				"software device that implements the entire\n" \
				"Direct3D feature set, but runs very slowly.");

			if (_initializeEnvironment())
				return true;
			else
				return false;
		}
	}

	return false;
}

void CD3DWidget::_buildPresentParamsFromSettings()
{
	m_presentParameters.Windowed = m_settings.Windowed;
	m_presentParameters.hDeviceWindow = (HWND)winId();
	m_presentParameters.BackBufferCount = 1;
	m_presentParameters.EnableAutoDepthStencil = m_enum.AppUsesDepthBuffer;
	m_presentParameters.MultiSampleType = m_settings.GetMSType();
	m_presentParameters.MultiSampleQuality = m_settings.GetMSQuality();
	m_presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_presentParameters.Flags = 0;

	if (m_enum.AppUsesDepthBuffer)
	{
		m_presentParameters.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		m_presentParameters.AutoDepthStencilFormat = m_settings.GetDSFormat();
	}

	if (!m_isFullscreen)
	{
		m_presentParameters.BackBufferWidth = width();
		m_presentParameters.BackBufferHeight = height();
		m_presentParameters.FullScreen_RefreshRateInHz = 0;
	}
	else
	{
		m_presentParameters.BackBufferWidth = m_settings.GetDisplayMode().Width;
		m_presentParameters.BackBufferHeight = m_settings.GetDisplayMode().Height;
		m_presentParameters.FullScreen_RefreshRateInHz = m_settings.GetDisplayMode().RefreshRate;
	}

	m_presentParameters.BackBufferFormat = m_settings.GetBackBufferFormat();
	m_presentParameters.PresentationInterval = m_settings.GetPresentInterval();
}

bool CD3DWidget::_chooseInitialSettings()
{
	bool foundFullscreen = _findBestFullscreenMode(false, false);
	bool foundWindowed = _findBestWindowedMode(false, false);

	if (m_isFullscreen && foundFullscreen)
		m_settings.Windowed = 0;

	if (!foundWindowed && foundFullscreen)
		m_settings.Windowed = 0;

	if (!foundFullscreen && !foundWindowed)
	{
		qCritical("Could not find any compatible Direct3D devices.");
		return false;
	}

	if (!m_isFullscreen && !foundWindowed)
	{
		qCritical("Could not find any compatible Direct3D devices.");
		return false;
	}

	return true;
}

bool CD3DWidget::_findBestWindowedMode(bool HAL, bool REF)
{
	// get the display mode of the primary adapter, which is assumed to be
	// where the window will appear
	D3DDISPLAYMODE dm;

	m_d3d->GetAdapterDisplayMode(0, &dm);

	// 'best' storage
	AdapterInfo* paiBest = NULL;
	DeviceInfo*  pdiBest = NULL;
	DeviceCombo* pdcBest = NULL;

	// iterators
	AdapterInfo* pai;
	DeviceInfo*  pdi;
	DeviceCombo* pdc;

	// success flags
	bool bBetter, bBest;

	UINT i, j, k;

	// prefered multisample type
	D3DMULTISAMPLE_TYPE multisamples;
#ifdef WORLD_EDITOR
	multisamples = D3DMULTISAMPLE_2_SAMPLES;
#else
	multisamples = D3DMULTISAMPLE_4_SAMPLES;
#endif // WORLD_EDITOR

	// traverse the enumerated adapters information
	for (i = 0; i < m_enum.AdapterInfos.Length(); i++)
	{
		pai = &m_enum.AdapterInfos[i];

		// for each adapter, traverse the device infos
		for (j = 0; j < pai->DeviceInfos.Length(); j++)
		{
			pdi = &pai->DeviceInfos[j];

			// skip according to the requirements
			if (HAL && pdi->DevType != D3DDEVTYPE_HAL)
				continue;

			if (REF && pdi->DevType != D3DDEVTYPE_REF)
				continue;

			// traverse device combos for this device
			for (k = 0; k < pdi->DeviceCombos.Length(); k++)
			{
				pdc = &pdi->DeviceCombos[k];

				// skip the non-windowed or distinct format combos
				if (!pdc->Windowed)
					continue;

				if (pdc->DisplayFormat != dm.Format)
					continue;

				bBetter = !pdcBest
					|| (pdc->DevType == D3DDEVTYPE_HAL && pdcBest->DevType != D3DDEVTYPE_HAL)
					|| (pdc->DevType == D3DDEVTYPE_HAL && pdc->SupportsMultiSampleType(multisamples) && !pdcBest->SupportsMultiSampleType(multisamples))
					|| (pdc->DevType != D3DDEVTYPE_HAL && pdcBest->DevType != D3DDEVTYPE_HAL && pdc->SupportsMultiSampleType(multisamples) && !pdcBest->SupportsMultiSampleType(multisamples));

				bBest = pdc->DevType == D3DDEVTYPE_HAL
					&& pdc->BackBufferFormat == pdc->DisplayFormat
					&& pdc->SupportsMultiSampleType(multisamples);

				bBetter |= bBest;

				if (bBetter)
				{
					// save it as the current best
					paiBest = pai;
					pdiBest = pdi;
					pdcBest = pdc;

					// this dc looks great -- take it
					if (bBest)
						goto DoneSearchingWDC;
				}
			}
		}
	}

DoneSearchingWDC:

	// none found!!
	if (pdcBest == NULL)
		return false;

	m_settings.Windowed = 1;
	m_settings.AdapterInfos[1] = paiBest;

	int l = paiBest->DisplayModes.Find(dm);

	// for some bizarre multi-monitor setups in which the primary adapter's
	// current dm is not available in a secondary one, there's nothing else
	// we can do...
	if (i > 0 && l == -1)
		return false;

	// index to the best dm within the ai
	m_settings.ndm[1] = l;

	// indices to the best di and dc
	if (bBest)
	{
		m_settings.ndi[1] = j;
		m_settings.ndc[1] = k;
	}
	else
	{
		// retract to the 'better' di and dc
		m_settings.ndi[1] = (UINT)paiBest->DeviceInfos.Find(*pdiBest);
		m_settings.ndc[1] = (UINT)pdiBest->DeviceCombos.Find(*pdcBest);
	}

	return true;
}

bool CD3DWidget::_findBestFullscreenMode(bool HAL, bool REF)
{
	// for fullscreen, default to the first HAL device combo that supports the
	// current desktop display mode, to any display mode if HAL is incompatible
	// with the desktop mode, or to a non-HAL if no HAL is available
	D3DDISPLAYMODE dmDesktop;
	D3DDISPLAYMODE dmDesktopBest;
	D3DDISPLAYMODE dmBest;

	// fortunately for us, D3DFMT_UNKNOWN == 0
	ZeroMemory(&dmDesktopBest, sizeof(D3DDISPLAYMODE));
	ZeroMemory(&dmBest, sizeof(D3DDISPLAYMODE));

	// 'best' storage
	AdapterInfo* paiBest = NULL;
	DeviceInfo*  pdiBest = NULL;
	DeviceCombo* pdcBest = NULL;

	// iterators
	AdapterInfo* pai;
	DeviceInfo*  pdi;
	DeviceCombo* pdc;

	// success flags
	bool bBetter, bBest;

	UINT i, j, k;

	// traverse the adpater infos
	for (i = 0; i < m_enum.AdapterInfos.Length(); i++)
	{
		pai = &m_enum.AdapterInfos[i];

		// get the current display mode of each adapter
		m_d3d->GetAdapterDisplayMode(pai->AdapterOrdinal, &dmDesktop);

		// traverse device infos on each adapter info
		for (j = 0; j < pai->DeviceInfos.Length(); j++)
		{
			pdi = &pai->DeviceInfos[j];

			// skip devices with other than the requested type
			if (HAL && pdi->DevType != D3DDEVTYPE_HAL)
				continue;

			if (REF && pdi->DevType != D3DDEVTYPE_REF)
				continue;

			// traverse device combos for each device info
			for (k = 0; k < pdi->DeviceCombos.Length(); k++)
			{
				pdc = &pdi->DeviceCombos[k];

				// skip the windowed combos
				if (pdc->Windowed)
					continue;

				// this device combo is 'better' than the current best if:
				//  (a) there's no best yet;
				//  (b) it's a HAL and the current best is not;
				//  (c) it's a HAL matching the desktop's format, while the
				//      current best does not;
				//  (d) it's a HAL and both the display and backbuffer formats
				//      match the desktop's, in which case it is also the best

				bBetter = pdcBest == NULL ||

					pdc->DevType == D3DDEVTYPE_HAL &&
					pdcBest->DevType != D3DDEVTYPE_HAL ||

					pdc->DevType == D3DDEVTYPE_HAL &&
					pdc->DisplayFormat == dmDesktop.Format &&
					pdcBest->DisplayFormat != dmDesktop.Format;

				bBest = pdc->DevType == D3DDEVTYPE_HAL &&
					pdc->DisplayFormat == dmDesktop.Format &&
					pdc->BackBufferFormat == dmDesktop.Format;

				bBetter |= bBest;

				if (bBetter)
				{
					// make it the best so far
					dmDesktopBest = dmDesktop;
					paiBest = pai;
					pdiBest = pdi;
					pdcBest = pdc;

					// this one looks great -- take it
					if (bBest)
						goto DoneSearchingFDC;
				}
			}
		}
	}

DoneSearchingFDC:

	// no suitable dc found!
	if (pdcBest == NULL)
		return false;

	// now we need to find a display mode on the best ai that uses the best
	// dc's display format and is as close to the best desktop display mode
	// as possible	
	D3DDISPLAYMODE dm;

	for (i = 0; i < paiBest->DisplayModes.Length(); i++)
	{
		dm = paiBest->DisplayModes[i];

		// formats must match
		if (dm.Format != dmDesktopBest.Format)
			continue;

		// compare other properties
		if (dm.Width == dmDesktopBest.Width &&
			dm.Height == dmDesktopBest.Height &&
			dm.RefreshRate == dmDesktopBest.RefreshRate)
		{
			// perfect match, break out
			dmBest = dm;
			break;
		}
		else if (dm.Width == dmDesktopBest.Width &&
			dm.Height == dmDesktopBest.Height &&
			dm.RefreshRate > dmBest.RefreshRate)
		{
			// faster
			dmBest = dm;
		}
		else if (dm.Width == dmDesktopBest.Width)
		{
			// same width
			dmBest = dm;
		}
		else if (dmBest.Width == 0)
		{
			// we don't have anything better yet
			dmBest = dm;
		}
	}

	// save these settings
	m_settings.Windowed = 0;
	m_settings.AdapterInfos[0] = paiBest;

	// index to the best dm within the ai
	m_settings.ndm[0] = paiBest->DisplayModes.Find(dm);

	// indices to the best di and dc
	if (bBest)
	{
		m_settings.ndi[0] = j;
		m_settings.ndc[0] = k;
	}
	else
	{
		// retract to the 'better' di and dc
		m_settings.ndi[0] = (UINT)paiBest->DeviceInfos.Find(*pdiBest);
		m_settings.ndc[0] = (UINT)pdiBest->DeviceCombos.Find(*pdcBest);
	}

	return true;
}