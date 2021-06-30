///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "SfxModel.h"
#include "Sfx.h"
#include "ModelMng.h"
#include "TextureMng.h"

LPDIRECT3DVERTEXBUFFER9 CSfxModel::s_VB = null;

bool CSfxModel::InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	if (FAILED(device->CreateVertexBuffer(sizeof(SfxVertex) * 4, D3DUSAGE_WRITEONLY, SfxVertex::FVF, D3DPOOL_MANAGED, &s_VB, null)))
		return false;
	SfxVertex* vertices;
	s_VB->Lock(0, sizeof(SfxVertex) * 4, (void**)&vertices, 0);
	vertices->p = D3DXVECTOR3(-0.5f, 0.5f, 0.0f);
	vertices->t = D3DXVECTOR2(0.0f, 0.0f);
	vertices++;
	vertices->p = D3DXVECTOR3(0.5f, 0.5f, 0.0f);
	vertices->t = D3DXVECTOR2(1.0f, 0.0f);
	vertices++;
	vertices->p = D3DXVECTOR3(0.5f, -0.5f, 0.0f);
	vertices->t = D3DXVECTOR2(1.0f, 1.0f);
	vertices++;
	vertices->p = D3DXVECTOR3(-0.5f, -0.5f, 0.0f);
	vertices->t = D3DXVECTOR2(0.0f, 1.0f);
	s_VB->Unlock();
	return true;
}

void CSfxModel::DeleteStaticDeviceObjects()
{
	Release(s_VB);
}

CSfxModel::CSfxModel(LPDIRECT3DDEVICE9 device)
	: CModel(device),
	m_sfx(null)
{
	m_perSlerp = 1.0f;
	m_bounds.Min = D3DXVECTOR3(-1.0f, 0.0f, -1.0f);
	m_bounds.Max = D3DXVECTOR3(1.0f, 0.5f, 1.0f);
	m_currentFrame = -0.5f;
}

CSfxModel::~CSfxModel()
{
	CPtrArray<Particle>* particles;
	int j;
	for (int i = 0; i < m_particles.GetSize(); i++)
	{
		particles = m_particles[i];
		if (particles)
		{
			for (j = 0; j < particles->GetSize(); j++)
				Delete(particles->GetAt(j));
			Delete(particles);
		}
	}
}

bool CSfxModel::Load(const string& filename)
{
	m_sfx = ModelMng->GetSfx(filename);

	if (!m_sfx)
		return false;

	for (int i = 0; i < m_sfx->m_parts.GetSize(); i++)
	{
		if (m_sfx->m_parts[i]->GetType() == ESfxPartType::Particle)
			m_particles.Append(new CPtrArray<Particle>());
		else
			m_particles.Append(null);
	}

	return true;
}

void CSfxModel::Render(const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale)
{
	if (!m_sfx)
		return;

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_device->SetFVF(SfxVertex::FVF);

	bool billOrParticle = false;
	const ushort frame = (ushort)m_currentFrame;

	CSfxPart* part;
	for (int i = 0; i < m_sfx->m_parts.GetSize(); i++)
	{
		part = m_sfx->m_parts[i];
		if (!part->IsVisible())
			continue;

		switch (part->GetType())
		{
		case ESfxPartType::Bill:
			if (!billOrParticle)
			{
				m_device->SetStreamSource(0, s_VB, 0, sizeof(SfxVertex));
				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				billOrParticle = true;
			}

			part->Render(frame, pos, angle, scale);
			break;

		case ESfxPartType::Particle:
			if (i < m_particles.GetSize())
			{
				if (!billOrParticle)
				{
					m_device->SetStreamSource(0, s_VB, 0, sizeof(SfxVertex));
					m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
					billOrParticle = true;
				}

				_renderParticles(m_particles[i], (const CSfxPartParticle*)part, pos, angle, scale);
			}
			break;

		case ESfxPartType::CustomMesh:
			part->Render(frame, pos, angle, scale);
			billOrParticle = false;
			break;

		case ESfxPartType::Mesh:
			part->Render(frame, pos, angle, scale);
			billOrParticle = false;
			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
			m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			m_device->SetFVF(SfxVertex::FVF);
			break;
		}
	}
}

void CSfxModel::_renderParticles(CPtrArray<Particle>* particles, const CSfxPartParticle* part, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale)
{
	const ushort frame = (ushort)m_currentFrame;
	SfxKeyFrame key;
	if (!part->GetKey(frame, key))
		return;
	const ushort prevFrame = part->GetPrevKey(frame)->frame;

	const D3DXVECTOR3 rot = D3DXToRadian(angle);
	D3DXMATRIX matTemp, matAngle, matTemp2,
		matRot, matTrans, matScale, matTemp1, matTempRot;
	D3DXMatrixRotationYawPitchRoll(&matRot, rot.y, rot.x, rot.z);

	switch (part->m_billType)
	{
	case ESfxPartBillType::Bill:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		matTemp2 = g_global3D.view;	matTemp2._41 = matTemp2._42 = matTemp2._43 = .0f;
		matTemp = matAngle * g_global3D.invView;
		break;

	case ESfxPartBillType::Bottom:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		D3DXMatrixRotationX(&matTemp2, D3DXToRadian(90));
		matTemp = matAngle * matTemp2;
		break;

	case ESfxPartBillType::Pole:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		matTemp2 = g_global3D.view;	matTemp2._41 = matTemp2._42 = matTemp2._43 = .0f;
		matTemp = matAngle * g_global3D.invView;
		break;

	case ESfxPartBillType::Normal:
	{
		const D3DXVECTOR3 keyRot = D3DXToRadian(key.rotate);
		D3DXMatrixRotationYawPitchRoll(&matAngle, keyRot.y, keyRot.x, keyRot.z);
		matTemp = matAngle * matRot;
		break;
	}
	}

	if (part->m_alphaType == ESfxPartAlphaType::Glow)
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	const D3DXVECTOR3 keyPosRotate = D3DXToRadian(key.posRotate);
	D3DXMatrixRotationYawPitchRoll(&matTemp2, keyPosRotate.y, keyPosRotate.x, keyPosRotate.z);
	matTemp2 = matRot * matTemp2;

	D3DXVECTOR3 temp;
	D3DXVec3TransformCoord(&temp, &(key.pos), &matTemp2);

	D3DXVECTOR3 temp2, tempRot;
	Particle* particle;
	const int particleCount = particles->GetSize();
	int sendTexFrame = -1, tempAlpha;
	float lerp;

	if (part->m_texFrame <= 1)
	{
		if (part->m_texture)
			m_device->SetTexture(0, *part->m_texture);
		else
			m_device->SetTexture(0, null);
	}

	for (int i = 0; i < particleCount; i++)
	{
		particle = particles->GetAt(i);

		if (part->m_texFrame > 1)
		{
			const int texFrame = (part->m_texFrame * particle->frame / part->m_texLoop) + part->m_texFrame;
			if (texFrame != sendTexFrame)
			{
				if (part->m_textures[texFrame])
					m_device->SetTexture(0, *part->m_textures[texFrame]);
				else
					m_device->SetTexture(0, null);
				sendTexFrame = texFrame;
			}
		}

		if (part->m_repeatScal)
		{
			if (D3DXVec3Length(&particle->scale) >= D3DXVec3Length(&particle->scaleEnd))
				particle->swScal = true;
			else if (D3DXVec3Length(&particle->scale) <= D3DXVec3Length(&particle->scaleStart))
				particle->swScal = false;

			if (particle->swScal)
				particle->scale -= particle->scaleSpeed;
			else
				particle->scale += particle->scaleSpeed;
		}

		temp2 = D3DXToRadian(key.rotate + key.posRotate);
		D3DXMatrixRotationYawPitchRoll(&matTemp2, temp2.y, temp2.x, temp2.z);
		matTemp2 *= matRot;
		D3DXVec3TransformCoord(&temp2, &(particle->pos), &matTemp2);

		D3DXMatrixTranslation(&matTrans,
			temp.x + (temp2.x * scale.x) + pos.x,
			temp.y + (temp2.y * scale.y) + pos.y,
			temp.z + (temp2.z * scale.z) + pos.z);

		D3DXMatrixScaling(&matScale, particle->scale.x * scale.x, particle->scale.y * scale.y, 1.0f * scale.z);

		tempRot = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		if (key.frame != 0)
		{
			lerp = (float)(frame - prevFrame) / key.frame;
			D3DXVec3Lerp(&tempRot, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &particle->rotation, lerp);
		}

		D3DXMatrixRotationYawPitchRoll(&matTempRot, D3DXToRadian(tempRot.y), D3DXToRadian(tempRot.x), D3DXToRadian(tempRot.z));
		matTemp1 = matTempRot * matScale * matTemp * matTrans;
		m_device->SetTransform(D3DTS_WORLD, &matTemp1);

		if (particle->frame < part->m_particleFrameAppear)
			tempAlpha = particle->frame*key.alpha / part->m_particleFrameAppear;
		else if (particle->frame > part->m_particleFrameKeep)
			tempAlpha = key.alpha - (key.alpha * (particle->frame - part->m_particleFrameKeep) / (part->m_particleFrameDisappear - part->m_particleFrameKeep));
		else
			tempAlpha = key.alpha;

		m_device->SetRenderState(D3DRS_TEXTUREFACTOR, tempAlpha << 24 | 0x404040);
		m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	}

	if (part->m_alphaType == ESfxPartAlphaType::Glow)
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CSfxModel::SetFrame(float currentFrame)
{
	if (currentFrame == m_currentFrame
		|| currentFrame < 0.0f)
		return;

	if (currentFrame < m_currentFrame)
	{
		CPtrArray<Particle>* particles;
		int j;
		for (int i = 0; i < m_particles.GetSize(); i++)
		{
			particles = m_particles[i];
			if (particles)
			{
				for (j = 0; j < particles->GetSize(); j++)
					Delete(particles->GetAt(j));
				particles->RemoveAll();
			}
		}

		m_currentFrame = -0.5f;
	}

	while (currentFrame > m_currentFrame)
	{
		if (!_nextFrame())
			break;
	}

	if (m_currentFrame < 0.0f)
		m_currentFrame = 0.0f;
}

void CSfxModel::MoveFrame()
{
	if (!_nextFrame())
		m_currentFrame = -0.5f;
}

bool CSfxModel::_nextFrame()
{
	bool ret = false;
	m_currentFrame += m_perSlerp;
	const ushort frame = (ushort)m_currentFrame;

	ushort startFrame, endFrame;
	CPtrArray<Particle>* particles;
	Particle* particle;
	int j;
	for (int i = 0; i < m_particles.GetSize(); i++)
	{
		if (m_particles[i])
		{
			const CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_parts[i];
			particles = m_particles[i];

			for (j = 0; j < particles->GetSize(); j++)
			{
				particle = particles->GetAt(j);
				particle->pos += particle->speed;
				particle->speed += part->m_particleAccel;
				if (!part->m_repeatScal)
					particle->scale += part->m_scaleSpeed;
				particle->frame++;
				if (particle->frame >= part->m_particleFrameDisappear)
				{
					Delete(particle);
					particles->RemoveAt(j);
					j--;
				}
			}

			startFrame = 0;
			endFrame = 0;
			if (part->GetFirstKey())
			{
				startFrame = part->GetFirstKey()->frame;
				endFrame = part->GetLastKey()->frame;
			}

			if (frame >= startFrame && frame <= endFrame - part->m_particleFrameDisappear)
			{
				if ((part->m_particleCreate == 0 && frame == startFrame) ||
					(part->m_particleCreate != 0 && (frame - startFrame) + part->m_particleCreate == 0)
					)
				{
					const float rand1 = (rand() % 50000) / 50000.0f;
					const float rand2 = (rand() % 50000) / 50000.0f;
					const float rand3 = (rand() % 50000) / 50000.0f;

					for (j = 0; j < part->m_particleCreateNum; j++)
					{
						particle = new Particle();
						particle->frame = 0;

						const float angle = ((rand() % 50000) / 50000.0f) * 360.0f;
						particle->pos =
							D3DXVECTOR3(
							sin(angle) * part->m_particleStartPosVar,
							rand1 * part->m_particleStartPosVarY,
							cos(angle) * part->m_particleStartPosVar
							);
						const float factor = part->m_particleXZLow + rand2 * (part->m_particleXZHigh - part->m_particleXZLow);
						particle->speed =
							D3DXVECTOR3(
							sin(angle) * factor,
							part->m_particleYLow + rand2 * (part->m_particleYHigh - part->m_particleYLow),
							cos(angle) * factor
							);
						particle->scaleStart = particle->scale = part->m_scale;
						particle->rotation = D3DXVECTOR3(part->m_rotationLow.x + rand1 *
							(part->m_rotationHigh.x - part->m_rotationLow.x),
							part->m_rotationLow.y + rand3 *
							(part->m_rotationHigh.y - part->m_rotationLow.y),
							part->m_rotationLow.z + rand2 *
							(part->m_rotationHigh.z - part->m_rotationLow.z));
						particle->swScal = false;
						particle->scaleEnd = part->m_scaleEnd;
						particle->scaleSpeed = D3DXVECTOR3(part->m_scalSpeedXLow + rand3 *
							(part->m_scalSpeedXHigh - part->m_scalSpeedXLow),
							part->m_scalSpeedYLow + rand2 *
							(part->m_scalSpeedYHigh - part->m_scalSpeedYLow),
							part->m_scalSpeedZLow + rand1 *
							(part->m_scalSpeedZHigh - part->m_scalSpeedZLow));

						particles->Append(particle);
					}
				}
			}

			if (particles->GetSize() > 0 || frame < startFrame)
				ret = true;

#ifndef SFX_EDITOR
			if (part->m_repeat)
			{
				if (endFrame >= 0)
				{
					const float f = m_currentFrame / (float)endFrame;
					if (f >= 0.65f)
						m_currentFrame = (float)endFrame * 0.6f;
				}
			}
#endif // SFX_EDITOR
		}
		else
		{
			if (m_sfx->m_parts[i]->GetNextKey(frame))
				ret = true;
		}
	}

	return ret;
}