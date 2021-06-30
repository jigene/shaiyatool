///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef GUIEDITOR_H
#define GUIEDITOR_H

#include <D3DWidget.h>

class CTextureMng;
class CRender2D;
class CWndManager;

enum EResizeDir
{
	RESIZE_NONE = 0x00,
	RESIZE_LEFT = 0x02,
	RESIZE_TOP = 0x04,
	RESIZE_RIGHT = 0x08,
	RESIZE_BOTTOM = 0x10
};

class CGUIEditor : public CD3DWidget
{
	Q_OBJECT

public:
	CGUIEditor(QWidget* parent = null, Qt::WindowFlags flags = 0);
	~CGUIEditor();

	virtual bool InitDeviceObjects();
	virtual void DeleteDeviceObjects();
	virtual bool RestoreDeviceObjects();
	virtual void InvalidateDeviceObjects();
	virtual bool Render();

	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void resizeEvent(QResizeEvent* event);

	CRender2D* GetRender2D() const { return m_render2D; }
	void SetBackgroundColor(DWORD color){ m_backgroundColor = color; }
	DWORD GetBackgroundColor() const { return m_backgroundColor; }

private:
	CTextureMng* m_textureMng;
	CRender2D* m_render2D;
	DWORD m_backgroundColor;
	CWndManager* m_wndMng;
	bool m_movingSelection;
	QMenu* m_editMenu;
	QPoint m_movingOff;
	int m_resizeDir;
	QRect m_originalRect;
};

#endif // GUIEDITOR_H