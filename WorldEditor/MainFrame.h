///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "ui_MainFrame.h"

class CProject;
class CWorldEditor;
class CWorld;
class CNavigator;
class CRootFolderElement;
class CLandscape;
class CDialogWorldProperties;
class CDialogContinentEdit;
class CObject;
class CEditCommand;

enum EEditMode
{
	EDIT_CAMERA_POS,
	EDIT_CAMERA_ROT,
	EDIT_CAMERA_ZOOM,
	EDIT_TERRAIN_HEIGHT,
	EDIT_TERRAIN_TEXTURE,
	EDIT_TERRAIN_COLOR,
	EDIT_TERRAIN_WATER,
	EDIT_CONTINENT,
	EDIT_ADD_OBJECTS,
	EDIT_SELECT_OBJECTS,
	EDIT_MOVE_OBJECTS,
	EDIT_ROTATE_OBJECTS,
	EDIT_SCALE_OBJECTS
};

enum EEditAxis
{
	EDIT_X,
	EDIT_Y,
	EDIT_Z,
	EDIT_XZ
};

class CMainFrame : public QMainWindow
{
	Q_OBJECT

public:
	static CMainFrame* Instance;

	CMainFrame(QWidget *parent = 0);
	~CMainFrame();

	bool Initialize();

	void OpenFile(const string& filename);

	void UpdateNavigator();
	void UpdateWorldEditor();
	void UpdateContinentVertices();
	void UpdatePatrolList();

	void SetStatusBarInfo(const D3DXVECTOR3& pos, int landX, int landY, int layerCount, const string& obj, const string& texture, int waterHeight = -1);
	void SetLayerInfos(CLandscape* land);
	void RotateObjects(int key);

	void AddCommand(CEditCommand* command);

	void HideDialogs();
	void ShowDialogs();

	EEditMode GetEditMode() const;
	void GetWaterEditInfos(int& mode, byte& waterHeight, byte& waterTexture, int& size);
	void GetTerrainHeightEditInfos(int& mode, bool& rounded, int& radius, int& hardness, bool& useFixedheight, float& fixedHeight, int& attribute);
	int GetTerrainHeightEditMode() const;
	CLandscape* GetCurrentInfoLand();
	int GetTextureEditRadius() const;
	void GetTextureColorEditInfos(int& radius, int& hardness, QColor& color);
	int GetEditTexture() const;
	void GetTextureEditInfos(int& textureID, int& radius, int& hardness, int& alpha, bool& empty);
	QStandardItem* GetAddObject() const;
	bool IsSelectionLocked() const;
	EEditAxis GetEditAxis() const;
	bool UseGrid() const;
	float GetGridSize();
	QMenu* GetObjectMenu() const;
	QMenu* GetNoObjectMenu() const;
	QAction* GetObjPropertiesAction() const;
	bool UseGravity() const;
	int GetCurrentPatrol() const;
	void GetAddObjSettings(bool& rot, float& minRot, float& maxRot, bool& scale, float& minScale, float& maxScale);
	bool IsEditingContinents() const;

public slots:
	void NewFile();
	void OpenFile();
	void CloseFile();
	void Animate(bool animate);
	void ShowTerrain(bool show);
	void ShowGridUnity(bool show);
	void ShowGridPatch(bool show);
	void ShowGridLand(bool show);
	void EnableTerrainLOD(bool LOD);
	void ShowWater(bool show);
	void ShowTerrainAttributes(bool show);
	void SwitchFullscreen(bool fullscreen);
	void SetGameTime();
	void ShowLight(bool light);
	void ShowFog(bool show);
	void ShowSkybox(bool show);
	void ShowObjects(bool show);
	void EnableObjectLOD(bool LOD);
	void ShowMoverNames(bool show);
	void ShowSpawnZones(bool show);
	void OpenLastFile(QAction* action);
	void SetCameraTopView();
	void SetCameraSideView();
	void SpawnAllMonsters(bool spawn);
	void About();
	void AboutQt();
	void SaveBigmap();
	void SaveMinimaps();
	void ShowFavoritesMenu(const QPoint& pt);
	void ShowObj(bool show);
	void ShowSFX(bool show);
	void ShowRegions(bool show);
	void ShowItem(bool show);
	void ShowCtrl(bool show);
	void ShowNPC(bool show);
	void ShowMonster(bool show);
	void ShowCollision(bool show);
	void SaveFile();
	void SaveFileAs();
	void EditTerrainChoseTextureColor();
	void EditTerrainChangeMode(int index);
	void ChangeEditMode(QAction* action);
	void DeleteLandLayer();
	void SelectGameElement(const QModelIndex& index);
	void SetEditTexture(QListWidgetItem * item);
	void EditWorldProperties();
	void OptimizeWater();
	void DialogWorldPropertiesClosed(int result);
	void EditContinents();
	void DialogEditContinentsClosed(int result);
	void ChangeEditAxis(QAction* action);
	void SetOnLand();
	void SetGridSize();
	void RandomScaleAndSize();
	void DeleteObjects();
	void CopyObjects();
	void CutObjects();
	void PasteObjects();
	void HideObjects();
	void ShowAllObjects();
	void HideUpstairObjects();
	void SetLanguage(QAction* action);
	void FillAllMapWithWater();
	bool MsgBoxForErrors();
	void SetCurrentPath(int row);
	void AddPath();
	void RemovePath();
	void GenerateList();
	void TranslateObjects();
	void RotateObjects();
	void ShowEditionLight(bool light);
	void ScaleObjects();
	void SetAddObjectsSettings();
	void ResetDefaultEditionColor();
	void DeleteAllObjects();
	void SetFillMode(QAction* action);
	void SetFarPlane();
	void SetGravityEnabled(bool gravity);
	void SetOnGridEnabled(bool grid);

private:
	Ui::MainFrameClass ui;
	CProject* m_prj;
	CWorldEditor* m_editor;
	CWorld* m_world;
	string m_filename;
	string m_dragFilename;
	CNavigator* m_navigator;
	QStringList m_lastOpenFilenames;
	QLabel *m_curX,
		*m_curY,
		*m_curZ,
		*m_curLand,
		*m_curLayerCount,
		*m_curObject,
		*m_curTexture,
		*m_waterHeight;
	QMap<QAction*, string> m_lastOpenFilesActions;
	QMenu* m_favoritesAddMenu;
	QMenu* m_favoritesRemoveMenu;
	CRootFolderElement* m_favoritesFolder;
	QColor m_editTerrainTextureColor;
	QActionGroup* m_actionEditMode;
	EEditMode m_editMode;
	CLandscape* m_currentLand;
	int m_editTexture;
	CDialogWorldProperties* m_dialogWorldProperties;
	CDialogContinentEdit* m_dialogContinentEdit;
	QStandardItem* m_addObject;
	QActionGroup* m_actionEditAxis;
	EEditAxis m_editAxis;
	float m_gridSize;
	CPtrArray<CObject> m_clipboardObjects;
	QMenu* m_objectMenu;
	QMenu* m_noObjectMenu;
	QAction* m_objPropertiesAction;
	int m_currentPatrol;
	QUndoStack* m_undoStack;
	QAction* m_actionEditModel;
	bool m_addObjRandomScale,
		m_addObjRandomRot;
	float m_addObjRandomScaleMin,
		m_addObjRandomRotMin,
		m_addObjRandomScaleMax,
		m_addObjRandomRotMax;
	QTranslator m_translator;
	int m_language;
	QActionGroup* m_languageActionGroup;
	QActionGroup* m_fillModeActionGroup;

	void _connectWidgets();
	void _loadSettings();
	void _updateLastOpenFiles();
	void _setShortcuts();

	void _writeObjectsList(QTextStream& out, int objType, const string& filename, const string& name, const string& defBegin);

	void _editModel(QStandardItem* element);

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dragLeaveEvent(QDragLeaveEvent* event);
	virtual void dragMoveEvent(QDragMoveEvent* event);
	virtual void dropEvent(QDropEvent* event);
	virtual void changeEvent(QEvent* event);
	virtual void closeEvent(QCloseEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
};

#define MainFrame	CMainFrame::Instance

#endif // MAINFRAME_H
