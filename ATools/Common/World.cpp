///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "World.h"
#include "Landscape.h"
#include "Project.h"
#include "TextureMng.h"
#include "Skybox.h"
#include "ModelMng.h"
#include "Mover.h"
#include "Region.h"
#include "Path.h"

int MPU = 4;
string STR_DEFAULT = "default";
string STR_NO = "NO";
CFont* CWorld::s_objNameFont = null;
CPtrArray<CObject> CWorld::s_selection;

CWorld::CWorld(LPDIRECT3DDEVICE9 device)
	: m_device(device),
	m_width(0),
	m_height(0),
	m_ambient(D3DCOLOR_ARGB(255, 128, 128, 128)),
	m_worldRevivalID(WI_WORLD_NONE),
	m_inDoor(false),
	m_canFly(true),
	m_bgmID(0),
	m_modePK(0),
	m_diffuse(0),
	m_lightDir(0.0f, 0.0f, 0.0f),
	m_cameraPos(0.0f, 200.0f, 0.0f),
	m_cameraAngle(0.0f, 0.0f),
	m_fogStart(70.0f),
	m_fogEnd(400.0f),
	m_fogDensity(0.0f),
	m_MPU(4),
	m_sunTextureName(STR_DEFAULT),
	m_seaCloudTextureName(STR_DEFAULT),
	m_moonTextureName(STR_DEFAULT),
	m_lands(null),
	m_visibilityLand(0),
	m_seacloudTexture(null),
	m_seacloudPos(0.0f, 0.0f),
	m_fogColor(0xff000000),
	m_skybox(null),
	m_cullObjCount(0),
	m_cullSfxCount(0),
	m_continent(null),
	m_nextObjectID(1)
{
	for (int i = 0; i < 3; i++)
	{
		m_skyTextureNames[i] = STR_DEFAULT;
		m_cloudTextureNames[i] = STR_DEFAULT;
	}

	m_objects[OT_OBJ].Allocate(20000);
	m_objects[OT_SFX].Allocate(10000);
	m_objects[OT_MOVER].Allocate(5000);
}

CWorld::~CWorld()
{
	int i, j;
	for (i = 0; i < m_continents.GetSize(); i++)
		Delete(m_continents[i]);
	for (i = 0; i < MAX_OBJTYPE; i++)
		for (j = 0; j < m_objects[i].GetSize(); j++)
			Delete(m_objects[i].GetAt(j));
	if (m_lands)
	{
		for (i = 0; i < m_width * m_height; i++)
			Delete(m_lands[i]);
		DeleteArray(m_lands);
	}
	for (auto it = m_paths.begin(); it != m_paths.end(); it++)
		Delete(it.value());
	Delete(m_skybox);
}

bool CWorld::Load(const string& filename)
{
	if (!_loadWldFile(filename))
		return false;

	_initialize();

	const QFileInfo fileInfo(filename);
	m_filename = fileInfo.dir().path() + '/' + fileInfo.baseName();

	string temp = m_filename + ".txt.txt";
	if (QFileInfo(temp).exists())
	{
		if (!_loadTxtTxtFile(temp))
			return false;
	}
	temp = m_filename + ".rgn";
	if (QFileInfo(temp).exists())
	{
		if (!_loadRgnFile(temp))
			return false;
	}
	temp = m_filename + ".dyo";
	if (QFileInfo(temp).exists())
	{
		if (!_loadDyoFile(temp))
			return false;
	}
	temp = m_filename + ".wld.cnt";
	if (QFileInfo(temp).exists())
	{
		if (!_loadCntFile(temp))
			return false;

		UpdateContinent();
	}
	temp = m_filename + ".pat";
	if (QFileInfo(temp).exists())
	{
		if (!_loadPatFile(temp))
			return false;
	}

	return true;
}

bool CWorld::Save(const string& filename)
{
	const QFileInfo fileInfo(filename);
	m_filename = fileInfo.dir().path() + '/' + fileInfo.baseName();

	_setRegionTexts();

	if (!_saveWldFile(m_filename + ".wld")
		|| !_saveDyoFile(m_filename + ".dyo")
		|| !_saveRgnFile(m_filename + ".rgn")
		|| !_saveTxtTxtFile(m_filename + ".txt.txt")
		|| !_savePatFile(m_filename + ".pat")
		|| !_saveCntFile(m_filename + ".wld.cnt"))
		return false;

	int x, z;
	for (z = 0; z < m_height; z++)
	{
		for (x = 0; x < m_width; x++)
		{
			const int offset = z * m_width + x;
			if (m_lands[offset])
			{
				if (!m_lands[offset]->Save(m_filename + string().sprintf("%02d-%02d.lnd", x, z)))
					return false;
			}
		}
	}

	return true;
}

void CWorld::_initialize()
{
	m_lands = new CLandscape*[m_width * m_height];
	memset(m_lands, 0, sizeof(CLandscape*) * m_width * m_height);

	MPU = m_MPU;
	m_visibilityLand = (int)(g_global3D.farPlane / (MAP_SIZE * MPU));

	if (m_seaCloudTextureName == STR_DEFAULT)
		m_seacloudTexture = Project->GetTerrain(SEACLOUD_DEFAULT_TEXTURE_ID);
	else
		m_seacloudTexture = TextureMng->GetWeatherTexture(m_seaCloudTextureName);

	if (!m_inDoor)
		m_skybox = new CSkybox(m_device, this);

	_setLight(false);
}

void CWorld::Create(int width, int height, int textureID, float heightMap, int MPU, bool indoor, const string& bitmap)
{
	m_width = width;
	m_height = height;
	m_MPU = MPU;
	m_inDoor = indoor;

	m_cameraPos = D3DXVECTOR3(MPU * -14, heightMap + MPU * 25, MPU * -14);
	m_cameraAngle.x = 45.0f;
	m_cameraAngle.y = -30.0f;

	_initialize();

	int x, i, y, x2, y2;
	CLandscape* land;
	for (y = 0; y < m_height; y++)
	{
		for (x = 0; x < m_width; x++)
		{
			land = m_lands[y * m_width + x] = new CLandscape(m_device, this, x * MAP_SIZE, y * MAP_SIZE);
			land->GetLayer(textureID);
		}
	}

	QImage image;
	if (!bitmap.isEmpty() && image.load(bitmap))
	{
		QTransform rot;
		rot.rotate(90);
		image = image.transformed(rot);
		for (y = 0; y < m_height; y++)
		{
			for (x = 0; x < m_width; x++)
			{
				land = m_lands[y * m_width + x];
				for (x2 = 0; x2 < (MAP_SIZE + 1); x2++)
				{
					for (y2 = 0; y2 < (MAP_SIZE + 1); y2++)
					{
						land->m_heightMap[x2 *(MAP_SIZE + 1) + y2] = QColor(image.pixel(
							(int)(((float)(x2 + y * MAP_SIZE) / (float)(m_height * MAP_SIZE)) * (float)image.width()),
							(int)(((float)(y2 + x * MAP_SIZE) / (float)(m_width * MAP_SIZE)) * (float)image.height())
						)).redF() * heightMap;
					}
				}
			}
		}
	}
	else
	{
		for (y = 0; y < m_height; y++)
		{
			for (x = 0; x < m_width; x++)
			{
				land = m_lands[y * m_width + x];
				for (i = 0; i < (MAP_SIZE + 1) * (MAP_SIZE + 1); i++)
					land->m_heightMap[i] = heightMap;
			}
		}
	}
}

std::vector<float> CWorld::GetMapHeight() {
	
	std::vector<float> heightData(heightMapTotal, 0.0);
	int covered = 0;

	while (covered < heightMapTotal) {

		heightData[covered] = ((*(heightMap + covered)) - 1000) / 10;
		covered++;
	}

	return heightData;
}

int CWorld::GetTextureID(int x, int y) {

	int location = y * ((m_width / 2) + 1) + x,
		textureID = -1;

	if (location < textureDataTotal) {

		textureID = *(textureData + location);
	}
	else {
	
		qCritical("Texture ID was not found.");
	}

	return textureID;
}

void CWorld::AddObject(CObject* obj)
{
	if (!obj)
		return;

	if (!obj->m_ID)
	{
		obj->m_ID = m_nextObjectID;
		m_nextObjectID++;
	}

	m_objects[obj->m_type].Append(obj);

	CLandscape* land = GetLand(obj->m_pos);
	if (land)
	{
		obj->m_world = this;
		land->m_objects[obj->m_type].Append(obj);

		if (g_global3D.spawnAllMovers
			&& (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
			&& ((CSpawnObject*)obj)->m_isRespawn)
			SpawnObject(obj, true);
	}
}

CObject* CWorld::GetObject(objid id) const
{
	int j;
	for (int i = 0; i < MAX_OBJTYPE; i++)
	{
		for (j = 0; j < m_objects[i].GetSize(); j++)
		{
			if (m_objects[i].GetAt(j)->m_ID == id)
				return m_objects[i].GetAt(j);
		}
	}
	return null;
}

CObject* CWorld::GetObject(objid id, uint type) const
{
	for (int i = 0; i < m_objects[type].GetSize(); i++)
	{
		if (m_objects[type].GetAt(i)->m_ID == id)
			return m_objects[type].GetAt(i);
	}
	return null;
}

CPtrArray<CPath>* CWorld::GetPath(int ID) const
{
	auto it = m_paths.find(ID);
	if (it != m_paths.end())
		return it.value();
	else
		return null;
}

const CPtrArray<CObject>& CWorld::GetObjects(uint type) const
{
	return m_objects[type];
}

void CWorld::MoveObject(CObject* obj, const D3DXVECTOR3& newPos)
{
	D3DXVECTOR3 pos = newPos;
	if (pos.x < 0.0f)
		pos.x = 0.0f;
	else if (pos.x >= (float)(m_width * MAP_SIZE * MPU))
		pos.x = (float)(m_width * MAP_SIZE * MPU) - 0.0001f;
	if (pos.z < 0.0f)
		pos.z = 0.0f;
	else if (pos.z >= (float)(m_height * MAP_SIZE * MPU))
		pos.z = (float)(m_height * MAP_SIZE * MPU) - 0.0001f;

	if (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
	{
		((CSpawnObject*)obj)->m_rect = QRect(
			((CSpawnObject*)obj)->m_rect.topLeft() - QPoint((int)(obj->m_pos.z + 0.5f), (int)(obj->m_pos.x + 0.5f)),
			((CSpawnObject*)obj)->m_rect.size()
			);
	}
	else if (obj->m_type == OT_REGION)
	{
		((CRegion*)obj)->m_rect = QRect(
			((CRegion*)obj)->m_rect.topLeft() - QPoint((int)(obj->m_pos.z + 0.5f), (int)(obj->m_pos.x + 0.5f)),
			((CRegion*)obj)->m_rect.size()
			);
	}

	CLandscape* oldLand = GetLand(obj->m_pos);
	CLandscape* newLand = GetLand(pos);
	if (oldLand != newLand)
	{
		if (oldLand)
		{
			const int find = oldLand->m_objects[obj->m_type].Find(obj);
			if (find != -1)
				oldLand->m_objects[obj->m_type].RemoveAt(find);
		}

		if (newLand)
		{
			obj->m_world = this;
			newLand->m_objects[obj->m_type].Append(obj);
		}
	}

	obj->SetPos(pos);

	if (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
	{
		CSpawnObject* dyna = ((CSpawnObject*)obj);
		dyna->m_rect = QRect(
			QPoint((int)(obj->m_pos.z + 0.5f), (int)(obj->m_pos.x + 0.5f)) + dyna->m_rect.topLeft(),
			dyna->m_rect.size()
			);

		if (dyna->m_rect.left() < 0)
			dyna->m_rect.setLeft(0);
		else if (dyna->m_rect.left() >= m_height * MAP_SIZE * MPU)
			dyna->m_rect.setLeft(m_height * MAP_SIZE * MPU - 1);
		if (dyna->m_rect.right() < 0)
			dyna->m_rect.setRight(0);
		else if (dyna->m_rect.right() >= m_height * MAP_SIZE * MPU)
			dyna->m_rect.setRight(m_height * MAP_SIZE * MPU - 1);
		if (dyna->m_rect.bottom() < 0)
			dyna->m_rect.setBottom(0);
		else if (dyna->m_rect.bottom() >= m_width * MAP_SIZE * MPU)
			dyna->m_rect.setBottom(m_width * MAP_SIZE * MPU - 1);
		if (dyna->m_rect.top() < 0)
			dyna->m_rect.setTop(0);
		else if (dyna->m_rect.top() >= m_width * MAP_SIZE * MPU)
			dyna->m_rect.setTop(m_height * MAP_SIZE * MPU - 1);
		dyna->m_rect = dyna->m_rect.normalized();
	}
	else if (obj->m_type == OT_REGION)
	{
		CRegion* dyna = ((CRegion*)obj);
		dyna->m_rect = QRect(
			QPoint((int)(obj->m_pos.z + 0.5f), (int)(obj->m_pos.x + 0.5f)) + dyna->m_rect.topLeft(),
			dyna->m_rect.size()
			);

		if (dyna->m_rect.left() < 0)
			dyna->m_rect.setLeft(0);
		else if (dyna->m_rect.left() >= m_height * MAP_SIZE * MPU)
			dyna->m_rect.setLeft(m_height * MAP_SIZE * MPU - 1);
		if (dyna->m_rect.right() < 0)
			dyna->m_rect.setRight(0);
		else if (dyna->m_rect.right() >= m_height * MAP_SIZE * MPU)
			dyna->m_rect.setRight(m_height * MAP_SIZE * MPU - 1);
		if (dyna->m_rect.bottom() < 0)
			dyna->m_rect.setBottom(0);
		else if (dyna->m_rect.bottom() >= m_width * MAP_SIZE * MPU)
			dyna->m_rect.setBottom(m_width * MAP_SIZE * MPU - 1);
		if (dyna->m_rect.top() < 0)
			dyna->m_rect.setTop(0);
		else if (dyna->m_rect.top() >= m_width * MAP_SIZE * MPU)
			dyna->m_rect.setTop(m_height * MAP_SIZE * MPU - 1);
		dyna->m_rect = dyna->m_rect.normalized();
	}
}

void CWorld::DeleteObject(CObject* obj)
{
	if (!obj || obj->m_type >= MAX_OBJTYPE)
		return;

	int find = m_objects[obj->m_type].Find(obj);
	if (find != -1)
		m_objects[obj->m_type].RemoveAt(find);

	CLandscape* land = GetLand(obj->m_pos);
	if (land)
	{
		find = land->m_objects[obj->m_type].Find(obj);
		if (find != -1)
			land->m_objects[obj->m_type].RemoveAt(find);
	}

	if (g_global3D.spawnAllMovers
		&& (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
		&& ((CSpawnObject*)obj)->m_isRespawn)
		SpawnObject(obj, false);

	if (obj->m_type == OT_PATH)
	{
		CPath* path = (CPath*)obj;
		for (auto it = m_paths.begin(); it != m_paths.end(); it++)
		{
			find = it.value()->Find(path);
			if (find != -1)
			{
				for (int i = find + 1; i < it.value()->GetSize(); i++)
					it.value()->GetAt(i)->m_index--;
				it.value()->RemoveAt(find);
			}
		}
	}

	Delete(obj);
}

bool CWorld::_loadLndFile(int i, int j)
{
	CLandscape* land = new CLandscape(m_device, this, j * MAP_SIZE, i * MAP_SIZE);

	land->SetupHeightMap(GetMapHeight(), (m_width / 2) + 1, i, j, heightMapTotal);

	if (!land->Load(i, j, GetTextureID(i, j)))
	{
		Delete(land);
		return false;
	}

	m_lands[i * m_width + j] = land;

	const float minX = j * MAP_SIZE * MPU;
	const float maxX = (j + 1) * MAP_SIZE * MPU;
	const float minZ = i * MAP_SIZE * MPU;
	const float maxZ = (i + 1) * MAP_SIZE * MPU;

	int k, l;
	CObject* obj;
	for (k = 0; k < MAX_OBJTYPE; k++)
	{
		for (l = 0; l < m_objects[k].GetSize(); l++)
		{
			obj = m_objects[k].GetAt(l);

			if (!obj->m_world
				&& obj->m_pos.x >= minX
				&& obj->m_pos.x < maxX
				&& obj->m_pos.z >= minZ
				&& obj->m_pos.z < maxZ)
			{
				obj->m_world = this;
				land->m_objects[k].Append(obj);

				if (g_global3D.spawnAllMovers
					&& (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
					&& ((CSpawnObject*)obj)->m_isRespawn)
					SpawnObject(obj, true);
			}
		}
	}

	return true;
}

void CWorld::SpawnObject(CObject* obj, bool addSpawn)
{
	if (obj->m_type != OT_MOVER && obj->m_type != OT_ITEM && obj->m_type != OT_CTRL)
		return;

	CSpawnObject* owner = (CSpawnObject*)obj;

	const QRect normalizedRect = owner->m_rect.normalized();

	CSpawnObject* spawn;
	int i;
	if (addSpawn)
	{
		for (i = 0; i < owner->m_count - 1; i++)
		{
			spawn = (CSpawnObject*)CObject::CreateObject(owner->m_type);
			spawn->m_modelID = owner->m_modelID;
			spawn->m_scale = owner->m_scale;

			if (!spawn->Init())
			{
				Delete(spawn);
				return;
			}

			D3DXVECTOR3 randomPos;
			randomPos.z = (float)normalizedRect.left() + (float)(rand() % ((normalizedRect.width() - MPU) * 20)) / 20.0f;
			randomPos.x = (float)normalizedRect.top() + (float)(rand() % ((normalizedRect.height() - MPU) * 20)) / 20.0f;

			if (spawn->m_type == OT_MOVER && ((CMover*)spawn)->m_moverProp->fly)
				randomPos.y = (float)((int)(owner->m_pos.y) - 20 + (rand() % 40));
			else
				randomPos.y = GetHeight(randomPos.x, randomPos.z);

			spawn->SetRot(D3DXVECTOR3(0, (float)(rand() % 36000) / 100.0f, 0));
			spawn->SetPos(randomPos);
			spawn->m_isReal = false;
			spawn->m_owner = owner;
			AddObject(spawn);
		}
	}
	else
	{
		CPtrArray<CObject> removeObjects(owner->m_count - 1);
		for (i = 0; i < m_objects[owner->m_type].GetSize(); i++)
		{
			if (((CSpawnObject*)m_objects[owner->m_type].GetAt(i))->m_owner == owner)
				removeObjects.Append(m_objects[owner->m_type].GetAt(i));
		}

		for (i = 0; i < removeObjects.GetSize(); i++)
			DeleteObject(removeObjects[i]);
	}
}

void CWorld::UpdateSpawns()
{
	int i;
	CSpawnObject* obj;
	CPtrArray<CObject> spawnObjects;

	for (i = 0; i < m_objects[OT_CTRL].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_CTRL].GetAt(i);
		if (obj->m_world && obj->m_isReal && obj->m_isRespawn)
			spawnObjects.Append(obj);
	}

	for (i = 0; i < m_objects[OT_ITEM].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_ITEM].GetAt(i);
		if (obj->m_world && obj->m_isReal && obj->m_isRespawn)
			spawnObjects.Append(obj);
	}

	for (i = 0; i < m_objects[OT_MOVER].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_MOVER].GetAt(i);
		if (obj->m_world && obj->m_isReal && obj->m_isRespawn)
			spawnObjects.Append(obj);
	}

	for (i = 0; i < spawnObjects.GetSize(); i++)
		SpawnObject(spawnObjects[i], g_global3D.spawnAllMovers);
}