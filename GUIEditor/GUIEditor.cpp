///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "GUIEditor.h"
#include <TextureMng.h>
#include <Render2D.h>
#include <WndManager.h>
#include "WndWindow.h"
#include <Font.h>
#include "MainFrame.h"

CGUIEditor::CGUIEditor(QWidget* parent, Qt::WindowFlags flags)
	: CD3DWidget(parent, flags)
{
	m_textureMng = null;
	m_render2D = null;
	m_wndMng = null;
	m_movingSelection = false;
	m_resizeDir = RESIZE_NONE;
	m_backgroundColor = D3DCOLOR_ARGB(255, 50, 50, 50);

	m_editMenu = new QMenu(this);
	m_editMenu->addAction(((CMainFrame*)parent)->GetDeleteCtrlAction());
	m_editMenu->addAction(((CMainFrame*)parent)->GetSetOnTopAction());
}

CGUIEditor::~CGUIEditor()
{
	Delete(m_editMenu);
	Destroy();
}

bool CGUIEditor::InitDeviceObjects()
{
	m_textureMng = new CTextureMng(m_device);
	m_render2D = new CRender2D(m_device);
	m_wndMng = new CWndManager(m_render2D);
	m_wndMng->Create(-1, QRect(0, 0, width(), height()), WBS_MANAGER);

	CWndControl::s_titleFont = new CFont(m_device);
	CWndControl::s_textFont = new CFont(m_device);
	if (!CWndControl::s_titleFont->Create("Verdana", 9, CFont::Normal, D3DCOLOR_ARGB(255, 255, 255, 255), 2,
#if __VER >= 19
		D3DCOLOR_ARGB(255, 0, 0, 0)
#else
		D3DCOLOR_ARGB(255, 217, 91, 51)
#endif
		)
		|| !CWndControl::s_textFont->Create("Verdana", 9, CFont::Normal, D3DCOLOR_ARGB(255, 255, 255, 255)))
		return false;
	CWndControl::s_defaultBackground = m_textureMng->GetGUITexture("WindField.bmp");

	return true;
}

void CGUIEditor::DeleteDeviceObjects()
{
	Delete(CWndControl::s_textFont);
	Delete(CWndControl::s_titleFont);
	Delete(m_wndMng);
	Delete(m_render2D);
	Delete(m_textureMng);
}

bool CGUIEditor::RestoreDeviceObjects()
{
	return CFont::RestoreStaticDeviceObjects(m_device);
}

void CGUIEditor::InvalidateDeviceObjects()
{
	CFont::InvalidateStaticDeviceObjects();
}

bool CGUIEditor::Render()
{
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_backgroundColor, 1.0f, 0L);

	if (FAILED(m_device->BeginScene()))
		return false;

	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHAREF, 0xb0);
	m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

	m_wndMng->PaintRoot();

	m_device->EndScene();
	return true;
}

void CGUIEditor::mouseMoveEvent(QMouseEvent * event)
{
	const QPoint mousePos = QPoint(event->x(), event->y());

	m_wndMng->MouseMove(mousePos);

	if (CWndControl::s_selection.GetSize() == 1)
	{
		CWndControl* ctrl = CWndControl::s_selection[0];
		QPoint pos = mousePos - m_movingOff - ctrl->GetParent()->GetClientRect(true).topLeft();

		if (m_resizeDir != RESIZE_NONE)
		{
			if (CWndWindow::s_cling)
			{
				pos /= CWndWindow::s_gridSize;
				pos *= CWndWindow::s_gridSize;
			}

			QRect rect = m_originalRect;

			if (m_resizeDir & RESIZE_LEFT)
			{
				rect.setLeft(pos.x());
				if (rect.left() > rect.right() - 1)
					rect.setLeft(rect.right() - 1);
			}
			if (m_resizeDir & RESIZE_TOP)
			{
				rect.setTop(pos.y());
				if (rect.top() > rect.bottom() - 1)
					rect.setTop(rect.bottom() - 1);
			}
			if (m_resizeDir & RESIZE_RIGHT)
			{
				rect.setRight(pos.x() + rect.width());
				if (rect.right() < rect.left() + 1)
					rect.setRight(rect.left() + 1);
			}
			if (m_resizeDir & RESIZE_BOTTOM)
			{
				rect.setBottom(pos.y() + rect.height());
				if (rect.bottom() < rect.top() + 1)
					rect.setBottom(rect.top() + 1);
			}

			if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
			{
				const QSize oldSize = rect.size();
				const QSize newSize((rect.width() / 16) * 16, (rect.height() / 16) * 16);
				rect.setSize(newSize);
				if (m_resizeDir & RESIZE_TOP)
					rect.setTop(rect.top() + (oldSize.height() % 16));
				if (m_resizeDir & RESIZE_LEFT)
					rect.setLeft(rect.left() + (oldSize.width() % 16));
				rect.setSize(newSize);
			}

			ctrl->SetRect(rect, true);
			((CMainFrame*)parent())->SetControlRect(rect);
		}
		else if (m_movingSelection)
		{
			if (CWndWindow::s_cling)
			{
				const QSize size = ctrl->GetSize();
				const QRect parentRect = ctrl->GetParent()->GetLayoutRect();
				pos /= CWndWindow::s_gridSize;
				pos *= CWndWindow::s_gridSize;
				if (pos.x() < parentRect.left() + 10)
					pos.setX(parentRect.left());
				else if (pos.x() + size.width() > parentRect.right() - 10)
					pos.setX(parentRect.right() - size.width());
				if (pos.y() < parentRect.top() + 10)
					pos.setY(parentRect.top());
				else if (pos.y() + size.height() > parentRect.bottom() - 10)
					pos.setY(parentRect.bottom() - size.height());
			}
			ctrl->Move(pos);
			((CMainFrame*)parent())->SetControlPos(pos);
		}
	}

	RenderEnvironment();
}

void CGUIEditor::mousePressEvent(QMouseEvent * event)
{
	const QPoint mousePos = QPoint(event->x(), event->y());
	m_wndMng->MouseButtonDown(event->button(), mousePos);

	m_resizeDir = RESIZE_NONE;

	if (event->button() == Qt::LeftButton && CWndControl::s_selection.GetSize() == 1)
	{
		CWndControl* ctrl = CWndControl::s_selection[0];
		QRect windowRect(ctrl->GetScreenPos(), ctrl->GetSize());
		QRect selectRect = windowRect;
		selectRect.adjust(-4, -4, 4, 4);
		if (selectRect.contains(mousePos) && !windowRect.contains(mousePos))
		{
			if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
				m_wndMng->SetControlOnTop(ctrl);

			QRect temp = selectRect;
			temp.setWidth(4);
			if (temp.contains(mousePos))
				m_resizeDir |= RESIZE_LEFT;
			temp = selectRect;
			temp.setHeight(4);
			if (temp.contains(mousePos))
				m_resizeDir |= RESIZE_TOP;
			temp = selectRect;
			temp.setLeft(temp.right() - 4);
			if (temp.contains(mousePos))
				m_resizeDir |= RESIZE_RIGHT;
			temp = selectRect;
			temp.setTop(temp.bottom() - 4);
			if (temp.contains(mousePos))
				m_resizeDir |= RESIZE_BOTTOM;
			m_movingOff = mousePos - ctrl->GetScreenPos();
			m_originalRect = ctrl->GetWindowRect(true);
			RenderEnvironment();
			return;
		}
	}

	if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
	{
		const int createCtrl = ((CMainFrame*)parent())->GetCreateCtrlType();
		if (event->button() == Qt::LeftButton && createCtrl != CREATECTRL_NONE)
		{
			CWndWindow* window = (CWndWindow*)m_wndMng->GetWindow(mousePos);
			if (window)
			{
				CWndControl* newCtrl = window->CreateControl(createCtrl, mousePos - window->GetClientRect(true).topLeft());
				CWndControl::s_selection.RemoveAll();
				if (newCtrl)
					CWndControl::s_selection.Append(newCtrl);
				((CMainFrame*)parent())->SetEditControl();
			}
		}
		else
		{
			CWndControl* oldEdit = null;
			if (CWndControl::s_selection.GetSize() >= 1)
				oldEdit = CWndControl::s_selection[0];

			const bool ctrlKeyPressed = QApplication::queryKeyboardModifiers() & Qt::ControlModifier;

			if (!ctrlKeyPressed)
				CWndControl::s_selection.RemoveAll();

			CWndControl* ctrl = m_wndMng->GetEditableControl(mousePos);
			if (ctrl)
			{
				if (ctrlKeyPressed)
				{
					for (int i = 0; i < CWndControl::s_selection.GetSize(); i++)
					{
						if (CWndControl::s_selection[i]->GetParent() != ctrl->GetParent())
						{
							CWndControl::s_selection.RemoveAll();
							break;
						}
					}

				}

				if (CWndControl::s_selection.Find(ctrl) == -1)
					CWndControl::s_selection.Append(ctrl);
			}

			if (CWndControl::s_selection.GetSize() >= 1)
			{
				if (CWndControl::s_selection[0] != oldEdit)
					((CMainFrame*)parent())->SetEditControl();
			}
			else if (oldEdit != null)
				((CMainFrame*)parent())->SetEditControl();

			if (CWndControl::s_selection.GetSize() > 0)
			{
				if (event->button() == Qt::RightButton)
				{
					m_editMenu->move(mapToGlobal(mousePos));
					m_editMenu->show();
				}
				else if (event->button() == Qt::LeftButton)
				{
					CWndControl* ctrl = CWndControl::s_selection[0];
					if (ctrl->GetType() == WTYPE_GUI_EDITOR_WND)
					{
						if (ctrl->HasFlag(WBS_CAPTION))
						{
							QRect rect = ctrl->GetWindowRect(true);
#if __VER >= 19
							rect.setTop(rect.top() + 26);
#else
							rect.setTop(rect.top() + 18);
#endif
							if (!rect.contains(mousePos))
							{
								if (!ctrl->GetControl(WTBID_CLOSE)->IsPush())
									m_movingSelection = true;
							}
						}
						else
						{
							if (!ctrl->GetControl(WTBID_CLOSE)->IsPush())
								m_movingSelection = true;
						}
					}
					else
						m_movingSelection = true;

					if (m_movingSelection)
						m_movingOff = mousePos - ctrl->GetScreenPos();
				}
			}
		}
	}

	RenderEnvironment();
}

void CGUIEditor::mouseReleaseEvent(QMouseEvent * event)
{
	m_wndMng->MouseButtonUp(event->button(), QPoint(event->x(), event->y()));

	if (event->button() == Qt::LeftButton)
	{
		m_movingSelection = false;
		m_resizeDir = RESIZE_NONE;
	}

	const int wndCount = m_wndMng->GetControlCount();
	RenderEnvironment();
	if (m_wndMng->GetControlCount() != wndCount)
		((CMainFrame*)parent())->SetEditControl();
}

void CGUIEditor::resizeEvent(QResizeEvent* event)
{
	m_render2D->SetOrigin(QPoint(0, 0));
	m_render2D->SetViewport(QRect(0, 0, width(), height()));
	m_wndMng->SetRect(QRect(0, 0, width(), height()), true);
	CD3DWidget::resizeEvent(event);
}