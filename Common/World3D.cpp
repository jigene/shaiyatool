///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "World.h"
#include "Landscape.h"
#include "ModelMng.h"
#include "Project.h"
#include "Skybox.h"
#include "Object.h"
#include "Model.h"
#include "TextureMng.h"
#include "Region.h"
#include "Mover.h"
#include "Path.h"
#include "Font.h"

LPDIRECT3DINDEXBUFFER9 CWorld::s_IB = null;
LPDIRECT3DINDEXBUFFER9 CWorld::s_gridIB = null;
LPDIRECT3DVERTEXBUFFER9 CWorld::s_bbVB = null;
D3DXPLANE g_planeFrustum[6];

void CWorld::Render()
{
	_applyCamera();
	_loadLndFiles();

	int x, z, i, j, X, Y;
	WorldPosToLand(m_cameraPos, x, z);

	for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
		for (j = x - m_visibilityLand; j <= (x + m_visibilityLand); j++)
			if (LandInWorld(j, i))
				m_lands[i * m_width + j]->UpdateCull();

	_cullObjects();

	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	m_device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	if (!g_global3D.renderMinimap)
	{
		if (!m_inDoor && g_global3D.skybox && m_skybox && !g_global3D.editionLight)
			m_skybox->Render();
	}

	if (g_global3D.fillMode != D3DFILL_SOLID)
		m_device->SetRenderState(D3DRS_FILLMODE, g_global3D.fillMode);

	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	_setLight(true);
	_setFog();

	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	const float bias = -0.0f;
	m_device->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, *((DWORD*)&bias));

	if (g_global3D.renderTerrain)
		_renderTerrain();

	m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetTexture(0, null);
	m_device->SetTexture(1, null);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetIndices(null);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

	if (g_global3D.renderTerrainAttributes)
		_renderTerrainAttributes();

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	m_device->SetFVF(TerrainVertex::FVF);
	m_device->SetIndices(s_gridIB);

	if (g_global3D.grid)
		_renderGrid();

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetTexture(0, null);

	CObject* obj;
	if (g_global3D.renderCollisions)
	{
		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		for (i = 0; i < m_cullObjCount; i++)
		{
			obj = m_objCull[i];
			if (obj->m_isReal && obj->m_type == OT_OBJ)
				obj->m_model->RenderCollision(&obj->m_TM);
		}
		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	m_device->SetIndices(null);

	if (g_global3D.renderObjects)
	{
		m_device->SetTransform(D3DTS_WORLD, &identity);
		m_device->SetFVF(Color3DVertex::FVF);

		for (i = 0; i < s_selection.GetSize(); i++)
		{
			obj = s_selection[i];
			if (obj->m_updateMatrix)
				obj->_updateMatrix();
			_renderBB(obj);
		}
	}

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHAREF, 0xb0);
	m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	for (i = 0; i < m_cullObjCount; i++)
	{
		obj = m_objCull[i];

		if (obj->m_model->HasSkin() && g_global3D.fog)
		{
			D3DXVECTOR4 fogConst(1.0f, 1.0f, 1.0f, 100.0f);
			if (m_continent && m_continent->useEnvir)
				fogConst.w = (m_continent->fogEnd - obj->m_distToCamera) / (m_continent->fogEnd - m_continent->fogStart);
			else
				fogConst.w = (m_fogEnd - obj->m_distToCamera) / (m_fogEnd - m_fogStart);
			m_device->SetVertexShaderConstantF(95, (float*)&fogConst, 1);
		}

		obj->Render();
	}

	m_device->SetTransform(D3DTS_WORLD, &identity);

	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetTexture(0, null);
	m_device->SetIndices(null);
	m_device->SetRenderState(D3DRS_FOGENABLE, g_global3D.fog ? TRUE : FALSE);

	if (g_global3D.renderWater)
		_renderWater();

	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);

	for (i = 0; i < m_cullSfxCount; i++)
		m_sfxCull[i]->Render();

	m_device->SetTransform(D3DTS_WORLD, &identity);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	m_device->SetFVF(TerrainVertex::FVF);
	m_device->SetIndices(s_gridIB);

	if (g_global3D.fillMode != D3DFILL_SOLID)
		m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	if (g_global3D.fog)
		m_device->SetRenderState(D3DRS_FOGENABLE, TRUE);

	for (i = 0; i < m_cullObjCount; i++)
	{
		obj = m_objCull[i];
		if (obj->m_type == OT_REGION && obj->GetRectColor() != 0)
			_renderGrids(((CRegion*)obj)->m_rect, obj->GetRectColor());
		if (g_global3D.renderSpawns && (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL) && ((CSpawnObject*)obj)->m_isRespawn)
			_renderGrids(((CSpawnObject*)obj)->m_rect, obj->GetRectColor());
	}

	m_device->SetIndices(null);
	m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);

	if (g_global3D.renderObjects && s_selection.GetSize() >= 1)
	{
		obj = s_selection[s_selection.GetSize() - 1];
		if (obj->m_visible)
		{
			if (obj->m_type == OT_REGION)
				_renderGridPoints(((CRegion*)obj)->m_rect);
			if (g_global3D.renderSpawns && (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL) && ((CSpawnObject*)obj)->m_isRespawn)
				_renderGridPoints(((CSpawnObject*)obj)->m_rect);
		}
	}

	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	if (g_global3D.renderMoverNames)
	{
		for (i = 0; i < m_cullObjCount; i++)
		{
			obj = m_objCull[i];
			if (obj->m_isReal && obj->m_type == OT_MOVER)
				obj->RenderName();
		}
	}

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetTexture(0, null);
	m_device->SetFVF(Color3DVertex::FVF);

	if (!g_global3D.renderMinimap)
	{
		if (m_continent && g_global3D.continentVertices)
			_renderContinentLines();

		if (m_paths.size() > 0 && (g_global3D.renderMonster || g_global3D.renderNPC))
			_renderPaths();
	}
}

void CWorld::_renderTerrain()
{
	int x, z, i, j, px, pz;
	WorldPosToLand(m_cameraPos, x, z);

	if (g_global3D.terrainLOD)
	{
		for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
		{
			for (j = x - m_visibilityLand; j <= x + m_visibilityLand; j++)
			{
				int offset = i * m_width + j;
				if (LandInWorld((int)j, (int)i) && m_lands[offset])
				{
					if (m_lands[offset]->m_visible)
					{
						if (j > 0 && LandInWorld((int)j - 1, (int)i) && m_lands[offset - 1])
							for (pz = 0; pz < NUM_PATCHES_PER_SIDE; pz++)
								if (m_lands[offset]->m_patches[pz][0].m_visible)
									if (m_lands[offset]->m_patches[pz][0].m_level < m_lands[offset - 1]->m_patches[pz][NUM_PATCHES_PER_SIDE - 1].m_level)
										m_lands[offset]->m_patches[pz][0].m_rightLevel = m_lands[offset - 1]->m_patches[pz][NUM_PATCHES_PER_SIDE - 1].m_level;
						if (i>0 && LandInWorld((int)j, (int)i - 1) && m_lands[offset - m_width])
							for (px = 0; px < NUM_PATCHES_PER_SIDE; px++)
								if (m_lands[offset]->m_patches[0][px].m_visible)
									if (m_lands[offset]->m_patches[0][px].m_level < m_lands[offset - m_width]->m_patches[NUM_PATCHES_PER_SIDE - 1][px].m_level)
										m_lands[offset]->m_patches[0][px].m_topLevel = m_lands[offset - m_width]->m_patches[NUM_PATCHES_PER_SIDE - 1][px].m_level;
						if (j < m_width - 1 && LandInWorld((int)j + 1, (int)i) && m_lands[offset + 1])
							for (pz = 0; pz < NUM_PATCHES_PER_SIDE; pz++)
								if (m_lands[offset]->m_patches[pz][NUM_PATCHES_PER_SIDE - 1].m_visible)
									if (m_lands[offset]->m_patches[pz][NUM_PATCHES_PER_SIDE - 1].m_level < m_lands[offset + 1]->m_patches[pz][0].m_level)
										m_lands[offset]->m_patches[pz][NUM_PATCHES_PER_SIDE - 1].m_leftLevel = m_lands[offset + 1]->m_patches[pz][0].m_level;
						if (i < m_width - 1 && LandInWorld((int)j, (int)i + 1) && m_lands[offset + m_width])
							for (px = 0; px < NUM_PATCHES_PER_SIDE; px++)
								if (m_lands[offset]->m_patches[NUM_PATCHES_PER_SIDE - 1][px].m_visible)
									if (m_lands[offset]->m_patches[NUM_PATCHES_PER_SIDE - 1][px].m_level < m_lands[offset + m_width]->m_patches[0][px].m_level)
										m_lands[offset]->m_patches[NUM_PATCHES_PER_SIDE - 1][px].m_bottomLevel = m_lands[offset + m_width]->m_patches[0][px].m_level;
					}
				}
			}
		}
	}

	m_device->SetFVF(TerrainVertex::FVF);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	m_device->SetIndices(s_IB);

	for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
	{
		for (j = x - m_visibilityLand; j <= (x + m_visibilityLand); j++)
		{
			if (LandInWorld(j, i))
			{
				const int offset = i * m_width + j;
				if (m_lands[offset] && m_lands[offset]->m_visible)
					m_lands[offset]->Render();
			}
		}
	}
}

void CWorld::_renderTerrainAttributes()
{
	int x, z, i, j;
	WorldPosToLand(m_cameraPos, x, z);
	m_device->SetFVF(Color3DVertex::FVF);

	for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
	{
		for (j = x - m_visibilityLand; j <= (x + m_visibilityLand); j++)
		{
			if (LandInWorld(j, i))
			{
				const int offset = i * m_width + j;
				if (m_lands[offset] && m_lands[offset]->m_visible)
					m_lands[offset]->RenderAttributes();
			}
		}
	}
}

void CWorld::_renderWater()
{
	int x, z, i, j;
	WorldPosToLand(m_cameraPos, x, z);

	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(133, 0, 0, 0));
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	m_device->SetFVF(WaterVertex::FVF);

	if (g_global3D.animate)
	{
		Project->MoveWaterFrame();
		m_seacloudPos.x += 0.001f;
		m_seacloudPos.y += 0.0015f;
	}

	for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
	{
		for (j = x - m_visibilityLand; j <= (x + m_visibilityLand); j++)
		{
			if (LandInWorld(j, i))
			{
				const int offset = i * m_width + j;
				if (m_lands[offset] && m_lands[offset]->m_visible)
					m_lands[offset]->RenderWater();
			}
		}
	}

	const WaterHeight* height = GetWaterHeight(m_cameraPos.x, m_cameraPos.z);
	if (height && (height->texture & (byte)(~MASK_WATERFRAME)) == WTYPE_WATER
		&& m_cameraPos.y < height->height)
	{
		const CTexture* texture = Project->GetWaterTexture(height->texture >> 2);
		if (texture)
		{
			m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
			m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);

			m_device->SetTexture(0, *texture);

			UnderWaterVertex vertices[4];
			vertices[0].p = D3DXVECTOR4(0.0f, 0.0f, 0.5f, 1.0f);
			vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
			vertices[1].p = D3DXVECTOR4(0.0f, (float)g_global3D.viewport.Height, 0.5f, 1.0f);
			vertices[1].t = D3DXVECTOR2(0.0f, 0.0f);
			vertices[2].p = D3DXVECTOR4((float)g_global3D.viewport.Width, 0.0f, 0.5f, 1.0f);
			vertices[2].t = D3DXVECTOR2(0.0f, 0.0f);
			vertices[3].p = D3DXVECTOR4((float)g_global3D.viewport.Width, (float)g_global3D.viewport.Height, 0.5f, 1.0f);
			vertices[3].t = D3DXVECTOR2(0.0f, 0.0f);

			m_device->SetFVF(UnderWaterVertex::FVF);
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(127, 0, 0, 0));
			m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(UnderWaterVertex));
		}
	}
}

void CWorld::_renderGrid()
{
	const QRect rect(0, 0, m_width * MAP_SIZE * MPU, m_height * MAP_SIZE * MPU);

	if (g_global3D.gridUnity)
		_renderGrids(rect, 0xff808080, 1);
	if (g_global3D.gridPatch)
		_renderGrids(rect, 0xffffffff, MAP_SIZE / NUM_PATCHES_PER_SIDE);
	if (g_global3D.gridLand)
		_renderGrids(rect, 0xffffff00, MAP_SIZE);
}

void CWorld::_renderGrids(const QRect& _rect, DWORD color, int dx)
{
	const QRect rect = _rect.normalized();

	int startx = rect.top() / MPU;
	int starty = rect.left() / MPU;
	int endx = rect.bottom() / MPU;
	int endy = rect.right() / MPU;

	if (startx < 0)
		startx = 0;
	if (starty < 0)
		starty = 0;
	if (endx > m_width * MAP_SIZE)
		endx = m_width * MAP_SIZE;
	if (endy > m_height * MAP_SIZE)
		endy = m_height * MAP_SIZE;

	int startworldx = startx / MAP_SIZE;
	int startworldy = starty / MAP_SIZE;
	int endworldx = endx / MAP_SIZE;
	int endworldy = endy / MAP_SIZE;

	if (dx == 0)
	{
		int i = 0;
		i++;
	}

	if (startworldx < 0)
		startworldx = 0;
	if (startworldy < 0)
		startworldy = 0;
	if (endworldx >= m_width)
		endworldx = m_width - 1;
	if (endworldy >= m_height)
		endworldy = m_height - 1;

	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, color);

	int wx, wy;
	for (wy = startworldy; wy < endworldy + 1; wy++)
		for (wx = startworldx; wx < endworldx + 1; wx++)
			_renderWorldGrids(wx, wy, QPoint(startx, starty), QPoint(endx, endy), dx, color);
}

void CWorld::_renderWorldGrids(int wx, int wy, const QPoint& ptLT, const QPoint& ptRB, int dx, DWORD color)
{
	CLandscape* land = GetLand(wx, wy);

	if (!land || !land->m_visible)
		return;

	int startx = ptLT.x() - wx * MAP_SIZE;
	int starty = ptLT.y() - wy * MAP_SIZE;
	int endx = ptRB.x() - wx * MAP_SIZE;
	int endy = ptRB.y() - wy * MAP_SIZE;

	if (startx < 0)
		startx = 0;
	if (starty < 0)
		starty = 0;
	if (endx >= MAP_SIZE)
		endx = MAP_SIZE - 1;
	if (endy >= MAP_SIZE)
		endy = MAP_SIZE - 1;

	const int startpatchx = startx / PATCH_SIZE;
	const int startpatchy = starty / PATCH_SIZE;
	const int endpatchx = endx / PATCH_SIZE;
	const int endpatchy = endy / PATCH_SIZE;

	ushort* IB;
	int count = 0, py, px, gy, gx;
	s_gridIB->Lock(0, 0, (void**)&IB, D3DLOCK_DISCARD);
	for (py = startpatchy; py <= endpatchy; py++)
	{
		for (px = startpatchx; px <= endpatchx; px++)
		{
			if (!land->m_patches[py][px].m_visible)
				continue;

			const int indexoffset = (px + py * NUM_PATCHES_PER_SIDE) * (PATCH_SIZE + 1) * (PATCH_SIZE + 1);

			int startgridx = startx - px*PATCH_SIZE;
			int startgridy = starty - py*PATCH_SIZE;
			int endgridx = endx - px*PATCH_SIZE;
			int endgridy = endy - py*PATCH_SIZE;

			if (startgridx < 0)
				startgridx = 0;
			if (startgridy < 0)
				startgridy = 0;
			if (endgridx >= PATCH_SIZE)
				endgridx = PATCH_SIZE - 1;
			if (endgridy >= PATCH_SIZE)
				endgridy = PATCH_SIZE - 1;

			if (endgridx + px*PATCH_SIZE + wx*MAP_SIZE == ptRB.x())
				endgridx--;
			if (endgridy + py*PATCH_SIZE + wy*MAP_SIZE == ptRB.y())
				endgridy--;

			if (dx == 0)
			{
				for (gy = startgridy; gy <= endgridy + 1; gy++)
				{
					if (gy + py*PATCH_SIZE + wy*MAP_SIZE == ptLT.y() || gy + py*PATCH_SIZE + wy*MAP_SIZE == ptRB.y())
					{
						for (gx = startgridx; gx < endgridx + 1; gx++)
						{
							IB[count++] = gx + (gy*(PATCH_SIZE + 1)) + indexoffset;
							IB[count++] = (gx + 1) + (gy*(PATCH_SIZE + 1)) + indexoffset;
						}
					}
				}
				for (gx = startgridx; gx <= endgridx + 1; gx++)
				{
					if (gx + px*PATCH_SIZE + wx*MAP_SIZE == ptLT.x() || gx + px*PATCH_SIZE + wx*MAP_SIZE == ptRB.x())
					{
						for (gy = startgridy; gy < endgridy + 1; gy++)
						{
							IB[count++] = gx + (gy*(PATCH_SIZE + 1)) + indexoffset;
							IB[count++] = gx + ((gy + 1)*(PATCH_SIZE + 1)) + indexoffset;
						}
					}
				}
			}
			else
			{
				for (gy = startgridy; gy <= endgridy + 1; gy++)
				{
					if ((gy + py*PATCH_SIZE + wy*MAP_SIZE - ptLT.y()) % dx == 0)
					{
						for (gx = startgridx; gx < endgridx + 1; gx++)
						{
							IB[count++] = gx + (gy*(PATCH_SIZE + 1)) + indexoffset;
							IB[count++] = (gx + 1) + (gy*(PATCH_SIZE + 1)) + indexoffset;
						}
					}
				}
				for (gx = startgridx; gx <= endgridx + 1; gx++)
				{
					if ((gx + px*PATCH_SIZE + wx*MAP_SIZE - ptLT.x()) % dx == 0)
					{
						for (gy = startgridy; gy < endgridy + 1; gy++)
						{
							IB[count++] = gx + (gy*(PATCH_SIZE + 1)) + indexoffset;
							IB[count++] = gx + ((gy + 1)*(PATCH_SIZE + 1)) + indexoffset;
						}
					}
				}
			}
		}
	}

	s_gridIB->Unlock();
	m_device->SetStreamSource(0, land->m_VB, 0, sizeof(TerrainVertex));

	if (count > 0)
		m_device->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, (PATCH_SIZE + 1)*(PATCH_SIZE + 1) * (NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE), 0, count / 2);
}

void CWorld::_renderBB(CObject* obj, DWORD c)
{
	Color3DVertex* VB;
	s_bbVB->Lock(0, 0, (void**)&VB, D3DLOCK_DISCARD);

	VB->p = obj->m_bounds[0];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[4];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[1];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[5];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[2];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[6];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[3];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[7];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[4];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[5];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[5];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[6];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[6];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[7];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[7];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[4];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[0];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[1];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[1];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[2];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[2];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[3];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[3];
	VB->c = c;
	VB++;
	VB->p = obj->m_bounds[0];
	VB->c = c;
	VB++;

	s_bbVB->Unlock();

	m_device->SetStreamSource(0, s_bbVB, 0, sizeof(Color3DVertex));
	m_device->DrawPrimitive(D3DPT_LINELIST, 0, 12);
}

void CWorld::_renderGridPoints(const QRect& _rect)
{
	QRect rect = _rect.normalized();
	rect.setTopLeft((rect.topLeft() - QPoint(MPU / 2, MPU / 2)) / MPU * MPU);
	rect.setBottomRight((rect.bottomRight() - QPoint(MPU / 2, MPU / 2)) / MPU * MPU);

	Color3DVertex* VB;
	s_bbVB->Lock(0, 0, (void**)&VB, D3DLOCK_DISCARD);

	VB->p = D3DXVECTOR3(rect.bottom(), GetHeight(rect.bottom(), rect.right()), rect.right());
	VB->c = 0xffffffff;
	VB++;
	VB->p = D3DXVECTOR3(rect.bottom(), GetHeight(rect.bottom(), rect.left()), rect.left());
	VB->c = 0xffffffff;
	VB++;
	VB->p = D3DXVECTOR3(rect.top(), GetHeight(rect.top(), rect.right()), rect.right());
	VB->c = 0xffffffff;
	VB++;
	VB->p = D3DXVECTOR3(rect.top(), GetHeight(rect.top(), rect.left()), rect.left());
	VB->c = 0xffffffff;
	VB++;

	s_bbVB->Unlock();

	const float pointSize = 4.0f;
	m_device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pointSize));
	m_device->SetFVF(Color3DVertex::FVF);
	m_device->SetStreamSource(0, s_bbVB, 0, sizeof(Color3DVertex));
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->DrawPrimitive(D3DPT_POINTLIST, 0, 4);
}

void CWorld::_renderContinentLines()
{
	const int vertexCount = m_continent->vertices.size();
	if (vertexCount == 0)
		return;

	static Color3DVertex vertices[MAX_CONTINENT_VERTICES];

	int i;
	if (vertexCount > 1)
	{
		for (i = 0; i < vertexCount; i++)
		{
			vertices[i].c = (i == vertexCount - 1) ? 0xffffffff : 0xff000000;
			vertices[i].p = m_continent->vertices[i];
		}

		vertices[i].c = 0xff000000;
		vertices[i].p = m_continent->vertices[0];

		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		m_device->DrawPrimitiveUP(D3DPT_LINESTRIP, vertexCount, vertices, sizeof(Color3DVertex));
		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	}

	for (i = 0; i < vertexCount; i++)
	{
		vertices[i].c = 0xffff0000;
		vertices[i].p = m_continent->vertices[i];
	}

	const float pointSize = 8.0f;
	m_device->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pointSize));
	m_device->DrawPrimitiveUP(D3DPT_POINTLIST, vertexCount, vertices, sizeof(Color3DVertex));
}

void CWorld::_renderPaths()
{
	for (auto it = m_paths.begin(); it != m_paths.end(); it++)
	{
		if (it.value()->GetSize() > 0)
			_renderPath(it.key(), D3DXVECTOR3(it.value()->GetAt(0)->m_pos));
	}

	CMover* mover;
	for (int i = 0; i < s_selection.GetSize(); i++)
	{
		if (s_selection[i]->m_type == OT_MOVER)
		{
			mover = (CMover*)s_selection[i];
			if (mover->m_patrolIndex != -1)
				_renderPath(mover->m_patrolIndex, mover->m_pos);
		}
	}
}

void CWorld::_renderPath(int index, const D3DXVECTOR3& origin)
{
	auto path = m_paths.find(index);
	if (path == m_paths.end())
		return;

	static Color3DVertex vertices[MAX_PATH_VERTICES];

	CPtrArray<CPath>* paths = path.value();
	if (paths->GetSize() <= 0)
		return;

	const D3DXVECTOR3 pos0 = paths->GetAt(0)->m_pos;

	if (paths->GetSize() > 1)
	{
		for (int i = 0; i < paths->GetSize(); i++)
		{
			vertices[i].c = 0xffffff00;
			vertices[i].p = origin + (paths->GetAt(i)->m_pos - pos0);
		}

		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		m_device->DrawPrimitiveUP(D3DPT_LINESTRIP, paths->GetSize() - 1, vertices, sizeof(Color3DVertex));
		m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	}

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	D3DXVECTOR3 out;
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	CPath* obj;
	int plane;
	bool render;

	for (int i = 0; i < paths->GetSize(); i++)
	{
		obj = paths->GetAt(i);
		out = origin + (obj->m_pos - pos0);
		render = true;

		for (plane = 0; plane < 6; plane++)
		{
			if (g_planeFrustum[plane].a * out.x +
				g_planeFrustum[plane].b * out.y +
				g_planeFrustum[plane].c * out.z +
				g_planeFrustum[plane].d < 0)
			{
				render = false;
				break;
			}
		}

		if (render)
		{
			D3DXVec3Project(&out, &out, &g_global3D.viewport, &g_global3D.proj, &g_global3D.view, &identity);
			const string name = string::number(obj->m_index);
			CWorld::s_objNameFont->Render(name,
				QPoint(((int)out.x) - CWorld::s_objNameFont->GetSize(name).width() / 2 - 2, ((int)out.y)),
				0xffff00ff);
		}
	}

	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetTexture(0, null);
	m_device->SetFVF(Color3DVertex::FVF);
}

void CWorld::_applyCamera()
{
	D3DXMATRIX proj, view, invView, mat, world;

	if (g_global3D.renderMinimap)
	{
		D3DXMatrixOrthoLH(&proj, (float)(MAP_SIZE * MPU), (float)(MAP_SIZE * MPU), 1.0f, g_global3D.farPlane);
		D3DXVECTOR3 lookAt = m_cameraPos;
		lookAt.y -= 1.0f;
		D3DXMatrixLookAtLH(&view, &m_cameraPos, &lookAt, &D3DXVECTOR3(0, 0, 1));
	}
	else
	{
		D3DXMatrixPerspectiveFovLH(&proj, M_PI_4, (float)g_global3D.viewport.Width / (float)g_global3D.viewport.Height, 1.0f, g_global3D.farPlane);
		const float phiRadian = m_cameraAngle.y * M_PI / 180.0f;
		const float thetaRadian = m_cameraAngle.x * M_PI / 180.0f;
		D3DXVECTOR3 lookAt;
		lookAt.x = cos(phiRadian) * sin(thetaRadian);
		lookAt.y = sin(phiRadian);
		lookAt.z = cos(phiRadian) * cos(thetaRadian);
		lookAt += m_cameraPos;
		D3DXMatrixLookAtLH(&view, &m_cameraPos, &lookAt, &D3DXVECTOR3(0, 1, 0));
	}

	D3DXMatrixIdentity(&world);

	g_global3D.cameraPos = m_cameraPos;
	g_global3D.view = view;
	g_global3D.proj = proj;

	m_device->SetTransform(D3DTS_VIEW, &view);
	m_device->SetTransform(D3DTS_PROJECTION, &proj);
	m_device->SetTransform(D3DTS_WORLD, &world);

	D3DXMatrixInverse(&invView, null, &view);
	invView._41 = 0.0f; invView._42 = 0.0f; invView._43 = 0.0f;
	g_global3D.invView = invView;

	D3DXMatrixMultiply(&mat, &view, &proj);
	D3DXMatrixInverse(&mat, null, &mat);

	D3DXVECTOR3 vecFrustum[8];
	vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f, 0.0f); // xyz
	vecFrustum[1] = D3DXVECTOR3(1.0f, -1.0f, 0.0f); // Xyz
	vecFrustum[2] = D3DXVECTOR3(-1.0f, 1.0f, 0.0f); // xYz
	vecFrustum[3] = D3DXVECTOR3(1.0f, 1.0f, 0.0f); // XYz
	vecFrustum[4] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f); // xyZ
	vecFrustum[5] = D3DXVECTOR3(1.0f, -1.0f, 1.0f); // XyZ
	vecFrustum[6] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f); // xYZ
	vecFrustum[7] = D3DXVECTOR3(1.0f, 1.0f, 1.0f); // XYZ

	for (uint i = 0; i < 8; i++)
		D3DXVec3TransformCoord(&vecFrustum[i], &vecFrustum[i], &mat);

	D3DXPlaneFromPoints(&g_planeFrustum[0], &vecFrustum[0],
		&vecFrustum[1], &vecFrustum[2]); // Near
	D3DXPlaneFromPoints(&g_planeFrustum[1], &vecFrustum[6],
		&vecFrustum[7], &vecFrustum[5]); // Far
	D3DXPlaneFromPoints(&g_planeFrustum[2], &vecFrustum[2],
		&vecFrustum[6], &vecFrustum[4]); // Left
	D3DXPlaneFromPoints(&g_planeFrustum[3], &vecFrustum[7],
		&vecFrustum[3], &vecFrustum[5]); // Right
	D3DXPlaneFromPoints(&g_planeFrustum[4], &vecFrustum[2],
		&vecFrustum[3], &vecFrustum[6]); // Top
	D3DXPlaneFromPoints(&g_planeFrustum[5], &vecFrustum[1],
		&vecFrustum[0], &vecFrustum[4]); // Bottom
}

void CWorld::_cullObjects()
{
	static const float objDistant[4] = { 400, 200, 70, 400 };

	m_cullObjCount = 0;
	m_cullSfxCount = 0;

	CPtrArray<CObject>* objArray;
	CObject* obj;
	int x, z, i, j, k, l;
	WorldPosToLand(m_cameraPos, x, z);
	bool show;

	if (g_global3D.renderObjects)
	{
		for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
		{
			for (j = x - m_visibilityLand; j <= (x + m_visibilityLand); j++)
			{
				if (LandInWorld(j, i))
				{
					const int offset = i * m_width + j;
					if (m_lands[offset])
					{
						for (k = 0; k < MAX_OBJTYPE; k++)
						{
							show = false;

							switch (k)
							{
							case OT_OBJ:
							case OT_SHIP:
								show = g_global3D.renderObj;
								break;
							case OT_SFX:
								show = g_global3D.renderSFX;
								break;
							case OT_ITEM:
								show = g_global3D.renderItem;
								break;
							case OT_CTRL:
								show = g_global3D.renderCtrl;
								break;
							case OT_PATH:
							case OT_MOVER:
								show = true;
								break;
							case OT_REGION:
								show = g_global3D.renderRegions;
								break;
							}

							if (!show)
								continue;

							objArray = &m_lands[offset]->m_objects[k];
							for (l = 0; l < objArray->GetSize(); l++)
							{
								obj = objArray->GetAt(l);

								if (k == OT_MOVER)
								{
									const bool peaceful = ((CMover*)obj)->IsPeaceful();
									if (peaceful && !g_global3D.renderNPC)
										continue;
									if (!peaceful && !g_global3D.renderMonster)
										continue;
								}
								else if (k == OT_PATH)
								{
									if (!g_global3D.renderNPC && !g_global3D.renderMonster)
										continue;
								}

								if (obj->m_isUnvisible)
									continue;

								obj->m_distToCamera = D3DXVec3Length(&(m_cameraPos - obj->m_pos));

								if (g_global3D.renderMinimap)
								{
									if (obj->m_modelProp->distant == MD_NEAR || !obj->m_isReal)
										continue;
								}
								else
								{
									if (g_global3D.objectLOD)
									{
										if ((obj->m_type == OT_SFX || obj->m_type == OT_OBJ) && obj->m_isReal)
										{
											if (obj->m_distToCamera > objDistant[obj->m_modelProp->distant])
												continue;
										}
										else
										{
											if (obj->m_distToCamera > objDistant[MD_FAR])
												continue;
										}
									}
								}

								obj->Cull();

								if (obj->m_visible)
								{
									if (obj->m_modelProp->modelType == MODELTYPE_SFX)
									{
										if (m_cullSfxCount < MAX_DISPLAYSFX)
										{
											m_sfxCull[m_cullSfxCount] = obj;
											m_cullSfxCount++;
										}
									}
									else
									{
										if (m_cullObjCount < MAX_DISPLAYOBJ)
										{
											m_objCull[m_cullObjCount] = obj;
											m_cullObjCount++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (m_cullObjCount > 0)
		std::sort(m_objCull, m_objCull + m_cullObjCount, ObjSortFarToNear);
}

float CWorld::GetHeight(float x, float z) const
{
	if (!VecInWorld(x, z))
		return 0.0f;

	x /= (float)m_MPU;
	z /= (float)m_MPU;

	int mX = int(x / MAP_SIZE);
	int mZ = int(z / MAP_SIZE);
	if (x == m_width * MAP_SIZE)
		mX--;
	if (z == m_height * MAP_SIZE)
		mZ--;

	CLandscape* land = m_lands[mX + mZ * m_width];
	if (!land)
		return 0.0f;

	return land->GetHeight(x - (mX * MAP_SIZE), z - (mZ * MAP_SIZE));
}

float CWorld::GetHeight_fast(float x, float z) const
{
	if (!VecInWorld(x, z))
		return 0.0f;

	x /= (float)m_MPU;
	z /= (float)m_MPU;

	int mX = int(x / MAP_SIZE);
	int mZ = int(z / MAP_SIZE);
	if (x == m_width * MAP_SIZE)
		mX--;
	if (z == m_height * MAP_SIZE)
		mZ--;

	CLandscape* land = m_lands[mX + mZ * m_width];
	if (!land)
		return 0.0f;

	return land->GetHeight(((int)x - (mX * MAP_SIZE)) + ((int)z - (mZ * MAP_SIZE)) * (MAP_SIZE + 1));
}

float CWorld::GetHeightAttribute(float x, float z) const
{
	if (!VecInWorld(x, z))
		return 0.0f;

	x /= (float)m_MPU;
	z /= (float)m_MPU;

	int mX = int(x / MAP_SIZE);
	int mZ = int(z / MAP_SIZE);
	if (x == m_width * MAP_SIZE)
		mX--;
	if (z == m_height * MAP_SIZE)
		mZ--;

	CLandscape* land = m_lands[mX + mZ * m_width];
	if (!land)
		return 0.0f;

	return land->GetHeightAttribute(((int)x - (mX * MAP_SIZE)) + ((int)z - (mZ * MAP_SIZE)) * (MAP_SIZE + 1));
}

WaterHeight* CWorld::GetWaterHeight(float x, float z)
{
	if (!VecInWorld(x, z))
		return null;

	x /= (float)m_MPU;
	z /= (float)m_MPU;

	int mX = int(x / MAP_SIZE);
	int mZ = int(z / MAP_SIZE);
	if (x == m_width * MAP_SIZE)
		mX--;
	if (z == m_height * MAP_SIZE)
		mZ--;

	CLandscape* land = m_lands[mX + mZ * m_width];
	if (!land)
		return null;

	const int tx = ((int)x % MAP_SIZE) / PATCH_SIZE;
	const int tz = ((int)z % MAP_SIZE) / PATCH_SIZE;

	return land->GetWaterHeight(tx, tz);
}

bool CWorld::InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->CreateIndexBuffer((128 + 48 + 32 + 16 + 8 + 4 + 2) * 3 * sizeof(ushort),
		D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &s_IB, null)))
		return false;

	const ushort tempTriList[] = {
		0, 10, 1, 1, 10, 2, 2, 10, 11, 2, 11, 12, 2, 12, 3, 3, 12, 4, 4, 12, 13, 4, 13, 14, 4, 14, 5, 5, 14, 6, 6, 14, 15, 6, 15, 16, 6, 16, 7, 7, 16, 8,
		10, 20, 11, 11, 20, 12, 12, 20, 21, 12, 21, 22, 12, 22, 13, 13, 22, 14, 14, 22, 23, 14, 23, 24, 14, 24, 15, 15, 24, 16,
		20, 30, 21, 21, 30, 22, 22, 30, 31, 22, 31, 32, 22, 32, 23, 23, 32, 24, 30, 40, 31, 31, 40, 32,
		8, 16, 17, 16, 24, 25, 16, 25, 26, 16, 26, 17, 24, 32, 33, 24, 33, 34, 24, 34, 25, 25, 34, 26, 26, 34, 35,
		32, 40, 41, 32, 41, 42, 32, 42, 33, 33, 42, 34, 34, 42, 43, 34, 43, 44, 34, 44, 35, 40, 50, 41, 41, 50, 42, 42, 50, 51, 42, 51, 52, 42, 52, 43, 43, 52, 44, 44, 52, 53,
		50, 60, 51, 51, 60, 52, 52, 60, 61, 52, 61, 62, 52, 62, 53, 60, 70, 61, 61, 70, 62, 62, 70, 71, 70, 80, 71,
		0, 9, 10, 9, 18, 10, 10, 18, 19, 10, 19, 20, 18, 27, 28, 18, 28, 19, 19, 28, 20, 20, 28, 29, 20, 29, 30,
		27, 36, 28, 28, 36, 37, 28, 37, 38, 28, 38, 29, 29, 38, 30, 30, 38, 39, 30, 39, 40, 36, 45, 46, 36, 46, 37, 37, 46, 38, 38, 46, 47, 38, 47, 48, 38, 48, 39, 39, 48, 40,
		45, 54, 46, 46, 54, 55, 46, 55, 56, 46, 56, 47, 47, 56, 48, 54, 63, 64, 54, 64, 55, 55, 64, 56, 63, 72, 64,
		48, 49, 40, 40, 49, 50, 48, 56, 57, 48, 57, 58, 48, 58, 49, 49, 58, 50, 50, 58, 59, 50, 59, 60,
		56, 64, 65, 56, 65, 66, 56, 66, 57, 57, 66, 58, 58, 66, 67, 58, 67, 68, 58, 68, 59, 59, 68, 60, 60, 68, 69, 60, 69, 70,
		64, 72, 73, 64, 73, 74, 64, 74, 65, 65, 74, 66, 66, 74, 75, 66, 75, 76, 66, 76, 67, 67, 76, 68, 68, 76, 77, 68, 77, 78, 68, 78, 69, 69, 78, 70, 70, 78, 79, 70, 79, 80,
		0, 10, 2, 2, 10, 20, 2, 20, 4, 4, 20, 22, 4, 22, 24, 4, 24, 6, 6, 24, 16, 6, 16, 8,
		20, 30, 22, 22, 30, 40, 22, 40, 32, 22, 32, 24,
		8, 16, 26, 16, 24, 26, 32, 40, 42, 32, 42, 24, 24, 42, 44, 24, 44, 26,
		40, 50, 42, 42, 50, 60, 42, 60, 44, 44, 60, 62, 60, 70, 62, 62, 70, 80,
		0, 18, 10, 10, 18, 20, 20, 18, 36, 20, 36, 38, 20, 38, 30, 30, 38, 40,
		36, 54, 56, 36, 56, 38, 38, 56, 48, 38, 48, 40, 54, 72, 64, 54, 64, 56,
		48, 56, 58, 48, 58, 40, 50, 40, 58, 50, 58, 60,
		64, 72, 74, 64, 74, 56, 56, 74, 76, 58, 56, 76, 58, 76, 60, 60, 76, 78, 60, 78, 70, 70, 78, 80,
		0, 20, 2, 2, 20, 4, 4, 20, 22, 4, 22, 24, 4, 24, 6, 6, 24, 8, 20, 40, 22, 22, 40, 24,
		8, 24, 26, 24, 40, 42, 24, 42, 44, 24, 44, 26, 40, 60, 42, 42, 60, 44, 44, 60, 62, 60, 80, 62,
		0, 18, 20, 18, 36, 20, 20, 36, 38, 20, 38, 40, 36, 54, 56, 36, 56, 38, 38, 56, 40, 54, 72, 56,
		40, 56, 58, 40, 58, 60, 56, 72, 74, 56, 74, 76, 56, 76, 58, 58, 76, 60, 60, 76, 78, 60, 78, 80,
		4, 0, 20, 4, 20, 40, 4, 40, 24, 4, 24, 8,
		44, 8, 24, 44, 24, 40, 44, 40, 60, 44, 60, 80,
		36, 72, 56, 36, 56, 40, 36, 40, 20, 36, 20, 0,
		76, 80, 60, 76, 60, 40, 76, 40, 56, 76, 56, 72,
		4, 0, 40, 4, 40, 8,
		44, 8, 40, 44, 40, 80,
		36, 72, 40, 36, 40, 0,
		76, 80, 40, 76, 40, 72,
		0, 40, 8,
		8, 40, 80,
		0, 72, 40,
		40, 72, 80,

		// LOD 3
		0, (PATCH_SIZE + 1) * PATCH_SIZE, PATCH_SIZE, PATCH_SIZE, (PATCH_SIZE + 1) * PATCH_SIZE, (PATCH_SIZE + 1)*(PATCH_SIZE + 1) - 1
	};

	ushort* IB;
	s_IB->Lock(0, 0, (void**)&IB, 0);
	memcpy(IB, tempTriList, (128 + 48 + 32 + 16 + 8 + 4 + 2) * 3 * sizeof(WORD));
	s_IB->Unlock();
	return true;
}

void CWorld::DeleteStaticDeviceObjects()
{
	Release(s_IB);
}

bool CWorld::RestoreStaticDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->CreateVertexBuffer(12 * 2 * sizeof(Color3DVertex),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, Color3DVertex::FVF, D3DPOOL_DEFAULT, &s_bbVB, null)))
		return false;

	return SUCCEEDED(device->CreateIndexBuffer(NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE * (PATCH_SIZE + 1) * (PATCH_SIZE + 1) * 4 * sizeof(ushort),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &s_gridIB, null));
}

void CWorld::InvalidateStaticDeviceObjects()
{
	Release(s_gridIB);
	Release(s_bbVB);
}