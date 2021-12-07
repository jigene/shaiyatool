///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "TerrainEdition.h"
#include "MainFrame.h"
#include <World.h>

void CEditTerrainHeightCommand::Edit(const D3DXVECTOR3& pos, const QPoint& mousePos, float baseHeight)
{
	int mode, radius, hardness, attribute;
	bool rounded, useFixedHeight;
	float fixedHeight;
	MainFrame->GetTerrainHeightEditInfos(mode, rounded, radius, hardness, useFixedHeight, fixedHeight, attribute);

	float heightAttribute = 0.0f;
	if (mode == 4)
	{
		switch (attribute)
		{
		case 1:
			heightAttribute = HGT_NOWALK;
			break;
		case 2:
			heightAttribute = HGT_NOFLY;
			break;
		case 3:
			heightAttribute = HGT_NOMOVE;
			break;
		case 4:
			heightAttribute = HGT_DIE;
			break;
		}
	}

	const float cameraFactor = D3DXVec3Length(&(m_world->GetCameraPos() - pos)) / 180.0f;
	const float radius1 = (float)((radius - 1) * MPU);
	const D3DXVECTOR3 terrainPos((int)(pos.x + ((float)MPU / 2.0f)) / MPU * MPU, 0.0f, (int)(pos.z + ((float)MPU / 2.0f)) / MPU * MPU);
	const float baseHeight2 = useFixedHeight ? fixedHeight : baseHeight;
	const float radius4 = (float)hardness / 100.0f + 0.1f;
	const int worldWidth = m_world->GetWidth() * MAP_SIZE;

	float x, z, temp, len, radius2, a, x2, z2;
	int pointCount;
	size_t i;
	bool exists;

	for (x = terrainPos.x - radius1; x <= terrainPos.x + radius1; x += MPU)
	{
		for (z = terrainPos.z - radius1; z <= terrainPos.z + radius1; z += MPU)
		{
			if (m_world->VecInWorld(x, z))
			{
				if (rounded)
				{
					len = sqrt((float)((x - terrainPos.x) * (x - terrainPos.x) + (z - terrainPos.z) * (z - terrainPos.z)));
					if ((len - 1.0f) > radius1)
						continue;
					radius2 = radius1;
				}
				else
				{
					const float absX = abs(x - terrainPos.x);
					const float absZ = abs(z - terrainPos.z);
					if (absX == absZ)
						len = absX + (float)MPU / 2.0f;
					else if (absX > absZ)
						len = absX;
					else
						len = absZ;
					radius2 = radius1 + (float)MPU / 2.0f;
				}

				temp = 0.0f;

				const float height = m_world->GetHeight_fast(x, z);
				const float attribute = m_world->GetHeightAttribute(x, z);

				switch (mode)
				{
				case 0:
					temp = (float)mousePos.y() * 0.2f * cameraFactor;
					if (radius > 1)
						temp *= (1.0f - ((len - 1.0f) / radius1) * ((len - 1.0f) / radius2));
					temp += height;
					if (temp < 0.0f)
						temp = 0.0f;
					else if (temp > 999.0f)
						temp = 999.0f;
					temp += attribute;
					break;
				case 1:
					if (len <= radius4 * radius2 || radius <= 1 || (radius4 * radius2 - radius2) == 0.0f)
						temp = baseHeight2;
					else
					{
						a = (len - radius2) / (radius4 * radius2 - radius2) * 0.4f;
						if (a < 0.0f)
							a = 0.0f;
						else if (a > 0.5f)
							a = 0.5f;
						temp = height * (1.0f - a) + baseHeight2 * a;
						if (temp < 0.0f)
							temp = 0.0f;
						else if (temp > 999.0f)
							temp = 999.0f;
					}
					temp += attribute;
					break;
				case 2:
					pointCount = 0;
					for (x2 = x - MPU; x2 <= x + MPU; x2 += MPU)
					{
						for (z2 = z - MPU; z2 <= z + MPU; z2 += MPU)
						{
							if (m_world->VecInWorld(x2, z2))
							{
								temp += m_world->GetHeight_fast(x2, z2);
								pointCount++;
							}
						}
					}
					temp /= (float)pointCount;
					temp += attribute;
					break;
				case 3:
					temp = (float)(rand()) / ((float)RAND_MAX / 0.3f);
					if (temp >= 0.15f)
						temp = -(temp - 0.15f);
					temp += height;
					if (temp < 0.0f)
						temp = 0.0f;
					else if (temp > 999.0f)
						temp = 999.0f;
					temp += attribute;
					break;
				case 4:
					temp = height + heightAttribute;
					break;
				}

				if (temp == height && mode != 4)
					continue;

				exists = false;
				const int offset = ((int)x / MPU) + ((int)z / MPU) * worldWidth;

				for (i = 0; i < m_heights.size(); i++)
				{
					if (m_heights[i].offset == offset)
					{
						m_heights[i].height = temp;
						exists = true;
						break;
					}
				}

				if (!exists)
				{
					HeightEntry entry;
					entry.offset = offset;
					entry.height = temp;
					entry.original = height + attribute;
					m_heights.push_back(entry);
				}
			}
		}
	}

	Apply();
}

void CEditTerrainHeightCommand::Apply(bool undo)
{
	int x, z, mX, mZ;
	const int worldWidth = m_world->GetWidth() * MAP_SIZE;
	const int worldHeight = m_world->GetHeight() * MAP_SIZE;
	CLandscape* land;
	CPtrArray<CLandscape> updateLands(4);
	for (size_t i = 0; i < m_heights.size(); i++)
	{
		const HeightEntry& entry = m_heights[i];

		z = entry.offset / worldWidth;
		x = entry.offset - z * worldWidth;

		mX = x / MAP_SIZE;
		mZ = z / MAP_SIZE;
		if (x == worldWidth)
			mX--;
		if (z == worldHeight)
			mZ--;

		land = m_world->GetLand(mX, mZ);
		if (!land)
			continue;

		const float height = undo ? entry.original : m_heights[i].height;
		const bool updateAttributes = entry.original >= HGT_NOWALK || entry.height >= HGT_NOWALK;

		x -= mX * MAP_SIZE;
		z -= mZ * MAP_SIZE;

		land->SetHeight(x, z, height);
		if (updateAttributes && !updateLands.Contains(land))
			updateLands.Append(land);

		if (x == 0 && mX > 0 && z == 0 && mZ > 0)
		{
			land = m_world->GetLand(mX - 1, mZ - 1);
			if (land)
			{
				land->SetHeight(MAP_SIZE, MAP_SIZE, height);
				if (updateAttributes && !updateLands.Contains(land))
					updateLands.Append(land);
			}
		}
		if (x == 0 && mX > 0)
		{
			land = m_world->GetLand(mX - 1, mZ);
			if (land)
			{
				land->SetHeight(MAP_SIZE, z, height);
				if (updateAttributes && !updateLands.Contains(land))
					updateLands.Append(land);
			}
		}
		if (z == 0 && mZ > 0)
		{
			land = m_world->GetLand(mX, mZ - 1);
			if (land)
			{
				land->SetHeight(x, MAP_SIZE, height);
				if (updateAttributes && !updateLands.Contains(land))
					updateLands.Append(land);
			}
		}
	}

	for (int i = 0; i < updateLands.GetSize(); i++)
		updateLands[i]->MakeAttributesVertexBuffer();
}

void CEditWaterCommand::Edit(const D3DXVECTOR3& pos)
{
	const int mX = int((pos.x / (float)MPU) / PATCH_SIZE);
	const int mZ = int((pos.z / (float)MPU) / PATCH_SIZE);
	const int worldWidth = m_world->GetWidth() * NUM_PATCHES_PER_SIDE;

	if (mX < 0 || mZ < 0 || mX >= worldWidth || mZ >= m_world->GetHeight() * NUM_PATCHES_PER_SIDE)
		return;

	int mode, size, x, z;
	byte height, texture;
	WaterHeight* waterHeight;
	size_t i;
	bool exists;

	MainFrame->GetWaterEditInfos(mode, height, texture, size);

	WaterHeight newWaterHeight;
	if (mode == WTYPE_NONE)
		newWaterHeight.texture = WTYPE_NONE;
	else if (mode == WTYPE_CLOUD)
		newWaterHeight.texture = WTYPE_CLOUD;
	else if (mode == WTYPE_WATER)
	{
		newWaterHeight.height = height;
		newWaterHeight.texture = WTYPE_WATER;
		newWaterHeight.texture |= (texture << 2);
	}

	for (x = mX - size + 1; x < mX + size; x++)
	{
		for (z = mZ - size + 1; z < mZ + size; z++)
		{
			waterHeight = m_world->GetWaterHeight(x * MPU * PATCH_SIZE, z * MPU * PATCH_SIZE);

			if (!waterHeight || (waterHeight->height == newWaterHeight.height && waterHeight->texture == newWaterHeight.texture))
				continue;

			exists = false;
			const int offset = (int)x + (int)z * worldWidth;

			for (i = 0; i < m_heights.size(); i++)
			{
				if (m_heights[i].offset == offset)
				{
					m_heights[i].height = newWaterHeight;
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				HeightEntry entry;
				entry.offset = offset;
				entry.height = newWaterHeight;
				entry.original = *waterHeight;
				m_heights.push_back(entry);
			}
		}
	}

	Apply();
}

void CEditWaterCommand::Optimize()
{
	const int worldWidth = m_world->GetWidth() * NUM_PATCHES_PER_SIDE;
	const int worldHeight = m_world->GetHeight() * NUM_PATCHES_PER_SIDE;
	int x, z;
	WaterHeight* waterHeight;
	CLandscape* land;

	WaterHeight empty;
	memset(&empty, 0, sizeof(empty));

	for (x = 0; x < worldWidth; x++)
	{
		for (z = 0; z < worldHeight; z++)
		{
			const D3DXVECTOR3 v(x * MPU * PATCH_SIZE, 1.0f, z * MPU * PATCH_SIZE);

			waterHeight = m_world->GetWaterHeight(v.x, v.z);
			if (!waterHeight)
				continue;

			land = m_world->GetLand(v);
			if (!land)
				continue;

			const int minHeight = (int)(land->GetMinHeight(x % NUM_PATCHES_PER_SIDE, z % NUM_PATCHES_PER_SIDE) + 0.5f);
			if ((waterHeight->texture == WTYPE_CLOUD && minHeight > 80) ||
				((waterHeight->texture & (byte)(~MASK_WATERFRAME)) == WTYPE_WATER && minHeight > waterHeight->height))
			{
				HeightEntry entry;
				entry.offset = x + z * worldWidth;
				entry.height = empty;
				entry.original = *waterHeight;
				m_heights.push_back(entry);
			}
		}
	}

	Apply();
}

void CEditWaterCommand::FillAllMap()
{
	int mode, size;
	byte mapHeight, texture;

	MainFrame->GetWaterEditInfos(mode, mapHeight, texture, size);

	WaterHeight newWaterHeight;
	memset(&newWaterHeight, 0, sizeof(WaterHeight));

	if (mode == WTYPE_NONE)
		newWaterHeight.texture = WTYPE_NONE;
	else if (mode == WTYPE_CLOUD)
		newWaterHeight.texture = WTYPE_CLOUD;
	else if (mode == WTYPE_WATER)
	{
		newWaterHeight.height = mapHeight;
		newWaterHeight.texture = WTYPE_WATER;
		newWaterHeight.texture |= (texture << 2);
	}

	const int worldWidth = m_world->GetWidth() * NUM_PATCHES_PER_SIDE;
	const int worldHeight = m_world->GetHeight() * NUM_PATCHES_PER_SIDE;
	int x, z;
	WaterHeight* waterHeight;
	CLandscape* land;

	WaterHeight empty, newHeight;
	memset(&empty, 0, sizeof(empty));

	for (x = 0; x < worldWidth; x++)
	{
		for (z = 0; z < worldHeight; z++)
		{
			const D3DXVECTOR3 v(x * MPU * PATCH_SIZE, 1.0f, z * MPU * PATCH_SIZE);

			waterHeight = m_world->GetWaterHeight(v.x, v.z);
			if (!waterHeight)
				continue;

			land = m_world->GetLand(v);
			if (!land)
				continue;

			const int minHeight = (int)(land->GetMinHeight(x % NUM_PATCHES_PER_SIDE, z % NUM_PATCHES_PER_SIDE) + 0.5f);
			if ((mode == WTYPE_CLOUD && minHeight <= 80) || (mode == WTYPE_WATER && minHeight <= (int)mapHeight))
				newHeight = newWaterHeight;
			else
				newHeight = empty;

			if (waterHeight->height == newHeight.height && waterHeight->texture == newHeight.texture)
				continue;

			HeightEntry entry;
			entry.offset = x + z * worldWidth;
			entry.height = newHeight;
			entry.original = *waterHeight;
			m_heights.push_back(entry);
		}
	}

	Apply();
}

void CEditWaterCommand::Apply(bool undo)
{
	const int worldWidth = m_world->GetWidth() * NUM_PATCHES_PER_SIDE;
	int x, z, mX, mZ;
	CLandscape* land;
	CPtrArray<CLandscape> updateLands(4);
	for (size_t i = 0; i < m_heights.size(); i++)
	{
		const HeightEntry& entry = m_heights[i];

		z = entry.offset / worldWidth;
		x = entry.offset - z * worldWidth;

		mX = x / NUM_PATCHES_PER_SIDE;
		mZ = z / NUM_PATCHES_PER_SIDE;

		land = m_world->GetLand(mX, mZ);
		if (!land)
			continue;

		const WaterHeight height = undo ? m_heights[i].original : m_heights[i].height;

		x -= mX * NUM_PATCHES_PER_SIDE;
		z -= mZ * NUM_PATCHES_PER_SIDE;

		land->SetWaterHeight(x, z, height);
		if (!updateLands.Contains(land))
			updateLands.Append(land);
	}

	for (int i = 0; i < updateLands.GetSize(); i++)
		updateLands[i]->MakeWaterVertexBuffer();
}

CDeleteTerrainLayerCommand::CDeleteTerrainLayerCommand(CWorld* world, CLandscape* land, int layerID)
	: CEditCommand(world),
	m_land(land),
	m_layerID(layerID),
	m_position(-1)
{
	if (!m_land)
		return;

	LandLayer* layer = m_land->GetLayer(m_layerID);
	if (layer)
	{
		memcpy(m_layerAlpha, layer->alphaMap, MAP_SIZE * MAP_SIZE);
		memcpy(m_layerPatchEnable, layer->patchEnable, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);
	}
}

void CDeleteTerrainLayerCommand::Apply(bool undo)
{
	if (!m_land)
		return;

	LandLayer* layer = m_land->GetLayer(m_layerID, m_position);
	if (!layer)
		return;

	if (undo)
	{
		memcpy(layer->alphaMap, m_layerAlpha, MAP_SIZE * MAP_SIZE);
		memcpy(layer->patchEnable, m_layerPatchEnable, NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE);
		m_land->UpdateTextureLayers();
	}
	else
		m_position = m_land->DeleteLayer(m_layerID);

	if (MainFrame->GetCurrentInfoLand() == m_land)
	{
		MainFrame->SetLayerInfos(null);
		MainFrame->SetLayerInfos(m_land);
	}
}

void CEditTerrainColorCommand::Edit(const D3DXVECTOR3& pos)
{
	int radius, hardness;
	QColor color;
	MainFrame->GetTextureColorEditInfos(radius, hardness, color);
	radius--;

	const int worldWidth = m_world->GetWidth() * LIGHTMAP_SIZE;
	const int worldHeight = m_world->GetHeight() * LIGHTMAP_SIZE;
	const float lenRadius = (float)radius;
	const float hardnessRadius = lenRadius * ((float)hardness / 100.0f);

	const int posX = (int)((pos.x + (LIGHTMAP_UNITY / 2.0f)) / LIGHTMAP_UNITY);
	const int posZ = (int)((pos.z + (LIGHTMAP_UNITY / 2.0f)) / LIGHTMAP_UNITY);

	int offX, offZ;
	CLandscape* land;
	bool exists;
	float colorFactor;
	size_t i;
	byte oldColor[3];
	byte newColor[3];

	for (offX = posX - radius; offX <= posX + radius; offX++)
	{
		for (offZ = posZ - radius; offZ <= posZ + radius; offZ++)
		{
			const int offset = offX + offZ * worldWidth;
			if (offset < 0 || offset >= worldWidth * worldHeight)
				continue;

			const float len = sqrt((float)((offX - posX) * (offX - posX) + (offZ - posZ) * (offZ - posZ)));
			if (radius && len > lenRadius)
				continue;

			const int mX = offX / LIGHTMAP_SIZE;
			const int mZ = offZ / LIGHTMAP_SIZE;
			const int x = offX - mX * LIGHTMAP_SIZE;
			const int z = offZ - mZ * LIGHTMAP_SIZE;

			land = m_world->GetLand(mX, mZ);
			if (!land)
				continue;

			if (!radius || hardness == 100)
				colorFactor = color.alphaF();
			else
			{
				if (len > hardnessRadius)
					colorFactor = (1.0f - ((len - hardnessRadius) / (lenRadius - hardnessRadius))) * color.alphaF();
				else
					colorFactor = color.alphaF();
			}

			if (colorFactor < 0.0f)
				colorFactor = 0.0f;
			else if (colorFactor > 1.0f)
				colorFactor = 1.0f;

			land->GetColor(x, z, oldColor);

			newColor[0] = (byte)((((float)oldColor[0] / 255.0f) * (1.0f - colorFactor) + color.redF() * colorFactor) * 255.0f + 0.5f);
			newColor[1] = (byte)((((float)oldColor[1] / 255.0f) * (1.0f - colorFactor) + color.greenF() * colorFactor) * 255.0f + 0.5f);
			newColor[2] = (byte)((((float)oldColor[2] / 255.0f) * (1.0f - colorFactor) + color.blueF() * colorFactor) * 255.0f + 0.5f);

			if (newColor[0] == oldColor[0]
				&& newColor[1] == oldColor[1]
				&& newColor[2] == oldColor[2])
				continue;

			exists = false;

			for (i = 0; i < m_colors.size(); i++)
			{
				if (m_colors[i].offset == offset)
				{
					m_colors[i].color[0] = newColor[0];
					m_colors[i].color[1] = newColor[1];
					m_colors[i].color[2] = newColor[2];
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				ColorEntry entry;
				entry.offset = offset;
				entry.color[0] = newColor[0];
				entry.color[1] = newColor[1];
				entry.color[2] = newColor[2];
				entry.original[0] = oldColor[0];
				entry.original[1] = oldColor[1];
				entry.original[2] = oldColor[2];
				m_colors.push_back(entry);
			}
		}
	}

	Apply();
}

void CEditTerrainColorCommand::Apply(bool undo)
{
	const int worldWidth = m_world->GetWidth() * LIGHTMAP_SIZE;

	int x, z, mX, mZ;
	CLandscape* land;
	byte color[3];
	CPtrArray<CLandscape> updateLands(4);

	for (size_t i = 0; i < m_colors.size(); i++)
	{
		const ColorEntry& entry = m_colors[i];

		z = entry.offset / worldWidth;
		x = entry.offset - z * worldWidth;

		mX = x / LIGHTMAP_SIZE;
		mZ = z / LIGHTMAP_SIZE;

		x -= mX * LIGHTMAP_SIZE;
		z -= mZ * LIGHTMAP_SIZE;

		if (x < 0 || x >= LIGHTMAP_SIZE || z < 0 || z >= LIGHTMAP_SIZE)
			continue;

		land = m_world->GetLand(mX, mZ);
		if (!land)
			continue;

		if (undo)
		{
			color[0] = entry.original[0];
			color[1] = entry.original[1];
			color[2] = entry.original[2];
		}
		else
		{
			color[0] = entry.color[0];
			color[1] = entry.color[1];
			color[2] = entry.color[2];
		}

		land->SetColor(x, z, color);
		if (!updateLands.Contains(land))
			updateLands.Append(land);

		if (x == 0 && mX > 0 && z == 0 && mZ > 0)
		{
			land = m_world->GetLand(mX - 1, mZ - 1);
			if (land)
			{
				land->SetColor(LIGHTMAP_SIZE, LIGHTMAP_SIZE, color);
				if (!updateLands.Contains(land))
					updateLands.Append(land);
			}
		}
		if (x == 0 && mX > 0)
		{
			land = m_world->GetLand(mX - 1, mZ);
			if (land)
			{
				land->SetColor(LIGHTMAP_SIZE, z, color);
				if (!updateLands.Contains(land))
					updateLands.Append(land);
			}
		}
		if (z == 0 && mZ > 0)
		{
			land = m_world->GetLand(mX, mZ - 1);
			if (land)
			{
				land->SetColor(x, LIGHTMAP_SIZE, color);
				if (!updateLands.Contains(land))
					updateLands.Append(land);
			}
		}
	}

	for (int i = 0; i < updateLands.GetSize(); i++)
		updateLands[i]->UpdateTextureLayers();
}

void CEditTerrainTextureCommand::Edit(const D3DXVECTOR3& pos)
{
	int textureID, radius, hardness, alpha;
	bool emptyMode;
	MainFrame->GetTextureEditInfos(textureID, radius, hardness, alpha, emptyMode);
	radius--;

	if (!emptyMode && textureID == -1)
		return;

	const int worldWidth = m_world->GetWidth() * LIGHTMAP_SIZE;
	const int worldHeight = m_world->GetHeight() * LIGHTMAP_SIZE;
	const float lenRadius = (float)radius;
	const float hardnessRadius = lenRadius * ((float)hardness / 100.0f);
	const float alphaStaticFactor = (float)alpha / 255.0f;

	const int posX = (int)((pos.x + (LIGHTMAP_UNITY / 2.0f)) / LIGHTMAP_UNITY);
	const int posZ = (int)((pos.z + (LIGHTMAP_UNITY / 2.0f)) / LIGHTMAP_UNITY);

	int offX, offZ, i, currentLayer, totalAlpha;
	CLandscape* land;
	float alphaFactor;
	LandLayer* layer;

	if (emptyMode)
	{
		QVector<int> patches;

		const int width = m_world->GetWidth() * NUM_PATCHES_PER_SIDE;
		const int height = m_world->GetHeight() * NUM_PATCHES_PER_SIDE;

		for (offX = posX - radius; offX <= posX + radius; offX++)
		{
			for (offZ = posZ - radius; offZ <= posZ + radius; offZ++)
			{
				const int offset = offX + offZ * worldWidth;
				if (offset < 0 || offset >= worldWidth * worldHeight)
					continue;

				const float len = sqrt((float)((offX - posX) * (offX - posX) + (offZ - posZ) * (offZ - posZ)));
				if (radius && len > lenRadius)
					continue;

				const int patchOff = (offX / (PATCH_SIZE - 1)) + (offZ / (PATCH_SIZE - 1)) * width;

				if (patchOff < 0 || patchOff >= width * height)
					continue;

				if (!patches.contains(patchOff))
					patches.push_back(patchOff);
			}
		}

		int j, x, z;
		for (i = 0; i < patches.size(); i++)
		{
			const int offset = patches[i];

			offZ = offset / width;
			offX = offset - offZ * width;

			const int mX = offX / NUM_PATCHES_PER_SIDE;
			const int mZ = offZ / NUM_PATCHES_PER_SIDE;

			land = m_world->GetLand(mX, mZ);
			if (!land)
				continue;

			offX *= (PATCH_SIZE - 1);
			offZ *= (PATCH_SIZE - 1);

			const int maxX = offX + (PATCH_SIZE - 1);
			const int maxZ = offZ + (PATCH_SIZE - 1);

			for (j = 0; j < land->m_layers.GetSize(); j++)
			{
				layer = land->m_layers[j];

				for (x = offX; x <= maxX; x++)
					for (z = offZ; z <= maxZ; z++)
						_setAlpha(layer->textureID, x + z * worldWidth, 0);
			}
		}
	}
	else
	{
		for (offX = posX - radius; offX <= posX + radius; offX++)
		{
			for (offZ = posZ - radius; offZ <= posZ + radius; offZ++)
			{
				const int offset = offX + offZ * worldWidth;
				if (offset < 0 || offset >= worldWidth * worldHeight)
					continue;

				const float len = sqrt((float)((offX - posX) * (offX - posX) + (offZ - posZ) * (offZ - posZ)));
				if (radius && len > lenRadius)
					continue;

				const int mX = offX / LIGHTMAP_SIZE;
				const int mZ = offZ / LIGHTMAP_SIZE;
				const int x = offX - mX * LIGHTMAP_SIZE;
				const int z = offZ - mZ * LIGHTMAP_SIZE;

				land = m_world->GetLand(mX, mZ);
				if (!land)
					continue;

				if (!radius || hardness == 100)
					alphaFactor = alphaStaticFactor;
				else
				{
					if (len > hardnessRadius)
						alphaFactor = (1.0f - ((len - hardnessRadius) / (lenRadius - hardnessRadius))) * alphaStaticFactor;
					else
						alphaFactor = alphaStaticFactor;
				}

				if (alphaFactor < 0.0f)
					alphaFactor = 0.0f;
				else if (alphaFactor > 1.0f)
					alphaFactor = 1.0f;

				const byte finalAlpha = (byte)(alphaFactor * 255.0f);
				const int layerCount = land->m_layers.GetSize();
				const int texOff = x + z * MAP_SIZE;

				currentLayer = -1;
				for (i = 0; i < layerCount; i++)
				{
					if (land->m_layers[i]->textureID == textureID)
					{
						currentLayer = i;
						break;
					}
				}

				const byte oldAlpha = currentLayer != -1 ? land->m_layers[currentLayer]->alphaMap[texOff] : 0;

				if (currentLayer == -1 || finalAlpha > oldAlpha)
				{
					_setAlpha(textureID, offset, finalAlpha);
					totalAlpha = (int)finalAlpha;
				}
				else
					totalAlpha = (int)oldAlpha;

				if (currentLayer != -1 && currentLayer < layerCount - 1)
				{
					for (i = currentLayer + 1; i < layerCount; i++)
					{
						layer = land->m_layers[i];

						const int curAlpha = (int)layer->alphaMap[texOff];
						if (curAlpha > 0)
						{
							if (curAlpha + totalAlpha >= 255)
							{
								if (totalAlpha < 255)
								{
									if (255 - totalAlpha < curAlpha)
										_setAlpha(layer->textureID, offset, (byte)(255 - totalAlpha));
									totalAlpha += curAlpha;
								}
								else
									_setAlpha(layer->textureID, offset, 0);
							}
						}
					}
				}

				if (currentLayer != 0 && totalAlpha >= 255)
				{
					const int maxLayer = currentLayer != -1 ? currentLayer : layerCount;
					for (i = 0; i < maxLayer; i++)
					{
						layer = land->m_layers[i];

						if (layer->alphaMap[texOff] >= 0)
							_setAlpha(layer->textureID, offset, 0);
					}
				}
			}
		}
	}

	Apply();
}

void CEditTerrainTextureCommand::Apply(bool undo)
{
	const int worldWidth = m_world->GetWidth() * LIGHTMAP_SIZE;
	CLandscape* currentInfoLand = MainFrame->GetCurrentInfoLand();
	const int oldLayerCount = currentInfoLand ? currentInfoLand->m_layers.GetSize() : 0;

	int x, z, mX, mZ;
	size_t i, k;
	CLandscape* land;
	LandLayer* layer;
	CPtrArray<CLandscape> updateLands(4);
	bool updateLayerList = false;

	for (i = 0; i < m_layers.size(); i++)
	{
		const LayerEntry& layerEntry = m_layers[i];
		for (k = 0; k < layerEntry.alphas.size(); k++)
		{
			const AlphaEntry& entry = layerEntry.alphas[k];

			z = entry.offset / worldWidth;
			x = entry.offset - z * worldWidth;

			mX = x / LIGHTMAP_SIZE;
			mZ = z / LIGHTMAP_SIZE;

			x -= mX * LIGHTMAP_SIZE;
			z -= mZ * LIGHTMAP_SIZE;

			if (x < 0 || x >= LIGHTMAP_SIZE || z < 0 || z >= LIGHTMAP_SIZE)
				continue;

			land = m_world->GetLand(mX, mZ);
			if (!land)
				continue;

			layer = land->GetLayer(layerEntry.texID);
			if (!layer)
				continue;

			const byte alpha = undo ? entry.original : entry.alpha;

			layer->alphaMap[x + z * MAP_SIZE] = alpha;
			if (!updateLands.Contains(land))
				updateLands.Append(land);

			if (x == 0 && mX > 0 && z == 0 && mZ > 0)
			{
				land = m_world->GetLand(mX - 1, mZ - 1);
				if (land)
				{
					layer = land->GetLayer(layerEntry.texID);
					if (layer)
					{
						layer->alphaMap[LIGHTMAP_SIZE + LIGHTMAP_SIZE * MAP_SIZE] = alpha;
						if (!updateLands.Contains(land))
							updateLands.Append(land);
					}
				}
			}
			if (x == 0 && mX > 0)
			{
				land = m_world->GetLand(mX - 1, mZ);
				if (land)
				{
					layer = land->GetLayer(layerEntry.texID);
					if (layer)
					{
						layer->alphaMap[LIGHTMAP_SIZE + z * MAP_SIZE] = alpha;
						if (!updateLands.Contains(land))
							updateLands.Append(land);
					}
				}
			}
			if (z == 0 && mZ > 0)
			{
				land = m_world->GetLand(mX, mZ - 1);
				if (land)
				{
					layer = land->GetLayer(layerEntry.texID);
					if (layer)
					{
						layer->alphaMap[x + LIGHTMAP_SIZE * MAP_SIZE] = alpha;
						if (!updateLands.Contains(land))
							updateLands.Append(land);
					}
				}
			}
		}
	}

	if (currentInfoLand && currentInfoLand->m_layers.GetSize() != oldLayerCount)
		updateLayerList = true;

	int j, x2, z2;
	bool visible;
	for (int i = 0; i < updateLands.GetSize(); i++)
	{
		land = updateLands[i];

		for (j = 0; j < land->m_layers.GetSize(); j++)
		{
			layer = land->m_layers[j];
			visible = false;

			for (x = 0; x < NUM_PATCHES_PER_SIDE; x++)
			{
				for (z = 0; z < NUM_PATCHES_PER_SIDE; z++)
				{
					layer->patchEnable[x + z * NUM_PATCHES_PER_SIDE] = false;
					for (x2 = x * (PATCH_SIZE - 1); x2 <= (x + 1) * (PATCH_SIZE - 1); x2++)
					{
						for (z2 = z * (PATCH_SIZE - 1); z2 <= (z + 1) * (PATCH_SIZE - 1); z2++)
						{
							if (layer->alphaMap[x2 + z2 * MAP_SIZE] > 0)
							{
								layer->patchEnable[x + z * NUM_PATCHES_PER_SIDE] = true;
								visible = true;
								break;
							}
						}
						if (layer->patchEnable[x + z * NUM_PATCHES_PER_SIDE])
							break;
					}
				}
			}

			if (!visible)
			{
				land->DeleteLayer(layer->textureID);
				j--;

				if (land == currentInfoLand)
					updateLayerList = true;
			}
		}

		land->UpdateTextureLayers();
	}

	if (updateLayerList)
	{
		MainFrame->SetLayerInfos(null);
		MainFrame->SetLayerInfos(currentInfoLand);
	}
}

void CEditTerrainTextureCommand::_setAlpha(int texID, int offset, byte alpha)
{
	int x, z;

	for (size_t i = 0; i < m_layers.size(); i++)
	{
		if (m_layers[i].texID == texID)
		{
			const std::vector<AlphaEntry>& alphas = m_layers[i].alphas;
			const size_t alphaCount = alphas.size();

			for (size_t j = 0; j < alphaCount; j++)
			{
				if (alphas[j].offset == offset)
				{
					m_layers[i].alphas[j].alpha = alpha;
					return;
				}
			}

			AlphaEntry entry;
			entry.offset = offset;
			entry.alpha = alpha;
			entry.original = 0;

			const int worldWidth2 = m_world->GetWidth() * LIGHTMAP_SIZE;
			z = entry.offset / worldWidth2;
			x = entry.offset - z * worldWidth2;
			const int mX = x / LIGHTMAP_SIZE;
			const int mZ = z / LIGHTMAP_SIZE;

			const CLandscape* land = m_world->GetLand(mX, mZ);
			if (land)
			{
				for (int j = 0; j < land->m_layers.GetSize(); j++)
				{
					if (land->m_layers[j]->textureID == texID)
					{
						x -= mX * LIGHTMAP_SIZE;
						z -= mZ * LIGHTMAP_SIZE;
						entry.original = land->m_layers[j]->alphaMap[x + z * MAP_SIZE];
						break;
					}
				}
			}

			m_layers[i].alphas.push_back(entry);
			return;
		}
	}

	AlphaEntry entry;
	entry.offset = offset;
	entry.alpha = alpha;
	entry.original = 0;

	const int worldWidth = m_world->GetWidth() * LIGHTMAP_SIZE;
	z = entry.offset / worldWidth;
	x = entry.offset - z * worldWidth;
	const int mX = x / LIGHTMAP_SIZE;
	const int mZ = z / LIGHTMAP_SIZE;

	const CLandscape* land = m_world->GetLand(mX, mZ);
	if (land)
	{
		for (int j = 0; j < land->m_layers.GetSize(); j++)
		{
			if (land->m_layers[j]->textureID == texID)
			{
				x -= mX * LIGHTMAP_SIZE;
				z -= mZ * LIGHTMAP_SIZE;
				entry.original = land->m_layers[j]->alphaMap[x + z * MAP_SIZE];
				break;
			}
		}
	}

	LayerEntry layer;
	layer.texID = texID;
	layer.alphas.push_back(entry);
	m_layers.push_back(layer);
}