///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "ui_MainFrame.h"

class CDataManager;
class CGUIEditor;
class CWndControl;
struct ControlData;

class CMainFrame : public QMainWindow
{
	Q_OBJECT

public:
	CMainFrame(QWidget *parent = null);
	~CMainFrame();

	bool Initialize();

	void SetEditControl();
	QAction* GetDeleteCtrlAction() { return ui.actionSupprimer_le_control; }
	QAction* GetSetOnTopAction() { return ui.actionRamener_au_premier_plan; }
	int GetCreateCtrlType() const { return m_createControlType; }

public slots:
	void Save();
	void SwitchFullscreen(bool fullscreen);
	void About();
	void AboutQt();
	void ShowWindow(const QModelIndex& index);
	void ShowGrid(bool show);
	void SetBackgroundColor();
	void SetGridSize();
	void SetCling(bool cling);
	void AddNewWindow();
	void DeleteControl();
	void CopyControl();
	void PasteControl();
	void CutControl();
	void DeleteWindow();
	void SetCreateControl(QAction* action);
	void SetControlOnTop();
	void SetLanguage(QAction* action);

	void EditControlID(const QString& text);
	void EditControlWidth(int width);
	void EditControlHeight(int height);
	void EditControlText(const QString& text);
	void EditControlTooltip(const QString& text);
	void EditControlTexture();
	void SetControlTexture();
	void ButtonFitTextureSize();
	void EditControlTiles(bool tiles);
	void EditControlNoDrawFrame(bool noDrawFrame);
	void EditControlPosX(int x);
	void EditControlPosY(int y);
	void EditControlVisible(bool visible);
	void EditControlEnabled(bool enabled);
	void EditControlTabStop(bool tabStop);
	void EditControlGroup(bool group);
	void EditControlScrollBar(bool scrollBar);
	void EditControlHAlignLeft(bool align);
	void EditControlHAlignMiddle(bool align);
	void EditControlHAlignRight(bool align);
	void EditControlVAlignTop(bool align);
	void EditControlVAlignCenter(bool align);
	void EditControlVAlignBottom(bool align);
	void EditWindowCaption(bool caption);
	void EditWindowNoFrame(bool noFrame);
	void EditWindowFormat(const QString& format);
	void EditWindowIcon(const QString& icon);
	void EditControlColor();

	void AlignControlsOnTop();
	void AlignControlsOnLeft();
	void AlignControlsOnRight();
	void AlignControlsOnBottom();

	void SetControlPos(const QPoint& pos);
	void SetControlRect(const QRect& rect);

private:
	Ui::CMainFrameClass ui;
	CDataManager* m_dataMng;
	QLabel* m_status;
	QStringListModel* m_windowList;
	CGUIEditor* m_editor;
	QActionGroup* m_controlsGroup;
	int m_createControlType;
	CPtrArray<ControlData> m_clipboardControls;
	QTranslator m_translator;
	int m_language;
	QActionGroup* m_languageActionGroup;

	void _connectWidgets();
	void _loadSettings();
	void _setShortcuts();

protected:
	virtual void changeEvent(QEvent* event);
	virtual void closeEvent(QCloseEvent* event);
};

#endif // MAINFRAME_H