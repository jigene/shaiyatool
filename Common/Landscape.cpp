///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Landscape.h"
#include "File.h"
#include "TextureMng.h"
#include "Project.h"
#include "ModelMng.h"
#include "Object.h"

CLandscape::CLandscape(LPDIRECT3DDEVICE9 device, CWorld* world, int posX, int posY)
	: m_device(device),
	m_posX(posX),
	m_posY(posY),
	m_VB(null),
	m_dirty(true),
	m_visible(false),
	m_waterVBs(null),
	m_world(world),
	m_attrVB(null),
	m_attrVertexCount(0)
{
	memset(m_heightMap, 0, sizeof(m_heightMap));
	memset(m_waterHeight, 0, sizeof(m_waterHeight));
	memset(m_colorMap, 127, sizeof(m_colorMap));

	memset(&m_cloudVB, 0, sizeof(m_cloudVB));
	m_waterVBs = new WaterVertexBuffer[g_waterCount];
	memset(m_waterVBs, 0, sizeof(WaterVertexBuffer) * g_waterCount);

	m_objects[OT_OBJ].Allocate(5000);
	m_objects[OT_SFX].Allocate(2500);
	m_objects[OT_MOVER].Allocate(1000);

	m_device->CreateVertexBuffer((PATCH_SIZE + 1) * (PATCH_SIZE + 1) * (NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE) * sizeof(TerrainVertex),
		D3DUSAGE_WRITEONLY, TerrainVertex::FVF, D3DPOOL_MANAGED, &m_VB, null);

	_initPatches();
}

CLandscape::~CLandscape()
{
	int i;
	for (i = 0; i < m_layers.GetSize(); i++)
	{
		Release(m_layers[i]->lightMap);
		Delete(m_layers[i]);
	}

	Release(m_VB);
	Release(m_attrVB);
	for (i = 0; i < g_waterCount; i++)
		Release(m_waterVBs[i].VB);
	DeleteArray(m_waterVBs);
	Release(m_cloudVB.VB);
}

void CLandscape::_initPatches()
{
	m_dirty = true;
	int Y, X;
	for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
		for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
			m_patches[Y][X].Init(m_device, m_posX + X * PATCH_SIZE, m_posY + Y * PATCH_SIZE, X, Y, m_heightMap);
}

int CLandscape::GetTextureID(const D3DXVECTOR3& pos)
{
	D3DXVECTOR3 realPos((int)((pos.x + (LIGHTMAP_UNITY / 2.0f)) / LIGHTMAP_UNITY) * LIGHTMAP_UNITY,
		0.0f,
		(int)((pos.z + (LIGHTMAP_UNITY / 2.0f)) / LIGHTMAP_UNITY) * LIGHTMAP_UNITY);
	realPos -= GetPosition();

	const int textureOffsetX = realPos.x < (LIGHTMAP_UNITY - 0.0099f) ? 0 : (int)(realPos.x / LIGHTMAP_UNITY + 0.5f);
	const int textureOffsetZ = realPos.z < (LIGHTMAP_UNITY - 0.0099f) ? 0 : (int)(realPos.z / LIGHTMAP_UNITY + 0.5f);

	const int textureOffset = textureOffsetX + textureOffsetZ * MAP_SIZE;
	if (textureOffset < 0 || textureOffset >= MAP_SIZE * MAP_SIZE)
		return -1;

	const int patchOffset = (textureOffsetX / (PATCH_SIZE - 1)) + (textureOffsetZ / (PATCH_SIZE - 1)) * NUM_PATCHES_PER_SIDE;

	byte alpha = 0, alphaCount = 0;
	int textureID = -1;

	if (m_layers.GetSize() > 1)
	{
		LandLayer* layer;
		for (int i = m_layers.GetSize() - 1; i >= 1; i--)
		{
			layer = m_layers[i];
			if (layer->patchEnable[patchOffset])
			{
				alphaCount += layer->alphaMap[textureOffset];
				if (layer->alphaMap[textureOffset] > alpha)
				{
					alpha = layer->alphaMap[textureOffset];
					textureID = m_layers[i]->textureID;
					if (alpha > 127)
						return textureID;
				}
			}
		}

		if (textureID == -1 || alphaCount < 64)
			textureID = m_layers[0]->textureID;
	}
	else if (m_layers.GetSize() == 1)
		textureID = m_layers[0]->textureID;

	return textureID;
}

float CLandscape::GetMaxHeight() const
{
	int X, Y;
	float maxy = -9999999.0f;
	for (X = 0; X <= MAP_SIZE; X++)
	{
		for (Y = 0; Y <= MAP_SIZE; Y++)
		{
			const float y = GetHeight((X *(MAP_SIZE + 1)) + Y);
			if (y > maxy)
				maxy = y;
		}
	}
	if (maxy < 0.0f)
		maxy = 0.0f;
	return maxy;
}

float CLandscape::GetHeight(float x, float z) const
{
	if (x < 0.0f || x > MAP_SIZE || z < 0.0f || z > MAP_SIZE)
		return 0.0f;

	int px, pz;
	float dx, dz;
	float dy1, dy2, dy3, dy4;
	px = (int)x;
	pz = (int)z;
	dx = x - px;
	dz = z - pz;
	float y1 = GetHeight(px + pz * (MAP_SIZE + 1));
	float y2 = GetHeight(px + 1 + pz * (MAP_SIZE + 1));
	float y3 = GetHeight(px + (pz + 1) * (MAP_SIZE + 1));
	float y4 = GetHeight(px + 1 + (pz + 1) * (MAP_SIZE + 1));
	dy1 = y1 * (1 - dx) * (1 - dz);
	dy2 = y2 * dx * (1 - dz);
	dy3 = y3 * (1 - dx) * dz;
	dy4 = y4 * dx * dz;
	return dy1 + dy2 + dy3 + dy4;
}

void CLandscape::Render()
{
	static bool patchRendered[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	memset(patchRendered, 0, sizeof(patchRendered));

	m_device->SetStreamSource(0, m_VB, 0, sizeof(TerrainVertex));

	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	bool blendEnabled = false;

	LandLayer* layer;
	CLandPatch* patch;
	int X, Y;
	for (int i = 0; i < m_layers.GetSize(); i++)
	{
		layer = m_layers[i];

		if (layer->texture)
			m_device->SetTexture(0, *layer->texture);
		else
			m_device->SetTexture(0, null);
		m_device->SetTexture(1, layer->lightMap);

		for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
		{
			for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
			{
				patch = &m_patches[Y][X];
				if (patch->m_visible)
				{
					if (layer->patchEnable[X + Y * NUM_PATCHES_PER_SIDE])
					{
						if (patchRendered[X + Y * NUM_PATCHES_PER_SIDE])
						{
							if (!blendEnabled)
							{
								m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
								blendEnabled = true;
							}
						}
						else
						{
							patchRendered[X + Y * NUM_PATCHES_PER_SIDE] = true;
							if (blendEnabled)
							{
								m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
								blendEnabled = false;
							}
						}
						patch->Render();
					}
				}
			}
		}
	}
}

void CLandscape::RenderAttributes()
{
	if (!m_attrVertexCount)
		return;

	m_device->SetStreamSource(0, m_attrVB, 0, sizeof(Color3DVertex));
	m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_attrVertexCount / 3);
}

void CLandscape::RenderWater()
{
	for (int i = 0; i < g_waterCount; i++)
	{
		if (m_waterVBs[i].vertexCount > 0)
		{
			const CTexture* texture = Project->GetWaterTexture(i);
			if (texture)
				m_device->SetTexture(0, *texture);
			else
				m_device->SetTexture(0, null);
			m_device->SetStreamSource(0, m_waterVBs[i].VB, 0, sizeof(WaterVertex));
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_waterVBs[i].vertexCount / 3);
		}
	}

	if (m_cloudVB.vertexCount > 0)
	{
		const CTexture* texture = m_world->GetSeacloudTexture();
		if (texture)
			m_device->SetTexture(0, *texture);
		else
			m_device->SetTexture(0, null);

		m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		m_device->SetStreamSource(0, m_cloudVB.VB, 0, sizeof(WaterVertex));

		const D3DXVECTOR2 seacloudPos = m_world->GetSeacloudPos();
		D3DXMATRIX mat;
		D3DXMatrixIdentity(&mat);

		mat._31 = seacloudPos.x;
		mat._32 = seacloudPos.x;
		m_device->SetTransform(D3DTS_TEXTURE0, &mat);
		D3DXMatrixTranslation(&mat, 0.0f, 40.0f, 0.0f);
		m_device->SetTransform(D3DTS_WORLD, &mat);
		m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_cloudVB.vertexCount / 3);

		mat._31 = seacloudPos.y;
		mat._32 = seacloudPos.y;
		m_device->SetTransform(D3DTS_TEXTURE0, &mat);
		D3DXMatrixTranslation(&mat, 0.0f, 80.0f, 0.0f);
		m_device->SetTransform(D3DTS_WORLD, &mat);
		m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_cloudVB.vertexCount / 3);

		D3DXMatrixIdentity(&mat);
		m_device->SetTransform(D3DTS_TEXTURE0, &mat);
		m_device->SetTransform(D3DTS_WORLD, &mat);

		m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	}
}

void CLandscape::UpdateTextureLayers()
{
	D3DLOCKED_RECT rectLock;
	D3DCOLOR* pixels;
	LandLayer* layer;
	int j;

	for (int i = 0; i < m_layers.GetSize(); i++)
	{
		layer = m_layers[i];
		layer->lightMap->LockRect(0, &rectLock, 0, 0);
		pixels = (D3DCOLOR*)rectLock.pBits;
		for (j = 0; j < MAP_SIZE * MAP_SIZE; j++)
			pixels[j] = D3DCOLOR_ARGB(layer->alphaMap[j], m_colorMap[j * 3], m_colorMap[j * 3 + 1], m_colorMap[j * 3 + 2]);
		layer->lightMap->UnlockRect(0);
	}
}

bool CLandscape::Load(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	uint version;
	file.Read(version);

	if (version >= 1)
		file.Skip(8); // x, y pos

	file.Read(m_heightMap, (MAP_SIZE + 1) * (MAP_SIZE + 1));
	file.Read(m_waterHeight, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);

	if (version >= 2)
		file.Skip(NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE); // land attributes

	byte layerCount;
	int patchEnable[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	LandLayer* layer = null;
	ushort textureID;
	byte lightMap[MAP_SIZE * MAP_SIZE * 4];
	int j;
	D3DLOCKED_RECT rectLock;

	file.Read(layerCount);
	for (byte i = 0; i < layerCount; i++)
	{
		file.Read(textureID);
		layer = GetLayer((int)textureID);
		file.Read(patchEnable, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);
		for (j = 0; j < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; j++)
			layer->patchEnable[j] = patchEnable[j] != 0;
		file.Read(lightMap);
		for (j = 0; j < MAP_SIZE * MAP_SIZE; j++)
		{
			layer->alphaMap[j] = lightMap[j * 4 + 3];
			if (layer->alphaMap[j] > 0)
			{
				m_colorMap[j * 3] = lightMap[j * 4 + 2];
				m_colorMap[j * 3 + 1] = lightMap[j * 4 + 1];
				m_colorMap[j * 3 + 2] = lightMap[j * 4];
			}
		}
	}

	uint objCount;
	CObject* obj;

	file.Read(objCount);
	for (uint i = 0; i < objCount; i++)
	{
		obj = CObject::CreateObject(file);
		if (obj)
		{
			if (version >= 1)
				obj->SetPos(obj->GetPos() + GetPosition());
			m_world->AddObject(obj);
		}
	}

	file.Read(objCount);
	for (uint i = 0; i < objCount; i++)
	{
		obj = CObject::CreateObject(file);
		if (obj)
		{
			if (version >= 1)
				obj->SetPos(obj->GetPos() + GetPosition());
			m_world->AddObject(obj);
		}
	}

	file.Close();

	UpdateTextureLayers();
	MakeWaterVertexBuffer();
	MakeAttributesVertexBuffer();
	return true;
}

bool CLandscape::Save(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	const uint version = 3;
	file.Write(version);

	file.Write(m_posX / MAP_SIZE);
	file.Write(m_posY / MAP_SIZE);

	file.Write(m_heightMap, (MAP_SIZE + 1) * (MAP_SIZE + 1));
	file.Write(m_waterHeight, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);

	byte unused[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	memset(unused, 0, sizeof(unused));
	file.Write(unused, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);

	file.Write((byte)m_layers.GetSize());
	int i, j;
	LandLayer* layer;
	D3DLOCKED_RECT rectLock;

	for (i = 0; i < m_layers.GetSize(); i++)
	{
		layer = m_layers[i];
		file.Write((ushort)layer->textureID);
		for (j = 0; j < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; j++)
			file.Write(layer->patchEnable[j] ? 1 : 0);
		layer->lightMap->LockRect(0, &rectLock, 0, D3DLOCK_READONLY);
		file.Write(((DWORD*)rectLock.pBits), MAP_SIZE * MAP_SIZE);
		layer->lightMap->UnlockRect(0);
	}

	const D3DXVECTOR3 pos = GetPosition();

	CObject* obj;
	uint objCount = 0;
	for (i = 0; i < (int)m_objects[OT_OBJ].GetSize(); i++)
	{
		if (m_objects[OT_OBJ].GetAt(i)->IsReal())
			objCount++;
	}

	file.Write(objCount);
	for (i = 0; i < (int)m_objects[OT_OBJ].GetSize(); i++)
	{
		obj = m_objects[OT_OBJ].GetAt(i);
		if (obj->IsReal())
		{
			file.Write(obj->GetType());
			obj->Write(file, pos);
		}
	}

	objCount = 0;
	for (i = 0; i < (int)m_objects[OT_SFX].GetSize(); i++)
	{
		if (m_objects[OT_SFX].GetAt(i)->IsReal())
			objCount++;
	}

	file.Write(objCount);
	for (i = 0; i < (int)m_objects[OT_SFX].GetSize(); i++)
	{
		obj = m_objects[OT_SFX].GetAt(i);
		if (obj->IsReal())
		{
			file.Write(obj->GetType());
			obj->Write(file, pos);
		}
	}

	file.Close();
	return true;
}

void CLandscape::UpdateCull()
{
	if (m_dirty)
	{
		_calculateBounds();
		_setVertices();
		m_dirty = false;
	}

	_cull();

	if (!m_visible)
		return;

	int i, j;
	for (i = 0; i < NUM_PATCHES_PER_SIDE; i++)
		for (j = 0; j < NUM_PATCHES_PER_SIDE; j++)
			m_patches[i][j].Cull();

	if (g_global3D.terrainLOD)
	{
		for (j = 0; j < NUM_PATCHES_PER_SIDE; j++)
		{
			for (i = 0; i < NUM_PATCHES_PER_SIDE; i++)
			{
				if (m_patches[j][i].m_visible)
				{
					if (j > 0)
					{
						if (m_patches[j][i].m_level < m_patches[j - 1][i].m_level)
							m_patches[j][i].m_topLevel = m_patches[j - 1][i].m_level;
						else m_patches[j][i].m_topLevel = m_patches[j][i].m_level;
					}
					else
						m_patches[j][i].m_topLevel = m_patches[j][i].m_level;

					if (i < NUM_PATCHES_PER_SIDE - 1)
					{
						if (m_patches[j][i].m_level < m_patches[j][i + 1].m_level)
							m_patches[j][i].m_leftLevel = m_patches[j][i + 1].m_level;
						else
							m_patches[j][i].m_leftLevel = m_patches[j][i].m_level;
					}
					else
						m_patches[j][i].m_leftLevel = m_patches[j][i].m_level;

					if (i > 0)
					{
						if (m_patches[j][i].m_level < m_patches[j][i - 1].m_level)
							m_patches[j][i].m_rightLevel = m_patches[j][i - 1].m_level;
						else
							m_patches[j][i].m_rightLevel = m_patches[j][i].m_level;
					}
					else
						m_patches[j][i].m_rightLevel = m_patches[j][i].m_level;

					if (j < NUM_PATCHES_PER_SIDE - 1)
					{
						if (m_patches[j][i].m_level < m_patches[j + 1][i].m_level)
							m_patches[j][i].m_bottomLevel = m_patches[j + 1][i].m_level;
						else
							m_patches[j][i].m_bottomLevel = m_patches[j][i].m_level;
					}
					else
						m_patches[j][i].m_bottomLevel = m_patches[j][i].m_level;
				}
			}
		}
	}
}

void CLandscape::MakeWaterVertexBuffer()
{
	Release(m_cloudVB.VB);
	m_cloudVB.vertexCount = 0;
	int i, j;

	for (i = 0; i < g_waterCount; i++)
	{
		Release(m_waterVBs[i].VB);
		m_waterVBs[i].vertexCount = 0;
	}

	byte waterTexture;
	for (i = 0; i < NUM_PATCHES_PER_SIDE; i++)
	{
		for (j = 0; j < NUM_PATCHES_PER_SIDE; j++)
		{
			waterTexture = m_waterHeight[j + i * NUM_PATCHES_PER_SIDE].texture;
			if ((waterTexture & (byte)(~MASK_WATERFRAME)) == WTYPE_WATER)
				m_waterVBs[(waterTexture >> 2)].vertexCount += 6;
			if (waterTexture == WTYPE_CLOUD)
				m_cloudVB.vertexCount += 6;
		}
	}

	WaterVertex** waterVertices = new WaterVertex*[g_waterCount];
	WaterVertex* cloudVertices;

	for (i = 0; i < g_waterCount; i++)
	{
		if (m_waterVBs[i].vertexCount > 0)
		{
			m_device->CreateVertexBuffer(m_waterVBs[i].vertexCount * sizeof(WaterVertex),
				D3DUSAGE_WRITEONLY, WaterVertex::FVF, D3DPOOL_MANAGED, &m_waterVBs[i].VB, null);
			m_waterVBs[i].VB->Lock(0, m_waterVBs[i].vertexCount * sizeof(WaterVertex), (void**)&waterVertices[i], 0);
		}
	}
	if (m_cloudVB.vertexCount > 0)
	{
		m_device->CreateVertexBuffer(m_cloudVB.vertexCount * sizeof(WaterVertex),
			D3DUSAGE_WRITEONLY, WaterVertex::FVF, D3DPOOL_MANAGED, &m_cloudVB.VB, null);
		m_cloudVB.VB->Lock(0, m_cloudVB.vertexCount * sizeof(WaterVertex), (void**)&cloudVertices, 0);
	}

	const int worldX = m_posX * MPU;
	const int worldZ = m_posY * MPU;
	for (i = 0; i < NUM_PATCHES_PER_SIDE; i++)
	{
		for (j = 0; j < NUM_PATCHES_PER_SIDE; j++)
		{
			const WaterHeight& waterHeight = m_waterHeight[j + i * NUM_PATCHES_PER_SIDE];
			if ((waterHeight.texture & (byte)(~MASK_WATERFRAME)) == WTYPE_WATER)
			{
				const int loop = waterHeight.texture >> 2;
				if (loop >= 0 && loop < g_waterCount && m_waterVBs[loop].vertexCount > 0)
				{
					waterVertices[loop]->p.x = (float)(worldX + (j * PATCH_SIZE * MPU));
					waterVertices[loop]->p.y = (float)(waterHeight.height);
					waterVertices[loop]->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU));
					waterVertices[loop]->n.x = 0;
					waterVertices[loop]->n.y = 1.0;
					waterVertices[loop]->n.z = 0;
					waterVertices[loop]->u = 0.0f;
					waterVertices[loop]->v = 0.0f;
					waterVertices[loop]++;

					waterVertices[loop]->p.x = (float)(worldX + (j * PATCH_SIZE * MPU));
					waterVertices[loop]->p.y = (float)(waterHeight.height);
					waterVertices[loop]->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
					waterVertices[loop]->n.x = 0;
					waterVertices[loop]->n.y = 1.0;
					waterVertices[loop]->n.z = 0;
					waterVertices[loop]->u = 0.0f;
					waterVertices[loop]->v = 3.0f;
					waterVertices[loop]++;

					waterVertices[loop]->p.x = (float)(worldX + (j * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
					waterVertices[loop]->p.y = (float)(waterHeight.height);
					waterVertices[loop]->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU));
					waterVertices[loop]->n.x = 0;
					waterVertices[loop]->n.y = 1.0;
					waterVertices[loop]->n.z = 0;
					waterVertices[loop]->u = 3.0f;
					waterVertices[loop]->v = 0.0f;
					waterVertices[loop]++;

					waterVertices[loop]->p.x = (float)(worldX + (j * PATCH_SIZE * MPU));
					waterVertices[loop]->p.y = (float)(waterHeight.height);
					waterVertices[loop]->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
					waterVertices[loop]->n.x = 0;
					waterVertices[loop]->n.y = 1.0;
					waterVertices[loop]->n.z = 0;
					waterVertices[loop]->u = 0.0f;
					waterVertices[loop]->v = 3.0f;
					waterVertices[loop]++;

					waterVertices[loop]->p.x = (float)(worldX + (j * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
					waterVertices[loop]->p.y = (float)(waterHeight.height);
					waterVertices[loop]->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU));
					waterVertices[loop]->n.x = 0;
					waterVertices[loop]->n.y = 1.0;
					waterVertices[loop]->n.z = 0;
					waterVertices[loop]->u = 3.0f;
					waterVertices[loop]->v = 0.0f;
					waterVertices[loop]++;

					waterVertices[loop]->p.x = (float)(worldX + (j * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
					waterVertices[loop]->p.y = (float)(waterHeight.height);
					waterVertices[loop]->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
					waterVertices[loop]->n.x = 0;
					waterVertices[loop]->n.y = 1.0;
					waterVertices[loop]->n.z = 0;
					waterVertices[loop]->u = 3.0f;
					waterVertices[loop]->v = 3.0f;
					waterVertices[loop]++;
				}
			}
			else if (waterHeight.texture == WTYPE_CLOUD && m_cloudVB.vertexCount > 0)
			{
				cloudVertices->p.x = (float)(worldX + (j * PATCH_SIZE * MPU));
				cloudVertices->p.y = 0.0f;
				cloudVertices->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU));
				cloudVertices->n.x = 0;
				cloudVertices->n.y = 1.0;
				cloudVertices->n.z = 0;
				cloudVertices->u = 0.0f;
				cloudVertices->v = 0.0f;
				cloudVertices++;

				cloudVertices->p.x = (float)(worldX + (j * PATCH_SIZE * MPU));
				cloudVertices->p.y = 0.0f;
				cloudVertices->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
				cloudVertices->n.x = 0;
				cloudVertices->n.y = 1.0;
				cloudVertices->n.z = 0;
				cloudVertices->u = 0.0f;
				cloudVertices->v = 1.0f;
				cloudVertices++;

				cloudVertices->p.x = (float)(worldX + (j * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
				cloudVertices->p.y = 0.0f;
				cloudVertices->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU));
				cloudVertices->n.x = 0;
				cloudVertices->n.y = 1.0;
				cloudVertices->n.z = 0;
				cloudVertices->u = 1.0f;
				cloudVertices->v = 0.0f;
				cloudVertices++;

				cloudVertices->p.x = (float)(worldX + (j * PATCH_SIZE * MPU));
				cloudVertices->p.y = 0.0f;
				cloudVertices->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
				cloudVertices->n.x = 0;
				cloudVertices->n.y = 1.0;
				cloudVertices->n.z = 0;
				cloudVertices->u = 0.0f;
				cloudVertices->v = 1.0f;
				cloudVertices++;

				cloudVertices->p.x = (float)(worldX + (j * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
				cloudVertices->p.y = 0.0f;
				cloudVertices->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU));
				cloudVertices->n.x = 0;
				cloudVertices->n.y = 1.0;
				cloudVertices->n.z = 0;
				cloudVertices->u = 1.0f;
				cloudVertices->v = 0.0f;
				cloudVertices++;

				cloudVertices->p.x = (float)(worldX + (j * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
				cloudVertices->p.y = 0.0f;
				cloudVertices->p.z = (float)(worldZ + (i * PATCH_SIZE * MPU) + (PATCH_SIZE * MPU));
				cloudVertices->n.x = 0;
				cloudVertices->n.y = 1.0;
				cloudVertices->n.z = 0;
				cloudVertices->u = 1.0f;
				cloudVertices->v = 1.0f;
				cloudVertices++;
			}
		}
	}

	for (i = 0; i < g_waterCount; i++)
	{
		if (m_waterVBs[i].vertexCount > 0)
			m_waterVBs[i].VB->Unlock();
	}
	if (m_cloudVB.vertexCount > 0)
		m_cloudVB.VB->Unlock();

	DeleteArray(waterVertices);
}

void CLandscape::MakeAttributesVertexBuffer()
{
	Release(m_attrVB);

	m_attrVertexCount = 0;

	int x, z;
	for (x = 0; x < MAP_SIZE; x++)
	{
		for (z = 0; z < MAP_SIZE; z++)
		{
			if (m_heightMap[x + z * (MAP_SIZE + 1)] >= HGT_NOWALK)
				m_attrVertexCount += 6;
		}
	}

	if (!m_attrVertexCount)
		return;

	m_device->CreateVertexBuffer(m_attrVertexCount * sizeof(Color3DVertex), D3DUSAGE_WRITEONLY, Color3DVertex::FVF, D3DPOOL_MANAGED, &m_attrVB, null);

	Color3DVertex* VB;
	m_attrVB->Lock(0, 0, (void**)&VB, 0);

	const D3DXVECTOR3 addPos(m_posX * MPU, 0.0f, m_posY * MPU);
	float posY;
	DWORD c;
	for (x = 0; x < MAP_SIZE; x++)
	{
		for (z = 0; z < MAP_SIZE; z++)
		{
			posY = m_heightMap[x + z * (MAP_SIZE + 1)];
			if (posY >= HGT_NOWALK)
			{
				if (posY >= HGT_DIE)
				{
					posY -= HGT_DIE;
					c = D3DCOLOR_ARGB(120, 255, 0, 0);
				}
				else if (posY >= HGT_NOMOVE)
				{
					posY -= HGT_NOMOVE;
					c = D3DCOLOR_ARGB(120, 255, 255, 255);
				}
				else if (posY >= HGT_NOFLY)
				{
					posY -= HGT_NOFLY;
					c = D3DCOLOR_ARGB(120, 0, 0, 255);
				}
				else
				{
					posY -= HGT_NOWALK;
					c = D3DCOLOR_ARGB(120, 0, 255, 0);
				}

				if (x % 2 == 0)
				{
					if (z % 2 == 0)
					{
						VB[0].p = D3DXVECTOR3((float)(x * MPU), posY, (float)(z * MPU)) + addPos;
						VB[1].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[2].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + z * (MAP_SIZE + 1)), (float)(z * MPU)) + addPos;
						VB[3].p = D3DXVECTOR3((float)(x * MPU), GetHeight(x + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[4].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[5].p = D3DXVECTOR3((float)(x * MPU), posY, (float)(z * MPU)) + addPos;
					}
					else
					{
						VB[0].p = D3DXVECTOR3((float)(x * MPU), posY, (float)(z * MPU)) + addPos;
						VB[1].p = D3DXVECTOR3((float)(x * MPU), GetHeight(x + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[2].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + z * (MAP_SIZE + 1)), (float)(z * MPU)) + addPos;
						VB[3].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + z * (MAP_SIZE + 1)), (float)(z * MPU)) + addPos;
						VB[4].p = D3DXVECTOR3((float)(x * MPU), GetHeight(x + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[5].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
					}
				}
				else
				{
					if (z % 2 == 0)
					{
						VB[0].p = D3DXVECTOR3((float)(x * MPU), posY, (float)(z * MPU)) + addPos;
						VB[1].p = D3DXVECTOR3((float)(x * MPU), GetHeight(x + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[2].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + z * (MAP_SIZE + 1)), (float)(z * MPU)) + addPos;
						VB[3].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + z * (MAP_SIZE + 1)), (float)(z * MPU)) + addPos;
						VB[4].p = D3DXVECTOR3((float)(x * MPU), GetHeight(x + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[5].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
					}
					else
					{
						VB[0].p = D3DXVECTOR3((float)(x * MPU), posY, (float)(z * MPU)) + addPos;
						VB[1].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[2].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + z * (MAP_SIZE + 1)), (float)(z * MPU)) + addPos;
						VB[3].p = D3DXVECTOR3((float)(x * MPU), GetHeight(x + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[4].p = D3DXVECTOR3((float)((x + 1) * MPU), GetHeight((x + 1) + (z + 1) * (MAP_SIZE + 1)), (float)((z + 1) * MPU)) + addPos;
						VB[5].p = D3DXVECTOR3((float)(x * MPU), posY, (float)(z * MPU)) + addPos;
					}
				}

				VB[0].c = c;
				VB[1].c = c;
				VB[2].c = c;
				VB[3].c = c;
				VB[4].c = c;
				VB[5].c = c;
				VB += 6;
			}
		}
	}

	m_attrVB->Unlock();
}

void CLandscape::_setVertices()
{
	if (!m_VB)
		return;

	CLandPatch* patch;
	TerrainVertex* VB;
	m_VB->Lock(0, 0, (void**)&VB, 0);

	int vI = 0, i, j, Y, X;
	const float temp = 1.0f / MAP_SIZE;
	const float temp2 = temp * (PATCH_SIZE - 1) * NUM_PATCHES_PER_SIDE / MAP_SIZE;
	D3DXVECTOR3 v1, v2, v3, v4, vecTemp, vecTempPos, vecTempNormal;
	for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
	{
		for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
		{
			patch = &m_patches[Y][X];
			if (patch->m_dirty)
			{
				patch->m_dirty = false;
				for (i = 0; i < PATCH_SIZE + 1; i++)
				{
					for (j = 0; j < PATCH_SIZE + 1; j++)
					{
						const int offset = ((i + Y * PATCH_SIZE) * (MAP_SIZE + 1)) + (j + X * PATCH_SIZE);
						vecTempPos = VB[vI].p = D3DXVECTOR3((float)(X*PATCH_SIZE + j + m_posX) * MPU, GetHeight(offset), (float)(Y * PATCH_SIZE + i + m_posY) * MPU);
						vecTempNormal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
						if ((j - 1 + X*PATCH_SIZE) > 0)
							v1 = D3DXVECTOR3((float)(X*PATCH_SIZE + j - 1 + m_posX) * MPU, (float)GetHeight(((i + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j - 1 + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i + m_posY) * MPU);
						if ((i + 1 + Y*PATCH_SIZE) < MAP_SIZE + 1)
							v2 = D3DXVECTOR3((float)(X*PATCH_SIZE + j + m_posX) * MPU, (float)GetHeight(((i + 1 + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i + 1 + m_posY) * MPU);
						if ((j + 1 + X*PATCH_SIZE) < MAP_SIZE + 1)
							v3 = D3DXVECTOR3((float)(X*PATCH_SIZE + j + 1 + m_posX) * MPU, (float)GetHeight(((i + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j + 1 + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i + m_posY) * MPU);
						if ((i - 1 + Y*PATCH_SIZE) > 0)
							v4 = D3DXVECTOR3((float)(X*PATCH_SIZE + j + m_posX) * MPU, (float)GetHeight(((i - 1 + Y*PATCH_SIZE)*(MAP_SIZE + 1)) + (j + X*PATCH_SIZE)), (float)(Y*PATCH_SIZE + i - 1 + m_posY) * MPU);
						if ((j - 1 + X*PATCH_SIZE) > 0 && (i + 1 + Y*PATCH_SIZE) < MAP_SIZE + 1)
						{
							D3DXVec3Cross(&vecTemp, &(vecTempPos - v1), &(v1 - v2));
							vecTempNormal += vecTemp;
						}
						if ((i + 1 + Y*PATCH_SIZE) < MAP_SIZE + 1 && (j + 1 + X*PATCH_SIZE) < MAP_SIZE + 1)
						{
							D3DXVec3Cross(&vecTemp, &(vecTempPos - v2), &(v2 - v3));
							vecTempNormal += vecTemp;
						}
						if ((j + 1 + X*PATCH_SIZE) < MAP_SIZE + 1 && (i - 1 + Y*PATCH_SIZE) > 0)
						{
							D3DXVec3Cross(&vecTemp, &(vecTempPos - v3), &(v3 - v4));
							vecTempNormal += vecTemp;
						}
						if ((i - 1 + Y*PATCH_SIZE) > 0 && (j - 1 + X*PATCH_SIZE) > 0)
						{
							D3DXVec3Cross(&vecTemp, &(vecTempPos - v4), &(v4 - v1));
							vecTempNormal += vecTemp;
						}
						D3DXVec3Normalize(&(vecTempNormal), &(vecTempNormal));
						VB[vI].n = vecTempNormal;
						VB[vI].tu1 = (float)((float)((float)j) / (PATCH_SIZE)) * 3;
						VB[vI].tv1 = (float)((float)((float)i) / (PATCH_SIZE)) * 3;
						VB[vI].tu2 = temp2 * (float)(j + X * PATCH_SIZE) + (temp / 2.0f);
						VB[vI].tv2 = temp2 * (float)(i + Y * PATCH_SIZE) + (temp / 2.0f);
						vI++;
					}
				}
			}
			else
				vI += (PATCH_SIZE + 1)*(PATCH_SIZE + 1);
		}
	}

	m_VB->Unlock();
}

void CLandscape::_calculateBounds()
{
	int X, Y;
	float maxy = -9999999.0f;
	float miny = 9999999.0f;

	for (X = 0; X <= MAP_SIZE; X++)
	{
		for (Y = 0; Y <= MAP_SIZE; Y++)
		{
			const float y = GetHeight((X *(MAP_SIZE + 1)) + Y);
			if (y > maxy)
				maxy = y;
			if (y < miny)
				miny = y;
		}
	}

	for (Y = 0; Y < NUM_PATCHES_PER_SIDE; Y++)
	{
		for (X = 0; X < NUM_PATCHES_PER_SIDE; X++)
		{
			if (m_patches[Y][X].m_dirty)
				m_patches[Y][X].CalculateBounds();
		}
	}

	m_bounds[0] = D3DXVECTOR3((float)(m_posX)*MPU, miny, (float)m_posY * MPU); // xyz
	m_bounds[1] = D3DXVECTOR3((float)(m_posX + MAP_SIZE) * MPU, miny, (float)m_posY * MPU); // Xyz
	m_bounds[2] = D3DXVECTOR3((float)(m_posX)* MPU, maxy, (float)m_posY * MPU); // xYz
	m_bounds[3] = D3DXVECTOR3((float)(m_posX + MAP_SIZE) * MPU, maxy, (float)m_posY * MPU); // XYz
	m_bounds[4] = D3DXVECTOR3((float)(m_posX)* MPU, miny, (float)(m_posY + MAP_SIZE) * MPU); // xyZ
	m_bounds[5] = D3DXVECTOR3((float)(m_posX + MAP_SIZE) * MPU, miny, (float)(m_posY + MAP_SIZE) * MPU); // XyZ
	m_bounds[6] = D3DXVECTOR3((float)(m_posX)* MPU, maxy, (float)(m_posY + MAP_SIZE) * MPU); // xYZ
	m_bounds[7] = D3DXVECTOR3((float)(m_posX + MAP_SIZE) * MPU, maxy, (float)(m_posY + MAP_SIZE) * MPU); // XYZ
}

void CLandscape::_cull()
{
	byte outside[8];
	memset(outside, 0, sizeof(outside));
	int plane;
	for (int point = 0; point < 8; point++)
	{
		for (plane = 0; plane < 6; plane++)
		{
			if (g_planeFrustum[plane].a * m_bounds[point].x +
				g_planeFrustum[plane].b * m_bounds[point].y +
				g_planeFrustum[plane].c * m_bounds[point].z +
				g_planeFrustum[plane].d < 0)
			{
				outside[point] |= (1 << plane);
			}
		}
		if (outside[point] == 0)
		{
			m_visible = true;
			return;
		}
	}
	if ((outside[0] & outside[1] & outside[2] & outside[3] &
		outside[4] & outside[5] & outside[6] & outside[7]) != 0)
	{
		m_visible = false;
		return;
	}
	m_visible = true;
}

LandLayer* CLandscape::GetLayer(int textureID, int createPosition)
{
	for (int i = 0; i < m_layers.GetSize(); i++)
	{
		if (m_layers[i]->textureID == textureID)
			return m_layers[i];
	}

	LandLayer* layer = new LandLayer();
	layer->textureID = textureID;
	layer->texture = Project->GetTerrain(layer->textureID);

	m_device->CreateTexture(MAP_SIZE, MAP_SIZE, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &layer->lightMap, null);

	D3DLOCKED_RECT rectLock;
	layer->lightMap->LockRect(0, &rectLock, 0, 0);
	DWORD* bits = (DWORD*)rectLock.pBits;

	int i;
	if (m_layers.GetSize() == 0)
	{
		for (i = 0; i < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; i++)
			layer->patchEnable[i] = true;
		for (i = 0; i < MAP_SIZE * MAP_SIZE; i++)
			bits[i] = D3DCOLOR_ARGB(255, 127, 127, 127);
		memset(layer->alphaMap, 255, sizeof(layer->alphaMap));
	}
	else
	{
		for (i = 0; i < NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE; i++)
			layer->patchEnable[i] = false;
		for (i = 0; i < MAP_SIZE * MAP_SIZE; i++)
			bits[i] = D3DCOLOR_ARGB(0, 127, 127, 127);
		memset(layer->alphaMap, 0, sizeof(layer->alphaMap));
	}

	layer->lightMap->UnlockRect(0);
	m_layers.Append(layer, createPosition);
	return layer;
}

int CLandscape::DeleteLayer(int textureID)
{
	for (int i = 0; i < m_layers.GetSize(); i++)
	{
		if (m_layers[i]->textureID == textureID)
		{
			Release(m_layers[i]->lightMap);
			Delete(m_layers[i]);
			m_layers.RemoveAt(i);
			return i;
		}
	}
	return -1;
}

float CLandscape::GetHeight(int offset) const
{
	const float height = m_heightMap[offset];
	if (height >= HGT_NOWALK)
	{
		if (height >= HGT_DIE)
			return m_heightMap[offset] - HGT_DIE;
		if (height >= HGT_NOMOVE)
			return m_heightMap[offset] - HGT_NOMOVE;
		if (height >= HGT_NOFLY)
			return m_heightMap[offset] - HGT_NOFLY;
		return m_heightMap[offset] - HGT_NOWALK;
	}
	return height;
}

float CLandscape::GetHeightAttribute(int offset) const
{
	const float height = m_heightMap[offset];
	if (height >= HGT_NOWALK)
	{
		if (height >= HGT_DIE)
			return HGT_DIE;
		if (height >= HGT_NOMOVE)
			return HGT_NOMOVE;
		if (height >= HGT_NOFLY)
			return HGT_NOFLY;
		return HGT_NOWALK;
	}
	return 0.0f;
}

void CLandscape::SetHeight(int x, int z, float height)
{
	m_heightMap[x + z * (MAP_SIZE + 1)] = height;

	const int tx = x / PATCH_SIZE;
	const int tz = z / PATCH_SIZE;

	m_patches[tz][tx].m_dirty = true;
	if (tx > 0)
		m_patches[tz][tx - 1].m_dirty = true;
	if (tz > 0)
		m_patches[tz - 1][tx].m_dirty = true;
	if (tz > 0 && tx > 0)
		m_patches[tz - 1][tx - 1].m_dirty = true;
	m_dirty = true;
}

void CLandscape::SetWaterHeight(int x, int z, WaterHeight height)
{
	const int offset = x + z * NUM_PATCHES_PER_SIDE;
	if (offset < 0 || offset >= NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE)
		return;

	m_waterHeight[offset] = height;
}

float CLandscape::GetMinHeight(int x, int z) const
{
	return m_patches[z][x].m_bounds[0].y;
}

void CLandscape::GetColor(int x, int z, byte color[3])
{
	const int offset = (x + z * MAP_SIZE) * 3;
	color[0] = m_colorMap[offset + 0];
	color[1] = m_colorMap[offset + 1];
	color[2] = m_colorMap[offset + 2];
}

void CLandscape::SetColor(int x, int z, byte color[3])
{
	const int offset = (x + z * MAP_SIZE) * 3;
	m_colorMap[offset + 0] = color[0];
	m_colorMap[offset + 1] = color[1];
	m_colorMap[offset + 2] = color[2];
}

CLandPatch::CLandPatch()
	: m_dirty(true),
	m_visible(false),
	m_heightMap(null),
	m_posX(0),
	m_posY(0),
	m_level(0)
{
}

void CLandPatch::Init(LPDIRECT3DDEVICE9 device, int posX, int posY, int X, int Y, float* heightMap)
{
	m_device = device;
	m_posX = posX;
	m_posY = posY;
	m_heightMap = &heightMap[(Y * PATCH_SIZE) * (MAP_SIZE + 1) + (X * PATCH_SIZE)];
	m_visible = false;
	m_dirty = true;
	m_offset = ((PATCH_SIZE + 1)*(PATCH_SIZE + 1)) * (Y *NUM_PATCHES_PER_SIDE + X);
}

void CLandPatch::CalculateBounds()
{
	int i, j;
	float maxy = -9999999.0f;
	float miny = 9999999.0f;
	for (i = 0; i <= PATCH_SIZE; i++)
	{
		for (j = 0; j <= PATCH_SIZE; j++)
		{
			float y = (float)m_heightMap[(i *(MAP_SIZE + 1)) + j];
			if (y >= HGT_NOWALK)
			{
				if (y >= HGT_DIE)
					y -= HGT_DIE;
				else if (y >= HGT_NOMOVE)
					y -= HGT_NOMOVE;
				else if (y >= HGT_NOFLY)
					y -= HGT_NOFLY;
				else
					y -= HGT_NOWALK;
			}
			if (y > maxy)
				maxy = y;
			if (y < miny)
				miny = y;
		}
	}

	m_bounds[0] = D3DXVECTOR3((float)(m_posX)* MPU, miny, (float)(m_posY)* MPU); // xyz
	m_bounds[1] = D3DXVECTOR3((float)(m_posX + PATCH_SIZE) * MPU, miny, (float)(m_posY)* MPU); // Xyz
	m_bounds[2] = D3DXVECTOR3((float)(m_posX)* MPU, maxy, (float)(m_posY)* MPU); // xYz
	m_bounds[3] = D3DXVECTOR3((float)(m_posX + PATCH_SIZE) * MPU, maxy, (float)(m_posY)* MPU); // XYz
	m_bounds[4] = D3DXVECTOR3((float)(m_posX)* MPU, miny, (float)(m_posY + PATCH_SIZE) * MPU); // xyZ
	m_bounds[5] = D3DXVECTOR3((float)(m_posX + PATCH_SIZE) * MPU, miny, (float)(m_posY + PATCH_SIZE) * MPU); // XyZ
	m_bounds[6] = D3DXVECTOR3((float)(m_posX)* MPU, maxy, (float)(m_posY + PATCH_SIZE) * MPU); // xYZ
	m_bounds[7] = D3DXVECTOR3((float)(m_posX + PATCH_SIZE) * MPU, maxy, (float)(m_posY + PATCH_SIZE) * MPU); // XYZ

	m_center = (m_bounds[0] + m_bounds[1] + m_bounds[2] + m_bounds[3] + m_bounds[4] + m_bounds[5] + m_bounds[6] + m_bounds[7]) / 8.0f;
}

void CLandPatch::Cull()
{
	byte outside[8];
	memset(outside, 0, sizeof(outside));
	int plane;
	for (int point = 0; point < 8; point++)
	{
		for (plane = 0; plane < 6; plane++)
		{
			if (g_planeFrustum[plane].a * m_bounds[point].x +
				g_planeFrustum[plane].b * m_bounds[point].y +
				g_planeFrustum[plane].c * m_bounds[point].z +
				g_planeFrustum[plane].d < 0)
			{
				outside[point] |= (1 << plane);
			}
		}
		if (outside[point] == 0)
		{
			m_visible = true;
			_calculateLevel();
			return;
		}
	}
	if ((outside[0] & outside[1] & outside[2] & outside[3] &
		outside[4] & outside[5] & outside[6] & outside[7]) != 0)
	{
		m_visible = false;
		return;
	}
	_calculateLevel();
	m_visible = true;
}

void CLandPatch::_calculateLevel()
{
	if (g_global3D.terrainLOD)
	{
		const float dist = D3DXVec3Length(&(g_global3D.cameraPos - m_center));
		if (dist > 128 * MPU)
			m_level = 3;
		else if (dist > 64 * MPU)
			m_level = 2;
		else if (dist > 32 * MPU)
			m_level = 1;
		else
			m_level = 0;
	}
}

void CLandPatch::Render() const
{
	static const uint g_anPrimitive[3] = { 32, 8, 2 };
	static const uint g_anPrimitiveA[3] = { 12, 4, 1 };
	static const uint g_anStartIndex[4] = { 0, (128 + 48) * 3, (128 + 48 + 32 + 16) * 3, (128 + 48 + 32 + 16 + 8 + 4) * 3 };

	if (g_global3D.terrainLOD)
	{
		if (m_level == 3)
			m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[3], 2);
		else
		{
			if (m_level != m_topLevel)
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 4 + g_anPrimitiveA[m_level] * 0) * 3, g_anPrimitiveA[m_level]);
			else
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level], g_anPrimitive[m_level]);
			if (m_level != m_leftLevel)
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 4 + g_anPrimitiveA[m_level] * 1) * 3, g_anPrimitiveA[m_level]);
			else
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 1) * 3, g_anPrimitive[m_level]);
			if (m_level != m_rightLevel)
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 4 + g_anPrimitiveA[m_level] * 2) * 3, g_anPrimitiveA[m_level]);
			else
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 2) * 3, g_anPrimitive[m_level]);
			if (m_level != m_bottomLevel)
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 4 + g_anPrimitiveA[m_level] * 3) * 3, g_anPrimitiveA[m_level]);
			else
				m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), g_anStartIndex[m_level] + (g_anPrimitive[m_level] * 3) * 3, g_anPrimitive[m_level]);
		}
	}
	else
		m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_offset, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1), 0, PATCH_SIZE * PATCH_SIZE * 2);
}