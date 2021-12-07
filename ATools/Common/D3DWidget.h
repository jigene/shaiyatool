///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef D3DWIDGET_H
#define D3DWIDGET_H

#include "D3DEnum.h"
#include "D3DSettings.h"

class CD3DWidget : public QWidget
{
	Q_OBJECT

public:
	CD3DWidget(QWidget* parent = null, Qt::WindowFlags flags = 0);
	void Destroy();

	bool CreateEnvironment();
	void CleanupEnvironment();

	bool IsAutoRefresh() const;
	void SetAutoRefresh(bool autoRefresh);

	LPDIRECT3DDEVICE9 GetDevice() const;
	string GetDeviceStats() const;

public slots:
	bool RenderEnvironment();

public:
	virtual bool InitDeviceObjects();
	virtual void DeleteDeviceObjects();
	virtual bool RestoreDeviceObjects();
	virtual void InvalidateDeviceObjects();
	virtual bool Render();
	virtual bool MoveFrame();
	virtual QPaintEngine* paintEngine() const;
	virtual void resizeEvent(QResizeEvent* event);

protected:
	LPDIRECT3D9 m_d3d;
	bool m_isFullscreen;
	LPDIRECT3DDEVICE9 m_device;
	wchar_t m_deviceStats[512];

private:
	D3DPRESENT_PARAMETERS m_presentParameters;
	D3D::CEnum m_enum;
	D3D::CSettings m_settings;
	bool m_deviceObjectsInitied;
	bool m_deviceObjectsRestored;
	bool m_deviceLost;
	QElapsedTimer m_elapsedTimer;
	QTimer m_timer;
	bool m_autoRefresh;
	bool m_skipFrame;

private:
	bool _chooseInitialSettings();
	bool _findBestWindowedMode(bool HAL, bool REF);
	bool _findBestFullscreenMode(bool HAL, bool REF);
	bool _initializeEnvironment();
	void _buildPresentParamsFromSettings();
};

#endif // D3DWIDGET_H