///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "WorldEditor.h"
#include <ModelMng.h>
#include <World.h>
#include "Navigator.h"
#include <AboutDialog.h>

void CMainFrame::SetFarPlane()
{
	bool ok = false;

	HideDialogs();
	const float farPlane = (float)QInputDialog::getDouble(this, tr("Champ de vision"), tr("Longueur maximale :"), (float)g_global3D.farPlane, 2.0, 9999999.0, 3, &ok);
	ShowDialogs();

	if (ok)
	{
		g_global3D.farPlane = farPlane;

		if (m_world)
			m_world->m_visibilityLand = (int)(g_global3D.farPlane / (MAP_SIZE * MPU));

		if (!m_editor->IsAutoRefresh())
			m_editor->RenderEnvironment();
	}
}

void CMainFrame::SetFillMode(QAction* action)
{
	if (action == ui.actionSolide)
		g_global3D.fillMode = D3DFILL_SOLID;
	else if (action == ui.actionWireframe)
		g_global3D.fillMode = D3DFILL_WIREFRAME;

	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::SetStatusBarInfo(const D3DXVECTOR3& pos, int landX, int landY, int layerCount, const string& obj, const string& texture, int waterHeight)
{
	if (!m_curX)
		return;

	m_curX->setText("X = " + QString::number(pos.x, 'f', 3));
	m_curY->setText("Y = " + QString::number(pos.y, 'f', 3));
	m_curZ->setText("Z = " + QString::number(pos.z, 'f', 3));
	m_curLand->setText(string().sprintf("Land = %02d-%02d", landX, landY));
	m_curLayerCount->setText(tr("Nombre de calques = ") + QString::number(layerCount));
	m_curTexture->setText("Texture = " + texture);
	m_curObject->setText(tr("Objet = ") + obj);
	if (waterHeight != -1)
		m_waterHeight->setText(tr("Hauteur de l'eau = ") + QString::number(waterHeight));
	else
		m_waterHeight->setText(tr("Hauteur de l'eau ="));
}

void CMainFrame::UpdateNavigator()
{
	if (m_navigator)
		m_navigator->update();
}

void CMainFrame::UpdateWorldEditor()
{
	if (m_editor)
	{
		if (!m_editor->IsAutoRefresh())
			m_editor->RenderEnvironment();
	}
}

void CMainFrame::Animate(bool animate)
{
	g_global3D.animate = animate;
	m_editor->SetAutoRefresh(animate);
}

void CMainFrame::ShowTerrain(bool show)
{
	g_global3D.renderTerrain = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowEditionLight(bool light)
{
	g_global3D.editionLight = light;
	if (m_world)
		m_world->_setLight(false);
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::EnableTerrainLOD(bool LOD)
{
	g_global3D.terrainLOD = LOD;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowWater(bool show)
{
	g_global3D.renderWater = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowTerrainAttributes(bool show)
{
	g_global3D.renderTerrainAttributes = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowGridUnity(bool show)
{
	g_global3D.gridUnity = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowGridPatch(bool show)
{
	g_global3D.gridPatch = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowGridLand(bool show)
{
	g_global3D.gridLand = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowObjects(bool show)
{
	g_global3D.renderObjects = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::EnableObjectLOD(bool LOD)
{
	g_global3D.objectLOD = LOD;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::SetCameraTopView()
{
	if (m_world)
	{
		m_world->m_cameraAngle.x = 0.0f;
		m_world->m_cameraAngle.y = -89.89f;
	}
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::SetCameraSideView()
{
	if (m_world)
	{
		m_world->m_cameraAngle.x = 0.0f;
		m_world->m_cameraAngle.y = 0.0f;
	}
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::SpawnAllMonsters(bool spawn)
{
	g_global3D.spawnAllMovers = spawn;
	if (m_world)
		m_world->UpdateSpawns();
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowObj(bool show)
{
	g_global3D.renderObj = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowSFX(bool show)
{
	g_global3D.renderSFX = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowItem(bool show)
{
	g_global3D.renderItem = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowCtrl(bool show)
{
	g_global3D.renderCtrl = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowNPC(bool show)
{
	g_global3D.renderNPC = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowMonster(bool show)
{
	g_global3D.renderMonster = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowCollision(bool show)
{
	g_global3D.renderCollisions = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}


void CMainFrame::ShowRegions(bool show)
{
	g_global3D.renderRegions = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowMoverNames(bool show)
{
	g_global3D.renderMoverNames = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowSpawnZones(bool show)
{
	g_global3D.renderSpawns = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::SetGameTime()
{
	bool ok = false;
	HideDialogs();
	int hour = QInputDialog::getInt(this, tr("Heure en jeu"), tr("Définissez l'heure en jeu :"), g_global3D.hour, 0, 23, 1, &ok);
	ShowDialogs();
	if (ok)
		g_global3D.hour = hour;

	if (m_world)
		m_world->_setLight(false);
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowLight(bool light)
{
	g_global3D.light = light;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowFog(bool show)
{
	g_global3D.fog = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::ShowSkybox(bool show)
{
	g_global3D.skybox = show;
	if (!m_editor->IsAutoRefresh())
		m_editor->RenderEnvironment();
}

void CMainFrame::SwitchFullscreen(bool fullscreen)
{
	if (fullscreen)
		showFullScreen();
	else
		showNormal();
}

void CMainFrame::About()
{
	HideDialogs();
	CAboutDialog(this, m_editor).exec();
	ShowDialogs();
}

void CMainFrame::AboutQt()
{
	HideDialogs();
	QMessageBox::aboutQt(this, tr("À propos de Qt"));
	ShowDialogs();
}