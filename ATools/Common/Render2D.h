///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef RENDER2D_H
#define RENDER2D_H

class CFont;

class CRender2D
{
public:
	CRender2D(LPDIRECT3DDEVICE9 device);
	~CRender2D();

	void RenderTexture(const QRect& rect, const CTexture* texture, int alpha = 255, bool adaptSize = true);
	void RenderTexture(const QPoint& pt, const CTexture* texture, int alpha = 255);
	void RenderTexture(const QPoint& pt, const CTexture* texture, const QRectF& textureRect, int alpha = 255);
	void Render12Tiles(const QRect& rect, CTexture** tiles, int alpha = 255);
	void Render9Tiles(const QRect& rect, CTexture** tiles, int alpha = 255);

	void RenderText(CFont* font, const string& text, const QPoint& point, DWORD color = 0xffffffff, DWORD shadowColor = 0x00000000);

	void RenderRoundRect(const QRect& rect, DWORD colorLT, DWORD colorRT, DWORD colorLB, DWORD colorRB);
	void RenderRoundRect(const QRect& rect, DWORD color) {
		RenderRoundRect(rect, color, color, color, color);
	}

	void RenderRect(const QRect& rect, DWORD colorLT, DWORD colorRT, DWORD colorLB, DWORD colorRB);
	void RenderRect(const QRect& rect, DWORD color) {
		RenderRect(rect, color, color, color, color);
	}

	void RenderFillRect(const QRect& rect, DWORD colorLT, DWORD colorRT, DWORD colorLB, DWORD colorRB);
	void RenderFillRect(const QRect& rect, DWORD color) {
		RenderFillRect(rect, color, color, color, color);
	}

	void RenderGradationRect(const QRect& rect, DWORD color1t, DWORD color1b, DWORD color2b, int midPercent = 40);

	void RenderPoint(const QPoint& pos, DWORD color);

	void RenderLine(const QPoint& p1, const QPoint& p2, DWORD color1, DWORD color2);
	void RenderLine(const QPoint& p1, const QPoint& p2, DWORD color) {
		RenderLine(p1, p2, color, color);
	}

	void SetViewport(const QRect& rect);
	const QRect& Viewport() const {
		return m_viewport;
	}

	void SetOrigin(const QPoint& origin) {
		m_origin = origin;
	}
	const QPoint& Origin() const {
		return m_origin;
	}

private:
	LPDIRECT3DDEVICE9 m_device;
	QPoint m_origin;
	QRect m_viewport;
};

#endif // RENDER2D_H