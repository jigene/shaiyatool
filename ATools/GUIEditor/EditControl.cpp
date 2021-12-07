///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "DataManager.h"
#include "GUIEditor.h"
#include "WndWindow.h"
#include <WndManager.h>
#include <WndControls.h>

void CMainFrame::CopyControl()
{
	if (CWndControl::s_selection.GetSize() < 1
		|| !CWndControl::s_selection[0]
		|| CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	int i;
	for (i = 0; i < m_clipboardControls.GetSize(); i++)
		Delete(m_clipboardControls[i]);
	m_clipboardControls.RemoveAll();

	CWndControl* ctrl;
	for (i = 0; i < CWndControl::s_selection.GetSize(); i++)
	{
		ctrl = CWndControl::s_selection[i];
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				ControlData* newData = new ControlData();
				(*newData) = (*data);
				m_clipboardControls.Append(newData);
			}
		}
	}

	QPoint minPos(99999, 99999);
	for (i = 0; i < m_clipboardControls.GetSize(); i++)
	{
		if (m_clipboardControls[i]->rect.left() < minPos.x())
			minPos.setX(m_clipboardControls[i]->rect.left());
		if (m_clipboardControls[i]->rect.top() < minPos.y())
			minPos.setY(m_clipboardControls[i]->rect.top());
	}

	for (i = 0; i < m_clipboardControls.GetSize(); i++)
	{
		m_clipboardControls[i]->rect = QRect(m_clipboardControls[i]->rect.topLeft() - minPos,
			m_clipboardControls[i]->rect.size());
	}
}

void CMainFrame::PasteControl()
{
	if (CWndControl::s_selection.GetSize() < 1
		|| !CWndControl::s_selection[0]
		|| m_clipboardControls.GetSize() <= 0)
		return;

	CWndWindow* wnd = null;
	if (CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		wnd = (CWndWindow*)CWndControl::s_selection[0];
	else
		wnd = (CWndWindow*)CWndControl::s_selection[0]->GetParent();

	if (!wnd)
		return;

	WindowData* wndData = m_dataMng->GetWindow(wnd->GetID());
	if (!wndData)
		return;

	CWndControl::s_selection.RemoveAll();

	ControlData* controlData, *ctrl;
	int i, id, j;
	for (i = 0; i < m_clipboardControls.GetSize(); i++)
	{
		controlData = new ControlData();
		(*controlData) = (*m_clipboardControls[i]);

		id = 0;
		for (j = 0; j < wndData->controls.GetSize(); j++)
		{
			ctrl = wndData->controls[j];
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
		controlData->tooltipID = DataMng->GetNewText(controlData->tooltip);
		wndData->controls.Append(controlData);
		CWndControl::s_selection.Append(wnd->CreateControl(controlData));
	}

	SetEditControl();
}

void CMainFrame::CutControl()
{
	if (CWndControl::s_selection.GetSize() < 1
		|| !CWndControl::s_selection[0]
		|| CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	CopyControl();
	DeleteControl();
}

void CMainFrame::SetControlPos(const QPoint& pos)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			const QSize size = data->rect.size();
			data->rect.setTopLeft(pos);
			data->rect.setSize(size);
		}
	}

	CWndControl::s_selection[0] = null;
	ui.editControlX->setValue(pos.x());
	ui.editControlY->setValue(pos.y());
	CWndControl::s_selection[0] = ctrl;
}

void CMainFrame::SetControlRect(const QRect& rect)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	CWndControl::s_selection[0] = null;

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
			wnd->size = rect.size();
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
				data->rect = rect;
		}

		ui.editControlX->setValue(rect.left());
		ui.editControlY->setValue(rect.top());
	}

	ui.editControlWidth->setValue(rect.width());
	ui.editControlHeight->setValue(rect.height());
	CWndControl::s_selection[0] = ctrl;
}

void CMainFrame::SetEditControl()
{
	CWndControl* editCtrl = null;
	if (CWndControl::s_selection.GetSize() >= 1)
		editCtrl = CWndControl::s_selection[0];

	if (editCtrl)
	{
		CWndControl::s_selection[0] = null;

		WindowData* wndData = null;
		ControlData* ctrlData = null;

		if (editCtrl->GetType() == WTYPE_GUI_EDITOR_WND)
			wndData = ((CWndWindow*)editCtrl)->Data();
		else
		{
			wndData = ((CWndWindow*)editCtrl->GetParent())->Data();

			for (int i = 0; i < wndData->controls.GetSize(); i++)
			{
				if (wndData->controls[i]->ID == editCtrl->GetID())
				{
					ctrlData = wndData->controls[i];
					break;
				}
			}

			wndData = null;
		}

		ui.dockEdit->setEnabled(true);
		if (wndData)
		{
			ui.editControlID->setEditable(true);
			ui.editControlID->setCurrentText(((CWndWindow*)editCtrl)->Data()->define);
			ui.editControlID->setEnabled(false);
			ui.editControlWidth->setValue(wndData->size.width());
			ui.editControlHeight->setValue(wndData->size.height());
			ui.editControlText->setText(wndData->title);
			ui.editControlTexture->setText(wndData->texture);
			ui.editControlTooltip->setText(wndData->tooltip);
			ui.btnAdaptSizeToTexture->setEnabled(false);
			ui.editControlNoDrawFrame->setChecked((wndData->flags & WBS_NODRAWFRAME) != 0);
			ui.editControlNoDrawFrame->setEnabled(true);
			ui.editControlTiles->setChecked(wndData->tiles);

			ui.groupEditControl->setEnabled(false);
			ui.editControlEnabled->setChecked(false);
			ui.editControlVisible->setChecked(false);
			ui.editControlGroup->setChecked(false);
			ui.editControlTabStop->setChecked(false);
			ui.editControlScrollBar->setChecked(false);
			ui.editControlX->setValue(0);
			ui.editControlY->setValue(0);
			ui.editVAlignTop->setChecked(true);
			ui.editHAlignLeft->setChecked(true);

			ui.groupEditWindow->setEnabled(true);
			ui.editWindowCaption->setChecked((wndData->flags & WBS_CAPTION) != 0);
			ui.editWindowNoFrame->setChecked((wndData->flags & WBS_NOFRAME) != 0);

			if (wndData->format == D3DFMT_A4R4G4B4)
				ui.editWindowD3DFormat->setCurrentText("A4R4G4B4");
			else if (wndData->format == D3DFMT_A8R8G8B8)
				ui.editWindowD3DFormat->setCurrentText("A8R8G8B8");

#if __VER >= 19
			ui.editWindowIcon->setEnabled(true);
			ui.editWindowIcon->setText(wndData->icon);

			ui.editControlColor->setEnabled(false);
			ui.editControlViewColor->setStyleSheet("background-color: #7f7f7f;");
#endif
		}
		else
		{
			ui.editControlID->setEditable(false);
			ui.editControlID->setCurrentText(ctrlData->define);
			ui.editControlID->setEnabled(true);
			ui.editControlWidth->setValue(ctrlData->rect.width());
			ui.editControlHeight->setValue(ctrlData->rect.height());
			ui.editControlText->setText(ctrlData->text);
			ui.editControlTexture->setText(ctrlData->texture);
			ui.editControlTooltip->setText(ctrlData->tooltip);
			ui.editControlTiles->setChecked(ctrlData->tiles);
			if (ctrlData->type == WTYPE_BUTTON)
				ui.btnAdaptSizeToTexture->setEnabled(true);
			else
				ui.btnAdaptSizeToTexture->setEnabled(false);
			ui.editControlNoDrawFrame->setChecked((ctrlData->flags & WBS_NODRAWFRAME) != 0);
			if (ctrlData->type == WTYPE_TABCTRL)
				ui.editControlNoDrawFrame->setEnabled(false);
			else
				ui.editControlNoDrawFrame->setEnabled(true);

			ui.groupEditControl->setEnabled(true);
			ui.editControlEnabled->setChecked(!ctrlData->disabled);
			ui.editControlVisible->setChecked(ctrlData->visible);
			ui.editControlGroup->setChecked(ctrlData->group);
			ui.editControlTabStop->setChecked(ctrlData->tabStop);
			ui.editControlScrollBar->setChecked((ctrlData->flags & WBS_VSCROLL) != 0);
			ui.editControlX->setValue(ctrlData->rect.left());
			ui.editControlY->setValue(ctrlData->rect.top());

			if (ctrlData->type == WTYPE_STATIC)
			{
				ui.groupEditHAlign->setEnabled(true);
				ui.groupEditVAlign->setEnabled(true);

				if ((ctrlData->flags & WSS_ALIGNHRIGHT) != 0)
					ui.editHAlignRight->setChecked(true);
				else if ((ctrlData->flags & WSS_ALIGNHCENTER) != 0)
					ui.editHAlignCenter->setChecked(true);
				else
					ui.editHAlignLeft->setChecked(true);

				if ((ctrlData->flags & WSS_ALIGNVBOTTOM) != 0)
					ui.editVAlignBottom->setChecked(true);
				else if ((ctrlData->flags & WSS_ALIGNVCENTER) != 0)
					ui.editVAlignMiddle->setChecked(true);
				else
					ui.editVAlignTop->setChecked(true);
			}
			else
			{
				ui.editVAlignTop->setChecked(true);
				ui.editHAlignLeft->setChecked(true);
				ui.groupEditHAlign->setEnabled(false);
				ui.groupEditVAlign->setEnabled(false);
			}

			ui.groupEditWindow->setEnabled(false);
			ui.editWindowCaption->setChecked(false);
			ui.editWindowNoFrame->setChecked(false);
			ui.editWindowD3DFormat->setCurrentText("A4R4G4B4");

#if __VER >= 19
			ui.editWindowIcon->setEnabled(false);
			ui.editWindowIcon->setText("");

			ui.editControlColor->setEnabled(true);
			ui.editControlViewColor->setStyleSheet("background-color: " + ctrlData->color.name() + ';');
#endif
		}

		CWndControl::s_selection[0] = editCtrl;
	}
	else
	{
		ui.editControlID->setEditable(true);
		ui.editControlID->setCurrentText("");
		ui.editControlWidth->setValue(1);
		ui.editControlHeight->setValue(1);
		ui.editControlText->setText("");
		ui.editControlTexture->setText("");
		ui.editControlTooltip->setText("");
		ui.btnAdaptSizeToTexture->setEnabled(false);
		ui.editControlNoDrawFrame->setChecked(false);
		ui.editControlNoDrawFrame->setEnabled(true);
		ui.editControlTiles->setChecked(false);
		ui.dockEdit->setEnabled(false);

		ui.groupEditControl->setEnabled(false);
		ui.editControlEnabled->setChecked(false);
		ui.editControlVisible->setChecked(false);
		ui.editControlGroup->setChecked(false);
		ui.editControlTabStop->setChecked(false);
		ui.editControlScrollBar->setChecked(false);
		ui.editControlX->setValue(0);
		ui.editControlY->setValue(0);
		ui.editVAlignTop->setChecked(true);
		ui.editHAlignLeft->setChecked(true);

		ui.groupEditWindow->setEnabled(false);
		ui.editWindowCaption->setChecked(false);
		ui.editWindowNoFrame->setChecked(false);
		ui.editWindowD3DFormat->setCurrentText("A4R4G4B4");

#if __VER >= 19
		ui.editWindowIcon->setEnabled(false);
		ui.editWindowIcon->setText("");

		ui.editControlColor->setEnabled(false);
		ui.editControlViewColor->setStyleSheet("background-color: #7f7f7f;");
#endif
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlID(const QString & text)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (!wnd)
		return;

	ControlData* ctrlData = null;
	int i;
	for (int i = 0; i < wnd->controls.GetSize(); i++)
	{
		if (wnd->controls[i]->ID == ctrl->GetID())
		{
			ctrlData = wnd->controls[i];
			break;
		}
	}

	if (!ctrlData)
		return;

	for (int i = 0; i < wnd->controls.GetSize(); i++)
	{
		if (wnd->controls[i]->define == text)
		{
			QMessageBox::warning(this, tr("Erreur"), tr("Cet ID est déjà utilisé."));
			CWndControl::s_selection[0] = null;
			ui.editControlID->setCurrentText(ctrlData->define);
			CWndControl::s_selection[0] = ctrl;
			return;
		}
	}

	ctrlData->define = text;
	ctrlData->ID = m_dataMng->GetControlID(text);
	ctrl->SetID(ctrlData->ID);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlWidth(int width)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
			wnd->size.setWidth(width);
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
				data->rect.setWidth(width);
		}
	}

	QRect rect = ctrl->GetWindowRect(true);
	rect.setWidth(width);
	ctrl->SetRect(rect, true);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlHeight(int height)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
			wnd->size.setHeight(height);
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
				data->rect.setHeight(height);
		}
	}

	QRect rect = ctrl->GetWindowRect(true);
	rect.setHeight(height);
	ctrl->SetRect(rect, true);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlText(const QString& text)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
		{
			wnd->title = text;
			m_dataMng->SetText(wnd->titleID, text);
		}
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				data->text = text;
				m_dataMng->SetText(data->textID, text);
			}
		}
	}

	switch (ctrl->GetType())
	{
	case WTYPE_GUI_EDITOR_WND:
	case WTYPE_STATIC:
	case WTYPE_BUTTON:
	case WTYPE_TEXT:
		ctrl->SetText(text);
		break;
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlTooltip(const QString& text)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
		{
			wnd->tooltip = text;
			m_dataMng->SetText(wnd->tooltipID, text);
		}
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				data->tooltip = text;
				m_dataMng->SetText(data->tooltipID, text);
			}
		}
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlTexture()
{
	const string texture = ui.editControlTexture->text();

	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
			wnd->texture = texture;
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
				data->texture = texture;
		}
	}

	if (ctrl->GetType() != WTYPE_TABCTRL)
		ctrl->SetTexture(texture, ctrl->Tiles());
	m_editor->RenderEnvironment();
}

void CMainFrame::SetControlTexture()
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	const string texture = QFileDialog::getOpenFileName(this, tr("Charger une texture"), "Theme/Default/" + CWndControl::s_selection[0]->GetTextureName(), tr("Fichier texture") + " (*.bmp *.tga)");
	if (!texture.isEmpty())
	{
		ui.editControlTexture->setText(QFileInfo(texture).fileName());
		EditControlTexture();
	}
}

void CMainFrame::ButtonFitTextureSize()
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_BUTTON)
		return;

	((CWndButton*)ctrl)->FitTextureSize();

	const QSize size = ctrl->GetWindowRect(true).size();

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
			data->rect.setSize(size);
	}

	CWndControl::s_selection[0] = null;
	ui.editControlWidth->setValue(size.width());
	ui.editControlHeight->setValue(size.height());
	CWndControl::s_selection[0] = ctrl;

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlTiles(bool tiles)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
			wnd->tiles = tiles;
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
				data->tiles = tiles;
		}
	}

	if (ctrl->GetType() != WTYPE_TABCTRL)
		ctrl->SetTexture(ctrl->GetTextureName(), tiles);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlNoDrawFrame(bool enable)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	const uint flag = WBS_NODRAWFRAME;

	CWndControl* ctrl = CWndControl::s_selection[0];

	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
		if (wnd)
		{
			if (enable)
				wnd->flags |= flag;
			else
				wnd->flags &= ~flag;
		}
	}
	else
	{
		WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				if (enable)
					data->flags |= flag;
				else
					data->flags &= ~flag;
			}
		}
	}

	if (enable)
		ctrl->SetFlag(flag);
	else
		ctrl->RemoveFlag(flag);

	m_editor->RenderEnvironment();
}

void CMainFrame::SetCreateControl(QAction* action)
{
	m_createControlType = CREATECTRL_NONE;

	if (action == ui.actionText)
		m_createControlType = CREATECTRL_TEXT;
	else if (action == ui.actionPicture)
		m_createControlType = CREATECTRL_PICTURE;
	else if (action == ui.actionStatic)
		m_createControlType = CREATECTRL_STATIC;
	else if (action == ui.actionText_Edit)
		m_createControlType = CREATECTRL_TEXTEDIT;
	else if (action == ui.actionGroupBox)
		m_createControlType = CREATECTRL_GROUPBOX;
	else if (action == ui.actionButton)
		m_createControlType = CREATECTRL_BUTTON;
	else if (action == ui.actionCheckBox)
		m_createControlType = CREATECTRL_CHECKBOX;
	else if (action == ui.actionRadioButton)
		m_createControlType = CREATECTRL_RADIO;
	else if (action == ui.actionComboBox)
		m_createControlType = CREATECTRL_COMBOBOX;
	else if (action == ui.actionListBox)
		m_createControlType = CREATECTRL_LISTBOX;
	else if (action == ui.actionListCtrl)
		m_createControlType = CREATECTRL_LISTCTRL;
	else if (action == ui.actionTreeCtrl)
		m_createControlType = CREATECTRL_TREECTRL;
	else if (action == ui.actionTabCtrl)
		m_createControlType = CREATECTRL_TABCTRL;
	else if (action == ui.actionCustom)
		m_createControlType = CREATECTRL_CUSTOM;

	if (m_createControlType != CREATECTRL_NONE)
	{
		CWndControl::s_selection.RemoveAll();
		SetEditControl();
	}
}

void CMainFrame::SetControlOnTop()
{
	if (CWndControl::s_selection.GetSize() <= 0)
		return;

	CWndControl* ctrl = null;
	for (int i = CWndControl::s_selection.GetSize() - 1; i >= 0; i--)
	{
		ctrl = CWndControl::s_selection[i];
		ctrl->GetParent()->SetControlOnTop(ctrl);

		if (ctrl->GetType() != WTYPE_GUI_EDITOR_WND)
		{
			WindowData* data = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
			if (data)
			{
				for (int j = 0; j < data->controls.GetSize(); j++)
				{
					if (data->controls[j]->ID == ctrl->GetID())
					{
						ControlData* ctrlData = data->controls[j];
						data->controls.RemoveAt(j);
						data->controls.Append(ctrlData);
						break;
					}
				}
			}
		}
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::AddNewWindow()
{
	const int newID = m_dataMng->GetNewWindowID();
	bool ok = false;
	const string text = QInputDialog::getText(this, tr("Nouvelle fenêtre"),
		tr("ID de la nouvelle fenêtre: "), QLineEdit::Normal, "APP_APPLET" + string::number(newID), &ok);
	if (ok)
	{
		if (m_dataMng->WindowExists(text))
		{
			QMessageBox::warning(null, tr("Erreur"), tr("Cet ID est déjà utilisé."));
			return;
		}

		m_dataMng->AddNewWindow(text, newID);
		m_dataMng->FillWindowList(m_windowList);
		const int find = m_windowList->stringList().indexOf(text);
		if (find != -1)
		{
			QModelIndex index = m_windowList->index(find, 0);
			ui.listWindows->setCurrentIndex(index);
			ShowWindow(index);
		}
	}
}

void CMainFrame::DeleteControl()
{
	if (CWndControl::s_selection.GetSize() <= 0)
		return;

	bool updateWindowList = false;
	CWndControl* ctrl = null;
	while (CWndControl::s_selection.GetSize() > 0)
	{
		ctrl = CWndControl::s_selection[0];

		if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		{
			updateWindowList = true;
			m_dataMng->RemoveWindow(ctrl->GetID());
		}
		else
			m_dataMng->RemoveControl(ctrl->GetParent()->GetID(), ctrl->GetID());

		ctrl->GetParent()->RemoveControl(ctrl->GetID());
	}

	if (updateWindowList)
		m_dataMng->FillWindowList(m_windowList);

	SetEditControl();
}

void CMainFrame::DeleteWindow()
{
	QModelIndex index = ui.listWindows->currentIndex();
	if (index.isValid())
	{
		const QString windowIDStr = m_windowList->stringList().at(index.row());
		const int windowID = m_dataMng->GetWindowID(windowIDStr);
		if (windowID != -1)
		{
			CWndControl* ctrl = WndMng->GetControl(windowID);
			if (ctrl)
			{
				CWndControl::s_selection.RemoveAll();
				CWndControl::s_selection.Append(ctrl);
				DeleteControl();
			}
		}
	}
}

void CMainFrame::ShowWindow(const QModelIndex& index)
{
	const QString windowID = m_windowList->stringList().at(index.row());
	WindowData* data = DataMng->GetWindow(windowID);

	if (data)
	{
		const int ID = data->ID;

		CWndWindow* window = (CWndWindow*)WndMng->GetControl(ID);
		if (window)
			WndMng->SetControlOnTop(window);
		else
		{
			window = new CWndWindow(m_editor->GetRender2D());
			window->Create(data->ID, QRect(100, 100, 100, 100), 0, WndMng);
		}

		CWndControl::s_selection.RemoveAll();
		CWndControl::s_selection.Append(window);

		SetEditControl();
	}
}

void CMainFrame::AlignControlsOnTop()
{
	if (CWndControl::s_selection.GetSize() <= 1)
		return;
	if (CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	const int align = CWndControl::s_selection[0]->GetWindowRect(true).top();

	WindowData* wnd = m_dataMng->GetWindow(CWndControl::s_selection[0]->GetParent()->GetID());

	CWndControl* ctrl = null;
	QPoint pt;
	for (int i = 0; i < CWndControl::s_selection.GetSize(); i++)
	{
		ctrl = CWndControl::s_selection[i];
		pt = ctrl->GetWindowRect(true).topLeft();
		pt.setY(align);
		ctrl->Move(pt);

		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				const QSize size = data->rect.size();
				data->rect.setTop(align);
				data->rect.setSize(size);
			}
		}
	}

	SetControlOnTop();
	m_editor->RenderEnvironment();
}

void CMainFrame::AlignControlsOnLeft()
{
	if (CWndControl::s_selection.GetSize() <= 1)
		return;
	if (CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	const int align = CWndControl::s_selection[0]->GetWindowRect(true).left();

	WindowData* wnd = m_dataMng->GetWindow(CWndControl::s_selection[0]->GetParent()->GetID());

	CWndControl* ctrl = null;
	QPoint pt;
	for (int i = 0; i < CWndControl::s_selection.GetSize(); i++)
	{
		ctrl = CWndControl::s_selection[i];
		pt = ctrl->GetWindowRect(true).topLeft();
		pt.setX(align);
		ctrl->Move(pt);

		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				const QSize size = data->rect.size();
				data->rect.setLeft(align);
				data->rect.setSize(size);
			}
		}
	}

	SetControlOnTop();
	m_editor->RenderEnvironment();
}

void CMainFrame::AlignControlsOnRight()
{
	if (CWndControl::s_selection.GetSize() <= 1)
		return;
	if (CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	const int align = CWndControl::s_selection[0]->GetWindowRect(true).right();

	WindowData* wnd = m_dataMng->GetWindow(CWndControl::s_selection[0]->GetParent()->GetID());

	CWndControl* ctrl = null;
	QPoint pt;
	for (int i = 0; i < CWndControl::s_selection.GetSize(); i++)
	{
		ctrl = CWndControl::s_selection[i];
		pt = ctrl->GetWindowRect(true).topLeft();
		pt.setX(align - ctrl->GetWindowRect(true).width() + 1);
		ctrl->Move(pt);

		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				const QSize size = data->rect.size();
				data->rect.setLeft(align - data->rect.width() + 1);
				data->rect.setSize(size);
			}
		}
	}

	SetControlOnTop();
	m_editor->RenderEnvironment();
}

void CMainFrame::AlignControlsOnBottom()
{
	if (CWndControl::s_selection.GetSize() <= 1)
		return;
	if (CWndControl::s_selection[0]->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	const int align = CWndControl::s_selection[0]->GetWindowRect(true).bottom();

	WindowData* wnd = m_dataMng->GetWindow(CWndControl::s_selection[0]->GetParent()->GetID());

	CWndControl* ctrl = null;
	QPoint pt;
	for (int i = 0; i < CWndControl::s_selection.GetSize(); i++)
	{
		ctrl = CWndControl::s_selection[i];
		pt = ctrl->GetWindowRect(true).topLeft();
		pt.setY(align - ctrl->GetWindowRect(true).height() + 1);
		ctrl->Move(pt);

		if (wnd)
		{
			ControlData* data = wnd->GetControl(ctrl->GetID());
			if (data)
			{
				const QSize size = data->rect.size();
				data->rect.setTop(align - data->rect.height() + 1);
				data->rect.setSize(size);
			}
		}
	}

	SetControlOnTop();
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlPosX(int x)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			const QSize size = data->rect.size();
			data->rect.setX(x);
			data->rect.setSize(size);
		}
	}

	QRect rect = ctrl->GetWindowRect(true);
	const QSize size = rect.size();
	rect.setX(x);
	rect.setSize(size);
	ctrl->SetRect(rect, true);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlPosY(int y)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			const QSize size = data->rect.size();
			data->rect.setY(y);
			data->rect.setSize(size);
		}
	}

	QRect rect = ctrl->GetWindowRect(true);
	const QSize size = rect.size();
	rect.setY(y);
	rect.setSize(size);
	ctrl->SetRect(rect, true);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlVisible(bool visible)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
			data->visible = visible;
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlEnabled(bool enabled)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
			data->disabled = !enabled;
	}

	ctrl->SetEnabled(enabled);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlTabStop(bool tabStop)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
			data->tabStop = tabStop;
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlGroup(bool group)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
			data->group = group;
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlScrollBar(bool scrollBar)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	const uint flag = WBS_VSCROLL;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			if (scrollBar)
				data->flags |= flag;
			else
				data->flags &= ~flag;
		}
	}

	if (scrollBar)
		ctrl->SetFlag(flag);
	else
		ctrl->RemoveFlag(flag);

	switch (ctrl->GetType())
	{
	case WTYPE_TEXT:
	case WTYPE_EDITCTRL:
	case WTYPE_LISTBOX:
	case WTYPE_TREECTRL:
	case WTYPE_LISTCTRL:
		((CWndText*)ctrl)->UpdateScrollBar();
		break;
	}

	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlHAlignLeft(bool align)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_STATIC || !align)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			data->flags &= ~WSS_ALIGNHRIGHT;
			data->flags &= ~WSS_ALIGNHCENTER;
		}
	}

	ctrl->RemoveFlag(WSS_ALIGNHRIGHT);
	ctrl->RemoveFlag(WSS_ALIGNHCENTER);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlHAlignMiddle(bool align)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_STATIC || !align)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			data->flags &= ~WSS_ALIGNHRIGHT;
			data->flags |= WSS_ALIGNHCENTER;
		}
	}

	ctrl->RemoveFlag(WSS_ALIGNHRIGHT);
	ctrl->SetFlag(WSS_ALIGNHCENTER);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlHAlignRight(bool align)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_STATIC || !align)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			data->flags &= ~WSS_ALIGNHCENTER;
			data->flags |= WSS_ALIGNHRIGHT;
		}
	}

	ctrl->RemoveFlag(WSS_ALIGNHCENTER);
	ctrl->SetFlag(WSS_ALIGNHRIGHT);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlVAlignTop(bool align)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_STATIC || !align)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			data->flags &= ~WSS_ALIGNVBOTTOM;
			data->flags &= ~WSS_ALIGNVCENTER;
		}
	}

	ctrl->RemoveFlag(WSS_ALIGNVBOTTOM);
	ctrl->RemoveFlag(WSS_ALIGNVCENTER);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlVAlignCenter(bool align)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_STATIC || !align)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			data->flags &= ~WSS_ALIGNVBOTTOM;
			data->flags |= WSS_ALIGNVCENTER;
		}
	}

	ctrl->RemoveFlag(WSS_ALIGNVBOTTOM);
	ctrl->SetFlag(WSS_ALIGNVCENTER);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditControlVAlignBottom(bool align)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_STATIC || !align)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			data->flags &= ~WSS_ALIGNVCENTER;
			data->flags |= WSS_ALIGNVBOTTOM;
		}
	}

	ctrl->RemoveFlag(WSS_ALIGNVCENTER);
	ctrl->SetFlag(WSS_ALIGNVBOTTOM);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditWindowCaption(bool caption)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	const uint flag = WBS_CAPTION;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
	if (wnd)
	{
		if (caption)
			wnd->flags |= flag;
		else
			wnd->flags &= ~flag;
	}


	if (caption)
		ctrl->SetFlag(flag);
	else
		ctrl->RemoveFlag(flag);

	ctrl->SetRect(ctrl->GetWindowRect(true), true);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditWindowNoFrame(bool noFrame)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	const uint flag = WBS_NOFRAME;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
	if (wnd)
	{
		if (noFrame)
			wnd->flags |= flag;
		else
			wnd->flags &= ~flag;
	}


	if (noFrame)
		ctrl->SetFlag(flag);
	else
		ctrl->RemoveFlag(flag);

	ctrl->SetRect(ctrl->GetWindowRect(true), true);
	m_editor->RenderEnvironment();
}

void CMainFrame::EditWindowFormat(const QString& format)
{
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_GUI_EDITOR_WND)
		return;

	D3DFORMAT value = D3DFMT_A4R4G4B4;
	if (format == "A8R8G8B8")
		value = D3DFMT_A8R8G8B8;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
	if (wnd)
		wnd->format = value;

	m_editor->RenderEnvironment();
}

void CMainFrame::EditWindowIcon(const QString& icon)
{
#if __VER >= 19
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() != WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetID());
	if (wnd)
		wnd->icon = icon;

	m_editor->RenderEnvironment();
#endif
}

void CMainFrame::EditControlColor()
{
#if __VER >= 19
	if (CWndControl::s_selection.GetSize() <= 0 || !CWndControl::s_selection[0])
		return;

	CWndControl* ctrl = CWndControl::s_selection[0];
	if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
		return;

	WindowData* wnd = m_dataMng->GetWindow(ctrl->GetParent()->GetID());
	if (wnd)
	{
		ControlData* data = wnd->GetControl(ctrl->GetID());
		if (data)
		{
			const QColor color = QColorDialog::getColor(data->color, this, tr("Changer la couleur"));
			if (color.isValid())
			{
				data->color = color;
				ctrl->SetColor(color);
				ui.editControlViewColor->setStyleSheet("background-color: " + color.name() + ';');
				m_editor->RenderEnvironment();
			}
		}
	}
#endif
}