///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include <Sfx.h>
#include "ModelViewer.h"
#include <SfxModel.h>

void CMainFrame::UpdateKeyPos()
{
	SfxKeyFrame* key = m_currentEditKey;
	m_currentEditKey = null;
	if (key)
	{
		ui.editKeyPosX->setValue(key->pos.x);
		ui.editKeyPosY->setValue(key->pos.y);
		ui.editKeyPosZ->setValue(key->pos.z);
		m_currentEditKey = key;
	}
}

void CMainFrame::EditKeyModified(int row, int frame, bool removed)
{
	if (row < 0 || row >= m_sfx->m_sfx->m_parts.GetSize())
		return;

	CSfxPart* part = m_sfx->m_sfx->m_parts[row];

	if (removed)
	{
		part->RemoveKey((ushort)frame);
		m_currentEditKey = null;
	}
	else
	{
		part->AddKey((ushort)frame);
		m_currentEditKey = part->GetKey((ushort)frame);
	}

	if (m_sfx->m_sfx->m_parts[row]->GetType() == ESfxPartType::Particle)
	{
		const float f = m_sfx->GetCurrentFrame();
		m_sfx->SetFrame(0.0f);
		m_sfx->SetFrame(f);
	}

	_setKey();
}

void CMainFrame::_setKey()
{
	SfxKeyFrame* key = m_currentEditKey;
	m_currentEditKey = null;

	if (key)
	{
		ui.dockEditKey->setEnabled(true);
		ui.editKeyPosX->setValue(key->pos.x);
		ui.editKeyPosY->setValue(key->pos.y);
		ui.editKeyPosZ->setValue(key->pos.z);
		ui.editKeyPosRotX->setValue(key->posRotate.x);
		ui.editKeyPosRotY->setValue(key->posRotate.y);
		ui.editKeyPosRotZ->setValue(key->posRotate.z);
		ui.editKeyRotX->setValue(key->rotate.x);
		ui.editKeyRotY->setValue(key->rotate.y);
		ui.editKeyRotZ->setValue(key->rotate.z);
		ui.editKeyScaleX->setValue(key->scale.x);
		ui.editKeyScaleY->setValue(key->scale.y);
		ui.editKeyScaleZ->setValue(key->scale.z);
		ui.editKeyAlpha->setValue(key->alpha);
		ui.currentKeyFrame->setText(string("Frame %1").arg(key->frame));
		m_currentEditKey = key;
	}
	else
	{
		ui.dockEditKey->setEnabled(false);
		ui.editKeyPosX->setValue(0.0);
		ui.editKeyPosY->setValue(0.0);
		ui.editKeyPosZ->setValue(0.0);
		ui.editKeyPosRotX->setValue(0.0);
		ui.editKeyPosRotY->setValue(0.0);
		ui.editKeyPosRotZ->setValue(0.0);
		ui.editKeyRotX->setValue(0.0);
		ui.editKeyRotY->setValue(0.0);
		ui.editKeyRotZ->setValue(0.0);
		ui.editKeyScaleX->setValue(1.0);;
		ui.editKeyScaleY->setValue(1.0);
		ui.editKeyScaleZ->setValue(1.0);
		ui.editKeyAlpha->setValue(255);
		ui.currentKeyFrame->setText("Frame -");
	}
}

void CMainFrame::_editKey()
{
	if (m_currentEditPart > -1)
	{
		if (m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
		{
			const float f = m_sfx->GetCurrentFrame();
			m_sfx->SetFrame(0.0f);
			m_sfx->SetFrame(f);
		}
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CMainFrame::EditKeyPosX(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->pos.x = (float)value;
	_editKey();
}

void CMainFrame::EditKeyPosY(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->pos.y = (float)value;
	_editKey();
}

void CMainFrame::EditKeyPosZ(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->pos.z = (float)value;
	_editKey();
}

void CMainFrame::EditKeyPosRotX(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->posRotate.x = (float)value;
	_editKey();
}

void CMainFrame::EditKeyPosRotY(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->posRotate.y = (float)value;
	_editKey();
}

void CMainFrame::EditKeyPosRotZ(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->posRotate.z = (float)value;
	_editKey();
}

void CMainFrame::EditKeyRotX(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->rotate.x = (float)value;
	_editKey();
}

void CMainFrame::EditKeyRotY(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->rotate.y = (float)value;
	_editKey();
}

void CMainFrame::EditKeyRotZ(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->rotate.z = (float)value;
	_editKey();
}

void CMainFrame::EditKeyScaleX(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->scale.x = (float)value;
	_editKey();
}

void CMainFrame::EditKeyScaleY(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->scale.y = (float)value;
	_editKey();
}

void CMainFrame::EditKeyScaleZ(double value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->scale.z = (float)value;
	_editKey();
}

void CMainFrame::EditKeyAlpha(int value)
{
	if (!m_currentEditKey)
		return;
	m_currentEditKey->alpha = value;
	_editKey();
}