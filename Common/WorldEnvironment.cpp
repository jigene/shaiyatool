///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "World.h"
#include "ModelMng.h"
#include "Skybox.h"

LightColor CWorld::s_light[24] =
{
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f,
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f,
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f,
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f,
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f,
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f,
	0.5f, 0.5f, 0.6f, 0.4f, 0.4f, 0.4f,
	0.7f, 0.7f, 0.7f, 0.5f, 0.5f, 0.5f,
	0.8f, 0.8f, 0.8f, 0.5f, 0.5f, 0.5f,
	0.9f, 0.9f, 0.9f, 0.5f, 0.5f, 0.5f,
	1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f,
	1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f,
	1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f,
	1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f,
	1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f,
	1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f,
	0.9f, 0.9f, 0.9f, 0.5f, 0.5f, 0.5f,
	0.9f, 0.6f, 0.2f, 0.5f, 0.5f, 0.4f,
	0.6f, 0.6f, 0.4f, 0.4f, 0.4f, 0.4f,
	0.5f, 0.5f, 0.4f, 0.4f, 0.4f, 0.4f,
	0.45f, 0.45f, 0.4f, 0.35f, 0.35f, 0.35f,
	0.43f, 0.43f, 0.5f, 0.33f, 0.33f, 0.3f,
	0.41f, 0.41f, 0.5f, 0.31f, 0.31f, 0.3f,
	0.4f, 0.4f, 0.5f, 0.3f, 0.3f, 0.4f
};

void CWorld::_setLight(bool sendToDevice)
{
	D3DLIGHT9 light;
	memset(&light, 0, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Attenuation0 = 0.0f;
	light.Range = 40.0f;
	light.Specular.r = 2.0f;
	light.Specular.r = 2.0f;
	light.Specular.r = 2.0f;
	light.Position.x = 0.0f;
	light.Position.y = 150.0f;
	light.Position.z = 0.0f;

	if (g_global3D.editionLight)
	{
		m_fogColor = D3DCOLOR_ARGB(255, 127, 127, 153);

		light.Ambient.r = light.Ambient.g = light.Ambient.b = (81.0f / 255.0f) * 0.9f;
		light.Diffuse.r = light.Diffuse.g = light.Diffuse.b = (113.0f / 255.0f) + 0.1f;

		D3DXVECTOR3 vec(0.0f, -0.5f, -1.0f);
		D3DXVec3Normalize(&vec, &vec);
		light.Direction.x = vec.x;
		light.Direction.y = vec.y;
		light.Direction.z = vec.z;
	}
	else
	{
		if (m_continent && m_continent->useEnvir)
		{
			m_fogColor = D3DCOLOR_ARGB(255,
				(int)(m_continent->diffuse.x * 255.0f),
				(int)(m_continent->diffuse.y * 255.0f),
				(int)(m_continent->diffuse.z * 255.0f));

			light.Diffuse.r = m_continent->diffuse.x * 1.2f;
			light.Diffuse.g = m_continent->diffuse.y * 1.2f;
			light.Diffuse.b = m_continent->diffuse.z * 1.2f;

			light.Ambient.r = m_continent->ambient.x * 0.8f;
			light.Ambient.g = m_continent->ambient.y * 0.8f;
			light.Ambient.b = m_continent->ambient.z * 0.8f;

			D3DXVECTOR3 vecSun = D3DXVECTOR3(0.0f, 0.5f, 0.5f);
			D3DXVec3Normalize(&vecSun, &vecSun);
			light.Direction.x = -vecSun.x;
			light.Direction.y = -vecSun.y;
			light.Direction.z = -vecSun.z;
		}
		else
		{
			if (m_inDoor)
			{
				m_fogColor = D3DCOLOR_ARGB(255,
					(m_diffuse >> 16) & 0xff,
					(m_diffuse >> 8) & 0xff,
					m_diffuse & 0xff);

				light.Diffuse.r = ((float)((m_diffuse >> 16) & 0xff) / 255.f) + 0.1f;
				light.Diffuse.g = ((float)((m_diffuse >> 8) & 0xff) / 255.f) + 0.1f;
				light.Diffuse.b = ((float)(m_diffuse & 0xff) / 255.f) + 0.1f;

				light.Ambient.r = ((float)((m_ambient >> 16) & 0xff) / 255.f) * 0.9f;
				light.Ambient.g = ((float)((m_ambient >> 8) & 0xff) / 255.f) * 0.9f;
				light.Ambient.b = ((float)(m_ambient & 0xff) / 255.f) * 0.9f;

				light.Direction.x = m_lightDir.x;
				light.Direction.y = m_lightDir.y;
				light.Direction.z = m_lightDir.z;
			}
			else
			{
				int hour = g_global3D.hour;
				hour--;
				if (hour < 0)
					hour = 0;
				if (hour > 23)
					hour = 23;

				LightColor lightColorPrev = s_light[(hour - 1 == -1) ? 23 : hour - 1];
				const LightColor lightColor = s_light[hour];
				lightColorPrev.r1 += (lightColor.r1 - lightColorPrev.r1) * g_global3D.min / 60;
				lightColorPrev.g1 += (lightColor.g1 - lightColorPrev.g1) * g_global3D.min / 60;
				lightColorPrev.b1 += (lightColor.b1 - lightColorPrev.b1) * g_global3D.min / 60;
				lightColorPrev.r2 += (lightColor.r2 - lightColorPrev.r2) * g_global3D.min / 60;
				lightColorPrev.g2 += (lightColor.g2 - lightColorPrev.g2) * g_global3D.min / 60;
				lightColorPrev.b2 += (lightColor.b2 - lightColorPrev.b2) * g_global3D.min / 60;

				m_fogColor = D3DCOLOR_ARGB(255,
					(int)(lightColorPrev.r1 * 255.0f),
					(int)(lightColorPrev.g1 * 255.0f),
					(int)(lightColorPrev.b1 * 255.0f));

				light.Diffuse.r = lightColorPrev.r1 * 1.1f;
				light.Diffuse.g = lightColorPrev.g1 * 1.1f;
				light.Diffuse.b = lightColorPrev.b1 * 1.1f;

				light.Ambient.r = lightColorPrev.r2 * 0.9f;
				light.Ambient.g = lightColorPrev.g2 * 0.9f;
				light.Ambient.b = lightColorPrev.b2 * 0.9f;

				int temp2 = g_global3D.hour;
				if (g_global3D.hour >= 1 && g_global3D.hour <= 6)
					temp2 = 24 + g_global3D.hour;

				const uint secCount = temp2 * 24 * 60 + g_global3D.min * 60 * g_global3D.sec;
				const float sunAngle = (secCount * 0.00015f) - 90.0f;

				D3DXVECTOR3 vecSun = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				D3DXMATRIX  matTemp;
				D3DXMatrixRotationX(&matTemp, (sunAngle + 180.0f) * (3.1415926f / 180.f));
				D3DXVec3TransformCoord(&vecSun, &vecSun, &matTemp);

				light.Direction.x = vecSun.x;
				light.Direction.y = vecSun.y;
				light.Direction.z = vecSun.z;
			}
		}
	}

	if (sendToDevice)
	{
		if (g_global3D.light)
		{
			m_device->SetLight(0, &light);
			m_device->LightEnable(0, TRUE);
			m_device->SetRenderState(D3DRS_LIGHTING, TRUE);

			g_global3D.lightVec = D3DXVECTOR4(light.Direction.x, light.Direction.y, light.Direction.z, 0.0f);
			const D3DXVECTOR4 diffuse(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b, 1.0f);
			m_device->SetVertexShaderConstantF(93, (float*)&diffuse, 1);
			const D3DXVECTOR4 ambient(light.Ambient.r, light.Ambient.g, light.Ambient.b, 1.0f);
			m_device->SetVertexShaderConstantF(94, (float*)&ambient, 1);
		}
		else
		{
			m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

			g_global3D.lightVec = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
			const D3DXVECTOR4 nullVec(1.0f, 1.0f, 1.0f, 0.0f);
			m_device->SetVertexShaderConstantF(93, (float*)&nullVec, 1);
			m_device->SetVertexShaderConstantF(94, (float*)&nullVec, 1);
		}

		D3DMATERIAL9 material;
		material.Diffuse.r = 1.0f;
		material.Diffuse.g = 1.0f;
		material.Diffuse.b = 1.0f;
		material.Diffuse.a = 1.0f;
		material.Ambient.r = 1.0f;
		material.Ambient.g = 1.0f;
		material.Ambient.b = 1.0f;
		material.Ambient.a = 1.0f;
		material.Specular.r = 1.0f;
		material.Specular.g = 1.0f;
		material.Specular.b = 1.0f;
		material.Specular.a = 1.0f;
		material.Emissive.r = 0.0f;
		material.Emissive.g = 0.0f;
		material.Emissive.b = 0.0f;
		material.Emissive.a = 0.0f;
		material.Power = 50.0f;
		m_device->SetMaterial(&material);

		m_device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(255,
			(int)(light.Ambient.r * 255.0f),
			(int)(light.Ambient.g * 255.0f),
			(int)(light.Ambient.b * 255.0f)));
	}

	if (g_global3D.hour >= 21 || g_global3D.hour <= 6 || g_global3D.editionLight)
		g_global3D.night = true;
	else
		g_global3D.night = false;
}

void CWorld::_setFog()
{
	if (g_global3D.fog)
	{
		m_device->SetRenderState(D3DRS_FOGCOLOR, m_fogColor);

		if (m_continent && m_continent->useEnvir)
		{
			m_device->SetRenderState(D3DRS_FOGSTART, *((DWORD*)&m_continent->fogStart));
			m_device->SetRenderState(D3DRS_FOGEND, *((DWORD*)&m_continent->fogEnd));
		}
		else
		{
			m_device->SetRenderState(D3DRS_FOGSTART, *((DWORD*)&m_fogStart));
			m_device->SetRenderState(D3DRS_FOGEND, *((DWORD*)&m_fogEnd));
		}

		m_device->SetRenderState(D3DRS_FOGDENSITY, *((DWORD*)&m_fogDensity));
		m_device->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
		m_device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
		m_device->SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
		m_device->SetRenderState(D3DRS_FOGENABLE, TRUE);
	}
	else
	{
		m_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	}

	const D3DXVECTOR4 fogConst(1.0f, 1.0f, 1.0f, 100.0f);
	m_device->SetVertexShaderConstantF(95, (float*)&fogConst, 1);
}

void CWorld::UpdateContinent()
{
	const Continent* oldContinent = m_continent;
	m_continent = null;

	int i, j, counter = 0;
	D3DXVECTOR3 p1, p2;
	float xinters;
	Continent* continent;
	for (i = 0; i < m_continents.GetSize(); i++)
	{
		continent = m_continents[i];
		if (!continent || !continent->useEnvir)
			continue;

		const QVector<D3DXVECTOR3>& vertices = continent->vertices;

		p1 = vertices[0];
		for (j = 1; j <= vertices.size(); j++)
		{
			p2 = vertices[j % vertices.size()];
			if (m_cameraPos.z > (p1.z < p2.z ? p1.z : p2.z))
			{
				if (m_cameraPos.z <= (p1.z > p2.z ? p1.z : p2.z))
				{
					if (m_cameraPos.x <= (p1.x > p2.x ? p1.x : p2.x))
					{
						if (p1.z != p2.z)
						{
							xinters = (m_cameraPos.z - p1.z) * (p2.x - p1.x) / (p2.z - p1.z) + p1.x;
							if (p1.x == p2.x || m_cameraPos.x <= xinters)
								counter++;
						}
					}
				}
			}
			p1 = p2;
		}

		if (counter % 2 != 0)
		{
			m_continent = continent;
			break;
		}
	}

	if (m_continent != oldContinent && m_skybox)
	{
		_setLight(false);
		m_skybox->LoadTextures();
	}
}