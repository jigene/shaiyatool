///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WNDCONTROLS_H
#define WNDCONTROLS_H

#include "WndControl.h"

class CWndButton : public CWndControl
{
public:
	CWndButton(CRender2D* render2D);

	virtual void InitialUpdate();
	virtual void PaintFrame();
	virtual void MouseButtonUp(Qt::MouseButton button, const QPoint& pos);
	void FitTextureSize();
	
	void SetChecked(bool checked) {
		m_checked = checked;
	}

	virtual int GetTilesCount() const {
		return 0;
	}
	virtual int GetType() const {
		return WTYPE_BUTTON;
	}

private:
	bool m_checked;
	QPoint _getTextPos() const;
	DWORD _getTextColor() const;
	void _render6Texture() const;
};

class CWndCustom : public CWndControl
{
public:
	CWndCustom(CRender2D* render2D);
	virtual void Draw();

	virtual int GetType() const {
		return WTYPE_CUSTOM;
	}
};

class CWndStatic : public CWndControl
{
public:
	CWndStatic(CRender2D* render2D);

	virtual void PaintFrame();
	virtual void InitialUpdate();
	virtual void Draw();

	virtual int GetType() const {
		return WTYPE_STATIC;
	}
};

class CWndScrollBar : public CWndControl
{
public:
	CWndScrollBar(CRender2D* render2D);

	virtual void PaintFrame();
	virtual void InitialUpdate();
	virtual void Resize(const QSize& size);

	virtual int GetType() const {
		return WTYPE_SCROLLBAR;
	}

private:
	CWndButton* m_arrow1;
	CWndButton* m_arrow2;
};

class CWndText : public CWndControl
{
public:
	CWndText(CRender2D* render2D);

	virtual void PaintFrame();
	virtual void InitialUpdate();
	virtual void Draw();
	virtual void Resize(const QSize& size);
	void UpdateScrollBar();

	virtual int GetType() const {
		return WTYPE_TEXT;
	}

protected:
	CWndScrollBar* m_scrollBar;
};

class CWndEdit : public CWndText
{
public:
	CWndEdit(CRender2D* render2D);

	virtual int GetType() const {
		return WTYPE_EDITCTRL;
	}
};

class CWndListBox : public CWndText
{
public:
	CWndListBox(CRender2D* render2D);

	virtual int GetType() const {
		return WTYPE_LISTBOX;
	}
};

class CWndTreeCtrl : public CWndText
{
public:
	CWndTreeCtrl(CRender2D* render2D);

	virtual int GetType() const {
		return WTYPE_TREECTRL;
	}
};

class CWndListCtrl : public CWndText
{
public:
	CWndListCtrl(CRender2D* render2D);

	virtual void PaintFrame();
	virtual int GetTilesCount() const {
		return 0;
	}
	virtual int GetType() const {
		return WTYPE_LISTCTRL;
	}
};

class CWndTabCtrl : public CWndControl
{
public:
	CWndTabCtrl(CRender2D* render2D);

	virtual void Draw();
	virtual void InitialUpdate();
	virtual int GetTilesCount() const {
		return 0;
	}
	virtual int GetType() const {
		return WTYPE_TABCTRL;
	}

private:
	CTexture* m_tabTextures[11];
};

class CWndComboBoxListBox : public CWndListBox
{
public:
	CWndComboBoxListBox(CRender2D* render2D);
	virtual void LostFocus();
};

class CWndComboBox : public CWndControl
{
public:
	CWndComboBox(CRender2D* render2D);

	virtual void InitialUpdate();
	virtual void Resize(const QSize& size);
	virtual void ChildNotify(int childID, EWndMsg msg);
	virtual int GetType() const {
		return WTYPE_COMBOBOX;
	}

private:
	CWndButton* m_button;
	CWndComboBoxListBox* m_wndListBox;
};

#endif // WNDCONTROLS_H