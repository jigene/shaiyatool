///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "WorldEditor.h"
#include <World.h>
#include <Landscape.h>
#include <Project.h>
#include <GameElements.h>

void CMainFrame::UpdatePatrolList()
{
	ui.patrolList->clear();
	bool exists = false;

	if (m_world)
	{
		QListWidgetItem* item;
		for (auto it = m_world->m_paths.begin(); it != m_world->m_paths.end(); it++)
		{
			item = new QListWidgetItem("Path " + string::number(it.key()), ui.patrolList);
			item->setData(Qt::UserRole + 1, it.key());
			ui.patrolList->addItem(item);

			if (it.key() == m_currentPatrol)
			{
				ui.patrolList->setCurrentItem(item);
				exists = true;
			}
		}
	}

	if (!exists)
		m_currentPatrol = -1;
}

bool CMainFrame::IsEditingContinents() const
{
	return m_dialogContinentEdit != null;
}

void CMainFrame::ResetDefaultEditionColor()
{
	m_editTerrainTextureColor = QColor(127, 127, 127, 255);
	ui.editTerrainTextureColor->setStyleSheet("background-color: " + m_editTerrainTextureColor.name() + ';');
}

void CMainFrame::GetAddObjSettings(bool& rot, float& minRot, float& maxRot, bool& scale, float& minScale, float& maxScale)
{
	rot = m_addObjRandomRot;
	minRot = m_addObjRandomRotMin;
	maxRot = m_addObjRandomRotMax;
	scale = m_addObjRandomScale;
	minScale = m_addObjRandomScaleMin;
	maxScale = m_addObjRandomScaleMax;
}

int CMainFrame::GetCurrentPatrol() const
{
	if (ui.dockPatrolEditor->isHidden())
		return -1;
	return m_currentPatrol;
}

void CMainFrame::SelectGameElement(const QModelIndex& index)
{
	if (!index.isValid())
		return;

	QStandardItem* item = ((QStandardItemModel*)ui.gameElementsTree->model())->itemFromIndex(index);

	if (!item)
		return;

	if (item->type() == GAMEELE_TERRAIN)
	{
		m_editTexture = ((CTerrainElement*)item)->GetTerrain();
		ui.actionTexture_du_terrain->setChecked(true);
		m_editMode = EDIT_TERRAIN_TEXTURE;
		if (ui.tabTerrain->currentIndex() != 4)
			ui.tabTerrain->setCurrentIndex(1);
	}
	else if ((item->type() == GAMEELE_FOLDER && QApplication::queryKeyboardModifiers() & Qt::ControlModifier)
		|| item->type() == GAMEELE_MODEL)
	{
		m_editMode = EDIT_ADD_OBJECTS;
		m_addObject = item;
		ui.actionAjouter_des_objets->setChecked(true);
	}

	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

QStandardItem* CMainFrame::GetAddObject() const
{
	return m_addObject;
}

QMenu* CMainFrame::GetObjectMenu() const
{
	return m_objectMenu;
}

QMenu* CMainFrame::GetNoObjectMenu() const
{
	return m_noObjectMenu;
}

bool CMainFrame::UseGravity() const
{
	return ui.actionGravit->isChecked();
}

QAction* CMainFrame::GetObjPropertiesAction() const
{
	return m_objPropertiesAction;
}

bool CMainFrame::IsSelectionLocked() const
{
	return ui.actionVerrouiller_la_s_lection->isChecked();
}

bool CMainFrame::UseGrid() const
{
	return ui.actionPlacer_sur_la_grille->isChecked();
}

void CMainFrame::SetGridSize()
{
	HideDialogs();
	bool ok = false;
	const float gridSize = (float)QInputDialog::getDouble(this, tr("Taille de la grille"), tr("Taille :"), (float)m_gridSize, 0.001, MAP_SIZE, 1, &ok);
	ShowDialogs();
	if (ok)
		m_gridSize = gridSize;
}

float CMainFrame::GetGridSize()
{
	return m_gridSize;
}

void CMainFrame::SetEditTexture(QListWidgetItem * item)
{
	if (!item)
		return;

	m_editTexture = item->type() - QListWidgetItem::UserType - 1;
	ui.actionTexture_du_terrain->setChecked(true);
	m_editMode = EDIT_TERRAIN_TEXTURE;

	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ChangeEditAxis(QAction* action)
{
	if (action == ui.actionEditX)
		m_editAxis = EDIT_X;
	else if (action == ui.actionEditY)
		m_editAxis = EDIT_Y;
	else if (action == ui.actionEditZ)
		m_editAxis = EDIT_Z;
	else if (action == ui.actionEditXZ)
		m_editAxis = EDIT_XZ;
}

EEditAxis CMainFrame::GetEditAxis() const
{
	return m_editAxis;
}

void CMainFrame::EditTerrainChoseTextureColor()
{
	HideDialogs();
	const QColor color = QColorDialog::getColor(m_editTerrainTextureColor, this, tr("Couleur d'édition"), QColorDialog::ColorDialogOption::ShowAlphaChannel);
	ShowDialogs();
	if (color.isValid())
	{
		m_editTerrainTextureColor = color;
		ui.editTerrainTextureColor->setStyleSheet("background-color: " + m_editTerrainTextureColor.name() + ';');
	}
}

void CMainFrame::EditTerrainChangeMode(int index)
{
	switch (index)
	{
	case 0:
		ui.actionHauteur_du_terrain->setChecked(true);
		m_editMode = EDIT_TERRAIN_HEIGHT;
		break;
	case 1:
		ui.actionTexture_du_terrain->setChecked(true);
		m_editMode = EDIT_TERRAIN_TEXTURE;
		break;
	case 2:
		ui.actionCouleur_du_terrain->setChecked(true);
		m_editMode = EDIT_TERRAIN_COLOR;
		break;
	case 3:
		ui.actionEau_et_nuages_edit->setChecked(true);
		m_editMode = EDIT_TERRAIN_WATER;
		break;
	}

	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ChangeEditMode(QAction* action)
{
	if (action == ui.actionHauteur_du_terrain)
	{
		ui.tabTerrain->setCurrentIndex(0);
		m_editMode = EDIT_TERRAIN_HEIGHT;
	}
	else if (action == ui.actionTexture_du_terrain)
	{
		ui.tabTerrain->setCurrentIndex(1);
		m_editMode = EDIT_TERRAIN_TEXTURE;
	}
	else if (action == ui.actionCouleur_du_terrain)
	{
		ui.tabTerrain->setCurrentIndex(2);
		m_editMode = EDIT_TERRAIN_COLOR;
	}
	else if (action == ui.actionEau_et_nuages_edit)
	{
		ui.tabTerrain->setCurrentIndex(3);
		m_editMode = EDIT_TERRAIN_WATER;
	}
	else if (action == ui.actionVertices_du_continent)
		m_editMode = EDIT_CONTINENT;
	else if (action == ui.actionAjouter_des_objets)
		m_editMode = EDIT_ADD_OBJECTS;
	else if (action == ui.actionS_lectionner_des_objets)
		m_editMode = EDIT_SELECT_OBJECTS;
	else if (action == ui.actionD_placer_les_objets)
		m_editMode = EDIT_MOVE_OBJECTS;
	else if (action == ui.actionTourner_les_objets)
		m_editMode = EDIT_ROTATE_OBJECTS;
	else if (action == ui.actionRedimensionner_les_objets)
		m_editMode = EDIT_SCALE_OBJECTS;
	else if (action == ui.actionD_placer_cam_ra)
		m_editMode = EDIT_CAMERA_POS;
	else if (action == ui.actionTourner_cam_ra)
		m_editMode = EDIT_CAMERA_ROT;
	else if (action == ui.actionZoomer_cam_ra)
		m_editMode = EDIT_CAMERA_ZOOM;

	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

EEditMode CMainFrame::GetEditMode() const
{
	return m_editMode;
}

void CMainFrame::GetWaterEditInfos(int& mode, byte& waterHeight, byte& waterTexture, int& size)
{
	mode = WTYPE_NONE;
	waterHeight = 0;
	waterTexture = 0;
	size = ui.editWaterSize->value();

	if (ui.editTerrainCloudMode->isChecked())
		mode = WTYPE_CLOUD;
	else if (ui.editTerrainWaterMode->isChecked())
	{
		mode = WTYPE_WATER;
		waterHeight = (byte)ui.editWaterHeight->value();
		waterTexture = (byte)ui.editWaterComboBox->itemData(ui.editWaterComboBox->currentIndex()).toInt();
	}
}

void CMainFrame::SetLayerInfos(CLandscape* land)
{
	if (m_currentLand != land)
	{
		m_currentLand = land;
		ui.editTerrainLayerList->clear();
		if (m_currentLand)
		{
			QListWidgetItem* item;
			for (int i = 0; i < m_currentLand->m_layers.GetSize(); i++)
			{
				item = new QListWidgetItem(string::number(i) + " - " + Project->GetTerrainFilename(m_currentLand->m_layers[i]->textureID),
					ui.editTerrainLayerList,
					QListWidgetItem::UserType + 1 + m_currentLand->m_layers[i]->textureID);
				ui.editTerrainLayerList->addItem(item);
			}
		}
	}
}

CLandscape* CMainFrame::GetCurrentInfoLand()
{
	return m_currentLand;
}

void CMainFrame::GetTerrainHeightEditInfos(int& mode, bool& rounded, int& radius, int& hardness, bool& useFixedheight, float& fixedHeight, int& attribute)
{
	mode = GetTerrainHeightEditMode();
	rounded = ui.checkTerrainRoundedBrush->isChecked();
	radius = ui.editTerrainHeightRadius->value();
	if (mode == 1)
	{
		hardness = ui.editTerrainHeightHardness->value();
		useFixedheight = ui.editCheckTerrainFixedHeight->isChecked();
		if (useFixedheight)
			fixedHeight = (float)ui.editTerrainFixedHeight->value();
		else
			fixedHeight = 0.0f;
	}
	else
	{
		useFixedheight = false;
		fixedHeight = 0.0f;
		hardness = 0;
	}
	if (mode == 4)
		attribute = ui.editTerrainHeightAttribute->currentIndex();
	else
		attribute = 0;
}

int CMainFrame::GetTerrainHeightEditMode() const
{
	if (ui.checkTerrainBrush2->isChecked())
		return 1;
	else if (ui.checkTerrainBrush3->isChecked())
		return 2;
	else if (ui.checkTerrainBrush4->isChecked())
		return 3;
	else if (ui.checkTerrainBrush5->isChecked())
		return 4;
	else
		return 0;
}

int CMainFrame::GetTextureEditRadius() const
{
	if (m_editMode == EDIT_TERRAIN_TEXTURE)
		return ui.editTextureRadius->value();
	else if (m_editMode == EDIT_TERRAIN_COLOR)
		return ui.editTextureColorRadius->value();
	else
		return 0;
}

void CMainFrame::GetTextureColorEditInfos(int& radius, int& hardness, QColor& color)
{
	radius = GetTextureEditRadius();
	hardness = ui.editTextureColorHardness->value();
	color = m_editTerrainTextureColor;
}

int CMainFrame::GetEditTexture() const
{
	return m_editTexture;
}

void CMainFrame::GetTextureEditInfos(int& textureID, int& radius, int& hardness, int& alpha, bool& empty)
{
	empty = ui.editTextureModeEmpty->isChecked();
	radius = GetTextureEditRadius();
	if (empty)
	{
		textureID = -1;
		hardness = 0;
		alpha = 0;
	}
	else
	{
		textureID = m_editTexture;
		hardness = ui.editTextureHardness->value();
		alpha = ui.editTextureAlpha->value();
	}
}