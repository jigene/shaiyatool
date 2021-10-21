///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Project.h"
#include "TextFile.h"
#include "GameElements.h"
#include "TextureMng.h"
#include <iostream>
#include <fstream>

CProject* Project = null;
int g_waterCount = 0;

CProject::CProject()
{
	Project = this;

	m_waterLists = null;
	m_waterListCount = null;
	m_waterListCount = 0;

	m_icons[0].addFile(":/MainFrame/Resources/icon_rootFolder.png");
	m_icons[1].addFile(":/MainFrame/Resources/icon_objectFolder.png");
	m_icons[2].addFile(":/MainFrame/Resources/icon_terrainFolder.png");
	m_icons[3].addFile(":/MainFrame/Resources/icon_object.png");
	m_icons[4].addFile(":/MainFrame/Resources/icon_terrain.png");
	m_icons[5].addFile(":/MainFrame/Resources/icon_favorites.png");
	QStandardItem* rootNode = m_gameElements.invisibleRootItem();
	m_objFolder = new CRootFolderElement();
	m_objFolder->setText(QObject::tr("Objet"));
	m_objFolder->setIcon(m_icons[0]);
	rootNode->appendRow(m_objFolder);
	m_terrainFolder = new CRootFolderElement();
	m_terrainFolder->setText(QObject::tr("Terrain"));
	m_terrainFolder->setIcon(m_icons[0]);
	rootNode->appendRow(m_terrainFolder);
}

CProject::~CProject()
{
	Project = null;

	for (int i = 0; i < m_waterListCount; i++)
	{
		DeleteArray(m_waterLists[i].textures);
		DeleteArray(m_waterLists[i].textureIDs);
	}
	DeleteArray(m_waterLists);
}

bool CProject::Load(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	QVector<string> models;
	QVector<string> terrains;
	QVector<string> propMovers;
	QVector<string> propItems;
	QVector<string> characters;

	const string keys[] =
	{
		"project",
		"define",
		"localdef",
		"string",
		"model",
		"terrain",
		"propmover",
		"propitem",
		"character"
	};

	string tok;
	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		tok = file.Token().toLower();

		if (tok == keys[0])
			m_name = file.GetString();
		else if (tok == keys[1] || tok == keys[2])
			CTextFile::LoadDefine(file.GetString(), false);
		else if (tok == keys[3])
			CTextFile::LoadText(file.GetString());
		else if (tok == keys[4])
			models.push_back(file.GetString());
		else if (tok == keys[5])
			terrains.push_back(file.GetString());
		else if (tok == keys[6])
			propMovers.push_back(file.GetString());
		else if (tok == keys[7])
			propItems.push_back(file.GetString());
		else if (tok == keys[8])
			characters.push_back(file.GetString());
		else
			file.NextToken();

		file.NextToken();
	}
	file.Close();

	QVector<string>::iterator it;
	for (it = propMovers.begin(); it != propMovers.end(); it++)
	{
		if (!_loadPropMover(*it))
			return false;
	}

	for (it = propItems.begin(); it != propItems.end(); it++)
	{
		if (!_loadPropItem(*it))
			return false;
	}

	for (it = characters.begin(); it != characters.end(); it++)
	{
		if (!_loadCharacter(*it))
			return false;
	}

	for (it = terrains.begin(); it != terrains.end(); it++)
	{
		if (!_loadTerrain(*it))
			return false;
	}

	for (it = models.begin(); it != models.end(); it++)
	{
		if (!_loadModel(*it))
			return false;
	}

	g_waterCount = m_waterListCount;
	return true;
}

bool CProject::_loadModel(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	CFolderElement* folders[32];
	CModelElement* model;

	ModelProp prop;
	byte type;
	int max, motion;
	string motionStr;
	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		folders[1] = new CFolderElement();
		folders[1]->setIcon(m_icons[1]);
		folders[1]->setText(file.Token());
		m_objFolder->appendRow(folders[1]);

		type = (byte)file.GetInt();

		file.NextToken(); // {
		file.NextToken(); // nom d'un objet ou }

		int brace = 1;
		while (brace)
		{
			if (file.TokenType() == ETokenType::EndBlock)
			{
				brace--;
				if (brace > 0)
				{
					file.NextToken(); // nom d'un objet ou }
					continue;
				}
				if (brace == 0)
					continue;
			}

			prop.filename = file.Token();
			file.SetMark();

			file.NextToken();
			if (file.TokenType() == ETokenType::StartBlock)
			{
				brace++;
				folders[brace] = new CFolderElement();
				folders[brace]->setIcon(m_icons[1]);
				folders[brace]->setText(prop.filename);
				folders[brace - 1]->appendRow(folders[brace]);
				file.NextToken(); // nom d'un objet ou }
				prop.filename = file.Token();
				continue;
			}
			else
				file.GoMark();

			prop.ID = file.GetInt();
			prop.type = type;
			prop.modelType = (byte)file.GetInt();
			prop.part = file.GetString();
			prop.fly = file.GetBool();
			prop.distant = (byte)file.GetInt();
			prop.pick = file.GetBool();
			prop.scale = file.GetFloat();
			prop.trans = file.GetBool();
			prop.shadow = file.GetBool();
			prop.textureEx = (byte)file.GetInt();
			prop.renderFlag = file.GetBool();

			file.NextToken();
			if (file.TokenType() == ETokenType::StartBlock)
			{
				file.SetMark();
				file.NextToken();

				max = 0;
				while (file.TokenType() != ETokenType::EndBlock)
				{
					motion = file.GetInt();
					if (motion > max)
						max = motion;
					file.NextToken();  // nom d'une animation ou }
				}
				max++;

				prop.motionCount = max;
				file.GoMark();
				file.NextToken(); // nom d'une animation ou }
				prop.motions = new char[max * 32];
				memset(prop.motions, 0, max * 32);
				while (file.TokenType() != ETokenType::EndBlock)
				{
					motionStr = file.Token();
					motion = file.GetInt();
					file.NextToken(); // nom d'une animation ou }

					strcpy(prop.GetMotion(motion), motionStr.toLocal8Bit().constData());
				}
				file.NextToken(); // nom d'un objet ou }
			}
			else
			{
				prop.motionCount = 0;
				prop.motions = null;
			}

			m_models[type][prop.ID] = prop;

			model = new CModelElement();
			model->setIcon(m_icons[3]);

			if (type == OT_MOVER)
			{
				auto it = m_moverProps.constFind(prop.ID);
				if (it != m_moverProps.constEnd() && !(*it).name.isEmpty())
					model->setText((*it).name);
				else
					model->setText(prop.filename);
			}
			else if (type == OT_ITEM)
			{
				auto it = m_itemProps.constFind(prop.ID);
				if (it != m_itemProps.constEnd() && !(*it).name.isEmpty())
					model->setText((*it).name);
				else
					model->setText(prop.filename);
			}
			else
				model->setText(prop.filename);

			model->SetModel(&m_models[type][prop.ID]);
			folders[brace]->appendRow(model);
		}

		file.NextToken(); // type d'objet ou }
	}

	file.Close();
	return true;
}

bool CProject::_loadTerrain(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	string tok, tok2;
	int brace = 1, j, k;
	file.SetMark();
	int i = file.GetInt();
	tok = file.Token();
	int frameCount = file.GetInt();
	int imageCount = 0, idCount = 0;
	if (brace == 1 && frameCount)
	{
		if (m_waterListCount)
		{
			for (k = 0; k < m_waterListCount; k++)
			{
				DeleteArray(m_waterLists[k].textures);
				DeleteArray(m_waterLists[k].textureIDs);
			}
			DeleteArray(m_waterLists);
		}
		m_waterListCount = frameCount;
		m_waterLists = new WaterList[frameCount];
		for (j = 0; j < frameCount; j++)
		{
			m_waterLists[j].frame = 0.0f;
			m_waterLists[j].textureCount = 0;
			m_waterLists[j].textureIDs = 0;
			m_waterLists[j].textures = 0;
		}
		imageCount = 0;
	}

	CFolderElement* folders[32];
	CTerrainElement* terrain;
	folders[1] = m_terrainFolder;

	while (brace)
	{
		if (file.TokenType() == ETokenType::EndBlock || file.TokenType() == ETokenType::End)
		{
			brace--;
			if (brace > 0)
			{
				file.SetMark();
				i = file.GetInt();
				tok = file.Token();
				frameCount = file.GetInt();
				idCount = 0;
				if (frameCount)
				{
					if (brace == 1)
					{
						if (m_waterListCount)
						{
							for (k = 0; k < m_waterListCount; k++)
							{
								DeleteArray(m_waterLists[k].textures);
								DeleteArray(m_waterLists[k].textureIDs);
							}
							DeleteArray(m_waterLists);
						}
						m_waterListCount = frameCount;
						m_waterLists = new WaterList[frameCount];
						for (j = 0; j < frameCount; j++)
						{
							m_waterLists[j].frame = 0.0f;
							m_waterLists[j].textureCount = 0;
							m_waterLists[j].textureIDs = 0;
							m_waterLists[j].textures = 0;
						}
						imageCount = 0;
					}
					else if (brace == 2)
					{
						m_waterLists[imageCount].name = tok;
						DeleteArray(m_waterLists[imageCount].textures);
						DeleteArray(m_waterLists[imageCount].textureIDs);
						m_waterLists[imageCount].textureCount = frameCount;
						m_waterLists[imageCount].textures = new CTexture*[frameCount];
						m_waterLists[imageCount].textureIDs = new int[frameCount];
						memset(m_waterLists[imageCount].textures, 0, sizeof(CTexture*) * frameCount);
						memset(m_waterLists[imageCount].textureIDs, 0, sizeof(int) * frameCount);
						imageCount++;
					}
				}
			}
			continue;
		}

		file.NextToken();
		if (file.TokenType() == ETokenType::StartBlock)
		{
			brace++;
			file.SetMark();
			i = file.GetInt();
			tok2 = file.Token();
			frameCount = file.GetInt();

			if (i == 0 && brace == 2 && frameCount)
			{
				m_waterLists[imageCount].name = tok2;
				DeleteArray(m_waterLists[imageCount].textures);
				DeleteArray(m_waterLists[imageCount].textureIDs);
				m_waterLists[imageCount].textureCount = frameCount;
				m_waterLists[imageCount].textures = new CTexture*[frameCount];
				m_waterLists[imageCount].textureIDs = new int[frameCount];
				memset(m_waterLists[imageCount].textures, 0, sizeof(CTexture*) * frameCount);
				memset(m_waterLists[imageCount].textureIDs, 0, sizeof(int) * frameCount);
				imageCount++;
			}

			folders[brace] = new CFolderElement();
			folders[brace]->setSelectable(false);
			folders[brace]->setIcon(m_icons[2]);
			if (brace == 3)
				folders[brace]->setText(m_waterLists[imageCount - 1].name);
			else
				folders[brace]->setText(tok);
			folders[brace - 1]->appendRow(folders[brace]);
			continue;
		}
		else
		{
			file.GoMark();
			i = file.GetInt();
			frameCount = file.GetInt();
			if (brace == 3)
			{
				m_waterLists[imageCount - 1].textureIDs[idCount] = i;
				idCount++;
			}
		}

		m_terrains[i] = file.GetString();
		file.NextToken();
		file.NextToken();

		terrain = new CTerrainElement();
		terrain->setIcon(m_icons[4]);
		terrain->setText(m_terrains[i]);
		terrain->SetTerrain(i);
		folders[brace]->appendRow(terrain);

		file.SetMark();
		i = file.GetInt();
	}

	file.Close();
	return true;
}

bool CProject::_loadPropMover(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	MoverProp prop;
	int ID;
	int i;
	do
	{
		ID = file.GetInt();
		prop.name = file.GetString();
		prop.AI = (uint)file.GetInt();

		for (i = 0; i < 7; i++)
			file.GetInt();

		prop.belligerence = (uint)file.GetInt();

		for (i = 0; i < 35; i++)
			file.NextToken();

		prop.fly = file.GetBool();

		file.GetLine();
		m_moverProps[ID] = prop;
	} while (file.TokenType() != ETokenType::End);

	file.Close();
	return true;
}

bool CProject::_loadPropItem(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	ItemProp prop;
	int ID;
	int i;

	do
	{
		file.GetInt();
		ID = file.GetInt();
		prop.name = file.GetString();

		for (i = 0; i < 15; i++)
			file.NextToken();

		prop.part = file.GetInt();

		file.GetLine();
		m_itemProps[ID] = prop;
	} while (file.TokenType() != ETokenType::End);

	file.Close();
	return true;
}

bool CProject::_loadCharacter(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	const string keys[] =
	{
		"randomItem",
		";",
		"SetEquip",
		")",
		"m_szName",
		"SetFigure",
		"SetName",
		"SetMusic",
		"m_nStructure",
		"m_szChar",
		"m_szDialog",
		"m_szDlgQuest",
		"SetImage",
		"AddMenuLang",
		"AddMenu",
		"AddVenderSlot",
		"AddVendorSlot",
		"AddVendorSlotLang",
		"AddVendorItemLang",
		"AddVenderItem",
		"AddVendorItem",
		"AddVenderItem2",
		"AddVendorItem2",
		"SetVenderType",
		"SetBuffSkill",
		"SetLang",
		"SetOutput",
		"AddTeleport",
		";"
	};

	int block;
	string key;
	do
	{
		Character character;

		character.ID = file.GetString();
		key = character.ID.toLower();
		file.NextToken();

		character.moverID = 0;
		character.head = 0;
		character.hairColor = 0;
		character.hair = 0;
		memset(&character.parts, 0, sizeof(character.parts));
		character.partCount = 0;

		block = 1;
		while (block && file.TokenType() != ETokenType::End)
		{
			file.NextToken();
			if (file.TokenType() == ETokenType::StartBlock)
				block++;
			else if (file.TokenType() == ETokenType::EndBlock)
				block--;
			else
			{
				const string tok = file.Token();
				if (tok == keys[0])
				{
					file.NextToken(); // {
					file.NextToken();
					while (file.TokenType() != ETokenType::EndBlock)
					{
						if (file.Token() == keys[1])
						{
							file.NextToken();
							continue;
						}
						file.NextToken();
					}
				}
				else if (tok == keys[2])
				{
					file.NextToken();
					while (file.Token() != keys[3])
					{
						const uint part = (uint)file.GetInt();
						if (character.partCount < MAX_HUMAN_PARTS)
						{
							character.parts[character.partCount] = part;
							character.partCount++;
						}
						file.NextToken();
					}
				}
				else if (tok == keys[4])
				{
					file.NextToken();
					character.name = file.GetString();
				}
				else if (tok == keys[5])
				{
					file.NextToken();
					character.moverID = file.GetInt();
					file.NextToken();
					character.hair = file.GetInt();
					file.NextToken();
					character.hairColor = file.GetUInt();
					file.NextToken();
					character.head = file.GetInt();
				}
				else if (tok == keys[6])
				{
					file.NextToken();
					character.name = file.GetString();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[7])
				{
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[8])
				{
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[9])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[10])
				{
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[11])
				{
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[12])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[13])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					if (file.TokenType() != ETokenType::Delimiter || file.Token() != keys[28])
					{
						file.NextToken();
						file.NextToken();
					}
				}
				else if (tok == keys[14])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[15])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[16])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[17])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					if (file.TokenType() != ETokenType::Delimiter || file.Token() != keys[28])
					{
						file.NextToken();
						file.NextToken();
					}
				}
				else if (tok == keys[18])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					if (file.TokenType() != ETokenType::Delimiter || file.Token() != keys[28])
					{
						file.NextToken();
						file.NextToken();
					}
				}
				else if (tok == keys[19] || tok == keys[20])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[21] || tok == keys[22])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[23])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[24])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[25])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[26])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
				else if (tok == keys[27])
				{
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
					file.NextToken();
				}
			}
		}

		if (!key.isEmpty())
			m_characters[key] = character;

	} while (file.TokenType() != ETokenType::End);

	file.Close();
	return true;
}

void CProject::FillTreeView(QTreeView* treeView)
{
	treeView->setModel(&m_gameElements);
	treeView->expand(m_objFolder->index());
	treeView->expand(m_terrainFolder->index());
}

void CProject::FillTerrainComboBox(QComboBox* comboBox)
{
	for (auto it = m_terrains.begin(); it != m_terrains.end(); it++)
		comboBox->addItem(it.value(), it.key());
	comboBox->model()->sort(0);
	comboBox->setCurrentText(m_terrains[0]);
}

void CProject::FillWaterComboBox(QComboBox* comboBox)
{
	if (m_waterListCount > 0)
	{
		for (int i = 0; i < m_waterListCount; i++)
			comboBox->addItem(m_waterLists[i].name, i);
		comboBox->setCurrentText(m_waterLists[0].name);
	}
}

CTexture* CProject::GetTerrain(int terrainID) const
{
	auto it = m_terrains.find(terrainID);
	if (it != m_terrains.end())
		return TextureMng->GetTerrainTexture(it.value());

	qCritical(("Terrain texture with ID " + string::number(terrainID) + " doesn't exist").toLocal8Bit());
	return null;
}

string CProject::GetTerrainFilename(int terrainID) const
{
	auto it = m_terrains.constFind(terrainID);
	if (it != m_terrains.constEnd())
		return it.value();
	return "";
}

void CProject::MoveWaterFrame()
{
	for (int i = 0; i < m_waterListCount; i++)
	{
		m_waterLists[i].frame += 0.15f;
		if ((int)m_waterLists[i].frame >= m_waterLists[i].textureCount)
			m_waterLists[i].frame = 0.0f;
	}
}

CTexture* CProject::GetWaterTexture(int index)
{
	const int currentFrame = (int)m_waterLists[index].frame;
	if (!m_waterLists[index].textures[currentFrame])
		m_waterLists[index].textures[currentFrame] = GetTerrain(m_waterLists[index].textureIDs[currentFrame]);
	return m_waterLists[index].textures[currentFrame];
}

ModelProp* CProject::GetModelProp(uint type, uint index)
{
	if (type >= MAX_OBJTYPE)
		return null;

	auto it = m_models[type].find((int)index);
	if (it != m_models[type].end())
		return &it.value();

	qCritical(("Model type " + string::number(type) + " width ID " + string::number(index) + " doesn't exist").toLocal8Bit());
	return null;
}

MoverProp* CProject::GetMoverProp(uint index)
{
	auto it = m_moverProps.find((int)index);
	if (it != m_moverProps.end())
		return &it.value();

	qCritical(("Mover prop width ID " + string::number(index) + " doesn't exist").toLocal8Bit());
	return null;
}

Character* CProject::GetCharacter(const string& key)
{
	auto it = m_characters.find(key.toLower());
	if (it != m_characters.end())
		return &it.value();
	return null;
}

ItemProp* CProject::GetItemProp(uint index)
{
	auto it = m_itemProps.find((int)index);
	if (it != m_itemProps.end())
		return &it.value();

	qCritical(("Item prop width ID " + string::number(index) + " doesn't exist").toLocal8Bit());
	return null;
}

QStandardItem* CProject::CreateFavorite(QDataStream& data)
{
	int type, terrainID;
	byte modeltype;
	uint modelID;

	data >> type;
	if (type == GAMEELE_TERRAIN)
	{
		data >> terrainID;
		auto it = m_terrains.constFind(terrainID);
		if (it != m_terrains.constEnd())
		{
			CTerrainElement* newItem = new CTerrainElement();
			newItem->SetTerrain(terrainID);
			newItem->setIcon(m_icons[4]);
			newItem->setText(it.value());
			return newItem;
		}
	}
	else if (type == GAMEELE_MODEL)
	{
		data >> modeltype;
		data >> modelID;
		if (modeltype < MAX_OBJTYPE)
		{
			auto it2 = m_models[modeltype].find(modelID);
			if (it2 != m_models[modeltype].end())
			{
				const ModelProp& prop = it2.value();
				CModelElement* newItem = new CModelElement();
				newItem->setIcon(m_icons[3]);
				newItem->SetModel(&it2.value());
				if (modeltype == OT_MOVER)
				{
					auto it = m_moverProps.constFind(prop.ID);
					if (it != m_moverProps.constEnd() && !(*it).name.isEmpty())
						newItem->setText((*it).name);
					else
						newItem->setText(prop.filename);
				}
				else if (modeltype == OT_ITEM)
				{
					auto it = m_itemProps.constFind(prop.ID);
					if (it != m_itemProps.constEnd() && !(*it).name.isEmpty())
						newItem->setText((*it).name);
					else
						newItem->setText(prop.filename);
				}
				else
					newItem->setText(prop.filename);
				return newItem;
			}
		}
	}

	return null;
}

void CProject::FillCharacterComboBox(QComboBox* comboBox, const string& current)
{
	comboBox->addItem("-");
	for (auto it = m_characters.begin(); it != m_characters.end(); it++)
		comboBox->addItem(it.key());

	if (!current.isEmpty())
	comboBox->setCurrentText(current.toLower());
}

void CProject::LoadWater(const std::string& filename)
{
	std::ifstream reader(filename, std::ios::binary);
	int waterCount = 0;

	if (reader.is_open()) {

		reader.seekg(0, reader.end);

		int total = reader.tellg();

		reader.seekg(0, reader.beg);

		char * filedata = new char[total + 1];
		reader.read(filedata, total);

		std::string filetext("");
		bool found = false;

		for (int counter = 0; counter < total; counter++) {

			filetext += *(filedata + counter);

			if (filetext.find(".") != std::string::npos) {

				found = true;
			}

			if (*(filedata + counter) == '\0') {

				if (found) {

					m_terrains[waterCount++] = string(filetext.c_str());
					found = false;
				}

				filetext = "";
			}
		}

		reader.close();

		if (waterCount > 0) {

			m_waterLists = new WaterList[waterCount];

			for (int counter = 0; counter < waterCount; counter++) {

				m_waterLists[counter].frame = 0.0f;
				m_waterLists[counter].name = m_terrains[counter];
				m_waterLists[counter].textureCount = 1;
				m_waterLists[counter].textures = new CTexture*[waterCount];
				m_waterLists[counter].textures[0] = TextureMng->GetTerrainTexture(m_terrains[counter]);
				m_waterLists[counter].textureIDs = new int[1]{counter};
			}

			m_waterListCount = waterCount;
		}
	}
}