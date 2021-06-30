///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Mesh.h"
#include "ModelMng.h"
#include "Object3D.h"

CMesh::CMesh(LPDIRECT3DDEVICE9 device)
	: CModel(device),
	m_object3D(null)
{
}

bool CMesh::Load(const string& filename)
{
	m_object3D = ModelMng->GetObject3D(filename);

	if (!m_object3D)
		return false;

	m_bounds = m_object3D->GetBounds();

	if (m_object3D->HasSkin())
		m_hasSkin = true;

	return true;
}

void CMesh::Render(const D3DXMATRIX* world, int lod, int alpha)
{
	if (m_object3D)
	{
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_device->SetRenderState(D3DRS_LIGHTING, g_global3D.light ? TRUE : FALSE);
		m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		m_object3D->Render(world, lod, 0.0f, 0, m_textureEx, alpha);
	}
}

void CMesh::RenderCollision(const D3DXMATRIX* world)
{
	if (m_object3D)
		m_object3D->RenderCollision(world);
}

bool CMesh::Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist)
{
	if (m_object3D)
		return m_object3D->Pick(world, origin, dir, dist);
	return false;
}