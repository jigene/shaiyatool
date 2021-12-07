///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QtWidgets/QMainWindow>
#include "ui_MainFrame.h"

class CModelViewer;
class CAnimatedMesh;
class CTimeline;
class CSoundMng;

class CMainFrame : public QMainWindow
{
	Q_OBJECT

public:
	CMainFrame(QWidget* parent = 0);
	~CMainFrame();

	bool Initialize();

	CTimeline* GetTimeline() const { return m_timeline; }

	void OpenFile(const string& filename);
	void SetTextureEx(int index);

public slots:
	void About();
	void AboutQt();
	void ShowGrid(bool show);
	void CloseFile();
	void LoadTextureEx();
	void PlayMotion(const QModelIndex & index);
	void Play(bool play);
	void Stop();
	void MotionAttributeModified(int row, int frame, bool removed);
	void ChangeLOD(QAction* action);
	void SaveFile();
	void OpenFile();
	void EditEffects();
	void ImportGuide();
	void SwitchFullscreen(bool fullscreen);
	void SetBackgroundColor();
	void SetLanguage(QAction* action);
	void ShowCollObj(bool show);
	void ChangeCW();
	void SkinAuto();
	void SetSoundVolume();
	void ShowBones(bool show);
	void SetFillMode(QAction* action);
	void SetScaleFactor();
	void SetReferenceModel();
	void CollisionAuto();

private:
	Ui::MainFrameClass ui;
	CModelViewer* m_modelViewer;
	QLabel* m_status;
	CAnimatedMesh* m_mesh;
	bool m_assimpImport;
	int m_meshSex;
	QStringListModel* m_motionList;
	string m_dragFilename;
	string m_motionName;
	string m_filename;
	CTimeline* m_timeline;
	QActionGroup* m_actionsLOD;
	string m_supportedImportFiles;
	CSoundMng* m_soundMng;
	QStringList m_soundsList;
	QActionGroup* m_fillModeActionGroup;
	float m_importScaleFactor;
	int m_referenceModelID;
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
	virtual void changeEvent(QEvent* event);
	virtual void closeEvent(QCloseEvent* event);
};

#endif // MAINFRAME_H