///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Skybox.h"
#include "World.h"
#include "TextureMng.h"
#include "ModelMng.h"

struct SkyboxVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 };
	D3DXVECTOR3 p;
	DWORD c;
	float tu1, tv1;
};

LPDIRECT3DVERTEXBUFFER9 CSkybox::s_topVB = null;
LPDIRECT3DVERTEXBUFFER9 CSkybox::s_cloudVB = null;

CSkybox::CSkybox(LPDIRECT3DDEVICE9 device, CWorld* world)
	: m_device(device),
	m_world(world),
	m_moonTexture(null),
	m_sunTexture(null),
	m_threeTexturesCloud(false),
	m_threeTexturesSky(false),
	m_cloudPos(0.0f)
{
	memset(m_topTextures, 0, sizeof(m_topTextures));
	memset(m_cloudTextures, 0, sizeof(m_cloudTextures));

	LoadTextures();
}

CSkybox::~CSkybox()
{
}

void CSkybox::Render()
{
	const float phiRadian = m_world->m_cameraAngle.y * M_PI / 180.0f;
	const float thetaRadian = m_world->m_cameraAngle.x * M_PI / 180.0f;
	D3DXVECTOR3 vecLookAt;
	vecLookAt.x = cos(phiRadian) * sin(thetaRadian);
	vecLookAt.y = sin(phiRadian);
	vecLookAt.z = cos(phiRadian) * cos(thetaRadian);

	float angle1 = (float)(atan(vecLookAt.x / vecLookAt.z) * 180 / 3.1415926f);
	if (vecLookAt.z < 0)
		angle1 += 180;

	D3DXMATRIX mat, matView;
	D3DXVECTOR3 vecPos(0.0f, 0.0f, 0.0f);
	const float temp = -angle1 / 360.0f;

	vecLookAt.z = (float)(sqrt((vecLookAt.x*vecLookAt.x) + (vecLookAt.z*vecLookAt.z)));
	vecLookAt.x = 0;
	D3DXMatrixRotationY(&mat, angle1*3.1415926f / 180.0f);
	D3DXVec3TransformCoord(&vecLookAt, &vecLookAt, &mat);
	vecLookAt.y += m_world->m_cameraPos.y;
	vecPos.y += m_world->m_cameraPos.y;
	D3DXMatrixLookAtLH(&matView, &vecPos, &vecLookAt, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	matView._41 = 0; matView._42 = 0; matView._43 = 0;
	m_device->SetTransform(D3DTS_VIEW, &matView);

	m_device->SetFVF(SkyboxVertex::FVF);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	m_device->SetStreamSource(0, s_topVB, 0, sizeof(SkyboxVertex));
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));

	if (m_threeTexturesSky)
	{
		if (g_global3D.hour < 6)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			if (m_topTextures[2])
				m_device->SetTexture(0, *m_topTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
		}
		else if (g_global3D.hour == 6)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255 - (g_global3D.min * 255 / 59), 0, 0, 0));
			if (m_topTextures[2])
				m_device->SetTexture(0, *m_topTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);

			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(g_global3D.min * 255 / 59, 0, 0, 0));
			if (m_topTextures[0])
				m_device->SetTexture(0, *m_topTextures[0]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
		}
		else if (g_global3D.hour < 17)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			if (m_topTextures[0])
				m_device->SetTexture(0, *m_topTextures[0]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
		}
		else if (g_global3D.hour == 17 || g_global3D.hour == 18)
		{
			const int min = g_global3D.hour == 17 ? g_global3D.min : g_global3D.min + 60;
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255 - (min * 255 / 119), 0, 0, 0));
			if (m_topTextures[0])
				m_device->SetTexture(0, *m_topTextures[0]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);

			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(min * 255 / 119, 0, 0, 0));
			if (m_topTextures[1])
				m_device->SetTexture(0, *m_topTextures[1]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
		}
		else if (g_global3D.hour == 19)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255 - (g_global3D.min * 255 / 59), 0, 0, 0));
			if (m_topTextures[1])
				m_device->SetTexture(0, *m_topTextures[1]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);

			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(g_global3D.min * 255 / 59, 0, 0, 0));
			if (m_topTextures[2])
				m_device->SetTexture(0, *m_topTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
		}
		else
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			if (m_topTextures[2])
				m_device->SetTexture(0, *m_topTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
		}

	}
	else
	{
		m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
		if (m_topTextures[0])
			m_device->SetTexture(0, *m_topTextures[0]);
		else
			m_device->SetTexture(0, null);
		m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 348 / 3);
	}

	if (g_global3D.animate)
		m_cloudPos += 0.0002f;

	D3DXMatrixIdentity(&mat);
	mat._31 = m_cloudPos;
	mat._32 = m_cloudPos;
	m_device->SetTransform(D3DTS_TEXTURE0, &mat);
	m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	m_device->SetStreamSource(0, s_cloudVB, 0, sizeof(SkyboxVertex));
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));

	if (m_threeTexturesCloud)
	{
		if (g_global3D.hour < 6)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			if (m_cloudTextures[2])
				m_device->SetTexture(0, *m_cloudTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
		}
		else if (g_global3D.hour == 6)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255 - (g_global3D.min * 255 / 59), 0, 0, 0));
			if (m_cloudTextures[2])
				m_device->SetTexture(0, *m_cloudTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);

			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(g_global3D.min * 255 / 59, 0, 0, 0));
			if (m_cloudTextures[0])
				m_device->SetTexture(0, *m_cloudTextures[0]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
		}
		else if (g_global3D.hour < 17)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			if (m_cloudTextures[0])
				m_device->SetTexture(0, *m_cloudTextures[0]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
		}
		else if (g_global3D.hour == 17 || g_global3D.hour == 18)
		{
			const int min = g_global3D.hour == 17 ? g_global3D.min : g_global3D.min + 60;
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255 - (min * 255 / 119), 0, 0, 0));
			if (m_cloudTextures[0])
				m_device->SetTexture(0, *m_cloudTextures[0]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);

			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(min * 255 / 119, 0, 0, 0));
			if (m_cloudTextures[1])
				m_device->SetTexture(0, *m_cloudTextures[1]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
		}
		else if (g_global3D.hour == 19)
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255 - (g_global3D.min * 255 / 59), 0, 0, 0));
			if (m_cloudTextures[1])
				m_device->SetTexture(0, *m_cloudTextures[1]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);

			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(g_global3D.min * 255 / 59, 0, 0, 0));
			if (m_cloudTextures[2])
				m_device->SetTexture(0, *m_cloudTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
		}
		else
		{
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			if (m_cloudTextures[2])
				m_device->SetTexture(0, *m_cloudTextures[2]);
			else
				m_device->SetTexture(0, null);
			m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
		}
	}
	else
	{
		m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
		if (m_cloudTextures[0])
			m_device->SetTexture(0, *m_cloudTextures[0]);
		else
			m_device->SetTexture(0, null);
		m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 28);
	}

	m_device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 255, 255, 255));

	int temp2 = g_global3D.hour;
	if (g_global3D.hour >= 1 && g_global3D.hour <= 6)
		temp2 = 24 + g_global3D.hour;

	const uint secCount = temp2 * 24 * 60 + g_global3D.min * 60 * g_global3D.sec;
	const float sunAngle = (secCount * 0.00015f) - 90.0f;

	if ((g_global3D.hour >= 20) || (g_global3D.hour >= 1 && g_global3D.hour <= 6))
	{
		SkyboxVertex vertices[4];
		vertices[0].p = D3DXVECTOR3(-2.0f, 2.0f, 30.0f);
		vertices[0].tu1 = 0.0f;
		vertices[0].tv1 = 0.0f;
		vertices[1].p = D3DXVECTOR3(2.0f, 2.0f, 30.0f);
		vertices[1].tu1 = 1.0f;
		vertices[1].tv1 = 0.0f;
		vertices[2].p = D3DXVECTOR3(2.0f, -2.0f, 30.0f);
		vertices[2].tu1 = 1.0f;
		vertices[2].tv1 = 1.0f;
		vertices[3].p = D3DXVECTOR3(-2.0f, -2.0f, 30.0f);
		vertices[3].tu1 = 0.0f;
		vertices[3].tv1 = 1.0f;

		D3DXMatrixRotationX(&mat, sunAngle);
		D3DXVec3TransformCoord(&vertices[0].p, &vertices[0].p, &mat);
		D3DXVec3TransformCoord(&vertices[1].p, &vertices[1].p, &mat);
		D3DXVec3TransformCoord(&vertices[2].p, &vertices[2].p, &mat);
		D3DXVec3TransformCoord(&vertices[3].p, &vertices[3].p, &mat);
		vertices[0].c = vertices[1].c = vertices[2].c = vertices[3].c = D3DCOLOR_ARGB(255, 255, 255, 255);

		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		if (m_moonTexture)
			m_device->SetTexture(0, *m_moonTexture);
		else
			m_device->SetTexture(0, null);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SkyboxVertex));
	}
	else if (g_global3D.hour >= 7 && g_global3D.hour <= 17)
	{
		SkyboxVertex vertices[4];
		vertices[0].p = D3DXVECTOR3(-1.0f, 1.0f, 30.0f);
		vertices[0].tu1 = 0.0f;
		vertices[0].tv1 = 0.0f;
		vertices[1].p = D3DXVECTOR3(1.0f, 1.0f, 30.0f);
		vertices[1].tu1 = 1.0f;
		vertices[1].tv1 = 0.0f;
		vertices[2].p = D3DXVECTOR3(1.0f, -1.0f, 30.0f);
		vertices[2].tu1 = 1.0f;
		vertices[2].tv1 = 1.0f;
		vertices[3].p = D3DXVECTOR3(-1.0f, -1.0f, 30.0f);
		vertices[3].tu1 = 0.0f;
		vertices[3].tv1 = 1.0f;

		D3DXMatrixRotationX(&mat, sunAngle);
		D3DXVec3TransformCoord(&vertices[0].p, &vertices[0].p, &mat);
		D3DXVec3TransformCoord(&vertices[1].p, &vertices[1].p, &mat);
		D3DXVec3TransformCoord(&vertices[2].p, &vertices[2].p, &mat);
		D3DXVec3TransformCoord(&vertices[3].p, &vertices[3].p, &mat);
		vertices[0].c = vertices[1].c = vertices[2].c = vertices[3].c = D3DCOLOR_ARGB(255, 255, 255, 255);

		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		if (m_sunTexture)
			m_device->SetTexture(0, *m_sunTexture);
		else
			m_device->SetTexture(0, null);
		m_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SkyboxVertex));
	}

	m_device->SetTransform(D3DTS_VIEW, &g_global3D.view);
}

void CSkybox::LoadTextures()
{
	m_threeTexturesCloud = true;
	m_threeTexturesSky = true;

	if (m_world->m_continent && m_world->m_continent->useEnvir)
	{
		const Continent* continent = m_world->m_continent;

		if (continent->moonTexture == STR_NO)
			m_moonTexture = TextureMng->GetWeatherTexture("Moon.dds");
		else
			m_moonTexture = TextureMng->GetWeatherTexture(continent->moonTexture);

		if (continent->sunTexture == STR_NO)
			m_sunTexture = TextureMng->GetWeatherTexture("sundisk.bmp");
		else
			m_sunTexture = TextureMng->GetWeatherTexture(continent->sunTexture);

		if (continent->skyTexture == STR_NO)
		{
			m_topTextures[0] = TextureMng->GetWeatherTexture("skybox01_low.dds");
			m_topTextures[1] = TextureMng->GetWeatherTexture("skybox02_low.dds");
			m_topTextures[2] = TextureMng->GetWeatherTexture("skybox03.dds");
		}
		else
		{
			m_threeTexturesSky = false;
			m_topTextures[0] = TextureMng->GetWeatherTexture(continent->skyTexture);
		}

		if (continent->cloudTexture == STR_NO)
		{
			m_cloudTextures[0] = TextureMng->GetWeatherTexture("cloud01_low.dds");
			m_cloudTextures[1] = TextureMng->GetWeatherTexture("cloud02_low.dds");
			m_cloudTextures[2] = TextureMng->GetWeatherTexture("cloud03.dds");
		}
		else
		{
			m_threeTexturesCloud = false;
			m_cloudTextures[0] = TextureMng->GetWeatherTexture(continent->cloudTexture);
		}
	}
	else
	{
		if (m_world->m_moonTextureName == STR_DEFAULT)
			m_moonTexture = TextureMng->GetWeatherTexture("Moon.dds");
		else
			m_moonTexture = TextureMng->GetWeatherTexture(m_world->m_moonTextureName);

		if (m_world->m_sunTextureName == STR_DEFAULT)
			m_sunTexture = TextureMng->GetWeatherTexture("sundisk.bmp");
		else
			m_sunTexture = TextureMng->GetWeatherTexture(m_world->m_sunTextureName);

		if (m_world->m_cloudTextureNames[0] == STR_DEFAULT)
			m_cloudTextures[0] = TextureMng->GetWeatherTexture("cloud01_low.dds");
		else
			m_cloudTextures[0] = TextureMng->GetWeatherTexture(m_world->m_cloudTextureNames[0]);

		if (m_world->m_cloudTextureNames[1] == STR_DEFAULT)
			m_cloudTextures[1] = TextureMng->GetWeatherTexture("cloud02_low.dds");
		else
			m_cloudTextures[1] = TextureMng->GetWeatherTexture(m_world->m_cloudTextureNames[1]);

		if (m_world->m_cloudTextureNames[2] == STR_DEFAULT)
			m_cloudTextures[2] = TextureMng->GetWeatherTexture("cloud03.dds");
		else
			m_cloudTextures[2] = TextureMng->GetWeatherTexture(m_world->m_cloudTextureNames[2]);

		if (m_world->m_skyTextureNames[0] == STR_DEFAULT)
			m_topTextures[0] = TextureMng->GetWeatherTexture("skybox01_low.dds");
		else
			m_topTextures[0] = TextureMng->GetWeatherTexture(m_world->m_skyTextureNames[0]);

		if (m_world->m_skyTextureNames[1] == STR_DEFAULT)
			m_topTextures[1] = TextureMng->GetWeatherTexture("skybox02_low.dds");
		else
			m_topTextures[1] = TextureMng->GetWeatherTexture(m_world->m_skyTextureNames[1]);

		if (m_world->m_skyTextureNames[2] == STR_DEFAULT)
			m_topTextures[2] = TextureMng->GetWeatherTexture("skybox03.dds");
		else
			m_topTextures[2] = TextureMng->GetWeatherTexture(m_world->m_skyTextureNames[2]);
	}
}

bool CSkybox::InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->CreateVertexBuffer(30 * sizeof(SkyboxVertex),
		D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
		&s_cloudVB, null)))
		return false;

	SkyboxVertex* VB;

	const float tiled = 1.0f;
	const float radius = 110.0f;
	float x, z;

	s_cloudVB->Lock(0, 0, (void**)&VB, 0);
	VB[0].p = D3DXVECTOR3(0.0f, 100.0f, 0.0f);
	VB[0].tu1 = tiled;
	VB[0].tv1 = tiled;
	VB[0].c = D3DCOLOR_ARGB(255, 255, 255, 255);
	for (int i = 1; i < 30; i++)
	{
		x = (float)sin(2 * D3DX_PI / 28 * i) * radius;
		z = (float)cos(2 * D3DX_PI / 28 * i) * radius;
		VB[i].p = D3DXVECTOR3(x, 40.0f, z);
		VB[i].tu1 = (x + radius) * tiled / radius;
		VB[i].tv1 = (z + radius) * tiled / radius;
		VB[i].c = D3DCOLOR_ARGB(0, 255, 255, 255);
	}
	s_cloudVB->Unlock();

	if (FAILED(device->CreateVertexBuffer(348 * sizeof(SkyboxVertex),
		D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED,
		&s_topVB, null)))
		return false;

	s_topVB->Lock(0, 0, (void**)&VB, 0);
	for (int i = 0; i < 28; i++)
	{
		const float x1 = (float)sin(2 * D3DX_PI / 28 * i) * radius;
		const float z1 = (float)cos(2 * D3DX_PI / 28 * i) * radius;
		const float x2 = (float)sin(2 * D3DX_PI / 28 * (i + 1)) * radius;
		const float z2 = (float)cos(2 * D3DX_PI / 28 * (i + 1)) * radius;
		const float fu1 = 1.0f / 28.0f * i;
		const float fu2 = 1.0f / 28.0f * (i + 1);

		VB->p = D3DXVECTOR3(x1, 150.0f, z1);
		VB->tu1 = fu1;
		VB->tv1 = 0.0f;
		VB->c = D3DCOLOR_ARGB(0, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x1, 0.0f, z1);
		VB->tu1 = fu1;
		VB->tv1 = 0.9f;
		VB->c = D3DCOLOR_ARGB(255, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x2, 150.0f, z2);
		VB->tu1 = fu2;
		VB->tv1 = 0.0f;
		VB->c = D3DCOLOR_ARGB(0, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x2, 150.0f, z2);
		VB->tu1 = fu2;
		VB->tv1 = 0.0f;
		VB->c = D3DCOLOR_ARGB(0, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x1, 0.0f, z1);
		VB->tu1 = fu1;
		VB->tv1 = 0.9f;
		VB->c = D3DCOLOR_ARGB(255, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x2, 0.0f, z2);
		VB->tu1 = fu2;
		VB->tv1 = 0.9f;
		VB->c = D3DCOLOR_ARGB(255, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x1, 0.0f, z1);
		VB->tu1 = fu1;
		VB->tv1 = 0.9f;
		VB->c = D3DCOLOR_ARGB(255, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x1, -20.0f, z1);
		VB->tu1 = fu1;
		VB->tv1 = 1.0f;
		VB->c = D3DCOLOR_ARGB(0, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x2, 0.0f, z2);
		VB->tu1 = fu2;
		VB->tv1 = 0.9f;
		VB->c = D3DCOLOR_ARGB(255, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x2, 0.0f, z2);
		VB->tu1 = fu2;
		VB->tv1 = 0.9f;
		VB->c = D3DCOLOR_ARGB(255, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x1, -20.0f, z1);
		VB->tu1 = fu1;
		VB->tv1 = 1.0f;
		VB->c = D3DCOLOR_ARGB(0, 255, 255, 255);
		VB++;

		VB->p = D3DXVECTOR3(x2, -20.0f, z2);
		VB->tu1 = fu2;
		VB->tv1 = 1.0f;
		VB->c = D3DCOLOR_ARGB(0, 255, 255, 255);
		VB++;
	}
	s_topVB->Unlock();

	return true;
}

void CSkybox::DeleteStaticDeviceObjects()
{
	Release(s_topVB);
	Release(s_cloudVB);
}