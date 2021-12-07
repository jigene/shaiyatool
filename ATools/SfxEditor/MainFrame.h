///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "ui_MainFrame.h"

class CModelViewer;
class CAnimatedMesh;
class CSfxModel;
class CTimeline;
class CSfxPart;
struct SfxKeyFrame;

class CMainFrame : public QMainWindow
{
	Q_OBJECT

public:
	CMainFrame(QWidget *parent = 0);
	~CMainFrame();

	bool Initialize();

	void OpenFile(const string& filename);

	CTimeline* GetTimeline() const { return m_timeline; }
	string MakePartName(const CSfxPart* part) const;

public slots:
	void CloseFile();
	void OpenFile();
	void MergeFile();
	void ShowGrid(bool show);
	void Play(bool play);
	void Stop();
	void SwitchFullscreen(bool fullscreen);
	void About();
	void AboutQt();
	void ListPartChanged(int currentRow);
	void RemovePart();
	void AddPart();
	void EditKey(int row, int keyIndex);
	void EditKeyModified(int row, int frame, bool removed);
	void SaveFile();
	void SaveFileAs();
	void SetBackgroundColor();
	void SetLanguage(QAction* action);

	void UpdateKeyPos();
	void EditKeyPosX(double value);
	void EditKeyPosY(double value);
	void EditKeyPosZ(double value);
	void EditKeyPosRotX(double value);
	void EditKeyPosRotY(double value);
	void EditKeyPosRotZ(double value);
	void EditKeyRotX(double value);
	void EditKeyRotY(double value);
	void EditKeyRotZ(double value);
	void EditKeyScaleX(double value);
	void EditKeyScaleY(double value);
	void EditKeyScaleZ(double value);
	void EditKeyAlpha(int value);

	void EditPartName(const QString& value);
	void EditPartVisible(bool value);
	void EditPartBill(const QString& value);
	void EditPartAlpha(const QString& value);
	void EditPartTextureName();
	void EditPartTexture();
	void EditPartTexFrame(int value);
	void EditPartTexLoop(int value);
	void EditPartPointCount(int value);
	void EditPartParticleCreate(int value);
	void EditPartParticleCreateNum(int value);
	void EditPartParticleFrameAppear(int value);
	void EditPartParticleFrameKeep(int value);
	void EditPartParticleFrameDisappear(int value);
	void EditPartParticleStartPosVar(double value);
	void EditPartParticleStartPosVarY(double value);
	void EditPartParticleYLow(double value);
	void EditPartParticleYHigh(double value);
	void EditPartParticleXZLow(double value);
	void EditPartParticleXZHigh(double value);
	void EditPartParticleAccelX(double value);
	void EditPartParticleAccelY(double value);
	void EditPartParticleAccelZ(double value);
	void EditPartParticleScaleX(double value);
	void EditPartParticleScaleY(double value);
	void EditPartParticleScaleZ(double value);
	void EditPartParticleRotationLowX(double value);
	void EditPartParticleRotationLowY(double value);
	void EditPartParticleRotationLowZ(double value);
	void EditPartParticleRotationHighX(double value);
	void EditPartParticleRotationHighY(double value);
	void EditPartParticleRotationHighZ(double value);
	void EditPartParticleScalSpeedXLow(double value);
	void EditPartParticleScalSpeedYLow(double value);
	void EditPartParticleScalSpeedZLow(double value);
	void EditPartParticleScalSpeedXHigh(double value);
	void EditPartParticleScalSpeedYHigh(double value);
	void EditPartParticleScalSpeedZHigh(double value);
	void EditPartParticleRepeatScale(bool value);
	void EditPartParticleRepeat(bool value);
	void EditPartParticleScaleEndX(double value);
	void EditPartParticleScaleEndY(double value);
	void EditPartParticleScaleEndZ(double value);
	void EditPartParticleScaleSpeedX(double value);
	void EditPartParticleScaleSpeedY(double value);
	void EditPartParticleScaleSpeedZ(double value);

private:
	Ui::MainFrameClass ui;
	CModelViewer* m_modelViewer;
	QLabel* m_status;
	CAnimatedMesh* m_mesh;
	int m_meshSex;
	CSfxModel* m_sfx;
	string m_dragFilename;
	CTimeline* m_timeline;
	string m_filename;
	int m_currentEditPart;
	SfxKeyFrame* m_currentEditKey;
	QLabel* m_modelRot;
	QDoubleSpinBox* m_modelRotX;
	QDoubleSpinBox* m_modelRotY;
	QDoubleSpinBox* m_modelRotZ;
	QTranslator m_translator;
	int m_language;
	QActionGroup* m_languageActionGroup;

	void _saveFile(const string& filename);
	void _setKey();
	void _editKey();
	void _setPart();
	void _editPart();

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