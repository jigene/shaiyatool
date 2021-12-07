///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "ui_MainFrame.h"

class CFileModel;

class CMainFrame : public QMainWindow
{
	Q_OBJECT

public:
	CMainFrame(QWidget *parent = 0);
	virtual ~CMainFrame();

	bool Initialize();

	void OpenFile(const string& filename);

	public slots:
	void NewFile();
	void CloseFile();
	void SetLanguage(QAction* action);
	void SwitchFullscreen(bool fullscreen);
	void OpenFile();
	void SetEncrypted(bool encrypted);
	void About();
	void AboutQt();
	void SetKey();
	void ShowListMenu(const QPoint& pt);
	void DeleteFiles();
	void SaveFile();
	void SaveFileAs();
	void AddFiles();
	void ExtractFiles();
	void ClientExtractor();
	void SetViewMode(QAction* action);
	void SetSortMode(QAction* action);

private:
	Ui::MainFrameClass ui;
	QLabel* m_status;
	CFileModel* m_files;
	string m_filename;
	QMenu* m_listMenu;
	int m_sortMode;
	int m_viewMode;
	QActionGroup* m_sortActionGroup;
	QActionGroup* m_viewActionGroup;
	QTranslator m_translator;
	int m_language;
	QActionGroup* m_languageActionGroup;

	void _saveFile(const string& filename);

	void _connectWidgets();
	void _loadSettings();
	void _setShortcuts();

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dragLeaveEvent(QDragLeaveEvent* event);
	virtual void dragMoveEvent(QDragMoveEvent* event);
	virtual void dropEvent(QDropEvent* event);
	virtual void closeEvent(QCloseEvent* event);
};

#endif // MAINFRAME_H
