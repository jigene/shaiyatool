///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Model.h"

CModel::CModel(LPDIRECT3DDEVICE9 device)
	: m_device(device),
	m_frameCount(0),
	m_currentFrame(0.0f),
	m_textureEx(0),
	m_perSlerp(0.5f),
	m_hasSkin(false)
{
	memset(&m_bounds, 0, sizeof(m_bounds));
}

void CModel::Render(const D3DXMATRIX* world, int lod, int alpha)
{
}

void CModel::Render(const D3DXVECTOR3& pos, const D3DXVECTOR3& rotate, const D3DXVECTOR3& scale)
{
}

void CModel::MoveFrame()
{
	if (!m_frameCount)
		return;

	m_currentFrame += m_perSlerp;

	int currentFrame = (int)m_currentFrame;
	const float slp = m_currentFrame - (float)currentFrame;

	if (m_currentFrame > (float)(m_frameCount - 1))
		currentFrame %= m_frameCount;

	m_currentFrame = (float)currentFrame + slp;
}

int CModel::GetNextFrame() const
{
	const int nextFrame = (int)m_currentFrame + 1;

	if (nextFrame >= m_frameCount)
		return 0;

	return nextFrame;
}

const Bounds& CModel::GetBounds() const
{
	return m_bounds;
}

int CModel::GetFrameCount() const
{
	return m_frameCount;
}

float CModel::GetCurrentFrame() const
{
	return m_currentFrame;
}

void CModel::SetFrame(float currentFrame)
{
	if (m_currentFrame < (float)m_frameCount)
		m_currentFrame = currentFrame;
}

void CModel::SetTextureEx(int ex)
{
	if (ex >= 0 && ex <= 7)
		m_textureEx = ex;
}

int CModel::GetTextureEx() const
{
	return m_textureEx;
}