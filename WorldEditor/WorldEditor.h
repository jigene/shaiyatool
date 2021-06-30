///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WORLDEDITOR_H
#define WORLDEDITOR_H

#include <D3DWidget.h>

#define MINIMAP_SIZE	256

class CTextureMng;
class CModelMng;
class CWorld;
class CRender2D;
class CObject;
class CEditTerrainHeightCommand;
class CEditWaterCommand;
class CEditTerrainColorCommand;
class CEditTerrainTextureCommand;
class CObjectTransformCommand;
class CObjectEditRectCommand;
class CObjectDeleteCommand;

class CWorldEditor : public CD3DWidget
{
	Q_OBJECT

public:
	CWorldEditor(QWidget* parent = null, Qt::WindowFlags flags = 0);
	~CWorldEditor();

	virtual bool InitDeviceObjects();
	virtual void DeleteDeviceObjects();
	virtual bool RestoreDeviceObjects();
	virtual void InvalidateDeviceObjects();
	virtual bool Render();

	void SetWorld(CWorld* world);
	void SaveBigmap(const string& filename);
	void SaveMinimaps();
	void MouseLost();

	bool IsPickingTerrain() const {
		return m_pickingTerrain;
	}
	D3DXVECTOR3 GetTerrainPos() const {
		return m_terrainPos;
	}

protected:
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);

private:
	CTextureMng* m_textureMng;
	CModelMng* m_modelMng;
	CWorld* m_world;
	CRender2D* m_render2D;
	bool m_pickingTerrain,
		m_rotatingCamera,
		m_movingCamera,
		m_editing,
		m_zoomingCamera;
	QPoint m_lastMousePos,
		m_editTexturePos;
	D3DXVECTOR3 m_moveVector,
		m_moveCrossVector,
		m_terrainPos,
		m_editTerrainPos;
	CObject* m_pickObject;
	QStandardItem* m_addObject;
	CPtrArray<CObject> m_addObjects;
	bool m_selectRect;
	QPoint m_selectRectPos;
	QPoint m_editBasePos;
	QPoint m_lastRightClickPos;

	void _updateTerrainPos3D(const QPoint& pt);
	void _updateMouse3D(const QPoint& pt);
	void _updateAddObjects();

	CEditTerrainHeightCommand* m_terrainEditHeightCommand;
	CEditWaterCommand* m_waterEditCommand;
	CEditTerrainColorCommand* m_terrainEditColorCommand;
	CEditTerrainTextureCommand* m_terrainEditTextureCommand;
	CObjectTransformCommand* m_objectTransformCommand;
	CObjectEditRectCommand* m_objectEditRectCommand;
	CObjectDeleteCommand* m_duplicateCommand;
};

#endif // WORLDEDITOR_H