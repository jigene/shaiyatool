///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Render2D.h"
#include "TextureMng.h"
#include "Font.h"

struct TextureVertex
{
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 };
	D3DXVECTOR2 p;
	float z = 0.5f;
	float rhw = 1.0f;
	D3DXVECTOR2 t;
};

struct ColorVertex
{
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE };
	D3DXVECTOR2 p;
	float z = 0.5f;
	float rhw = 1.0f;
	DWORD c;
};

CRender2D::CRender2D(LPDIRECT3DDEVICE9 device)
	: m_device(device),
	m_origin(0, 0),
	m_viewport(0, 0, 1, 1)
{
}

CRender2D::~CRender2D()
{

}

void CRender2D::SetViewport(const QRect& rect)
{
	D3DVIEWPORT9 viewport;
	viewport.X = rect.left();
	viewport.Y = rect.top();
	viewport.Width = rect.width();
	viewport.Height = rect.height();
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	m_device->SetViewport(&viewport);
	m_viewport = rect;
}

void CRender2D::RenderText(CFont* font, const string& text, const QPoint& point, DWORD color, DWORD shadowColor)
{
	if (!font)
		return;

	const QSize size = font->GetSize(text);
	const QPoint pt = m_origin + point;
	const QRect rect(pt, size);
	if (m_viewport.intersects(rect))
	{
		if (shadowColor & 0xff000000)
			font->Render(text, pt + QPoint(1, 1), shadowColor);
		font->Render(text, pt, color);
	}
}

void CRender2D::RenderTexture(const QRect& rect, const CTexture* texture, int alpha, bool adaptSize)
{
	if (!texture || !alpha)
		return;

	const float left = (float)(m_origin.x() + rect.left()) - 0.5f;
	const float top = (float)(m_origin.y() + rect.top()) - 0.5f;
	const float right = left + (float)rect.width();
	const float bottom = top + (float)rect.height();

	const float t1 = adaptSize ? (right - left) / (float)texture->GetWidth() : 1.0f;
	const float t2 = adaptSize ? (bottom - top) / (float)texture->GetHeight() : 1.0f;

	TextureVertex vertices[4];
	vertices[0].p = D3DXVECTOR2(left, top);
	vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].p = D3DXVECTOR2(left, bottom);
	vertices[1].t = D3DXVECTOR2(0.0f, t2);
	vertices[2].p = D3DXVECTOR2(right, top);
	vertices[2].t = D3DXVECTOR2(t1, 0.0f);
	vertices[3].p = D3DXVECTOR2(right, bottom);
	vertices[3].t = D3DXVECTOR2(t1, t2);

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetFVF(TextureVertex::FVF);
	m_device->SetTexture(0, *texture);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, 0, 0, 0));
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
}

void CRender2D::RenderTexture(const QPoint& pt, const CTexture* texture, int alpha)
{
	if (!texture || !alpha)
		return;

	const float left = (float)(m_origin.x() + pt.x()) - 0.5f;
	const float top = (float)(m_origin.y() + pt.y()) - 0.5f;
	const float right = left + (float)texture->GetWidth();
	const float bottom = top + (float)texture->GetHeight();

	TextureVertex vertices[4];
	vertices[0].p = D3DXVECTOR2(left, top);
	vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].p = D3DXVECTOR2(left, bottom);
	vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
	vertices[2].p = D3DXVECTOR2(right, top);
	vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
	vertices[3].p = D3DXVECTOR2(right, bottom);
	vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetFVF(TextureVertex::FVF);
	m_device->SetTexture(0, *texture);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, 0, 0, 0));
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
}

void CRender2D::RenderTexture(const QPoint& pt, const CTexture* texture, const QRectF& textureRect, int alpha)
{
	if (!texture || !alpha)
		return;

	const float left = (float)(m_origin.x() + pt.x()) - 0.5f;
	const float top = (float)(m_origin.y() + pt.y()) - 0.5f;
	const float right = left + (float)(int)((float)textureRect.width() * (float)texture->GetWidth() + 0.5f);
	const float bottom = top + (float)(int)((float)textureRect.height() * (float)texture->GetHeight() + 0.5f);

	TextureVertex vertices[4];
	vertices[0].p = D3DXVECTOR2(left, top);
	vertices[0].t = D3DXVECTOR2((float)textureRect.left(), (float)textureRect.top());
	vertices[1].p = D3DXVECTOR2(left, bottom);
	vertices[1].t = D3DXVECTOR2((float)textureRect.left(), (float)textureRect.bottom());
	vertices[2].p = D3DXVECTOR2(right, top);
	vertices[2].t = D3DXVECTOR2((float)textureRect.right(), (float)textureRect.top());
	vertices[3].p = D3DXVECTOR2(right, bottom);
	vertices[3].t = D3DXVECTOR2((float)textureRect.right(), (float)textureRect.bottom());

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetFVF(TextureVertex::FVF);
	m_device->SetTexture(0, *texture);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, 0, 0, 0));
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
}

void CRender2D::Render12Tiles(const QRect& rect, CTexture** tiles, int alpha)
{
	if (!tiles)
		return;

	TextureVertex vertices[4];
	m_device->SetFVF(TextureVertex::FVF);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, 0, 0, 0));

	float left, top, right, bottom;
	float tileWidth = 0.0f, tileHeight = 0.0f;
	for (int i = 0; i < 12; i++)
	{
		if (tiles[i])
		{
			tileWidth = (float)tiles[i]->GetWidth();
			tileHeight = (float)tiles[i]->GetHeight();
			break;
		}
	}

	if (tiles[0])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.top()) - 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[0]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[1])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.top()) - 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, 1.0f);
		m_device->SetTexture(0, *tiles[1]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[2])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.top()) - 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[2]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}

	if (tiles[3])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight - 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[3]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[4])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight - 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, 1.0f);
		m_device->SetTexture(0, *tiles[4]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[5])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight - 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[5]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}

	if (tiles[6])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight * 2.0f - 0.5f;
		right = left + tileWidth;
		bottom = top + (float)rect.height() - tileHeight * 3.0f;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, (top - bottom) / tileHeight);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, (top - bottom) / tileHeight);
		m_device->SetTexture(0, *tiles[6]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[7])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight * 2.0f - 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + (float)rect.height() - tileHeight * 3.0f;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, (top - bottom) / tileHeight);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, (top - bottom) / tileHeight);
		m_device->SetTexture(0, *tiles[7]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[8])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight * 2.0f - 0.5f;
		right = left + tileWidth;
		bottom = top + (float)rect.height() - tileHeight * 3.0f;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, (top - bottom) / tileHeight);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, (top - bottom) / tileHeight);
		m_device->SetTexture(0, *tiles[8]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}

	if (tiles[9])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.bottom()) - tileHeight + 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[9]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[10])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.bottom()) - tileHeight + 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, 1.0f);
		m_device->SetTexture(0, *tiles[10]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[11])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.bottom()) - tileHeight + 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[11]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
}

void CRender2D::Render9Tiles(const QRect& rect, CTexture** tiles, int alpha)
{
	if (!tiles)
		return;

	TextureVertex vertices[4];
	m_device->SetFVF(TextureVertex::FVF);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, 0, 0, 0));

	float left, top, right, bottom;
	float tileWidth = 0.0f, tileHeight = 0.0f;
	for (int i = 0; i < 9; i++)
	{
		if (tiles[i])
		{
			tileWidth = (float)tiles[i]->GetWidth();
			tileHeight = (float)tiles[i]->GetHeight();
			break;
		}
	}

	if (tiles[0])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.top()) - 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[0]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[1])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.top()) - 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, 1.0f);
		m_device->SetTexture(0, *tiles[1]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[2])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.top()) - 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[2]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}

	if (tiles[3])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight - 0.5f;
		right = left + tileWidth;
		bottom = top + (float)rect.height() - tileHeight * 2.0f;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, (top - bottom) / tileHeight);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, (top - bottom) / tileHeight);
		m_device->SetTexture(0, *tiles[3]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[4])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight - 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + (float)rect.height() - tileHeight * 2.0f;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, (top - bottom) / tileHeight);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, (top - bottom) / tileHeight);
		m_device->SetTexture(0, *tiles[4]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[5])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.top()) + tileHeight - 0.5f;
		right = left + tileWidth;
		bottom = top + (float)rect.height() - tileHeight * 2.0f;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, (top - bottom) / tileHeight);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, (top - bottom) / tileHeight);
		m_device->SetTexture(0, *tiles[5]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}

	if (tiles[6])
	{
		left = (float)(m_origin.x() + rect.left()) - 0.5f;
		top = (float)(m_origin.y() + rect.bottom()) - tileHeight + 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[6]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[7])
	{
		left = (float)(m_origin.x() + rect.left()) + tileWidth - 0.5f;
		top = (float)(m_origin.y() + rect.bottom()) - tileHeight + 0.5f;
		right = left + (float)rect.width() - tileWidth * 2.0f;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2((left - right) / tileWidth, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2((left - right) / tileWidth, 1.0f);
		m_device->SetTexture(0, *tiles[7]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
	if (tiles[8])
	{
		left = (float)(m_origin.x() + rect.right()) - tileWidth + 0.5f;
		top = (float)(m_origin.y() + rect.bottom()) - tileHeight + 0.5f;
		right = left + tileWidth;
		bottom = top + tileHeight;
		vertices[0].p = D3DXVECTOR2(left, top);
		vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
		vertices[1].p = D3DXVECTOR2(left, bottom);
		vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
		vertices[2].p = D3DXVECTOR2(right, top);
		vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
		vertices[3].p = D3DXVECTOR2(right, bottom);
		vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);
		m_device->SetTexture(0, *tiles[8]);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));
	}
}

void CRender2D::RenderRoundRect(const QRect& rect, DWORD colorLT, DWORD colorRT, DWORD colorLB, DWORD colorRB)
{
	ColorVertex vertices[8];

	const float left = (float)(m_origin.x() + rect.left());
	const float top = (float)(m_origin.y() + rect.top());
	const float right = (float)(m_origin.x() + rect.right());
	const float bottom = (float)(m_origin.y() + rect.bottom());

	vertices[0].p = D3DXVECTOR2(left + 1.0f, top);
	vertices[0].c = colorLT;
	vertices[1].p = D3DXVECTOR2(right - 1.0f, top);
	vertices[1].c = colorRT;
	vertices[2].p = D3DXVECTOR2(left, top + 1.0f);
	vertices[2].c = colorLT;
	vertices[3].p = D3DXVECTOR2(left, bottom - 1.0f);
	vertices[3].c = colorLB;
	vertices[4].p = D3DXVECTOR2(left + 1.0f, bottom - 1.0f);
	vertices[4].c = colorLB;
	vertices[5].p = D3DXVECTOR2(right - 1.0f, bottom - 1.0f);
	vertices[5].c = colorRB;
	vertices[6].p = D3DXVECTOR2(right - 1.0f, bottom - 2.0f);
	vertices[6].c = colorRB;
	vertices[7].p = D3DXVECTOR2(right - 1.0f, top + 1.0f);
	vertices[7].c = colorRT;

	m_device->SetFVF(ColorVertex::FVF);
	m_device->SetTexture(0, null);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 4, vertices, sizeof(ColorVertex));
}

void CRender2D::RenderRect(const QRect& rect, DWORD colorLT, DWORD colorRT, DWORD colorLB, DWORD colorRB)
{
	ColorVertex vertices[8];

	const float left = (float)(m_origin.x() + rect.left());
	const float top = (float)(m_origin.y() + rect.top());
	const float right = (float)(m_origin.x() + rect.right());
	const float bottom = (float)(m_origin.y() + rect.bottom());

	vertices[0].p = D3DXVECTOR2(left, top);
	vertices[0].c = colorLT;
	vertices[1].p = D3DXVECTOR2(right, top);
	vertices[1].c = colorRT;
	vertices[2].p = D3DXVECTOR2(left, top);
	vertices[2].c = colorLT;
	vertices[3].p = D3DXVECTOR2(left, bottom);
	vertices[3].c = colorLB;
	vertices[4].p = D3DXVECTOR2(left, bottom);
	vertices[4].c = colorLB;
	vertices[5].p = D3DXVECTOR2(right, bottom);
	vertices[5].c = colorRB;
	vertices[6].p = D3DXVECTOR2(right, bottom);
	vertices[6].c = colorRB;
	vertices[7].p = D3DXVECTOR2(right, top);
	vertices[7].c = colorRT;

	m_device->SetFVF(ColorVertex::FVF);
	m_device->SetTexture(0, null);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 4, vertices, sizeof(ColorVertex));
}

void CRender2D::RenderFillRect(const QRect& rect, DWORD colorLT, DWORD colorRT, DWORD colorLB, DWORD colorRB)
{
	ColorVertex vertices[4];

	const float left = (float)(m_origin.x() + rect.left());
	const float top = (float)(m_origin.y() + rect.top());
	const float right = (float)(m_origin.x() + rect.right());
	const float bottom = (float)(m_origin.y() + rect.bottom());

	vertices[0].p = D3DXVECTOR2(left, top);
	vertices[0].c = colorLT;
	vertices[1].p = D3DXVECTOR2(left, bottom);
	vertices[1].c = colorLB;
	vertices[2].p = D3DXVECTOR2(right, top);
	vertices[2].c = colorRT;
	vertices[3].p = D3DXVECTOR2(right, bottom);
	vertices[3].c = colorRB;

	m_device->SetFVF(ColorVertex::FVF);
	m_device->SetTexture(0, null);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(ColorVertex));
}

void CRender2D::RenderPoint(const QPoint& pos, DWORD color)
{
	ColorVertex vertex;

	vertex.p = D3DXVECTOR2((float)(m_origin.x() + pos.x()), (float)(m_origin.y() + pos.y()));
	vertex.c = color;

	m_device->SetFVF(ColorVertex::FVF);
	m_device->SetTexture(0, null);
	m_device->DrawPrimitiveUP(D3DPT_POINTLIST, 1, &vertex, sizeof(ColorVertex));
}

void CRender2D::RenderLine(const QPoint& p1, const QPoint& p2, DWORD color1, DWORD color2)
{
	ColorVertex vertices[2];

	vertices[0].p = D3DXVECTOR2((float)(m_origin.x() + p1.x()), (float)(m_origin.y() + p1.y()));
	vertices[0].c = color1;
	vertices[1].p = D3DXVECTOR2((float)(m_origin.x() + p2.x()), (float)(m_origin.y() + p2.y()));
	vertices[1].c = color2;

	m_device->SetFVF(ColorVertex::FVF);
	m_device->SetTexture(0, null);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(ColorVertex));
}

void CRender2D::RenderGradationRect(const QRect& rect, DWORD color1t, DWORD color1b, DWORD color2b, int midPercent)
{
	const int firstHeight = rect.height() * midPercent / 100;
	QRect rect1 = rect;
	rect1.setBottom(rect1.top() + firstHeight);
	QRect rect2 = rect;
	rect2.setTop(rect2.top() + firstHeight);
	RenderFillRect(rect1, color1t, color1t, color1b, color1b);
	RenderFillRect(rect2, color1b, color1b, color2b, color2b);
}