///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "WndControls.h"
#include "Render2D.h"
#include "TextureMng.h"
#include "Font.h"

CWndButton::CWndButton(CRender2D* render2D)
	: CWndControl(render2D),
	m_checked(false)
{
	m_flags = WBS_HIGHLIGHT | WBS_CHILD | WBS_NOFRAME;
}

void CWndButton::InitialUpdate()
{
#if __VER >= 19
	if (HasFlag(WBS_RADIO))
		m_textureName = "ButtRadio.tga";
	if (HasFlag(WBS_CHECK))
		m_textureName = "ButtCheck.tga";
#else
	if (HasFlag(WBS_RADIO))
		m_textureName = "ButtRadio.bmp";
	if (HasFlag(WBS_CHECK))
		m_textureName = "ButtCheck.bmp";
#endif
}

void CWndButton::FitTextureSize()
{
	if (m_hasTexture)
	{
		QRect rect = GetWindowRect(true);
		if (HasFlag(WBS_RADIO) | HasFlag(WBS_CHECK))
			rect.setSize(QSize(m_texture->GetWidth() / 6, m_texture->GetHeight()));
		else
			rect.setSize(QSize(m_texture->GetWidth() / 4, m_texture->GetHeight()));
		SetRect(rect, true);
	}
}

void CWndButton::PaintFrame()
{
	if (HasFlag(WBS_RADIO))
	{
		if (m_hasTexture)
		{
			_render6Texture();
			m_render2D->RenderText(s_textFont, m_text, QPoint(m_texture->GetWidth() / 6 + 2, m_render2D->Viewport().height() / 2 - s_textFont->GetMaxHeight() / 2), _getTextColor());
		}
		else
		{
			const DWORD color = _getTextColor();
			QRect rect(0, 0, 10, 10);
			m_render2D->RenderRoundRect(rect, color);
			if (m_checked)
			{
				rect.adjust(2, 2, -2, -2);
				m_render2D->RenderFillRect(rect, color);
			}
			m_render2D->RenderText(s_textFont, m_text, QPoint(s_textFont->GetMaxHeight() + 5, 0), color);
		}
	}
	else if (HasFlag(WBS_CHECK))
	{
		if (m_hasTexture)
		{
			_render6Texture();
			m_render2D->RenderText(s_textFont, m_text, QPoint(m_texture->GetWidth() / 6 + 2, m_render2D->Viewport().height() / 2 - s_textFont->GetMaxHeight() / 2), _getTextColor());
		}
		else
		{
			const DWORD color = _getTextColor();
			m_render2D->RenderRect(QRect(0, 0, 10, 10), color);
			if (m_checked)
			{
				m_render2D->RenderLine(QPoint(2, 2), QPoint(5, 8), color);
				m_render2D->RenderLine(QPoint(2, 2), QPoint(6, 8), color);
				m_render2D->RenderLine(QPoint(5, 8), QPoint(8, 3), color);
			}
			m_render2D->RenderText(s_textFont, m_text, QPoint(s_textFont->GetMaxHeight() + 5, 0), color);
		}
	}
	else if (HasFlag(WBS_TEXT))
	{
		m_render2D->RenderText(s_textFont, m_text, _getTextPos(), _getTextColor());
	}
	else
	{
		if (m_hasTexture)
		{
			QRectF textureRect(0.0f, 0.0f, 1.0f / 4.0f, 1.0f);
			if (!m_enabled)
				textureRect.setLeft(1.0f / 4.0f * 3.0f);
			else if (!m_push)
			{
				if (m_mouseHover)
					textureRect.setLeft(0.0f);
				else
					textureRect.setLeft(1.0f / 4.0f);
			}
			else
				textureRect.setLeft(1.0f / 2.0f);

			textureRect.setWidth(1.0f / 4.0f);
			m_render2D->RenderTexture(QPoint(0, 0), m_texture, textureRect);

#if __VER >= 19
			const QSize size = s_textFont->GetSize(m_text);
			QPoint textPos = QPoint(m_windowRect.width() / 2 - size.width() / 2, m_windowRect.height() / 2 - size.height() / 2);
			if (m_push)
				textPos.ry()++;
			m_render2D->RenderText(s_textFont, m_text, textPos, _getTextColor());
#endif
		}
		else
		{
			m_render2D->RenderText(s_textFont, m_text, _getTextPos(), _getTextColor(), 0xff000000);

			const DWORD color1 = (m_push ? D3DCOLOR_ARGB(255, 100, 255, 255) : D3DCOLOR_ARGB(250, 255, 255, 255));
			const DWORD color2 = (m_push ? D3DCOLOR_ARGB(50, 0, 0, 0) : D3DCOLOR_ARGB(50, 0, 0, 0));
			const DWORD color3 = (m_push ? D3DCOLOR_ARGB(200, 0, 150, 150) : D3DCOLOR_ARGB(200, 150, 150, 150));

			QRect rect = GetClientRect(false);
			rect.adjust(2, 2, -2, -2);
			m_render2D->RenderGradationRect(rect, color1, color2, color3);
			m_render2D->RenderLine(rect.topLeft(), rect.topRight(), color3);
			rect.adjust(-1, -1, 1, 1);
			m_render2D->RenderRoundRect(rect, D3DCOLOR_ARGB(155, 200, 200, 200));
			rect.adjust(-1, -1, 1, 1);
			m_render2D->RenderRoundRect(rect, D3DCOLOR_ARGB(155, 50, 50, 50));
		}
	}
}

QPoint CWndButton::_getTextPos() const
{
	if (!HasFlag(WBS_NOCENTER))
	{
		const QSize size = s_textFont->GetSize(m_text);
		return QPoint(m_windowRect.width() / 2 - size.width() / 2, m_windowRect.height() / 2 - size.height() / 2);
	}
	return QPoint(0, 0);
}

DWORD CWndButton::_getTextColor() const
{
	if (m_enabled)
	{
#if __VER >= 19
		if (m_mouseHover)
			return D3DCOLOR_ARGB(255, 191, 157, 95);
		return m_color;
#else
		if (m_mouseHover)
			return D3DCOLOR_ARGB(255, 64, 64, 255);
		return D3DCOLOR_ARGB(255, 64, 64, 64);
#endif
	}
	return D3DCOLOR_ARGB(255, 140, 140, 140);
}

void CWndButton::_render6Texture() const
{
	QRectF textureRect(0.0f, 0.0f, 1.0f / 4.0f, 1.0f);
	if (!m_enabled)
	{
		if (!m_push && !m_checked)
			textureRect.setLeft(1.0f / 3.0f * 2.0f);
		else
			textureRect.setLeft(1.0f / 6.0f * 5.0f);
	}
	else if (!m_push && !m_checked)
	{
		if (m_mouseHover)
			textureRect.setLeft(0.0f);
		else
			textureRect.setLeft(1.0f / 6.0f);
	}
	else
	{
		if (m_mouseHover)
			textureRect.setLeft(1.0f / 3.0f);
		else
			textureRect.setLeft(1.0f / 2.0f);
	}

	textureRect.setWidth(1.0f / 6.0f);
	m_render2D->RenderTexture(QPoint(0, 0), m_texture, textureRect);
}

void CWndButton::MouseButtonUp(Qt::MouseButton button, const QPoint& pos)
{
	if (m_push)
	{
		if (HasFlag(WBS_CHECK))
			m_checked = !m_checked;

		if (HasFlag(WBS_RADIO))
		{
			if (m_parent)
			{
				CPtrArray<CWndControl>* parentArray = m_parent->GetControls();
				CWndControl* ctrl = null;
				for (int i = 0; i < parentArray->GetSize(); i++)
				{
					ctrl = parentArray->GetAt(i);
					if (ctrl->GetType() == WTYPE_BUTTON && ctrl->HasFlag(WBS_RADIO))
						((CWndButton*)ctrl)->SetChecked(false);
				}
			}
			m_checked = true;
		}

		if (m_parent)
			m_parent->ChildNotify(m_ID, EWndMsg::Clicked);
	}

	CWndControl::MouseButtonUp(button, pos);
}

CWndCustom::CWndCustom(CRender2D* render2D)
	: CWndControl(render2D)
{
	m_flags = WBS_CHILD | WBS_NOFRAME | WBS_NODRAWFRAME;
}

void CWndCustom::Draw()
{
	m_render2D->RenderFillRect(GetWindowRect(false), 0xff808080);
}

CWndStatic::CWndStatic(CRender2D* render2D)
	: CWndControl(render2D)
{
	m_flags = WBS_CHILD | WBS_NOFRAME;
}

void CWndStatic::InitialUpdate()
{
	m_mustFocus = m_parent;

	if (HasFlag(WSS_GROUPBOX))
		m_color = 0xffffffff;
	else
		m_color = 0xff2e70a9;
}

void CWndStatic::PaintFrame()
{
	if (!m_hasTexture && HasFlag(WSS_PICTURE))
		m_render2D->RenderRect(GetWindowRect(false), 0xff000000);
	else if (m_hasTexture)
	{
		if (m_tiles)
			m_render2D->Render9Tiles(GetWindowRect(false), m_textureTiles);
		else
			m_render2D->RenderTexture(QPoint(0, 0), m_texture);
	}
}

void CWndStatic::Draw()
{
	QPoint pt;

	if (HasFlag(WSS_GROUPBOX))
	{
		pt = QPoint(4, 4);
		const int maxHeight = s_textFont->GetMaxHeight();
		if (maxHeight > 14)
			pt.ry() -= maxHeight - 14;
	}
	else
		pt = QPoint(2, 2);

	const QRect rect = GetWindowRect(false);
	const QSize size = s_textFont->GetSize(m_text);

	if (HasFlag(WSS_ALIGNHRIGHT))
	{
		pt.setX(rect.width() - size.width() - 4);
		if (pt.x() < 2)
			pt.setX(2);
	}
	if (HasFlag(WSS_ALIGNHCENTER))
	{
		pt.setX((rect.width() - size.width()) / 2);
		if (pt.x() < 2)
			pt.setX(2);
	}
	if (HasFlag(WSS_ALIGNVBOTTOM))
	{
		pt.setY(rect.height() - size.height());
		if (pt.y() < 2)
			pt.setY(2);
	}
	if (HasFlag(WSS_ALIGNVCENTER))
	{
		pt.setY((rect.height() - size.height() + 4) / 2);
		if (pt.y() < 2)
			pt.setY(2);
	}

	if (!HasFlag(WSS_GROUPBOX) && HasFlag(WSS_MONEY))
	{
		string str;
		for (int i = m_text.size() - 1; i >= 0; --i)
		{
			str.append(m_text[i]);
			if (((m_text.size() - i) % 3) == 0 && i != 0)
				str.append(',');
		}
		m_render2D->RenderText(s_textFont, str, pt, m_color);
		m_render2D->RenderText(s_textFont, str, pt + QPoint(1, 0), m_color);
	}
	else
	{
		m_render2D->RenderText(s_textFont, m_text, pt, m_color);
		m_render2D->RenderText(s_textFont, m_text, pt + QPoint(1, 0), m_color);
	}
}

CWndScrollBar::CWndScrollBar(CRender2D* render2D)
	: CWndControl(render2D),
	m_arrow1(null),
	m_arrow2(null)
{
	m_flags = WBS_CHILD | WBS_DOCKING | WBS_NOFRAME;
}

void CWndScrollBar::PaintFrame()
{
	if (m_hasTexture)
		m_render2D->RenderTexture(QRect(QPoint(0, 16), QSize(m_texture->GetWidth(), GetSize().height() - 19)), m_texture);
}

void CWndScrollBar::InitialUpdate()
{
	m_arrow1 = new CWndButton(m_render2D);
	m_arrow1->Create(1000, QRect(), 0, this);
	m_arrow1->SetTexture("ButtVScrUp.tga", false);
	m_arrow1->FitTextureSize();

	m_arrow2 = new CWndButton(m_render2D);
	m_arrow2->Create(1001, QRect(), 0, this);
	m_arrow2->SetTexture("ButtVScrDown.tga", false);
	m_arrow2->FitTextureSize();

	SetTexture("ButtVScrBar.bmp", false);

	m_mustFocus = m_parent;
	m_arrow1->MustFocus(m_parent);
	m_arrow2->MustFocus(m_parent);
}

void CWndScrollBar::Resize(const QSize& size)
{
	m_arrow1->Move(QPoint(0, 0));
	m_arrow2->Move(QPoint(0, size.height() - m_arrow2->GetSize().height() + 1));
}

CWndText::CWndText(CRender2D* render2D)
	: CWndControl(render2D),
	m_scrollBar(null)
{
	m_flags = WBS_CHILD;
	m_textureName = "WndEditTile00.tga";
}

void CWndText::PaintFrame()
{
	QRect rect = GetWindowRect(false);
	if (HasFlag(WBS_VSCROLL))
		rect.setRight(rect.right() - 17);
	if (m_hasTexture)
	{
		if (m_tiles)
			m_render2D->Render9Tiles(rect, m_textureTiles);
		else
			m_render2D->RenderTexture(rect, m_texture);
	}
}

void CWndText::InitialUpdate()
{
	UpdateScrollBar();
}

void CWndText::Resize(const QSize& size)
{
	if (HasFlag(WBS_VSCROLL) && m_scrollBar)
	{
		const QRect wndRect = GetWindowRect(false);
		m_scrollBar->SetRect(QRect(wndRect.topRight() - QPoint(16, 0), QSize(16, wndRect.height())), true);
	}
}

void CWndText::UpdateScrollBar()
{
	if (HasFlag(WBS_VSCROLL))
	{
		if (!m_scrollBar)
		{
			m_scrollBar = new CWndScrollBar(m_render2D);
			m_scrollBar->Create(1000, QRect(), 0, this);
			const QRect wndRect = GetWindowRect(false);
			m_scrollBar->SetRect(QRect(wndRect.topRight() - QPoint(16, 0), QSize(16, wndRect.height())), true);
		}
	}
	else
	{
		if (m_scrollBar)
		{
			RemoveControl(1000);
			m_scrollBar = null;
		}
	}
}

void CWndText::Draw()
{
#if __VER >= 19
	m_render2D->RenderText(s_textFont, m_text, QPoint(2, 2), m_color);
#else
	m_render2D->RenderText(s_textFont, m_text, QPoint(2, 2), 0xff000000);
#endif
}

CWndEdit::CWndEdit(CRender2D* render2D)
	: CWndText(render2D)
{
}

CWndListBox::CWndListBox(CRender2D* render2D)
	: CWndText(render2D)
{
}

CWndTreeCtrl::CWndTreeCtrl(CRender2D* render2D)
	: CWndText(render2D)
{
}

CWndListCtrl::CWndListCtrl(CRender2D* render2D)
	: CWndText(render2D)
{
}

void CWndListCtrl::PaintFrame()
{
	QRect rect = GetWindowRect(false);

	m_render2D->RenderFillRect(rect, D3DCOLOR_ARGB(255 - 32, 255, 255, 255));
	m_render2D->RenderRoundRect(rect, D3DCOLOR_ARGB(255 - 32, 226, 198, 181));

	m_render2D->RenderGradationRect(QRect(QPoint(0, 0), QSize(rect.width(), 21)),
		D3DCOLOR_ARGB(50, 100, 100, 100),
		D3DCOLOR_ARGB(90, 255, 255, 255),
		D3DCOLOR_ARGB(50, 000, 000, 000), 20);
}

CWndTabCtrl::CWndTabCtrl(CRender2D* render2D)
	: CWndControl(render2D)
{
	m_flags = WBS_CHILD | WBS_NOFRAME;
	memset(m_tabTextures, 0, sizeof(m_tabTextures));
}

void CWndTabCtrl::InitialUpdate()
{
	m_tabTextures[0] = TextureMng->GetGUITexture("WndEditTile00.tga");
	m_tabTextures[1] = TextureMng->GetGUITexture("WndEditTile01.tga");
	m_tabTextures[2] = TextureMng->GetGUITexture("WndEditTile02.tga");
	m_tabTextures[3] = TextureMng->GetGUITexture("WndEditTile03.tga");
	m_tabTextures[4] = TextureMng->GetGUITexture("WndEditTile04.tga");
	m_tabTextures[5] = TextureMng->GetGUITexture("WndEditTile05.tga");
	m_tabTextures[6] = TextureMng->GetGUITexture("WndTabTile00.bmp");
	m_tabTextures[7] = TextureMng->GetGUITexture("WndTabTile01.bmp");
	m_tabTextures[8] = TextureMng->GetGUITexture("WndTabTile04.bmp");
	m_tabTextures[9] = TextureMng->GetGUITexture("WndTabTile11.bmp");
	m_tabTextures[10] = TextureMng->GetGUITexture("WndTabTile15.bmp");
}

void CWndTabCtrl::Draw()
{
	if (!m_tabTextures[0] || !m_tabTextures[6])
		return;

	QRect rect = GetWindowRect(false);
	rect.setRight(rect.right() - 17);

	int editTexWidth = m_tabTextures[0]->GetWidth();
	int editTexHeight = m_tabTextures[0]->GetHeight();
	int texWidth = m_tabTextures[6]->GetWidth();
	int texHeight = m_tabTextures[6]->GetHeight();

	m_render2D->RenderTexture(QPoint(0, 0), m_tabTextures[0]);
	m_render2D->RenderTexture(QRect(QPoint(editTexWidth, 0), QSize(rect.width() - editTexWidth * 2, editTexHeight)), m_tabTextures[1]);
	m_render2D->RenderTexture(QPoint(rect.width() - editTexWidth, 0), m_tabTextures[2]);

	m_render2D->RenderTexture(QRect(QPoint(0, editTexHeight), QSize(editTexWidth, rect.height() - editTexHeight - texHeight)), m_tabTextures[3]);
	m_render2D->RenderTexture(QRect(QPoint(editTexWidth, editTexHeight), QSize(rect.width() - editTexWidth * 2, rect.height() - editTexHeight - texHeight)), m_tabTextures[4]);
	m_render2D->RenderTexture(QRect(QPoint(rect.width() - editTexWidth, editTexHeight), QSize(editTexWidth, rect.height() - editTexHeight - texHeight)), m_tabTextures[5]);

	m_render2D->RenderTexture(QPoint(0, rect.height() - texHeight), m_tabTextures[6]);
	m_render2D->RenderTexture(QRect(QPoint(texWidth, rect.height() - texHeight), QSize(rect.width() / 3 - texWidth, texHeight)), m_tabTextures[7]);

	m_render2D->RenderTexture(QPoint(rect.width() / 3, rect.height() - texHeight), m_tabTextures[8]);
	m_render2D->RenderTexture(QRect(QPoint(rect.width() / 3 + texWidth, rect.height() - texHeight), QSize(rect.width() / 3 - texWidth, texHeight)), m_tabTextures[7]);

	m_render2D->RenderTexture(QRect(QPoint(rect.width() / 3 * 2, rect.height() - texHeight), QSize(rect.width() / 3 - texWidth, texHeight)), m_tabTextures[9]);
	m_render2D->RenderTexture(QPoint(rect.width() - texWidth, rect.height() - texHeight), m_tabTextures[10]);
}

CWndComboBox::CWndComboBox(CRender2D* render2D)
	: CWndControl(render2D),
	m_wndListBox(null),
	m_button(null)
{
	m_flags = WBS_CHILD;
}

void CWndComboBox::InitialUpdate()
{
	m_button = new CWndButton(m_render2D);
	m_button->Create(1000, QRect(), WBS_DOCKING, this);
	m_button->SetTexture("ButtQuickListDn.tga", false);
	m_button->FitTextureSize();
	m_button->MustFocus(this);
}

void CWndComboBox::Resize(const QSize& size)
{
	m_button->Move(QPoint(size.width() - 9, 0));
}

void CWndComboBox::ChildNotify(int childID, EWndMsg msg)
{
	if (childID == 1000 && msg == EWndMsg::Clicked && m_parent)
	{
		if (!m_parent->GetControl(m_ID * 101))
		{
			m_wndListBox = new CWndComboBoxListBox(m_render2D);
			m_wndListBox->Create(m_ID * 101, QRect(GetWindowRect(true).bottomLeft() + QPoint(0, 1), QSize(GetSize().width(), 100)), WBS_POPUP | WBS_VSCROLL, m_parent);
			m_wndListBox->SetTexture(m_textureName, true);
			s_focus = m_wndListBox;
		}
	}
}

CWndComboBoxListBox::CWndComboBoxListBox(CRender2D* render2D)
	: CWndListBox(render2D)
{
}

void CWndComboBoxListBox::LostFocus()
{
	if (m_parent)
		m_parent->RemoveControl(m_ID);
}