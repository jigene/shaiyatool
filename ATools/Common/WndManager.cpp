///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "WndManager.h"
#include "Render2D.h"

CWndManager* WndMng = null;

CWndManager::CWndManager(CRender2D* render2D)
	: CWndControl(render2D)
{
	WndMng = this;
}

CWndManager::~CWndManager()
{
	WndMng = null;
}

void CWndManager::PaintRoot()
{
	const QPoint oldOrigin = m_render2D->Origin();
	const QRect oldViewport = m_render2D->Viewport();
	CWndControl* ctrl = null;
	QRect rect, viewport;
	int remove;

	for (int i = 0; i < m_controls.GetSize(); i++)
	{
		ctrl = m_controls[i];

		remove = m_removeControls.Find(ctrl);
		if (remove != -1)
		{
			Delete(ctrl);
			m_removeControls.RemoveAt(remove);
			m_controls.RemoveAt(i);
			i--;
			continue;
		}

		rect = ctrl->GetWindowRect(true);
		if (oldViewport.intersects(rect))
		{
			viewport = rect;
			if (viewport.left() < oldViewport.left()) viewport.setLeft(oldViewport.left());
			if (viewport.top() < oldViewport.top()) viewport.setTop(oldViewport.top());
			if (viewport.right() > oldViewport.right()) viewport.setRight(oldViewport.right());
			if (viewport.bottom() > oldViewport.bottom()) viewport.setBottom(oldViewport.bottom());
			m_render2D->SetOrigin(rect.topLeft());
			m_render2D->SetViewport(viewport);
			ctrl->Paint(rect, oldViewport);
		}
	}

	if (s_selection.GetSize() > 0)
	{
		for (int i = 0; i < m_controls.GetSize(); i++)
		{
			ctrl = m_controls[i];
			rect = ctrl->GetWindowRect(true);
			ctrl->RenderSelection(rect, oldViewport);
		}
	}

	m_render2D->SetOrigin(oldOrigin);
	m_render2D->SetViewport(oldViewport);
}

void CWndManager::RemoveWindow(int ID, bool deleteControl)
{
	CWndControl* ctrl = null;
	for (int i = 0; i < m_controls.GetSize(); i++)
	{
		ctrl = m_controls[i];
		if (ctrl->GetID() == ID)
			m_removeControls.Append(ctrl);
	}
}

void CWndManager::MouseButtonDown(Qt::MouseButton button, const QPoint& pos)
{
	CWndControl* oldFocus = s_focus;
	s_focus = null;

	CWndControl::MouseButtonDown(button, pos);

	if (s_focus != oldFocus && oldFocus)
		oldFocus->LostFocus();
}

CWndControl* CWndManager::GetEditableControl(const QPoint& mousePos)
{
	CWndControl* ctrl = null;
	for (int i = m_controls.GetSize() - 1; i >= 0; i--)
	{
		ctrl = m_controls[i];
		if (ctrl->GetWindowRect(true).contains(mousePos))
			return ctrl->GetEditableControl(mousePos - ctrl->GetWindowRect(true).topLeft());
	}
	return null;
}

CWndControl* CWndManager::GetWindow(const QPoint& mousePos)
{
	CWndControl* ctrl = null;
	for (int i = m_controls.GetSize() - 1; i >= 0; i--)
	{
		ctrl = m_controls[i];
		if (ctrl->GetWindowRect(true).contains(mousePos))
			return ctrl;
	}
	return null;
}