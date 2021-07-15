///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include <Project.h>
#include <GameElements.h>
#include "WorldEditor.h"
#include <ModelMng.h>
#include <World.h>

#include "ui_Random.h"

void CMainFrame::SetAddObjectsSettings()
{
	HideDialogs();

	QDialog dialog(this);
	Ui::RandomDialog ui;
	ui.setupUi(&dialog);

	ui.rotation->setChecked(m_addObjRandomRot);
	ui.minRot->setValue((double)m_addObjRandomRotMin);
	ui.maxRot->setValue((double)m_addObjRandomRotMax);
	ui.scale->setChecked(m_addObjRandomScale);
	ui.minScale->setValue((double)m_addObjRandomScaleMin);
	ui.maxScale->setValue((double)m_addObjRandomScaleMax);

	if (dialog.exec() == QDialog::Accepted)
	{
		m_addObjRandomRot = ui.rotation->isChecked();
		m_addObjRandomRotMin = (float)ui.minRot->value();
		m_addObjRandomRotMax = (float)ui.maxRot->value();

		if (m_addObjRandomRotMin > m_addObjRandomRotMax)
		{
			const float temp = m_addObjRandomRotMin;
			m_addObjRandomRotMin = m_addObjRandomRotMax;
			m_addObjRandomRotMax = temp;
		}

		m_addObjRandomScale = ui.scale->isChecked();
		m_addObjRandomScaleMin = (float)ui.minScale->value();
		m_addObjRandomScaleMax = (float)ui.maxScale->value();

		if (m_addObjRandomScaleMin > m_addObjRandomScaleMax)
		{
			const float temp = m_addObjRandomScaleMin;
			m_addObjRandomScaleMin = m_addObjRandomScaleMax;
			m_addObjRandomScaleMax = temp;
		}
	}

	ShowDialogs();
}

void CMainFrame::ShowFavoritesMenu(const QPoint& pt)
{
	QStandardItemModel* model = (QStandardItemModel*)ui.gameElementsTree->model();
	QModelIndex index = ui.gameElementsTree->indexAt(pt);
	if (index.isValid())
	{
		QStandardItem* element = (QStandardItem*)model->itemFromIndex(index);
		if (element && (element->type() == GAMEELE_TERRAIN || element->type() == GAMEELE_MODEL))
		{
			if (element->parent() == m_favoritesFolder)
			{
				QAction* result = m_favoritesRemoveMenu->exec(ui.gameElementsTree->mapToGlobal(pt));
				if (result)
				{
					if (result == m_actionEditModel)
						_editModel(element);
					else
						m_favoritesFolder->takeRow(element->row());
				}
			}
			else
			{
				QAction* result = m_favoritesAddMenu->exec(ui.gameElementsTree->mapToGlobal(pt));
				if (result)
				{
					if (result == m_actionEditModel)
						_editModel(element);
					else
					{
						QStandardItem* newItem;
						switch (element->type())
						{
						case GAMEELE_TERRAIN:
							newItem = new CTerrainElement();
							break;
						case GAMEELE_MODEL:
							newItem = new CModelElement();
							break;
						}
						newItem->setData(element->data());
						newItem->setIcon(element->icon());
						newItem->setText(element->text());
						m_favoritesFolder->insertRow(0, newItem);
						ui.gameElementsTree->expand(m_favoritesFolder->index());
					}
				}
			}
		}
	}
}

void CMainFrame::_loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "WorldEditor");
	if (settings.status() == QSettings::NoError)
	{
		// General
		if (settings.contains("WindowGeometry"))
		{
			restoreGeometry(settings.value("WindowGeometry").toByteArray());
			if (isFullScreen())
				ui.actionPlein_cran->setChecked(true);
		}
		if (settings.contains("WindowState"))
			restoreState(settings.value("WindowState").toByteArray());
		if (settings.contains("LastOpenFiles"))
		{
			m_lastOpenFilenames = settings.value("LastOpenFiles").toStringList();
			_updateLastOpenFiles();
		}
		if (settings.contains("Favorites"))
		{
			QByteArray favoritesBuffer = settings.value("Favorites").toByteArray();
			if (favoritesBuffer.size() > 4)
			{
				QDataStream favoritesData(favoritesBuffer);
				QStandardItem* item;
				int favoritesCount;
				favoritesData >> favoritesCount;
				for (int i = 0; i < favoritesCount; i++)
				{
					item = m_prj->CreateFavorite(favoritesData);
					if (item)
						m_favoritesFolder->insertRow(0, item);
				}
				ui.gameElementsTree->expand(m_favoritesFolder->index());
			}
		}
		if (settings.contains("Language"))
		{
			switch (settings.value("Language").toInt())
			{
			case LANG_ENG:
				ui.actionEnglish->setChecked(true);
				SetLanguage(ui.actionEnglish);
				break;
			case LANG_GER:
				ui.actionDeutsch->setChecked(true);
				SetLanguage(ui.actionDeutsch);
				break;
			}
		}

		// Edition
		if (settings.contains("EditMode"))
		{
			m_editMode = (EEditMode)settings.value("EditMode").toInt();
			switch (m_editMode)
			{
			case EDIT_TERRAIN_HEIGHT:
				ui.tabTerrain->setCurrentIndex(0);
				break;
			case EDIT_TERRAIN_TEXTURE:
				ui.tabTerrain->setCurrentIndex(1);
				break;
			case EDIT_TERRAIN_COLOR:
				ui.tabTerrain->setCurrentIndex(2);
				break;
			case EDIT_TERRAIN_WATER:
				ui.tabTerrain->setCurrentIndex(3);
				break;
			case EDIT_CONTINENT:
				ui.actionVertices_du_continent->setChecked(true);
				break;
			case EDIT_ADD_OBJECTS:
				ui.actionAjouter_des_objets->setChecked(true);
				break;
			case EDIT_SELECT_OBJECTS:
				ui.actionS_lectionner_des_objets->setChecked(true);
				break;
			case EDIT_MOVE_OBJECTS:
				ui.actionD_placer_les_objets->setChecked(true);
				break;
			case EDIT_ROTATE_OBJECTS:
				ui.actionTourner_les_objets->setChecked(true);
				break;
			case EDIT_SCALE_OBJECTS:
				ui.actionRedimensionner_les_objets->setChecked(true);
				break;
			case EDIT_CAMERA_POS:
				ui.actionD_placer_cam_ra->setChecked(true);
				break;
			case EDIT_CAMERA_ROT:
				ui.actionTourner_cam_ra->setChecked(true);
				break;
			case EDIT_CAMERA_ZOOM:
				ui.actionZoomer_cam_ra->setChecked(true);
				break;
			}
		}
		if (settings.contains("EditAxis"))
		{
			m_editAxis = (EEditAxis)settings.value("EditAxis").toInt();
			switch (m_editAxis)
			{
			case EDIT_X:
				ui.actionEditX->setChecked(true);
				break;
			case EDIT_Y:
				ui.actionEditY->setChecked(true);
				break;
			case EDIT_Z:
				ui.actionEditZ->setChecked(true);
				break;
			case EDIT_XZ:
				ui.actionEditXZ->setChecked(true);
				break;
			}
		}
		if (settings.contains("PlaceOnGrid"))
			ui.actionPlacer_sur_la_grille->setChecked(settings.value("PlaceOnGrid").toBool());
		if (settings.contains("GridSize"))
			m_gridSize = settings.value("GridSize").toFloat();
		if (settings.contains("Gravity"))
			ui.actionGravit->setChecked(settings.value("Gravity").toBool());
		if (settings.contains("MsgBox"))
			ui.actionBo_te_de_dialogue_pour_les_erreurs->setChecked(settings.value("MsgBox").toBool());

		if (settings.contains("AddObjRandomRot"))
			m_addObjRandomRot = settings.value("AddObjRandomRot").toBool();
		if (settings.contains("AddObjRandomRotMin"))
			m_addObjRandomRotMin = settings.value("AddObjRandomRotMin").toFloat();
		if (settings.contains("AddObjRandomRotMax"))
			m_addObjRandomRotMax = settings.value("AddObjRandomRotMax").toFloat();
		if (settings.contains("AddObjRandomScale"))
			m_addObjRandomScale = settings.value("AddObjRandomScale").toBool();
		if (settings.contains("AddObjRandomScaleMin"))
			m_addObjRandomScaleMin = settings.value("AddObjRandomScaleMin").toFloat();
		if (settings.contains("AddObjRandomScaleMax"))
			m_addObjRandomScaleMax = settings.value("AddObjRandomScaleMax").toFloat();

		if (settings.contains("TerrainHeightMode"))
		{
			const int mode = settings.value("TerrainHeightMode").toInt();
			switch (mode)
			{
			case 0:
				ui.checkTerrainBrush1->setChecked(true);
				break;
			case 1:
				ui.checkTerrainBrush2->setChecked(true);
				break;
			case 2:
				ui.checkTerrainBrush3->setChecked(true);
				break;
			case 3:
				ui.checkTerrainBrush4->setChecked(true);
				break;
			case 4:
				ui.checkTerrainBrush5->setChecked(true);
				break;
			}
		}
		if (settings.contains("TerrainHeightRoundedBrush"))
		{
			const bool rounded = settings.value("TerrainHeightRoundedBrush").toBool();
			if (rounded)
				ui.checkTerrainRoundedBrush->setChecked(true);
			else
				ui.checkTerrainSquaredBrush->setChecked(true);
		}
		if (settings.contains("TerrainHeightRadius"))
			ui.editTerrainHeightRadius->setValue(settings.value("TerrainHeightRadius").toInt());
		if (settings.contains("TerrainHeightHardness"))
			ui.editTerrainHeightHardness->setValue(settings.value("TerrainHeightHardness").toInt());
		if (settings.contains("TerrainHeightIsFixed"))
			ui.editCheckTerrainFixedHeight->setChecked(settings.value("TerrainHeightIsFixed").toBool());
		if (settings.contains("TerrainHeightFixedHeight"))
			ui.editTerrainFixedHeight->setValue(settings.value("TerrainHeightFixedHeight").toDouble());
		if (settings.contains("TerrainHeightAttribute"))
			ui.editTerrainHeightAttribute->setCurrentIndex(settings.value("TerrainHeightAttribute").toInt());

		if (settings.contains("TerrainTextureIsEmpty"))
		{
			const bool empty = settings.value("TerrainTextureIsEmpty").toBool();
			if (empty)
				ui.editTextureModeEmpty->setChecked(true);
			else
				ui.editTextureModeNormal->setChecked(true);
		}
		if (settings.contains("TerrainTextureRadius"))
			ui.editTextureRadius->setValue(settings.value("TerrainTextureRadius").toInt());
		if (settings.contains("TerrainTextureHardness"))
			ui.editTextureHardness->setValue(settings.value("TerrainTextureHardness").toInt());
		if (settings.contains("TerrainTextureAlpha"))
			ui.editTextureAlpha->setValue(settings.value("TerrainTextureAlpha").toInt());

		if (settings.contains("TerrainColor"))
		{
			m_editTerrainTextureColor = settings.value("TerrainColor").value<QColor>();
			ui.editTerrainTextureColor->setStyleSheet("background-color: " + m_editTerrainTextureColor.name() + ';');
		}
		if (settings.contains("TerrainColorRadius"))
			ui.editTextureColorRadius->setValue(settings.value("TerrainColorRadius").toInt());
		if (settings.contains("TerrainColorHardness"))
			ui.editTextureColorHardness->setValue(settings.value("TerrainColorHardness").toInt());

		if (settings.contains("TerrainWaterMode"))
		{
			const int mode = settings.value("TerrainWaterMode").toInt();
			switch (mode)
			{
			case WTYPE_CLOUD:
				ui.editTerrainCloudMode->setChecked(true);
				break;
			case WTYPE_WATER:
				ui.editTerrainWaterMode->setChecked(true);
				break;
			case WTYPE_NONE:
				ui.editTerrainRemoveWater->setChecked(true);
				break;
			}
		}
		if (settings.contains("TerrainWaterID"))
			ui.editWaterComboBox->setCurrentIndex(settings.value("TerrainWaterID").toInt());
		if (settings.contains("TerrainWaterHeight"))
			ui.editWaterHeight->setValue(settings.value("TerrainWaterHeight").toInt());
		if (settings.contains("TerrainWaterSize"))
			ui.editWaterSize->setValue(settings.value("TerrainWaterSize").toInt());

		// View
		if (settings.contains("GridUnity"))
		{
			g_global3D.gridUnity = settings.value("GridUnity").toBool();
			ui.actionGrille_des_unit_s->setChecked(g_global3D.gridUnity);
		}
		if (settings.contains("GridPatch"))
		{
			g_global3D.gridPatch = settings.value("GridPatch").toBool();
			ui.actionGrille_des_patchs->setChecked(g_global3D.gridPatch);
		}
		if (settings.contains("GridLand"))
		{
			g_global3D.gridLand = settings.value("GridLand").toBool();
			ui.actionGrille_des_landscapes->setChecked(g_global3D.gridLand);
		}

		if (settings.contains("RenderTerrain"))
		{
			g_global3D.renderTerrain = settings.value("RenderTerrain").toBool();
			ui.actionTerrain->setChecked(g_global3D.renderTerrain);
		}
		if (settings.contains("TerrainLOD"))
		{
			g_global3D.terrainLOD = settings.value("TerrainLOD").toBool();
			ui.actionTerrain_LOD->setChecked(g_global3D.terrainLOD);
		}
		if (settings.contains("RenderTerrainAttributes"))
		{
			g_global3D.renderTerrainAttributes = settings.value("RenderTerrainAttributes").toBool();
			ui.actionAttributs_du_terrain->setChecked(g_global3D.renderTerrainAttributes);
		}
		if (settings.contains("RenderWater"))
		{
			g_global3D.renderWater = settings.value("RenderWater").toBool();
			ui.actionEau_et_nuages->setChecked(g_global3D.renderWater);
		}

		if (settings.contains("RenderObjects"))
		{
			g_global3D.renderObjects = settings.value("RenderObjects").toBool();
			ui.actionObjets->setChecked(g_global3D.renderObjects);
		}
		if (settings.contains("ObjectLOD"))
		{
			g_global3D.objectLOD = settings.value("ObjectLOD").toBool();
			ui.actionObjets_LOD->setChecked(g_global3D.objectLOD);
		}
		if (settings.contains("RenderTypeObj"))
		{
			g_global3D.renderObj = settings.value("RenderTypeObj").toBool();
			ui.actionType_Objet->setChecked(g_global3D.renderObj);
		}
		if (settings.contains("RenderTypeSfx"))
		{
			g_global3D.renderSFX = settings.value("RenderTypeSfx").toBool();
			ui.actionType_SFX->setChecked(g_global3D.renderSFX);
		}
		if (settings.contains("RenderTypeItem"))
		{
			g_global3D.renderItem = settings.value("RenderTypeItem").toBool();
			ui.actionType_item->setChecked(g_global3D.renderItem);
		}
		if (settings.contains("RenderTypeCtrl"))
		{
			g_global3D.renderCtrl = settings.value("RenderTypeCtrl").toBool();
			ui.actionType_control->setChecked(g_global3D.renderCtrl);
		}
		if (settings.contains("RenderTypeMonster"))
		{
			g_global3D.renderMonster = settings.value("RenderTypeMonster").toBool();
			ui.actionType_mover_monstres->setChecked(g_global3D.renderMonster);
		}
		if (settings.contains("RenderTypeNPC"))
		{
			g_global3D.renderNPC = settings.value("RenderTypeNPC").toBool();
			ui.actionType_PNJ->setChecked(g_global3D.renderNPC);
		}
		if (settings.contains("RenderTypeRegion"))
		{
			g_global3D.renderRegions = settings.value("RenderTypeRegion").toBool();
			ui.actionType_Region->setChecked(g_global3D.renderRegions);
		}

		if (settings.contains("RenderSpawns"))
		{
			g_global3D.renderSpawns = settings.value("RenderSpawns").toBool();
			ui.actionZones_de_spawn->setChecked(g_global3D.renderSpawns);
		}
		if (settings.contains("RenderMoverNames"))
		{
			g_global3D.renderMoverNames = settings.value("RenderMoverNames").toBool();
			ui.actionNoms_des_movers->setChecked(g_global3D.renderMoverNames);
		}
		if (settings.contains("SpawnAllMovers"))
		{
			g_global3D.spawnAllMovers = settings.value("SpawnAllMovers").toBool();
			ui.actionSpawn_complet->setChecked(g_global3D.spawnAllMovers);
		}
		if (settings.contains("RenderCollisions"))
		{
			g_global3D.renderCollisions = settings.value("RenderCollisions").toBool();
			ui.actionCollisions->setChecked(g_global3D.renderCollisions);
		}

		if (settings.contains("GameTime"))
			g_global3D.hour = settings.value("GameTime").toInt();
		if (settings.contains("Lighting"))
		{
			g_global3D.light = settings.value("Lighting").toBool();
			ui.actionLumi_re->setChecked(g_global3D.light);
		}
		if (settings.contains("Skybox"))
		{
			g_global3D.skybox = settings.value("Skybox").toBool();
			ui.actionCiel->setChecked(g_global3D.skybox);
		}
		if (settings.contains("Fog"))
		{
			g_global3D.fog = settings.value("Fog").toBool();
			ui.actionBrouillard->setChecked(g_global3D.fog);
		}
		if (settings.contains("EditionLight"))
		{
			g_global3D.editionLight = settings.value("EditionLight").toBool();
			ui.actionLumi_re_d_dition->setChecked(g_global3D.editionLight);
		}
		if (settings.contains("FillMode"))
		{
			g_global3D.fillMode = (D3DFILLMODE)settings.value("FillMode").toUInt();
			switch (g_global3D.fillMode)
			{
			case D3DFILL_SOLID:
				ui.actionSolide->setChecked(true);
				break;
			case D3DFILL_WIREFRAME:
				ui.actionWireframe->setChecked(true);
				break;
			}
		}
		if (settings.contains("FarPlane"))
			g_global3D.farPlane = settings.value("FarPlane").toFloat();

		if (settings.contains("Animate"))
		{
			g_global3D.animate = settings.value("Animate").toBool();
			ui.actionAnimer->setChecked(g_global3D.animate);
			if (m_editor)
				m_editor->SetAutoRefresh(g_global3D.animate);
		}

		if (m_editor && !m_editor->IsAutoRefresh())
			m_editor->RenderEnvironment();
	}
}

void CMainFrame::closeEvent(QCloseEvent* event)
{
	if (m_world && !m_undoStack->isClean())
	{
		HideDialogs();
		QMessageBox::Button result = QMessageBox::question(this, tr("Quitter"), tr("Êtes vous sûr de vouloir quitter ?"));
		if (result != QMessageBox::Yes)
		{
			event->ignore();
			ShowDialogs();
			return;
		}
		ShowDialogs();
		m_undoStack->clear();
	}

	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "WorldEditor");
	if (settings.isWritable())
	{
		// General
		settings.setValue("WindowGeometry", saveGeometry());
		settings.setValue("WindowState", saveState());
		settings.setValue("LastOpenFiles", m_lastOpenFilenames);
		const int favoritesCount = m_favoritesFolder->rowCount();
		QByteArray favoritesBuffer;
		QDataStream favoritesData(&favoritesBuffer, QIODevice::WriteOnly);
		QStandardItem* child;
		favoritesData << favoritesCount;
		for (int i = favoritesCount - 1; i >= 0; i--)
		{
			child = m_favoritesFolder->child(i);
			favoritesData << child->type();
			if (child->type() == GAMEELE_TERRAIN)
				favoritesData << ((CTerrainElement*)child)->GetTerrain();
			else if (child->type() == GAMEELE_MODEL)
			{
				favoritesData << ((CModelElement*)child)->GetModel()->type;
				favoritesData << ((CModelElement*)child)->GetModel()->ID;
			}
		}
		settings.setValue("Favorites", favoritesBuffer);
		settings.setValue("Language", m_language);

		// Edition
		settings.setValue("EditMode", (int)m_editMode);
		settings.setValue("EditAxis", (int)m_editAxis);
		settings.setValue("PlaceOnGrid", ui.actionPlacer_sur_la_grille->isChecked());
		settings.setValue("GridSize", m_gridSize);
		settings.setValue("Gravity", ui.actionGravit->isChecked());
		settings.setValue("MsgBox", ui.actionBo_te_de_dialogue_pour_les_erreurs->isChecked());

		settings.setValue("AddObjRandomRot", m_addObjRandomRot);
		settings.setValue("AddObjRandomRotMin", m_addObjRandomRotMin);
		settings.setValue("AddObjRandomRotMax", m_addObjRandomRotMax);
		settings.setValue("AddObjRandomScale", m_addObjRandomScale);
		settings.setValue("AddObjRandomScaleMin", m_addObjRandomScaleMin);
		settings.setValue("AddObjRandomScaleMax", m_addObjRandomScaleMax);

		settings.setValue("TerrainHeightMode", GetTerrainHeightEditMode());
		settings.setValue("TerrainHeightRoundedBrush", ui.checkTerrainRoundedBrush->isChecked());
		settings.setValue("TerrainHeightRadius", ui.editTerrainHeightRadius->value());
		settings.setValue("TerrainHeightHardness", ui.editTerrainHeightHardness->value());
		settings.setValue("TerrainHeightIsFixed", ui.editCheckTerrainFixedHeight->isChecked());
		settings.setValue("TerrainHeightFixedHeight", ui.editTerrainFixedHeight->value());
		settings.setValue("TerrainHeightAttribute", ui.editTerrainHeightAttribute->currentIndex());

		settings.setValue("TerrainTextureIsEmpty", ui.editTextureModeEmpty->isChecked());
		settings.setValue("TerrainTextureRadius", ui.editTextureRadius->value());
		settings.setValue("TerrainTextureHardness", ui.editTextureHardness->value());
		settings.setValue("TerrainTextureAlpha", ui.editTextureAlpha->value());

		settings.setValue("TerrainColor", m_editTerrainTextureColor);
		settings.setValue("TerrainColorRadius", ui.editTextureColorRadius->value());
		settings.setValue("TerrainColorHardness", ui.editTextureColorHardness->value());

		if (ui.editTerrainCloudMode->isChecked())
			settings.setValue("TerrainWaterMode", WTYPE_CLOUD);
		else if (ui.editTerrainWaterMode->isChecked())
			settings.setValue("TerrainWaterMode", WTYPE_WATER);
		else
			settings.setValue("TerrainWaterMode", WTYPE_NONE);
		settings.setValue("TerrainWaterID", ui.editWaterComboBox->currentIndex());
		settings.setValue("TerrainWaterHeight", ui.editWaterHeight->value());
		settings.setValue("TerrainWaterSize", ui.editWaterSize->value());

		// View
		settings.setValue("GridUnity", g_global3D.gridUnity);
		settings.setValue("GridPatch", g_global3D.gridPatch);
		settings.setValue("GridLand", g_global3D.gridLand);

		settings.setValue("RenderTerrain", g_global3D.renderTerrain);
		settings.setValue("TerrainLOD", g_global3D.terrainLOD);
		settings.setValue("RenderTerrainAttributes", g_global3D.renderTerrainAttributes);
		settings.setValue("RenderWater", g_global3D.renderWater);

		settings.setValue("RenderObjects", g_global3D.renderObjects);
		settings.setValue("ObjectLOD", g_global3D.objectLOD);
		settings.setValue("RenderTypeObj", g_global3D.renderObj);
		settings.setValue("RenderTypeSfx", g_global3D.renderSFX);
		settings.setValue("RenderTypeItem", g_global3D.renderItem);
		settings.setValue("RenderTypeCtrl", g_global3D.renderCtrl);
		settings.setValue("RenderTypeMonster", g_global3D.renderMonster);
		settings.setValue("RenderTypeNPC", g_global3D.renderNPC);
		settings.setValue("RenderTypeRegion", g_global3D.renderRegions);

		settings.setValue("RenderSpawns", g_global3D.renderSpawns);
		settings.setValue("RenderMoverNames", g_global3D.renderMoverNames);
		settings.setValue("SpawnAllMovers", g_global3D.spawnAllMovers);
		settings.setValue("RenderCollisions", g_global3D.renderCollisions);

		settings.setValue("GameTime", g_global3D.hour);
		settings.setValue("Lighting", g_global3D.light);
		settings.setValue("Skybox", g_global3D.skybox);
		settings.setValue("Fog", g_global3D.fog);
		settings.setValue("EditionLight", g_global3D.editionLight);
		settings.setValue("FillMode", (uint)g_global3D.fillMode);
		settings.setValue("FarPlane", g_global3D.farPlane);

		settings.setValue("Animate", g_global3D.animate);
	}

	QMainWindow::closeEvent(event);
}

void CMainFrame::SetLanguage(QAction* action)
{
	if (action == ui.actionFran_ais)
	{
		m_language = LANG_FRE;
		m_translator.load("", "");
	}
	else if (action == ui.actionEnglish)
	{
		m_language = LANG_ENG;
		m_translator.load("worldeditor_en.qm", "Plugins/languages/English");
	}
	else if (action == ui.actionDeutsch)
	{
		m_language = LANG_GER;
		m_translator.load("worldeditor_de.qm", "Plugins/languages/Deutsch");
	}

	qApp->installTranslator(&m_translator);
	ui.retranslateUi(this);

	if (m_filename.isEmpty() || !m_world)
		setWindowTitle(tr("WorldEditor"));
	else
		setWindowTitle(tr("WorldEditor - ") + QFileInfo(m_filename).fileName());

	m_objPropertiesAction->setText(tr("Propriétés"));
	m_favoritesAddMenu->actions().at(0)->setText(tr("Ajouter aux favoris"));
	m_favoritesRemoveMenu->actions().at(0)->setText(tr("Retirer des favoris"));
	QStandardItem* root = ((QStandardItemModel*)ui.gameElementsTree->model())->invisibleRootItem();
	root->child(0)->setText(QObject::tr("Favoris"));
	root->child(1)->setText(QObject::tr("Objet"));
	root->child(2)->setText(QObject::tr("Terrain"));
	m_actionEditModel->setText(QObject::tr("Éditer"));
	SetStatusBarInfo(D3DXVECTOR3(0, 0, 0), 0, 0, 0, "", "");
}

bool CMainFrame::MsgBoxForErrors()
{
	return true;
	//return ui.actionBo_te_de_dialogue_pour_les_erreurs->isChecked();
}