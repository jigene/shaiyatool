///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Font.h"

#define MaxVertexCount	600

struct FontVertex
{
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 };
	D3DXVECTOR4 p;
	DWORD c;
	float tu, tv;
};

LPDIRECT3DVERTEXBUFFER9 CFont::s_VB = null;

bool CFont::RestoreStaticDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->CreateVertexBuffer(MaxVertexCount * sizeof(FontVertex), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &s_VB, null)))
		return false;
	return true;
}

void CFont::InvalidateStaticDeviceObjects()
{
	Release(s_VB);
}

CFont::CFont(LPDIRECT3DDEVICE9 pDevice)
	: m_device(pDevice),
	m_hDC(null),
	m_hFont(null),
	m_texture(null),
	m_maxHeight(0),
	m_hBitmap(null),
	m_bitmapBits(null),
	m_nextRow(3),
	m_spaceWidth(0),
	m_tabWidth(0)
{
}

CFont::~CFont()
{
	Release(m_texture);
	DeleteObject(m_hBitmap);
	DeleteObject(m_hFont);
	DeleteObject(m_hDC);
}

bool CFont::Create(const string& name, int charSize, DWORD flags, DWORD color, int outline, DWORD outlineColor)
{
	m_outline = outline;
	m_outlineColor = *(Pixel*)&outlineColor;
	m_color = color;

	m_hDC = CreateCompatibleDC(null);
	SetMapMode(m_hDC, MM_TEXT);

	wchar_t* fontname = new wchar_t[name.size() + 1];
	name.toWCharArray(fontname);
	fontname[name.size()] = '\0';

	INT height = -MulDiv(charSize, (INT)(GetDeviceCaps(m_hDC, LOGPIXELSY) * 1.0f), 72);
	DWORD dwBold = (flags & Bold) ? FW_BOLD : FW_NORMAL;
	DWORD dwItalic = (flags & Italic) ? TRUE : FALSE;
	m_hFont = CreateFont(height, 0, 0, 0, dwBold, dwItalic,
		FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		VARIABLE_PITCH, fontname);

	DeleteArray(fontname);

	if (!m_hFont)
		return false;

	SelectObject(m_hDC, m_hFont);

	wchar_t biggest = 'W';

	SIZE temp;
	if (GetTextExtentPoint32(m_hDC, &biggest, 1, &temp) == FALSE)
		return false;
	m_bitmapSize = QSize(temp.cx, temp.cy);

	biggest = ' ';
	if (GetTextExtentPoint32(m_hDC, &biggest, 1, &temp) == FALSE)
		return false;
	m_spaceWidth = temp.cx;

	biggest = '\t';
	if (GetTextExtentPoint32(m_hDC, &biggest, 1, &temp) == FALSE)
		return false;
	m_tabWidth = temp.cx;

	m_bitmapSize.rheight() += 2;
	m_maxHeight = m_bitmapSize.height() + (m_outline * 2);

	BITMAPINFO bmi;
	memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = m_bitmapSize.width();
	bmi.bmiHeader.biHeight = -m_bitmapSize.height();
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;

	m_hBitmap = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, (void**)&m_bitmapBits, null, 0);

	if (!m_hBitmap)
		return false;

	SelectObject(m_hDC, m_hBitmap);

	m_textureSize = QSize(256, 256);
	if (FAILED(m_device->CreateTexture(m_textureSize.width(), m_textureSize.height(), 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_texture, null)))
		return false;

	return true;
}

void CFont::Render(const string& text, const QPoint& pos, DWORD color)
{
	if (text.isEmpty())
		return;

	m_device->SetFVF(FontVertex::FVF);
	m_device->SetTexture(0, m_texture);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_device->SetStreamSource(0, s_VB, 0, sizeof(FontVertex));
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, color);

	int primitiveCount = 0;
	QPoint pt = pos;

	FontVertex* vertex = null;
	s_VB->Lock(0, 0, (void**)&vertex, D3DLOCK_DISCARD);

	const int len = text.size();
	for (int i = 0; i < len; i++)
	{
		const QChar c = text[i];

		if (c == ' ')
			pt.rx() += m_spaceWidth;
		else if (c == '\t')
			pt.rx() += m_tabWidth;
		else if (c == '\n')
		{
			pt.setX(pos.x());
			pt.ry() += m_maxHeight;
		}
		else
		{
			const Glyph& glyph = _getGlyph(c);
			const D3DXVECTOR4 coords((float)(pt.x() + glyph.bounds.left()) - 0.5f,
				(float)(pt.x() + glyph.bounds.right()) - 0.5f,
				(float)(pt.y() + glyph.bounds.top()) - 0.5f,
				(float)(pt.y() + glyph.bounds.bottom()) - 0.5f);

			vertex->p = D3DXVECTOR4(coords.x, coords.z, 0.5f, 1.0f);
			vertex->tu = glyph.tu1;
			vertex->tv = glyph.tv1;
			vertex->c = color;
			vertex++;
			vertex->p = D3DXVECTOR4(coords.y, coords.z, 0.5f, 1.0f);
			vertex->tu = glyph.tu2;
			vertex->tv = glyph.tv1;
			vertex->c = color;
			vertex++;
			vertex->p = D3DXVECTOR4(coords.y, coords.w, 0.5f, 1.0f);
			vertex->tu = glyph.tu2;
			vertex->tv = glyph.tv2;
			vertex->c = color;
			vertex++;

			vertex->p = D3DXVECTOR4(coords.y, coords.w, 0.5f, 1.0f);
			vertex->tu = glyph.tu2;
			vertex->tv = glyph.tv2;
			vertex->c = color;
			vertex++;
			vertex->p = D3DXVECTOR4(coords.x, coords.w, 0.5f, 1.0f);
			vertex->tu = glyph.tu1;
			vertex->tv = glyph.tv2;
			vertex->c = color;
			vertex++;
			vertex->p = D3DXVECTOR4(coords.x, coords.z, 0.5f, 1.0f);
			vertex->tu = glyph.tu1;
			vertex->tv = glyph.tv1;
			vertex->c = color;
			vertex++;

			primitiveCount += 2;
			pt.rx() += glyph.advance;

			if (primitiveCount * 3 > (MaxVertexCount - 6))
			{
				s_VB->Unlock();
				m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, primitiveCount);
				primitiveCount = 0;
				vertex = null;
				s_VB->Lock(0, 0, (void**)&vertex, D3DLOCK_DISCARD);
			}
		}
	}

	s_VB->Unlock();
	if (primitiveCount)
		m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, primitiveCount);
}

QSize CFont::GetSize(const string& text)
{
	if (text.size() == 0)
		return QSize(0, 0);

	QSize size(0, m_maxHeight);
	int width = 0;

	const int len = text.size();
	for (int i = 0; i < len; i++)
	{
		const QChar c = text[i];

		if (c == ' ')
			width += m_spaceWidth;
		else if (c == '\t')
			width += m_tabWidth;
		else if (c == '\n')
		{
			width = 0;
			size.rheight() += m_maxHeight;
		}
		else
			width += _getGlyph(c).advance;

		if (width > size.width())
			size.setWidth(width);
	}

	return size;
}

CFont::Glyph CFont::_loadGlyph(QChar c)
{
	Glyph glyph;
	memset(&glyph, 0, sizeof(Glyph));

	const wchar_t wc = c.unicode();

	SIZE size;
	if (GetTextExtentPoint32(m_hDC, &wc, 1, &size) == FALSE)
		return glyph;

	memset(m_bitmapBits, 0, m_bitmapSize.width() * m_bitmapSize.height() * sizeof(DWORD));
	if (ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, null, &wc, 1, null) == FALSE)
		return glyph;

	if (size.cy > m_maxHeight)
		size.cy = m_maxHeight;

	glyph.advance = size.cx;
	glyph.bounds.setLeft(0);
	glyph.bounds.setRight(size.cx + m_outline * 2);
	glyph.bounds.setTop(0);
	glyph.bounds.setBottom(size.cy + m_outline * 2);

	if (size.cx > 0 && size.cy > 0)
	{
		const int finalWidth = size.cx + (m_outline * 2);
		const int finalHeight = size.cy + (m_outline * 2);

		const QRect rect = _findGlyphRect(finalWidth, finalHeight);

		D3DLOCKED_RECT d3dlr;
		if (FAILED(m_texture->LockRect(0, &d3dlr, null, 0)))
			return glyph;

		char* dstRow = (char*)d3dlr.pBits;
		dstRow = &dstRow[((rect.top() + m_outline) * d3dlr.Pitch + ((rect.left() + m_outline) * sizeof(DWORD)))];
		DWORD* dest32;

		int x, y;
		for (y = 0; y < size.cy; y++)
		{
			dest32 = (DWORD*)dstRow;
			for (x = 0; x < size.cx; x++)
				*dest32++ = (m_bitmapBits[y * m_bitmapSize.width() + x] & 0x00ffffff) ? 0x00000000 : m_color;
			dstRow += d3dlr.Pitch;
		}

		if (m_outline)
		{
			dstRow = &(((char*)d3dlr.pBits)[rect.top() * d3dlr.Pitch + rect.left() * sizeof(DWORD)]);
			_makeOutline((Pixel*)dstRow, d3dlr.Pitch / sizeof(DWORD), finalWidth, finalHeight);
		}

		m_texture->UnlockRect(0);

		glyph.tu1 = (float)rect.left() / (float)m_textureSize.width();
		glyph.tu2 = (float)rect.right() / (float)m_textureSize.width();
		glyph.tv1 = (float)rect.top() / (float)m_textureSize.height();
		glyph.tv2 = (float)rect.bottom() / (float)m_textureSize.height();
	}

	return glyph;
}

void CFont::_makeOutline(Pixel* bitmap, int pitch, int width, int height)
{
	static const byte g_outLine1[9] =
	{
		0x0f, 0xef, 0x0f,
		0xef, 0xef, 0xef,
		0x0f, 0xef, 0x0f
	};

	static const byte g_outLine2[25] =
	{
		0x3f, 0x7f, 0x7f, 0x7f, 0x3f,
		0x7f, 0xaf, 0xef, 0xaf, 0x7f,
		0x7f, 0xef, 0xef, 0xef, 0x7f,
		0x7f, 0xaf, 0xef, 0xaf, 0x7f,
		0x3f, 0x7f, 0x7f, 0x7f, 0x3f
	};

	static const byte g_outLine3[49] =
	{
		0x0f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x0f,
		0x4f, 0x7f, 0xef, 0xef, 0xef, 0x7f, 0x4f,
		0x4f, 0xef, 0xef, 0xef, 0xef, 0xef, 0x4f,
		0x4f, 0xef, 0xef, 0xef, 0xef, 0xef, 0x4f,
		0x4f, 0xaf, 0xef, 0xef, 0xaf, 0xef, 0x4f,
		0x4f, 0x7f, 0xef, 0xef, 0xef, 0x7f, 0x4f,
		0x0f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x0f
	};

	static const byte g_outLine4[82] =
	{
		0x0f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x0f,
		0x4f, 0x7f, 0xef, 0xef, 0xef, 0xef, 0xef, 0x7f, 0x4f,
		0x4f, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0x4f,
		0x4f, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0x4f,
		0x4f, 0xaf, 0xef, 0xef, 0xaf, 0xef, 0xaf, 0xef, 0x4f,
		0x4f, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0x4f,
		0x4f, 0xaf, 0xef, 0xef, 0xaf, 0xef, 0xaf, 0xef, 0x4f,
		0x4f, 0x7f, 0xef, 0xef, 0xef, 0xef, 0xef, 0x7f, 0x4f,
		0x0f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x4f, 0x0f
	};

	const byte* outline;
	switch (m_outline)
	{
	case 1: outline = g_outLine1; break;
	case 2: outline = g_outLine2; break;
	case 3: outline = g_outLine3; break;
	case 4: outline = g_outLine4; break;
	}

	Pixel* pixel;
	const int outlineLentgh = m_outline * 2 + 1;
	int x, y, x2, y2, x3, y3;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			if (bitmap[y * pitch + x].a == 255)
			{
				for (y2 = y - m_outline, y3 = 0; y2 <= y + m_outline; y2++, y3++)
				{
					for (x2 = x - m_outline, x3 = 0; x2 <= x + m_outline; x2++, x3++)
					{
						if (x2 >= 0 && x2 < width && y2 >= 0 && y2 < height)
						{
							pixel = &bitmap[y2 * pitch + x2];
							if (pixel->a < 255)
							{
								const byte alpha = outline[y3 * outlineLentgh + x3];
								if (pixel->a < alpha)
								{
									*pixel = m_outlineColor;
									pixel->a = alpha;
								}
							}
						}
					}
				}
			}
		}
	}
}

QRect CFont::_findGlyphRect(int width, int height)
{
	Row* row = nullptr;
	float bestRatio = 0;
	for (auto it = m_rows.begin(); it != m_rows.end() && !row; ++it)
	{
		const float ratio = static_cast<float>(height) / (*it).height;

		if ((ratio < 0.7f) || (ratio > 1.f))
			continue;
		if (width > m_textureSize.width() - (*it).width)
			continue;
		if (ratio < bestRatio)
			continue;

		row = &(*it);
		bestRatio = ratio;
	}

	if (!row)
	{
		int rowHeight = height + height / 10;
		while (m_nextRow + rowHeight >= m_textureSize.width())
		{
			D3DCAPS9 d3dCaps;
			m_device->GetDeviceCaps(&d3dCaps);

			if ((m_textureSize.width() * 2 <= d3dCaps.MaxTextureWidth) && (m_textureSize.height() * 2 <= d3dCaps.MaxTextureHeight))
			{
				LPDIRECT3DTEXTURE9 pNewTexture;
				m_device->CreateTexture(m_textureSize.width() * 2, m_textureSize.height() * 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pNewTexture, null);

				LPDIRECT3DSURFACE9 destSurface = null;
				LPDIRECT3DSURFACE9 srcSurface = null;

				pNewTexture->GetSurfaceLevel(0, &destSurface);
				m_texture->GetSurfaceLevel(0, &srcSurface);

				const RECT rect[] = { 0, 0, m_textureSize.width(), m_textureSize.height() };

				D3DXLoadSurfaceFromSurface(destSurface, null, rect, srcSurface, null, rect, D3DX_DEFAULT, 0);

				Release(srcSurface);
				Release(destSurface);
				Release(m_texture);

				m_texture = pNewTexture;

				m_textureSize.rwidth() *= 2;
				m_textureSize.rheight() *= 2;

				for (auto it = m_glyphs.begin(); it != m_glyphs.end(); it++)
				{
					(*it).tu1 /= 2.0f;
					(*it).tu2 /= 2.0f;
					(*it).tv1 /= 2.0f;
					(*it).tv2 /= 2.0f;
				}
			}
			else
			{
				qWarning("Failed to add a new character to the font: the maximum texture size has been reached");
				return QRect(0, 0, 2, 2);
			}
		}

		m_rows.push_back(Row(m_nextRow, rowHeight));
		m_nextRow += rowHeight;
		row = &m_rows.back();
	}

	const QRect rect(QPoint(row->width, row->top), QPoint(row->width + width, row->top + height));
	row->width += width;
	return rect;
}