///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef FONT_H
#define FONT_H

class CFont
{
public:
	CFont(LPDIRECT3DDEVICE9 device);
	~CFont();

	bool Create(const string& name, int charSize, DWORD flags = Normal, DWORD color = 0xffffffff, int outline = 0, DWORD outlineColor = 0xff000000);
	QSize GetSize(const string& text);
	void Render(const string& text, const QPoint& pos, DWORD color = 0xffffffff);

	int GetMaxHeight() const {
		return m_maxHeight; }

public:
	enum
	{
		Normal = 0x0,
		Bold = 0x0001,
		Italic = 0x0002
	};

private:
	struct Row
	{
		Row() : width(0), top(0), height(0) {}
		Row(int rowTop, int rowHeight) : width(0), top(rowTop), height(rowHeight) {}
		int width;
		int top;
		int height;
	};
	struct Glyph
	{
		int advance = 0;
		QRect bounds;
		float tu1, tv1, tu2, tv2;
	};
	struct Pixel
	{
		byte r;
		byte g;
		byte b;
		byte a;
	};

	LPDIRECT3DDEVICE9 m_device;
	int m_outline;
	Pixel m_outlineColor;
	DWORD m_color;
	HDC m_hDC;
	HFONT m_hFont;
	HBITMAP m_hBitmap;
	DWORD* m_bitmapBits;
	QMap<QChar, Glyph> m_glyphs;
	LPDIRECT3DTEXTURE9 m_texture;
	int m_maxHeight;
	int m_spaceWidth;
	int m_tabWidth;
	QSize m_bitmapSize;
	QSize m_textureSize;
	QVector<Row> m_rows;
	int m_nextRow;

	Glyph _loadGlyph(QChar c);
	QRect _findGlyphRect(int width, int height);
	void _makeOutline(Pixel* bitmap, int pitch, int width, int height);

	const Glyph& _getGlyph(QChar c) {
		const QMap<QChar, Glyph>::const_iterator it = m_glyphs.constFind(c);
		if (it != m_glyphs.constEnd())
			return it.value();
		else
		{
			m_glyphs[c] = _loadGlyph(c);
			return m_glyphs[c];
		}
	}

public:
	static bool RestoreStaticDeviceObjects(LPDIRECT3DDEVICE9 device);
	static void InvalidateStaticDeviceObjects();

private:
	static LPDIRECT3DVERTEXBUFFER9 s_VB;
};

#endif // FONT_H