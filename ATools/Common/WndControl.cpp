///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "WndControl.h"
#include "WndManager.h"
#include "Render2D.h"
#include "TextureMng.h"
#include "Font.h"

CFont* CWndControl::s_titleFont = null;
CTexture* CWndControl::s_defaultBackground = null;
CFont* CWndControl::s_textFont = null;
CWndControl* CWndControl::s_focus = null;
CPtrArray<CWndControl> CWndControl::s_selection;

CWndControl::CWndControl(CRender2D* render2D)
	: m_render2D(render2D),
	m_flags(0),
	m_ID(0),
	m_windowRect(0, 0, 0, 0),
	m_clientRect(0, 0, 0, 0),
	m_parent(null),
	m_tiles(false),
	m_texture(null),
	m_textureTiles(null),
	m_hasTexture(false),
	m_color(0xffffffff),
	m_enabled(false),
	m_push(false),
	m_mouseHover(false)
{
}

CWndControl::~CWndControl()
{
	DeleteArray(m_textureTiles);
	for (int i = 0; i < m_controls.GetSize(); i++)
	{
		if (m_controls[i] == s_focus)
			s_focus = null;

		const int select = s_selection.Find(m_controls[i]);
		if (select != -1)
			s_selection.RemoveAt(select);

		Delete(m_controls[i]);
	}

	if (s_focus == this)
		s_focus = null;
	const int select = s_selection.Find(this);
	if (select != -1)
		s_selection.RemoveAt(select);
}

void CWndControl::Create(int ID, const QRect& rect, int flags, CWndControl* parent)
{
	m_ID = ID;
	m_flags |= flags;
	m_parent = parent;
	m_enabled = true;
	m_mustFocus = this;
	SetRect(rect, false);
	if (parent)
		parent->AddControl(this);
	InitialUpdate();
	Resize(m_clientRect.size());
}

void CWndControl::SetRect(const QRect& rect, bool sizeEvent)
{
	const QRect oldRect = m_clientRect;
	m_windowRect = rect;
	m_clientRect = rect;
	if (!IsRoot() && !HasFlag(WBS_NOFRAME))
	{
		if (HasFlag(WBS_CAPTION))
			m_clientRect.setTop(m_clientRect.top() + 18);
		m_clientRect.adjust(4, 4, -8, -10);
	}
	if (sizeEvent && (oldRect.size() != m_clientRect.size()))
		Resize(m_clientRect.size());
}

void CWndControl::Paint(const QRect& windowRect, const QRect& parentRect)
{
	if (!HasFlag(WBS_NODRAWFRAME))
		PaintFrame();

	const QRect viewport = m_render2D->Viewport();
	const QRect clientRect(GetClientRect(false).topLeft() + windowRect.topLeft(), m_clientRect.size());
	QRect viewRect, childViewRect;

	viewRect = clientRect;
	if (viewRect.left() < viewport.left()) viewRect.setLeft(viewport.left());
	if (viewRect.top() < viewport.top()) viewRect.setTop(viewport.top());
	if (viewRect.right() > viewport.right()) viewRect.setRight(viewport.right());
	if (viewRect.bottom() > viewport.bottom()) viewRect.setBottom(viewport.bottom());

	if (!HasFlag(WBS_CHILD))
	{
		m_render2D->SetViewport(viewRect);
		m_render2D->SetOrigin(clientRect.topLeft());
		EraseBackground();
	}

	if (m_controls.GetSize() > 0)
	{
		QRect rect;
		CWndControl* child;
		for (int i = 0; i < m_controls.GetSize(); i++)
		{
			child = m_controls[i];
			if (child->HasFlag(WBS_CHILD))
			{
				rect = child->GetWindowRect(true);
				if (child->HasFlag(WBS_DOCKING))
				{
					rect = QRect(windowRect.topLeft() + rect.topLeft(), rect.size());
					if (viewport.intersects(rect))
					{
						childViewRect = rect;
						if (childViewRect.left() < viewport.left()) childViewRect.setLeft(viewport.left());
						if (childViewRect.top() < viewport.top()) childViewRect.setTop(viewport.top());
						if (childViewRect.right() > viewport.right()) childViewRect.setRight(viewport.right());
						if (childViewRect.bottom() > viewport.bottom()) childViewRect.setBottom(viewport.bottom());
						m_render2D->SetOrigin(rect.topLeft());
						m_render2D->SetViewport(childViewRect);
						child->Paint(rect, viewport);
					}
				}
				else
				{
					rect = QRect(clientRect.topLeft() + rect.topLeft(), rect.size());
					if (viewport.intersects(rect))
					{
						childViewRect = rect;
						if (childViewRect.left() < viewport.left()) childViewRect.setLeft(viewport.left());
						if (childViewRect.top() < viewport.top()) childViewRect.setTop(viewport.top());
						if (childViewRect.right() > viewport.right()) childViewRect.setRight(viewport.right());
						if (childViewRect.bottom() > viewport.bottom()) childViewRect.setBottom(viewport.bottom());
						m_render2D->SetOrigin(rect.topLeft());
						m_render2D->SetViewport(childViewRect);
						child->Paint(rect, viewport);
					}
				}
			}
		}
	}

	m_render2D->SetOrigin(clientRect.topLeft());
	m_render2D->SetViewport(viewRect);
	Draw();

	if (s_selection.GetSize() > 0 && m_controls.GetSize() > 0)
	{
		QRect rect;
		CWndControl* child;
		for (int i = 0; i < m_controls.GetSize(); i++)
		{
			child = m_controls[i];
			if (child->HasFlag(WBS_CHILD))
			{
				rect = child->GetWindowRect(true);
				if (child->HasFlag(WBS_DOCKING))
				{
					rect = QRect(windowRect.topLeft() + rect.topLeft(), rect.size());
					child->RenderSelection(rect, viewport);
				}
				else
				{
					rect = QRect(clientRect.topLeft() + rect.topLeft(), rect.size());
					child->RenderSelection(rect, viewport);
				}
			}
		}
	}
}

void CWndControl::RenderSelection(const QRect& windowRect, const QRect& parentRect)
{
	const int select = s_selection.Find(this);
	if (select != -1)
	{
		QRect selectRect(windowRect.topLeft() - QPoint(4, 4), windowRect.size() + QSize(8, 8));
		m_render2D->SetOrigin(selectRect.topLeft());
		if (selectRect.left() < parentRect.left()) selectRect.setLeft(parentRect.left());
		if (selectRect.top() < parentRect.top()) selectRect.setTop(parentRect.top());
		if (selectRect.right() > parentRect.right()) selectRect.setRight(parentRect.right());
		if (selectRect.bottom() > parentRect.bottom()) selectRect.setBottom(parentRect.bottom());
		m_render2D->SetViewport(selectRect);
		const DWORD color1 = D3DCOLOR_ARGB(200, 128, 128, 255);
		DWORD color2;
		if (select == 0)
			color2 = D3DCOLOR_ARGB(200, 255, 128, 128);
		else
			color2 = D3DCOLOR_ARGB(200, 106, 187, 217);
		const QSize size = GetWindowRect(true).size();
		m_render2D->RenderFillRect(QRect(0, 0, 5, 5), color1);
		m_render2D->RenderFillRect(QRect(size.width() + 4, 0, 5, 5), color1);
		m_render2D->RenderFillRect(QRect(size.width() + 4, size.height() + 4, 5, 5), color1);
		m_render2D->RenderFillRect(QRect(0, size.height() + 4, 5, 5), color1);
		m_render2D->RenderFillRect(QRect(4, 0, size.width() + 1, 5), color2);
		m_render2D->RenderFillRect(QRect(0, 4, 5, size.height() + 1), color2);
		m_render2D->RenderFillRect(QRect(size.width() + 4, 4, 5, size.height() + 1), color2);
		m_render2D->RenderFillRect(QRect(4, size.height() + 4, size.width() + 1, 5), color2);
	}
}

void CWndControl::EraseBackground()
{
	if (!m_hasTexture)
		m_render2D->RenderTexture(QRect(QPoint(0, 0), m_clientRect.size()), s_defaultBackground);
}

void CWndControl::Draw()
{
}

void CWndControl::PaintFrame()
{
	if (m_hasTexture)
	{
		if (m_tiles && GetTilesCount() > 0)
		{
			if (GetTilesCount() == 12)
				m_render2D->Render12Tiles(GetWindowRect(false), m_textureTiles);
			else if (GetTilesCount() == 9)
				m_render2D->Render9Tiles(GetWindowRect(false), m_textureTiles);
		}
		else
			m_render2D->RenderTexture(QPoint(0, 0), m_texture);

		if (HasFlag(WBS_CAPTION))
#if __VER >= 19
			m_render2D->RenderText(s_titleFont, m_text, QPoint(m_windowRect.width() / 2 - s_titleFont->GetSize(m_text).width() / 2, 8), D3DCOLOR_XRGB(246, 204, 77));
#else
			m_render2D->RenderText(s_titleFont, m_text, QPoint(10, 4), m_color);
#endif
	}
	else
	{
		const QRect winRect = GetWindowRect(false);
		QRect rect = winRect;
		rect.setBottom(19);
		m_render2D->RenderTexture(rect, s_defaultBackground);
		rect = winRect;
		rect.setRight(rect.right() + 1);
		m_render2D->RenderRoundRect(rect, D3DCOLOR_ARGB(255, 0, 0, 0));
		rect.adjust(1, 1, -1, -1);
		m_render2D->RenderRoundRect(rect, D3DCOLOR_ARGB(255, 160, 160, 160));
		rect.adjust(1, 1, -1, -1);
		m_render2D->RenderRoundRect(rect, 0xffffffff);
		rect.adjust(1, 1, -1, -1);
		m_render2D->RenderRoundRect(rect, D3DCOLOR_ARGB(255, 160, 160, 160));

		if (HasFlag(WBS_CAPTION))
		{
			rect = winRect;
			rect.setBottom(21);
			rect.adjust(3, 3, -2, 0);
			m_render2D->RenderGradationRect(rect, D3DCOLOR_ARGB(255, 255, 255, 255), D3DCOLOR_ARGB(255, 150, 150, 150), D3DCOLOR_ARGB(255, 230, 230, 230));
			m_render2D->RenderText(s_textFont, m_text, QPoint(17, 7), m_color);
		}
	}
	}

void CWndControl::MouseMove(const QPoint& pos)
{
	if (m_controls.GetSize() > 0)
	{
		CWndControl* ctrl = null;
		QPoint pt;
		for (int i = m_controls.GetSize() - 1; i >= 0; i--)
		{
			ctrl = m_controls[i];
			if (ctrl->HasFlag(WBS_DOCKING))
				pt = pos;
			else
				pt = pos - (m_clientRect.topLeft() - m_windowRect.topLeft());
			if (ctrl->GetWindowRect(true).contains(pt))
			{
				if (!ctrl->IsMouseHover())
					ctrl->MouseEnter();
				ctrl->MouseMove(pt - ctrl->GetWindowRect(true).topLeft());
			}
			else if (ctrl->IsMouseHover())
				ctrl->MouseLeave();
		}
	}
}

void CWndControl::MouseButtonDown(Qt::MouseButton button, const QPoint& pos)
{
	m_mouseButtonsDown |= button;

	if (m_controls.GetSize() > 0)
	{
		CWndControl* ctrl = null, *setOnTop = null;
		QPoint pt;
		for (int i = m_controls.GetSize() - 1; i >= 0; i--)
		{
			ctrl = m_controls[i];
			if (ctrl->HasFlag(WBS_DOCKING))
				pt = pos;
			else
				pt = pos - (m_clientRect.topLeft() - m_windowRect.topLeft());
			if (ctrl->GetWindowRect(true).contains(pt) && ctrl->IsEnabled())
			{
				ctrl->MouseButtonDown(button, pt - ctrl->GetWindowRect(true).topLeft());
				if (IsRoot() && m_controls.GetSize() >= 2 && !setOnTop)
					setOnTop = ctrl;
			}
		}

		if (setOnTop && m_controls[m_controls.GetSize() - 1] != setOnTop)
			SetControlOnTop(setOnTop);
	}

	if (GetType() != WTYPE_STATIC || !HasFlag(WSS_GROUPBOX))
	{
		if ((button == Qt::LeftButton || button == Qt::RightButton) && !s_focus)
		{
			s_focus = m_mustFocus;
			m_push = true;
		}
	}
}

void CWndControl::MouseButtonUp(Qt::MouseButton button, const QPoint& pos)
{
	m_mouseButtonsDown &= ~button;

	if (!IsMouseButtonDown(Qt::LeftButton) && !IsMouseButtonDown(Qt::RightButton))
		m_push = false;

	if (m_controls.GetSize() > 0)
	{
		CWndControl* ctrl = null;
		QPoint pt;
		for (int i = m_controls.GetSize() - 1; i >= 0; i--)
		{
			ctrl = m_controls[i];
			if (ctrl->IsMouseButtonDown(button))
			{
				if (ctrl->HasFlag(WBS_DOCKING))
					pt = pos;
				else
					pt = pos - (m_clientRect.topLeft() - m_windowRect.topLeft());
				ctrl->MouseButtonUp(button, pt - ctrl->GetWindowRect(true).topLeft());
			}
		}
	}
}

void CWndControl::MouseEnter()
{
	m_mouseHover = true;

	if (s_focus == this && (IsMouseButtonDown(Qt::LeftButton) || IsMouseButtonDown(Qt::RightButton)))
		m_push = true;
}

void CWndControl::MouseLeave()
{
	m_mouseHover = false;
	m_push = false;

	if (m_controls.GetSize() > 0)
	{
		CWndControl* ctrl = null;
		for (int i = m_controls.GetSize() - 1; i >= 0; i--)
		{
			ctrl = m_controls[i];
			if (ctrl->IsMouseHover())
				ctrl->MouseLeave();
		}
	}
}

void CWndControl::Resize(const QSize& size)
{
}

void CWndControl::InitialUpdate()
{
}

bool CWndControl::IsRoot() const
{
	return this == (CWndControl*)WndMng;
}

QRect CWndControl::GetWindowRect(bool parent) const
{
	QRect rect = m_windowRect;
	if (!parent)
	{
		const QPoint topLeft = rect.topLeft();
		rect.setTopLeft(QPoint(0, 0));
		rect.setBottomRight(rect.bottomRight() - topLeft);
	}
	return rect;
}

QRect CWndControl::GetClientRect(bool parent) const
{
	QRect rect = m_clientRect;
	if (!parent)
	{
		rect.setTopLeft(rect.topLeft() - m_windowRect.topLeft());
		rect.setBottomRight(rect.bottomRight() - m_windowRect.topLeft());
	}
	return rect;
}

QRect CWndControl::GetLayoutRect() const
{
	QRect rect(QPoint(0, 0), m_clientRect.size());
	if (!IsRoot() && !HasFlag(WBS_NOFRAME))
		rect = QRect(QPoint(8, 8), m_clientRect.size() - QSize(16, 16));
	return rect;
}

QPoint CWndControl::GetScreenPos() const
{
	QPoint pos = m_windowRect.topLeft();
	if (m_parent)
	{
		pos += m_parent->GetScreenPos();
		if (!HasFlag(WBS_DOCKING))
			pos += m_parent->GetClientRect(false).topLeft();
	}
	return pos;
}

void CWndControl::AddControl(CWndControl* control)
{
	if (control == this || !control)
		return;

	m_controls.Append(control);
}

void CWndControl::RemoveControl(int ID, bool deleteControl)
{
	for (int i = 0; i < m_controls.GetSize(); i++)
	{
		if (m_controls[i]->GetID() == ID)
		{
			if (m_controls[i] == s_focus)
				s_focus = null;

			if (deleteControl)
				Delete(m_controls[i]);

			m_controls.RemoveAt(i);
			i--;
		}
	}
}

CWndControl* CWndControl::GetControl(int ID) const
{
	for (int i = 0; i < m_controls.GetSize(); i++)
		if (m_controls[i]->GetID() == ID)
			return m_controls[i];
	return null;
}

void CWndControl::SetControlOnTop(CWndControl* control)
{
	for (int i = 0; i < m_controls.GetSize(); i++)
	{
		if (m_controls[i] == control)
		{
			m_controls.RemoveAt(i);
			m_controls.Append(control);
			break;
		}
	}
}

void CWndControl::SetTexture(const string& filename, bool tiles)
{
	m_textureName = filename;
	m_tiles = tiles;
	m_hasTexture = false;
	m_texture = null;
	DeleteArray(m_textureTiles);

	if (m_textureName.isEmpty())
		return;

	if (m_tiles && GetTilesCount() > 0)
	{
		m_textureTiles = new CTexture*[GetTilesCount()];
		memset(m_textureTiles, 0, sizeof(CTexture*) * GetTilesCount());

		string temp;
		const string name = m_textureName.left(m_textureName.size() - 6);
		const string ext = m_textureName.right(4);
		for (int i = 0; i < GetTilesCount(); i++)
		{
			temp = name + string().sprintf("%02d", i) + ext;
			m_textureTiles[i] = TextureMng->GetGUITexture(temp);

			if (m_textureTiles[i])
				m_hasTexture = true;
		}
	}
	else
	{
		m_texture = TextureMng->GetGUITexture(m_textureName);
		if (m_texture)
			m_hasTexture = true;
	}
}

void CWndControl::Move(const QPoint& point)
{
	SetRect(QRect(point, m_windowRect.size()), true);
}

void CWndControl::ChildNotify(int childID, EWndMsg msg)
{
}

void CWndControl::LostFocus()
{
}

CWndControl* CWndControl::GetEditableControl(const QPoint& mousePos)
{
	return null;
}

void CWndControl::SetColor(const QColor& color)
{
	m_color = D3DCOLOR_ARGB(color.alpha(), color.red(), color.green(), color.blue());
}