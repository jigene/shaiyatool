///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WNDCONTROL_H
#define WNDCONTROL_H

class CRender2D;
class CFont;

enum class EWndMsg
{
	Error,
	Clicked
};

class CWndControl
{
public:
	CWndControl(CRender2D* render2D);
	virtual ~CWndControl();

	void Create(int ID, const QRect& rect, int flags, CWndControl* parent = null);
	void SetRect(const QRect& rect, bool sizeEvent);
	void SetTexture(const string& filename, bool tiles);

	void AddControl(CWndControl* control);
	void RemoveControl(int ID, bool deleteControl = true);
	CWndControl* GetControl(int ID) const;
	void SetControlOnTop(CWndControl* control);

	void Paint(const QRect& windowRect, const QRect& parentRect);
	void RenderSelection(const QRect& windowRect, const QRect& parentRect);

	bool HasFlag(int flag) const { return (m_flags & flag) != 0; }
	void SetFlag(int flag) { m_flags |= flag; }
	void RemoveFlag(int flag) { m_flags &= ~flag; }
	bool IsMouseButtonDown(Qt::MouseButton button) { return (m_mouseButtonsDown & button) != 0; }
	void SetText(const string& text) { m_text = text; }
	const string& GetText() const { return m_text; }
	int GetID() const { return m_ID; }
	void SetID(int id) { m_ID = id; }
	QSize GetSize() const { return m_windowRect.size(); }
	bool IsMouseHover() const { return m_mouseHover; }
	bool IsEnabled() const { return m_enabled; }
	const string& GetTextureName() const { return m_textureName; }
	void MustFocus(CWndControl* ctrl) { m_mustFocus = ctrl; }
	CWndControl* GetParent() const { return m_parent; }
	CPtrArray<CWndControl>* GetControls() { return &m_controls; }
	int GetControlCount() const { return m_controls.GetSize(); }
	bool Tiles() const { return m_tiles; }
	void SetEnabled(bool enabled) { m_enabled = enabled; }
	bool IsPush() const { return m_push; }
	bool IsRoot() const;
	QRect GetWindowRect(bool parent) const;
	QRect GetClientRect(bool parent) const;
	QRect GetLayoutRect() const;
	QPoint GetScreenPos() const;
	void Move(const QPoint& point);
	void SetColor(const QColor& color);

	virtual void PaintFrame();
	virtual void InitialUpdate();
	virtual void Resize(const QSize& size);
	virtual void EraseBackground();
	virtual void Draw();
	virtual void MouseMove(const QPoint& pos);
	virtual void MouseEnter();
	virtual void MouseLeave();
	virtual void MouseButtonDown(Qt::MouseButton button, const QPoint& pos);
	virtual void MouseButtonUp(Qt::MouseButton button, const QPoint& pos);
	virtual void ChildNotify(int childID, EWndMsg msg);
	virtual void LostFocus();
	virtual CWndControl* GetEditableControl(const QPoint& mousePos);

	virtual int GetTilesCount() const {
		return 9;
	}
	virtual int GetType() const {
		return WTYPE_BASE;
	}

protected:
	CRender2D* m_render2D;
	int m_ID;
	int m_flags;
	QRect m_windowRect;
	QRect m_clientRect;
	CWndControl* m_parent;
	CPtrArray<CWndControl> m_controls;
	string m_text;
	string m_textureName;
	bool m_tiles;
	CTexture* m_texture;
	CTexture** m_textureTiles;
	bool m_hasTexture;
	DWORD m_color;
	bool m_enabled;
	bool m_push;
	bool m_mouseHover;
	Qt::MouseButtons m_mouseButtonsDown;
	CWndControl* m_mustFocus;

public:
	static CFont* s_titleFont;
	static CFont* s_textFont;
	static CTexture* s_defaultBackground;
	static CWndControl* s_focus;
	static CPtrArray<CWndControl> s_selection;
};

#endif // WNDCONTROL_H