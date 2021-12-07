///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "WndWindow.h"
#include "DataManager.h"
#include <Render2D.h>
#include <WndControls.h>
#include <WndManager.h>

bool CWndWindow::s_showGrid = true;
int CWndWindow::s_gridSize = 2;
bool CWndWindow::s_cling = true;

CWndWindow::CWndWindow(CRender2D* render2D)
	: CWndControl(render2D),
	m_data(null),
	m_closeButton(null)
{
	m_flags = WBS_MOVE | WBS_SOUND | WBS_CAPTION;
}

CWndWindow::~CWndWindow()
{
}

void CWndWindow::InitialUpdate()
{
	m_data = DataMng->GetWindow(m_ID);
	if (!m_data)
		return;

	m_flags |= m_data->flags;
	if (HasFlag(WBS_NOFRAME))
		RemoveFlag(WBS_CAPTION);

	SetRect(QRect(QPoint(10, 10), m_data->size), false);
	m_text = m_data->title;
	SetTexture(m_data->texture, m_data->tiles);

	m_closeButton = new CWndButton(m_render2D);
	m_closeButton->Create(WTBID_CLOSE, QRect(), WBS_DOCKING, this);
	m_closeButton->SetTexture("ButtWndExit.tga", false);
	m_closeButton->FitTextureSize();

	for (int i = 0; i < m_data->controls.GetSize(); i++)
		CreateControl(m_data->controls[i]);
}

CWndControl* CWndWindow::CreateControl(int type, const QPoint& pos)
{
	ControlData* controlData = new ControlData();
	controlData->tiles = false;
	controlData->texture = "";
	controlData->text = "";
	controlData->rect = QRect(pos, QSize(30, 30));
	controlData->flags = WBS_CHILD;

	switch (type)
	{
	case CREATECTRL_TEXT:
		controlData->define = "WIDC_TEXT";
		controlData->type = WTYPE_TEXT;
		controlData->text = "Text";
		controlData->texture = "WndEditTile00.tga";
		controlData->tiles = true;
		controlData->rect.setSize(QSize(80, 40));
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_PICTURE:
		controlData->define = "WIDC_STATIC1";
		controlData->type = WTYPE_STATIC;
		controlData->text = "Picture";
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WSS_PICTURE;
		controlData->rect.setSize(QSize(128, 128));
#if __VER >= 19
		controlData->color = QColor(246, 204, 77);
#endif
		break;
	case CREATECTRL_STATIC:
		controlData->define = "WIDC_STATIC";
		controlData->type = WTYPE_STATIC;
		controlData->text = "Static";
		controlData->flags |= WBS_NOFRAME;
		controlData->rect.setSize(QSize(40, 20));
#if __VER >= 19
		controlData->color = QColor(246, 204, 77);
#endif
		break;
	case CREATECTRL_TEXTEDIT:
		controlData->define = "WIDC_EDIT";
		controlData->type = WTYPE_EDITCTRL;
		controlData->tiles = true;
		controlData->rect.setSize(QSize(120, 20));
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_GROUPBOX:
		controlData->define = "WIDC_GROUPBOX";
		controlData->type = WTYPE_STATIC;
		controlData->text = "GroupBox";
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WSS_GROUPBOX;
		controlData->rect.setSize(QSize(128, 128));
#if __VER >= 19
		controlData->color = QColor(246, 204, 77);
#endif
		break;
	case CREATECTRL_BUTTON:
		controlData->define = "WIDC_BUTTON";
		controlData->type = WTYPE_BUTTON;
		controlData->text = "Button";
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WBS_HIGHLIGHT;
		controlData->rect.setSize(QSize(80, 25));
#if __VER >= 19
		controlData->color = QColor(0, 0, 0);
#endif
		break;
	case CREATECTRL_CHECKBOX:
		controlData->define = "WIDC_CHECK";
		controlData->type = WTYPE_BUTTON;
		controlData->text = "CheckBox";
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WBS_CHECK;
		controlData->rect.setSize(QSize(80, 15));
#if __VER >= 19
		controlData->color = QColor(255, 249, 198);
#endif
		break;
	case CREATECTRL_RADIO:
		controlData->define = "WIDC_RADIO";
		controlData->type = WTYPE_BUTTON;
		controlData->text = "Radio";
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WBS_RADIO;
		controlData->rect.setSize(QSize(80, 15));
#if __VER >= 19
		controlData->color = QColor(255, 249, 198);
#endif
		break;
	case CREATECTRL_COMBOBOX:
		controlData->define = "WIDC_COMBOBOX";
		controlData->type = WTYPE_COMBOBOX;
		controlData->rect.setSize(QSize(120, 20));
		controlData->texture = "WndEditTile00.tga";
		controlData->tiles = true;
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_LISTBOX:
		controlData->define = "WIDC_LISTBOX";
		controlData->type = WTYPE_LISTBOX;
		controlData->rect.setSize(QSize(100, 120));
		controlData->texture = "WndEditTile00.tga";
		controlData->tiles = true;
		controlData->flags |= WBS_VSCROLL;
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_LISTCTRL:
		controlData->define = "WIDC_LISTCTRL";
		controlData->type = WTYPE_LISTCTRL;
		controlData->rect.setSize(QSize(100, 120));
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_TREECTRL:
		controlData->define = "WIDC_TREECTRL";
		controlData->type = WTYPE_TREECTRL;
		controlData->rect.setSize(QSize(100, 120));
		controlData->texture = "WndEditTile00.tga";
		controlData->tiles = true;
		controlData->flags |= WBS_VSCROLL;
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_TABCTRL:
		controlData->define = "WIDC_TABCTRL";
		controlData->type = WTYPE_TABCTRL;
		controlData->rect.setSize(QSize(119, 120));
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WBS_NODRAWFRAME;
#if __VER >= 19
		controlData->color = QColor(255, 255, 255);
#endif
		break;
	case CREATECTRL_CUSTOM:
		controlData->define = "WIDC_CUSTOM";
		controlData->type = WTYPE_CUSTOM;
		controlData->rect.setSize(QSize(64, 64));
		controlData->flags |= WBS_NOFRAME;
		controlData->flags |= WBS_NODRAWFRAME;
#if __VER >= 19
		controlData->color = QColor(46, 112, 169);
#endif
		break;
	}

	int id = 0;
	ControlData* ctrl = null;
	for (int i = 0; i < m_data->controls.GetSize(); i++)
	{
		ctrl = m_data->controls[i];
		if (ctrl->define.startsWith(controlData->define))
		{
			string define = ctrl->define;
			define = define.remove(controlData->define);
			define = define.trimmed();

			if (define.isEmpty() && id == 0)
				id = 1;
			else
			{
				bool ok = false;
				const int temp = define.toInt(&ok);
				if (ok)
				{
					if (temp >= id)
						id = temp + 1;
				}
			}
		}
	}

	if (id > 0)
		controlData->define += string::number(id);

	controlData->ID = DataMng->GetControlID(controlData->define);
	controlData->textID = DataMng->GetNewText(controlData->text);
	controlData->tooltip = "";
	controlData->tooltipID = DataMng->GetNewText(controlData->tooltip);
	controlData->disabled = false;
	controlData->group = false;
	controlData->visible = false;
	controlData->tabStop = false;
	m_data->controls.Append(controlData);

	return CreateControl(controlData);
}

CWndControl* CWndWindow::CreateControl(ControlData* controlData)
{
	CWndControl* control = null;
	int flags = controlData->flags;

	switch (controlData->type)
	{
	case WTYPE_BUTTON:
		control = new CWndButton(m_render2D);
		control->SetText(controlData->text);
		break;
	case WTYPE_STATIC:
		control = new CWndStatic(m_render2D);
		control->SetText(controlData->text);
		flags |= WBS_CAPTION;
		break;
	case WTYPE_TEXT:
		control = new CWndText(m_render2D);
		control->SetText(controlData->text);
		break;
	case WTYPE_LISTBOX:
		control = new CWndListBox(m_render2D);
		break;
	case WTYPE_EDITCTRL:
		control = new CWndEdit(m_render2D);
		break;
	case WTYPE_TREECTRL:
		control = new CWndTreeCtrl(m_render2D);
		break;
	case WTYPE_LISTCTRL:
		control = new CWndListCtrl(m_render2D);
		break;
	case WTYPE_TABCTRL:
		control = new CWndTabCtrl(m_render2D);
		break;
	case WTYPE_COMBOBOX:
		control = new CWndComboBox(m_render2D);
		break;
	case WTYPE_CUSTOM:
		control = new CWndCustom(m_render2D);
		break;
	default:
		control = new CWndControl(m_render2D);
		break;
	}

	control->Create(controlData->ID, controlData->rect, flags, this);
	control->SetEnabled(!controlData->disabled);

	if (controlData->type != WTYPE_TABCTRL)
	{
		if (controlData->texture.isEmpty())
			control->SetTexture(control->GetTextureName(), controlData->tiles);
		else
			control->SetTexture(controlData->texture, controlData->tiles);
	}

#if __VER >= 19
	control->SetColor(controlData->color);
#endif

	return control;
}

void CWndWindow::Draw()
{
	if (!s_showGrid)
		return;

	const QSize clientSize = m_clientRect.size();

	if (!HasFlag(WBS_NOFRAME))
		m_render2D->RenderRect(GetLayoutRect(), 0xff808080);
	else
		m_render2D->RenderRect(QRect(QPoint(8, 8), m_clientRect.size() - QSize(16, 16)), 0xff808080);

	int y;
	for (int x = 0; x < clientSize.width(); x += s_gridSize * 10)
		for (y = 0; y < clientSize.height(); y += s_gridSize * 10)
			m_render2D->RenderPoint(QPoint(x, y), 0xff808080);
}

void CWndWindow::Resize(const QSize& size)
{
	if (HasFlag(WBS_NOFRAME))
		m_closeButton->Move(QPoint(size.width() - 23, 5));
	else
#if __VER >= 19
		m_closeButton->Move(QPoint(size.width() - 11, 8));
#else
		m_closeButton->Move(QPoint(size.width() - 11, 5));
#endif
}

void CWndWindow::ChildNotify(int childID, EWndMsg msg)
{
	if (childID == WTBID_CLOSE && msg == EWndMsg::Clicked)
		WndMng->RemoveWindow(m_ID);
}

CWndControl* CWndWindow::GetEditableControl(const QPoint& mousePos)
{
	const QPoint clientMousePos = mousePos - GetClientRect(false).topLeft();
	CWndControl* ctrl = null;
	bool exists = false;
	for (int i = m_controls.GetSize() - 1; i >= 0; i--)
	{
		ctrl = m_controls[i];
		exists = false;
		for (int j = 0; j < m_data->controls.GetSize(); j++)
		{
			if (m_data->controls[j]->ID == ctrl->GetID())
			{
				exists = true;
				break;
			}
		}
		if (!exists)
			continue;

		if (ctrl->GetType() == WTYPE_STATIC && ctrl->HasFlag(BS_GROUPBOX))
		{
			QRect rect = ctrl->GetWindowRect(true);
			if (ctrl->HasFlag(WBS_DOCKING))
			{
				if (rect.contains(mousePos))
				{
					rect.adjust(4, 16, -8, -10);
					if (!rect.contains(mousePos))
						return ctrl;
				}
			}
			else
			{
				if (rect.contains(clientMousePos))
				{
					rect.adjust(4, 16, -8, -10);
					if (!rect.contains(clientMousePos))
						return ctrl;
				}
			}
		}
		else
		{
			if (ctrl->HasFlag(WBS_DOCKING))
			{
				if (ctrl->GetWindowRect(true).contains(mousePos))
					return ctrl;
			}
			else
			{
				if (ctrl->GetWindowRect(true).contains(clientMousePos))
					return ctrl;
			}
		}
	}
	return this;
}