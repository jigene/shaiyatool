///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "ContinentEdit.h"
#include <World.h>
#include "MainFrame.h"
#include <Skybox.h>

CDialogContinentEdit::CDialogContinentEdit(CWorld* world, QWidget *parent)
	: QDialog(parent, Qt::WindowStaysOnTopHint),
	m_world(world)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	_updateContinentList();

	connect(ui.continentList, SIGNAL(currentRowChanged(int)), this, SLOT(SelectContinent(int)));
	connect(ui.newContinent, SIGNAL(clicked()), this, SLOT(NewContinent()));
	connect(ui.deleteContinent, SIGNAL(clicked()), this, SLOT(DeleteContinent()));
	connect(ui.vertexList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(TeleportToVertex(QListWidgetItem*)));
	connect(ui.name, SIGNAL(editingFinished()), this, SLOT(SetName()));
	connect(ui.town, SIGNAL(clicked(bool)), this, SLOT(SetTown(bool)));
	connect(ui.ID, SIGNAL(valueChanged(int)), this, SLOT(SetID(int)));
	connect(ui.useEnvir, SIGNAL(clicked(bool)), this, SLOT(UseEnvir(bool)));
	connect(ui.startFog, SIGNAL(valueChanged(double)), this, SLOT(SetFogStart(double)));
	connect(ui.endFog, SIGNAL(valueChanged(double)), this, SLOT(SetFogEnd(double)));
	connect(ui.weather, SIGNAL(currentIndexChanged(int)), this, SLOT(SetWeather(int)));
	connect(ui.skyTexture, SIGNAL(editingFinished()), this, SLOT(SetSkyTexture()));
	connect(ui.cloudTexture, SIGNAL(editingFinished()), this, SLOT(SetCloudTexture()));
	connect(ui.sunTexture, SIGNAL(editingFinished()), this, SLOT(SetSunTexture()));
	connect(ui.moonTexture, SIGNAL(editingFinished()), this, SLOT(SetMoonTexture()));
	connect(ui.setAmbient, SIGNAL(clicked()), this, SLOT(SetAmbient()));
	connect(ui.setDiffuse, SIGNAL(clicked()), this, SLOT(SetDiffuse()));

	if (m_world->m_continents.GetSize() > 0)
	{
		if (m_world->m_continent)
		{
			for (int i = 0; i < m_world->m_continents.GetSize(); i++)
			{
				if (m_world->m_continents[i] == m_world->m_continent)
				{
					ui.continentList->setCurrentRow(i);
					break;
				}
			}
		}
		else
			ui.continentList->setCurrentRow(0);
	}
}

void CDialogContinentEdit::SetAmbient()
{
	if (!m_world->m_continent)
		return;

	const QColor color = QColorDialog::getColor(QColor((int)(m_world->m_continent->ambient.x * 255.0f),
		(int)(m_world->m_continent->ambient.y * 255.0f), (int)(m_world->m_continent->ambient.z * 255.0f)), this, tr("Couleur ambiante"));
	if (color.isValid())
	{
		m_world->m_continent->ambient = D3DXVECTOR3(color.redF(), color.greenF(), color.blueF());
		ui.ambient->setStyleSheet("background-color: " + QColor((int)(m_world->m_continent->ambient.x * 255.0f),
			(int)(m_world->m_continent->ambient.y * 255.0f), (int)(m_world->m_continent->ambient.z * 255.0f)).name() + ';');
	}

	m_world->_setLight(false);
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetDiffuse()
{
	if (!m_world->m_continent)
		return;

	const QColor color = QColorDialog::getColor(QColor((int)(m_world->m_continent->diffuse.x * 255.0f),
		(int)(m_world->m_continent->diffuse.y * 255.0f), (int)(m_world->m_continent->diffuse.z * 255.0f)), this, tr("Couleur diffuse"));
	if (color.isValid())
	{
		m_world->m_continent->diffuse = D3DXVECTOR3(color.redF(), color.greenF(), color.blueF());
		ui.diffuse->setStyleSheet("background-color: " + QColor((int)(m_world->m_continent->diffuse.x * 255.0f),
			(int)(m_world->m_continent->diffuse.y * 255.0f), (int)(m_world->m_continent->diffuse.z * 255.0f)).name() + ';');
	}

	m_world->_setLight(false);
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetSkyTexture()
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->skyTexture = ui.skyTexture->text();
	if (m_world->m_continent->skyTexture.isEmpty())
		m_world->m_continent->skyTexture = STR_NO;
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetCloudTexture()
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->cloudTexture = ui.cloudTexture->text();
	if (m_world->m_continent->cloudTexture.isEmpty())
		m_world->m_continent->cloudTexture = STR_NO;
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetSunTexture()
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->sunTexture = ui.sunTexture->text();
	if (m_world->m_continent->sunTexture.isEmpty())
		m_world->m_continent->sunTexture = STR_NO;
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetMoonTexture()
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->moonTexture = ui.moonTexture->text();
	if (m_world->m_continent->moonTexture.isEmpty())
		m_world->m_continent->moonTexture = STR_NO;
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetID(int id)
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->ID = id;
}

void CDialogContinentEdit::SetTown(bool town)
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->town = town;
}

void CDialogContinentEdit::SetName()
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->name = ui.name->text().replace(' ', '_');
	_updateContinentList();
}

void CDialogContinentEdit::UseEnvir(bool use)
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->useEnvir = use;
	m_world->_setLight(false);
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetWeather(int index)
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->weather = (index == 4) ? 5 : index;
}

void CDialogContinentEdit::SetFogStart(double value)
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->fogStart = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::SetFogEnd(double value)
{
	if (!m_world->m_continent)
		return;

	m_world->m_continent->fogEnd = (float)value;
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::TeleportToVertex(QListWidgetItem* item)
{
	const D3DXVECTOR3 vertex = m_world->m_continent->vertices[ui.vertexList->row(item)];

	m_world->m_cameraPos.x = vertex.x;
	m_world->m_cameraPos.z = vertex.z;
	m_world->m_cameraPos.y = 700.0f;
	m_world->m_cameraAngle.x = 0.0f;
	m_world->m_cameraAngle.y = -89.89f;

	MainFrame->UpdateNavigator();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::_updateContinentList()
{
	const int currentRow = ui.continentList->currentRow();
	ui.continentList->clear();
	for (int i = 0; i < m_world->m_continents.GetSize(); i++)
		ui.continentList->addItem(m_world->m_continents[i]->name);
	if (currentRow < m_world->m_continents.GetSize())
		ui.continentList->setCurrentRow(currentRow);
}

void CDialogContinentEdit::UpdateVertexList()
{
	ui.vertexList->clear();
	if (m_world->m_continent)
	{
		for (int i = 0; i < m_world->m_continent->vertices.size(); i++)
		{
			ui.vertexList->addItem(string::number(i + 1)
				+ " : [ " + string::number(m_world->m_continent->vertices[i].x)
				+ ", " + string::number(m_world->m_continent->vertices[i].z)
				+ " ]"
				);
		}
	}
}

void CDialogContinentEdit::SelectContinent(int index)
{
	m_world->m_continent = null;

	if (index == -1)
	{
		ui.groupProperties->setEnabled(false);
		ui.groupVertices->setEnabled(false);
		ui.useEnvir->setEnabled(false);

		ui.name->setText("");
		ui.ID->setValue(0);
		ui.town->setChecked(false);

		ui.useEnvir->setChecked(false);
		ui.weather->setCurrentIndex(0);
		ui.startFog->setValue(70);
		ui.endFog->setValue(400);

		ui.skyTexture->setText("");
		ui.cloudTexture->setText("");
		ui.sunTexture->setText("");
		ui.moonTexture->setText("");

		ui.ambient->setStyleSheet("background-color:#7f7f7f;");
		ui.diffuse->setStyleSheet("background-color:#000000;");
	}
	else
	{
		ui.groupProperties->setEnabled(true);
		ui.groupVertices->setEnabled(true);
		ui.useEnvir->setEnabled(true);

		Continent* continent = m_world->m_continents[index];

		ui.name->setText(continent->name);
		ui.ID->setValue(continent->ID);
		ui.town->setChecked(continent->town);

		ui.useEnvir->setChecked(continent->useEnvir);
		ui.weather->setCurrentIndex((continent->weather == 5) ? 4 : continent->weather);
		ui.startFog->setValue(continent->fogStart);
		ui.endFog->setValue(continent->fogEnd);

		ui.skyTexture->setText(continent->skyTexture);
		ui.cloudTexture->setText(continent->cloudTexture);
		ui.sunTexture->setText(continent->sunTexture);
		ui.moonTexture->setText(continent->moonTexture);

		ui.ambient->setStyleSheet("background-color: " + QColor((int)(continent->ambient.x * 255.0f), (int)(continent->ambient.y * 255.0f), (int)(continent->ambient.z * 255.0f)).name() + ';');
		ui.diffuse->setStyleSheet("background-color: " + QColor((int)(continent->diffuse.x * 255.0f), (int)(continent->diffuse.y * 255.0f), (int)(continent->diffuse.z * 255.0f)).name() + ';');

		m_world->m_continent = continent;
	}

	UpdateVertexList();

	m_world->_setLight(false);
	if (m_world->m_skybox)
		m_world->m_skybox->LoadTextures();
	MainFrame->UpdateWorldEditor();
}

void CDialogContinentEdit::NewContinent()
{
	Continent* newContinent = new Continent();
	newContinent->town = false;
	newContinent->weather = 0;
	newContinent->useEnvir = false;
	newContinent->useRealData = false;
	newContinent->ambient = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
	newContinent->diffuse = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	newContinent->fogStart = 70.0f;
	newContinent->fogEnd = 400.0f;
	newContinent->cloudTexture = STR_NO;
	newContinent->skyTexture = STR_NO;
	newContinent->moonTexture = STR_NO;
	newContinent->sunTexture = STR_NO;

	int newID = 0;
	int newNameID = 0;

	Continent* continent;
	for (int i = 0; i < m_world->m_continents.GetSize(); i++)
	{
		continent = m_world->m_continents[i];
		if (continent->ID >= newID)
			newID = continent->ID + 1;
		if (continent->name.startsWith("NEW_"))
		{
			bool ok = false;
			const int ID = string(continent->name).remove("NEW_").toInt(&ok);
			if (ok)
			{
				if (ID >= newNameID)
					newNameID = ID + 1;
			}
		}
	}

	newContinent->ID = newID;
	newContinent->name = "NEW_" + string::number(newNameID);

	m_world->m_continents.Append(newContinent);
	_updateContinentList();
	ui.continentList->setCurrentRow(m_world->m_continents.GetSize() - 1);
}

void CDialogContinentEdit::DeleteContinent()
{
	const int find = m_world->m_continents.Find(m_world->m_continent);
	if (find != -1)
	{
		Delete(m_world->m_continents[find]);
		m_world->m_continents.RemoveAt(find);
		_updateContinentList();
	}
}