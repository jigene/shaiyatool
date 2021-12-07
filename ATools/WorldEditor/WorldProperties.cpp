///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "WorldProperties.h"
#include <World.h>
#include <MainFrame.h>
#include <TextFile.h>
#include <Skybox.h>
#include <Project.h>
#include <TextureMng.h>

CDialogWorldProperties::CDialogWorldProperties(CWorld* world, QWidget *parent)
	: QDialog(parent, Qt::WindowStaysOnTopHint)
{
	ui.setupUi(this);
	m_world = world;

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_canFly = world->m_canFly;
	ui.checkCanFly->setChecked(m_canFly);
	connect(ui.checkCanFly, SIGNAL(clicked(bool)), this, SLOT(SetCanFly(bool)));

	m_bgmID = world->m_bgmID;
	CTextFile::FillComboBox(ui.comboBGM, "BGM_", world->m_bgmID);
	connect(ui.comboBGM, SIGNAL(currentIndexChanged(int)), this, SLOT(SetBGM(int)));

	m_modePK = m_world->m_modePK;
	switch (m_world->m_modePK)
	{
	case 0:
		ui.comboModePK->setCurrentIndex(0);
		break;
	case RA_PENALTY_PK:
		ui.comboModePK->setCurrentIndex(1);
		break;
	case RA_PK:
		ui.comboModePK->setCurrentIndex(2);
		break;
	}
	connect(ui.comboModePK, SIGNAL(currentIndexChanged(int)), this, SLOT(SetModePK(int)));

	m_worldRevivalID = m_world->m_worldRevivalID;
	CTextFile::FillComboBox(ui.comboWorldRevivalID, "WI_", world->m_worldRevivalID);
	connect(ui.comboWorldRevivalID, SIGNAL(currentIndexChanged(int)), this, SLOT(SetWorldRevivalID(int)));

	m_revivalPlace = m_world->m_revivalPlace;
	ui.editRevivalPlace->setText(m_world->m_revivalPlace);
	connect(ui.editRevivalPlace, SIGNAL(editingFinished()), this, SLOT(SetRevivalPlace()));

	m_fogStart = m_world->m_fogStart;
	ui.fogStart->setValue(m_world->m_fogStart);
	connect(ui.fogStart, SIGNAL(valueChanged(double)), this, SLOT(SetFogStart(double)));

	m_fogEnd = m_world->m_fogEnd;
	ui.fogEnd->setValue(m_world->m_fogEnd);
	connect(ui.fogEnd, SIGNAL(valueChanged(double)), this, SLOT(SetFogEnd(double)));

	m_fogDensity = m_world->m_fogDensity;
	ui.fogDensity->setValue(m_world->m_fogDensity);
	connect(ui.fogDensity, SIGNAL(valueChanged(double)), this, SLOT(SetFogDensity(double)));

	for (int i = 0; i < 3; i++)
		m_skyTextureNames[i] = m_world->m_skyTextureNames[i];
	ui.skyTexture1->setText(m_world->m_skyTextureNames[0]);
	connect(ui.skyTexture1, SIGNAL(editingFinished()), this, SLOT(SetSkyTexture1()));
	ui.skyTexture2->setText(m_world->m_skyTextureNames[1]);
	connect(ui.skyTexture2, SIGNAL(editingFinished()), this, SLOT(SetSkyTexture2()));
	ui.skyTexture3->setText(m_world->m_skyTextureNames[2]);
	connect(ui.skyTexture3, SIGNAL(editingFinished()), this, SLOT(SetSkyTexture3()));

	m_moonTextureName = m_world->m_moonTextureName;
	m_sunTextureName = m_world->m_sunTextureName;
	ui.sunTexture->setText(m_world->m_sunTextureName);
	connect(ui.sunTexture, SIGNAL(editingFinished()), this, SLOT(SetSunTexture()));
	ui.moonTexture->setText(m_world->m_moonTextureName);
	connect(ui.moonTexture, SIGNAL(editingFinished()), this, SLOT(SetMoonTexture()));

	for (int i = 0; i < 3; i++)
		m_cloudTextureNames[i] = m_world->m_cloudTextureNames[i];
	ui.cloudTexture1->setText(m_world->m_cloudTextureNames[0]);
	connect(ui.cloudTexture1, SIGNAL(editingFinished()), this, SLOT(SetCloudTexture1()));
	ui.cloudTexture2->setText(m_world->m_cloudTextureNames[1]);
	connect(ui.cloudTexture2, SIGNAL(editingFinished()), this, SLOT(SetCloudTexture2()));
	ui.cloudTexture3->setText(m_world->m_cloudTextureNames[2]);
	connect(ui.cloudTexture3, SIGNAL(editingFinished()), this, SLOT(SetCloudTexture3()));

	m_seaCloudTextureName = m_world->m_seaCloudTextureName;
	ui.seacloudTexture->setText(m_world->m_seaCloudTextureName);
	connect(ui.seacloudTexture, SIGNAL(editingFinished()), this, SLOT(SetSeacloudTexture()));

	m_inDoor = m_world->m_inDoor;
	ui.isInDoor->setChecked(m_world->m_inDoor);
	if (m_world->m_inDoor)
	{
		ui.groupBox_5->setEnabled(false);
		ui.cloudTexture1->setEnabled(false);
		ui.cloudTexture2->setEnabled(false);
		ui.cloudTexture3->setEnabled(false);
	}
	connect(ui.isInDoor, SIGNAL(clicked(bool)), this, SLOT(SetInDoor(bool)));

	m_ambient = m_world->m_ambient;
	ui.colorAmbient->setStyleSheet("background-color: " + QColor((m_world->m_ambient >> 16) & 0xff, (m_world->m_ambient >> 8) & 0xff, m_world->m_ambient & 0xff).name() + ';');
	connect(ui.editColorAmbient, SIGNAL(clicked()), this, SLOT(SetAmbient()));

	m_diffuse = m_world->m_diffuse;
	ui.colorDiffuse->setStyleSheet("background-color: " + QColor((m_world->m_diffuse >> 16) & 0xff, (m_world->m_diffuse >> 8) & 0xff, m_world->m_diffuse & 0xff).name() + ';');
	connect(ui.editColorDiffuse, SIGNAL(clicked()), this, SLOT(SetDiffuse()));

	m_lightDir = m_world->m_lightDir;
	ui.lightDirX->setValue(m_lightDir.x);
	connect(ui.lightDirX, SIGNAL(valueChanged(double)), this, SLOT(SetLightDirX(double)));

	ui.lightDirY->setValue(m_lightDir.y);
	connect(ui.lightDirY, SIGNAL(valueChanged(double)), this, SLOT(SetLightDirY(double)));

	ui.lightDirZ->setValue(m_lightDir.z);
	connect(ui.lightDirZ, SIGNAL(valueChanged(double)), this, SLOT(SetLightDirZ(double)));

	connect(this, SIGNAL(rejected()), this, SLOT(ResetProperties()));
}

void CDialogWorldProperties::ResetProperties()
{
	m_world->m_canFly = m_canFly;
	m_world->m_bgmID = m_bgmID;
	m_world->m_modePK = m_modePK;
	m_world->m_revivalPlace = m_revivalPlace;
	m_world->m_worldRevivalID = m_worldRevivalID;
	m_world->m_fogStart = m_fogStart;
	m_world->m_fogEnd = m_fogEnd;
	m_world->m_fogDensity = m_fogDensity;

	for (int i = 0; i < 3; i++)
		m_world->m_skyTextureNames[i] = m_skyTextureNames[i];

	m_world->m_moonTextureName = m_moonTextureName;
	m_world->m_sunTextureName = m_sunTextureName;

	for (int i = 0; i < 3; i++)
		m_world->m_cloudTextureNames[i] = m_cloudTextureNames[i];

	m_world->m_seaCloudTextureName = m_seaCloudTextureName;
	if (m_world->m_seaCloudTextureName == STR_DEFAULT)
		m_world->m_seacloudTexture = Project->GetTerrain(SEACLOUD_DEFAULT_TEXTURE_ID);
	else
		m_world->m_seacloudTexture = TextureMng->GetWeatherTexture(m_world->m_seaCloudTextureName);

	m_world->m_inDoor = m_inDoor;
	if (m_world->m_inDoor && m_world->m_skybox)
		Delete(m_world->m_skybox);
	else if (!m_world->m_inDoor && !m_world->m_skybox)
		m_world->m_skybox = new CSkybox(m_world->m_device, m_world);

	m_world->m_ambient = m_ambient;
	m_world->m_diffuse = m_diffuse;
	m_world->m_lightDir = m_lightDir;

	m_world->_setLight(false);
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetCanFly(bool canFly)
{
	m_world->m_canFly = canFly;
}

void CDialogWorldProperties::SetBGM(int index)
{
	m_world->m_bgmID = ui.comboBGM->itemData(index).toInt();
}

void CDialogWorldProperties::SetModePK(int index)
{
	switch (index)
	{
	case 0:
		m_world->m_modePK = 0;
		break;
	case 1:
		m_world->m_modePK = RA_PENALTY_PK;
		break;
	case 2:
		m_world->m_modePK = RA_PK;
		break;
	}
}

void CDialogWorldProperties::SetWorldRevivalID(int index)
{
	m_world->m_worldRevivalID = ui.comboWorldRevivalID->itemData(index).toInt();
}

void CDialogWorldProperties::SetRevivalPlace()
{
	m_world->m_revivalPlace = ui.editRevivalPlace->text();
}

void CDialogWorldProperties::SetFogStart(double value)
{
	m_world->m_fogStart = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetFogEnd(double value)
{
	m_world->m_fogEnd = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetFogDensity(double value)
{
	m_world->m_fogDensity = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetSkyTexture1()
{
	m_world->m_skyTextureNames[0] = ui.skyTexture1->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetSkyTexture2()
{
	m_world->m_skyTextureNames[1] = ui.skyTexture2->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetSkyTexture3()
{
	m_world->m_skyTextureNames[2] = ui.skyTexture3->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetMoonTexture()
{
	m_world->m_moonTextureName = ui.moonTexture->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetSunTexture()
{
	m_world->m_sunTextureName = ui.sunTexture->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetCloudTexture1()
{
	m_world->m_cloudTextureNames[0] = ui.cloudTexture1->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetCloudTexture2()
{
	m_world->m_cloudTextureNames[1] = ui.cloudTexture2->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetCloudTexture3()
{
	m_world->m_cloudTextureNames[2] = ui.cloudTexture3->text();
	if (m_world->m_skybox)
	{
		m_world->m_skybox->LoadTextures();
		MainFrame->UpdateWorldEditor();
	}
}

void CDialogWorldProperties::SetSeacloudTexture()
{
	m_world->m_seaCloudTextureName = ui.seacloudTexture->text();
	if (m_world->m_seaCloudTextureName == STR_DEFAULT)
		m_world->m_seacloudTexture = Project->GetTerrain(SEACLOUD_DEFAULT_TEXTURE_ID);
	else
		m_world->m_seacloudTexture = TextureMng->GetWeatherTexture(m_world->m_seaCloudTextureName);
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetInDoor(bool inDoor)
{
	m_world->m_inDoor = inDoor;
	if (m_world->m_inDoor && m_world->m_skybox)
		Delete(m_world->m_skybox);
	else if (!m_world->m_inDoor && !m_world->m_skybox)
		m_world->m_skybox = new CSkybox(m_world->m_device, m_world);
	m_world->_setLight(false);
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetAmbient()
{
	const QColor color = QColorDialog::getColor(QColor((m_world->m_ambient >> 16) & 0xff, (m_world->m_ambient >> 8) & 0xff, m_world->m_ambient & 0xff), this, tr("Couleur ambiante"));
	if (color.isValid())
	{
		m_world->m_ambient = D3DCOLOR_XRGB(color.red(), color.green(), color.blue());
		ui.colorAmbient->setStyleSheet("background-color: " + color.name() + ';');
	}
	m_world->_setLight(false);
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetDiffuse()
{
	const QColor color = QColorDialog::getColor(QColor((m_world->m_diffuse >> 16) & 0xff, (m_world->m_diffuse >> 8) & 0xff, m_world->m_diffuse & 0xff), this, tr("Couleur diffuse"));
	if (color.isValid())
	{
		m_world->m_diffuse = D3DCOLOR_XRGB(color.red(), color.green(), color.blue());
		ui.colorDiffuse->setStyleSheet("background-color: " + color.name() + ';');
	}
	m_world->_setLight(false);
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetLightDirX(double value)
{
	m_world->m_lightDir.x = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetLightDirY(double value)
{
	m_world->m_lightDir.y = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogWorldProperties::SetLightDirZ(double value)
{
	m_world->m_lightDir.z = (float)value;
	MainFrame->UpdateWorldEditor();
}