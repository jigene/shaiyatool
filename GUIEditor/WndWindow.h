///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WNDWINDOW_H
#define WNDWINDOW_H

#include <WndControl.h>

#define WTYPE_GUI_EDITOR_WND	92178
#define WTBID_CLOSE     10000

struct WindowData;
class CWndButton;
struct ControlData;

enum ECreateControlType
{
	CREATECTRL_NONE,
	CREATECTRL_TEXT,
	CREATECTRL_PICTURE,
	CREATECTRL_STATIC,
	CREATECTRL_TEXTEDIT,
	CREATECTRL_GROUPBOX,
	CREATECTRL_BUTTON,
	CREATECTRL_CHECKBOX,
	CREATECTRL_RADIO,
	CREATECTRL_COMBOBOX,
	CREATECTRL_LISTBOX,
	CREATECTRL_LISTCTRL,
	CREATECTRL_TREECTRL,
	CREATECTRL_TABCTRL,
	CREATECTRL_CUSTOM
};

class CWndWindow : public CWndControl
{
public:
	CWndWindow(CRender2D* render2D);
	virtual ~CWndWindow();

	virtual void InitialUpdate();
	virtual void Draw();
	virtual void Resize(const QSize& size);
	virtual void ChildNotify(int childID, EWndMsg msg);
	virtual CWndControl* GetEditableControl(const QPoint& mousePos);

	CWndControl* CreateControl(int type, const QPoint& pos);
	CWndControl* CreateControl(ControlData* controlData);

	WindowData* Data() const {
		return m_data;
	}

	virtual int GetTilesCount() const {
		return 12;
	}
	virtual int GetType() const {
		return WTYPE_GUI_EDITOR_WND;
	}

private:
	WindowData* m_data;
	CWndButton* m_closeButton;

public:
	static bool s_showGrid;
	static int s_gridSize;
	static bool s_cling;
};

#endif // WNDWINDOW_H