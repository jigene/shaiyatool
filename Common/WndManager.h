///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WNDMANAGER_H
#define WNDMANAGER_H

#include "WndControl.h"

class CWndManager : public CWndControl
{
public:
	static CWndManager* Instance;

	CWndManager(CRender2D* render2D);
	~CWndManager();

	void PaintRoot();
	void RemoveWindow(int ID, bool deleteControl = true);

	virtual void MouseButtonDown(Qt::MouseButton button, const QPoint& pos);
	virtual CWndControl* GetEditableControl(const QPoint& mousePos);
	virtual CWndControl* GetWindow(const QPoint& mousePos);

private:
	CPtrArray<CWndControl> m_removeControls;
};

#define WndMng CWndManager::Instance

#endif // WNDMANAGER_H