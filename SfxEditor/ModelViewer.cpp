///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "ModelViewer.h"
#include <TextureMng.h>
#include <ModelMng.h>
#include <Object3D.h>
#include <AnimatedMesh.h>
#include <ModelMng.h>
#include <MainFrame.h>
#include <Timeline.h>
#include <SfxModel.h>
#include <Sfx.h>

struct GridVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
	D3DXVECTOR3 p;
	uint c;
};

CModelViewer::CModelViewer(QWidget* parent, Qt::WindowFlags flags)
	: CD3DWidget(parent, flags)
{
	m_gridVB = null;
	m_cameraDist = 5.0f;
	m_cameraRot = D3DXVECTOR2(180.0f, 20.0f);
	m_textureMng = null;
	m_modelMng = null;
	m_mesh = null;
	m_movingCamera = false;
	m_renderMesh = true;
	m_sfx = null;
	m_playMeshMotion = false;
	m_movingKey = false;
	m_modelRot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

CModelViewer::~CModelViewer()
{
	Destroy();
}

void CModelViewer::SetFrame(int frame)
{
	if (m_sfx)
		m_sfx->SetFrame((float)frame);

	if (!IsAutoRefresh())
		RenderEnvironment();
}

void CModelViewer::ShowModel3D(bool show)
{
	m_renderMesh = show;

	if (!IsAutoRefresh())
		RenderEnvironment();
}

void CModelViewer::PlayMeshMotion(bool play)
{
	m_playMeshMotion = play;
}

bool CModelViewer::InitDeviceObjects()
{
	GridVertex vertices[17 * 4];

	float x = -8.0f;
	int vertexIndex = 0;

	for (int i = 0; i < 17; i++)
	{
		vertices[vertexIndex].p = D3DXVECTOR3(x, 0.0f, -8.0f);
		if (i == 8)
			vertices[vertexIndex].c = 0xff00ff00;
		else
			vertices[vertexIndex].c = 0xffffffff;
		vertexIndex++;
		vertices[vertexIndex].p = D3DXVECTOR3(x, 0.0f, 8.0f);
		if (i == 8)
			vertices[vertexIndex].c = 0xff00ff00;
		else
			vertices[vertexIndex].c = 0xffffffff;
		vertexIndex++;
		x += 1.0f;
	}

	float z = -8.0f;
	for (int i = 0; i < 17; i++)
	{
		vertices[vertexIndex].p = D3DXVECTOR3(-8.0f, 0.0f, z);
		if (i == 8)
			vertices[vertexIndex].c = 0xffff0000;
		else
			vertices[vertexIndex].c = 0xffffffff;
		vertexIndex++;
		vertices[vertexIndex].p = D3DXVECTOR3(8.0f, 0.0f, z);
		if (i == 8)
			vertices[vertexIndex].c = 0xffff0000;
		else
			vertices[vertexIndex].c = 0xffffffff;
		vertexIndex++;
		z += 1.0f;
	}

	if (FAILED(m_device->CreateVertexBuffer(sizeof(vertices), D3DUSAGE_WRITEONLY, GridVertex::FVF, D3DPOOL_MANAGED, &m_gridVB, NULL)))
		return false;

	void* data;
	m_gridVB->Lock(0, sizeof(vertices), &data, 0);
	memcpy(data, vertices, sizeof(vertices));
	m_gridVB->Unlock();

	m_textureMng = new CTextureMng(m_device);
	m_modelMng = new CModelMng(m_device);

	if (!CObject3D::InitStaticDeviceObjects(m_device)
		|| !CSfxModel::InitStaticDeviceObjects(m_device))
		return false;

	return true;
}

void CModelViewer::DeleteDeviceObjects()
{
	CObject3D::DeleteStaticDeviceObjects();
	CSfxModel::DeleteStaticDeviceObjects();
	Release(m_gridVB);
	Delete(m_modelMng);
	Delete(m_textureMng);
}

void CModelViewer::SetMesh(CAnimatedMesh* mesh)
{
	m_mesh = mesh;
	if (!IsAutoRefresh())
		RenderEnvironment();
}

void CModelViewer::SetSfx(CSfxModel* sfx)
{
	m_sfx = sfx;
	if (!IsAutoRefresh())
		RenderEnvironment();
}

void CModelViewer::SetModelRotX(double value)
{
	m_modelRot.x = (float)value;
	if (!IsAutoRefresh())
		RenderEnvironment();
}

void CModelViewer::SetModelRotY(double value)
{
	m_modelRot.y = (float)value;
	if (!IsAutoRefresh())
		RenderEnvironment();
}

void CModelViewer::SetModelRotZ(double value)
{
	m_modelRot.z = (float)value;
	if (!IsAutoRefresh())
		RenderEnvironment();
}

bool CModelViewer::Render()
{
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, g_global3D.backgroundColor, 1.0f, 0L);

	if (FAILED(m_device->BeginScene()))
		return false;

	D3DXMATRIX proj, view, world, invView;

	D3DXMatrixIdentity(&world);
	D3DXMatrixPerspectiveFovLH(&proj, M_PI_4, (float)width() / (float)height(), 0.1f, 400.0f);

	const float phiRadian = m_cameraRot.y * M_PI / 180.0f;
	const float thetaRadian = m_cameraRot.x * M_PI / 180.0f;
	D3DXVECTOR3 eye, min{ 0, 0, 0 }, max{ 0, 0, 0 };
	
	eye.x = cos(phiRadian) * sin(thetaRadian);
	eye.y = sin(phiRadian);
	eye.z = cos(phiRadian) * cos(thetaRadian);
	eye *= m_cameraDist;

	if (m_renderMesh && m_mesh)
	{
		const Bounds bounds = m_mesh->GetBounds();
		min = bounds.Min;
		max = bounds.Max;
	}

	const float center = (min.y + max.y) / 2.0f;
	eye.y += center;
	D3DXMatrixLookAtLH(&view, &eye, &D3DXVECTOR3(0.0f, center, 0.0f), &D3DXVECTOR3(0, 1, 0));

	m_device->SetTransform(D3DTS_VIEW, &view);
	m_device->SetTransform(D3DTS_PROJECTION, &proj);
	m_device->SetTransform(D3DTS_WORLD, &world);

	g_global3D.view = view;
	g_global3D.proj = proj;

	D3DXMatrixInverse(&invView, NULL, &view);
	invView._41 = 0.0f; invView._42 = 0.0f; invView._43 = 0.0f;
	g_global3D.invView = invView;

	g_global3D.light = false;
	g_global3D.night = false;
	g_global3D.lightVec = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	const D3DXVECTOR4 nullVec(1.0f, 1.0f, 1.0f, 0.0f);
	m_device->SetVertexShaderConstantF(93, (float*)&nullVec, 1);
	m_device->SetVertexShaderConstantF(94, (float*)&nullVec, 1);

	const D3DXVECTOR4 fogConst(1.0f, 1.0f, 1.0f, 100.0f);
	m_device->SetVertexShaderConstantF(95, (float*)&fogConst, 1);

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

	m_device->SetStreamSource(0, m_gridVB, 0, sizeof(GridVertex));
	m_device->SetFVF(GridVertex::FVF);
	m_device->SetTexture(0, NULL);

	if (g_global3D.grid)
		m_device->DrawPrimitive(D3DPT_LINELIST, 0, 17 * 2);

	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	m_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetRenderState(D3DRS_ALPHAREF, 0xb0);
	m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	CTimeline* timeline = ((CMainFrame*)parent())->GetTimeline();

	if (m_mesh && m_renderMesh)
	{
		if (m_playMeshMotion && m_sfx)
		{
			int frame = (int)m_sfx->GetCurrentFrame();
			if (frame >= m_mesh->GetFrameCount())
				frame = m_mesh->GetFrameCount() - 1;
			if (frame < 0)
				frame = 0;

			m_mesh->SetFrame((float)frame);
		}

		D3DXMATRIX meshWorld;
		D3DXMatrixRotationYawPitchRoll(&meshWorld, D3DXToRadian(m_modelRot.y), D3DXToRadian(m_modelRot.x), D3DXToRadian(m_modelRot.z));
		m_mesh->Render(&meshWorld, 0);
	}

	if (m_sfx)
	{
		if (IsAutoRefresh())
		{
			m_sfx->MoveFrame();
			timeline->SetCurrentFrame((int)m_sfx->GetCurrentFrame());
		}

		m_sfx->Render(D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(0, 0, 0), D3DXVECTOR3(1, 1, 1));

		const QPoint selection = timeline->GetSelection();
		if (selection.y() >= 0 && selection.y() < m_sfx->m_sfx->m_parts.GetSize() && selection.x() == (int)m_sfx->GetCurrentFrame())
		{
			CSfxPart* part = m_sfx->m_sfx->m_parts[selection.y()];
			if (part->GetKey(selection.x()))
				part->RenderSelectionRectangle((ushort)selection.x());
		}
	}

	m_device->EndScene();
	return true;
}

void CModelViewer::mouseMoveEvent(QMouseEvent* event)
{
	if (m_movingCamera || m_movingKey)
	{
		const QPoint mousePos = QPoint(event->x(), event->y());
		const QPoint mouseMove = m_lastMousePos - mousePos;

		if (m_movingCamera)
		{
			m_cameraRot.x -= (float)mouseMove.x() / 4.0f;
			m_cameraRot.y -= (float)mouseMove.y() / 4.0f;

			if (m_cameraRot.y > 89.9f)
				m_cameraRot.y = 89.9f;
			else if (m_cameraRot.y < -89.9f)
				m_cameraRot.y = -89.9f;
		}

		if (m_movingKey)
		{
			const QPoint selection = ((CMainFrame*)parent())->GetTimeline()->GetSelection();
			if (selection.y() >= 0 && selection.y() < m_sfx->m_sfx->m_parts.GetSize() && selection.x() == (int)m_sfx->GetCurrentFrame())
			{
				CSfxPart* part = m_sfx->m_sfx->m_parts[selection.y()];
				SfxKeyFrame* key = part->GetKey(selection.x());
				if (key)
				{
					D3DXVECTOR3 pos = key->pos;

					const float phiRadian = m_cameraRot.y * M_PI / 180.0f;
					const float thetaRadian = m_cameraRot.x * M_PI / 180.0f;
					D3DXVECTOR3 eye;
					eye.x = cos(phiRadian) * sin(thetaRadian);
					eye.y = sin(phiRadian);
					eye.z = cos(phiRadian) * cos(thetaRadian);
					eye *= m_cameraDist;

					const float length = D3DXVec3Length(&(D3DXVECTOR3(pos.x, 0.0f, pos.z) - D3DXVECTOR3(eye.x, 0.0f, eye.z))) + 1.0f;

					D3DXVECTOR3 moveVec;
					D3DXVec3Normalize(&eye, &eye);
					D3DXVec3Cross(&moveVec, &eye, &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
					D3DXVec3Normalize(&moveVec, &moveVec);

					pos -= eye * (float)mouseMove.y() * length / 500.0f;
					pos -= moveVec * (float)mouseMove.x() * length / 700.0f;
					key->pos = D3DXVECTOR3(pos.x, key->pos.y, pos.z);

					((CMainFrame*)parent())->UpdateKeyPos();
				}
			}
		}

		m_lastMousePos = mousePos;

		if (!IsAutoRefresh())
			RenderEnvironment();

		event->accept();
	}
}

void CModelViewer::wheelEvent(QWheelEvent* event)
{
	if (event->orientation() == Qt::Vertical)
	{
		m_cameraDist -= (float)event->delta() / 120.0f;

		if (m_cameraDist < 0.1f)
			m_cameraDist = 0.1f;
		else if (m_cameraDist > 400.0f)
			m_cameraDist = 400.0f;

		if (!IsAutoRefresh())
			RenderEnvironment();

		event->accept();
	}
}

void CModelViewer::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton)
	{
		m_movingCamera = true;
		m_lastMousePos = QPoint(event->x(), event->y());
		event->accept();
	}
	else if (event->button() == Qt::LeftButton)
	{
		m_movingKey = true;
		m_lastMousePos = QPoint(event->x(), event->y());
		event->accept();
	}
}

void CModelViewer::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton)
	{
		m_movingCamera = false;
		event->accept();
	}
	else if (event->button() == Qt::LeftButton)
	{
		m_movingKey = false;
		event->accept();
	}
}