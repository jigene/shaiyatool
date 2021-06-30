///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include <Sfx.h>
#include <SfxModel.h>
#include "DialogNewPart.h"
#include "ModelViewer.h"
#include <Timeline.h>
#include <TextureMng.h>

void CMainFrame::EditKey(int row, int keyIndex)
{
	m_currentEditKey = null;

	if (row != m_currentEditPart)
	{
		m_currentEditPart = row;
		ui.listParts->setCurrentRow(row);
		_setPart();
	}

	if (m_currentEditPart > -1 && keyIndex > -1)
		m_currentEditKey = m_sfx->m_sfx->m_parts[row]->GetKey(keyIndex);

	_setKey();

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CMainFrame::ListPartChanged(int currentRow)
{
	m_currentEditPart = currentRow;

	if (m_currentEditPart <= -1)
		m_currentEditPart = -1;
	else if (m_sfx && m_currentEditPart >= m_sfx->m_sfx->m_parts.GetSize())
		m_currentEditPart = m_sfx->m_sfx->m_parts.GetSize() - 1;

	if (m_sfx)
		m_timeline->SetSelectedRow(m_currentEditPart);

	_setPart();

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CMainFrame::RemovePart()
{
	if (!m_sfx || m_currentEditPart < 0)
		return;

	const int currentEditPart = m_currentEditPart;

	QListWidgetItem* item = ui.listParts->takeItem(m_currentEditPart);
	Delete(item);

	ui.listParts->setCurrentRow(-1);

	m_currentEditPart = currentEditPart;

	m_sfx->m_sfx->RemovePart(m_currentEditPart);
	if (m_sfx->m_particles[m_currentEditPart])
	{
		for (int i = 0; i < m_sfx->m_particles[m_currentEditPart]->GetSize(); i++)
			Delete(m_sfx->m_particles[m_currentEditPart]->GetAt(i));
		Delete(m_sfx->m_particles.GetAt(m_currentEditPart));
	}
	m_sfx->m_particles.RemoveAt(m_currentEditPart);
	m_timeline->RemoveRow(m_currentEditPart);

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CMainFrame::AddPart()
{
	if (!m_sfx)
		return;

	CDialogNewPart dialog(this);
	if (dialog.exec() == QDialog::Accepted)
	{
		CSfxPart* part = m_sfx->m_sfx->AddPart(dialog.GetSfxType());
		dialog.SetPart(part);

		const string name = MakePartName(part);
		m_timeline->AddRow(name);
		ui.listParts->addItem(name);
		ui.listParts->item(ui.listParts->count() - 1)->setSelected(true);
		m_currentEditPart = ui.listParts->count() - 1;
		m_timeline->SetSelectedRow(m_currentEditPart);
		_setPart();

		if (part->GetType() == ESfxPartType::Particle)
			m_sfx->m_particles.Append(new CPtrArray<Particle>());
		else
			m_sfx->m_particles.Append(null);
	}
}

void CMainFrame::_setPart()
{
	CSfxPart* part = null;

	const int editPart = m_currentEditPart;

	if (m_currentEditPart > -1)
		part = m_sfx->m_sfx->m_parts[m_currentEditPart];

	m_currentEditPart = -1;

	if (part)
	{
		ui.dockEditPart->setEnabled(true);

		ui.editPartName->setText(part->m_name);
		ui.editPartTexture->setText(part->m_textureName);
		ui.editPartVisible->setChecked(part->m_visible);
		ui.editPartTexFrame->setValue(part->m_texFrame);
		ui.editPartTexLoop->setValue(part->m_texLoop);

		switch (part->m_billType)
		{
		case ESfxPartBillType::Bill:
			ui.editPartBill->setCurrentText("Bill");
			break;
		case ESfxPartBillType::Bottom:
			ui.editPartBill->setCurrentText("Bottom");
			break;
		case ESfxPartBillType::Normal:
			ui.editPartBill->setCurrentText("Normal");
			break;
		case ESfxPartBillType::Pole:
			ui.editPartBill->setCurrentText("Pole");
			break;
		}

		switch (part->m_alphaType)
		{
		case ESfxPartAlphaType::Blend:
			ui.editPartBlend->setCurrentText("Blend");
			break;
		case ESfxPartAlphaType::Glow:
			ui.editPartBlend->setCurrentText("Glow");
			break;
		}

		if (part->GetType() == ESfxPartType::CustomMesh)
		{
			ui.editCustomMeshPointCount->setEnabled(true);
			ui.editCustomMeshPointCount->setValue(((CSfxPartCustomMesh*)part)->m_pointCount);
		}
		else
		{
			ui.editCustomMeshPointCount->setEnabled(false);
			ui.editCustomMeshPointCount->setValue(20);
		}

		if (part->GetType() == ESfxPartType::Particle)
		{
			const CSfxPartParticle* particles = (CSfxPartParticle*)part;
			ui.dockParticles->show();
			ui.dockParticles->setEnabled(true);

			ui.editParticleCreateNum->setValue(particles->m_particleCreateNum);
			ui.editParticleCreate->setValue(particles->m_particleCreate);
			ui.editParticleFrameAppear->setValue(particles->m_particleFrameAppear);
			ui.editParticleFrameKeep->setValue(particles->m_particleFrameKeep);
			ui.editParticleFrameDisappear->setValue(particles->m_particleFrameDisappear);
			ui.editParticleStartPosVar->setValue(particles->m_particleStartPosVar);
			ui.editParticleStartPosVarY->setValue(particles->m_particleStartPosVarY);
			ui.editParticleYLow->setValue(particles->m_particleYLow);
			ui.editParticleXZLow->setValue(particles->m_particleXZLow);
			ui.editParticleYHigh->setValue(particles->m_particleYHigh);
			ui.editParticleXZHigh->setValue(particles->m_particleXZHigh);
			ui.editParticleAccelX->setValue(particles->m_particleAccel.x);
			ui.editParticleAccelY->setValue(particles->m_particleAccel.y);
			ui.editParticleAccelZ->setValue(particles->m_particleAccel.z);
			ui.editParticleScaleX->setValue(particles->m_scale.x);
			ui.editParticleScaleY->setValue(particles->m_scale.y);
			ui.editParticleScaleZ->setValue(particles->m_scale.z);
			ui.editParticleScaleSpeedX->setValue(particles->m_scaleSpeed.x);
			ui.editParticleScaleSpeedY->setValue(particles->m_scaleSpeed.y);
			ui.editParticleScaleSpeedZ->setValue(particles->m_scaleSpeed.z);
			ui.editParticleRotationLowX->setValue(particles->m_rotationLow.x);
			ui.editParticleRotationLowY->setValue(particles->m_rotationLow.y);
			ui.editParticleRotationLowZ->setValue(particles->m_rotationLow.z);
			ui.editParticleRotationHighX->setValue(particles->m_rotationHigh.x);
			ui.editParticleRotationHighY->setValue(particles->m_rotationHigh.y);
			ui.editParticleRotationHighZ->setValue(particles->m_rotationHigh.z);
			ui.editParticleRepeatScale->setChecked(particles->m_repeatScal);
			ui.editParticleRepeat->setChecked(particles->m_repeat);
			ui.editParticleScalSpeedXLow->setValue(particles->m_scalSpeedXLow);
			ui.editParticleScalSpeedYLow->setValue(particles->m_scalSpeedYLow);
			ui.editParticleScalSpeedZLow->setValue(particles->m_scalSpeedZLow);
			ui.editParticleScalSpeedXHigh->setValue(particles->m_scalSpeedXHigh);
			ui.editParticleScalSpeedYHigh->setValue(particles->m_scalSpeedYHigh);
			ui.editParticleScalSpeedZHigh->setValue(particles->m_scalSpeedZHigh);
			ui.editParticleScaleEndX->setValue(particles->m_scaleEnd.x);
			ui.editParticleScaleEndY->setValue(particles->m_scaleEnd.y);
			ui.editParticleScaleEndZ->setValue(particles->m_scaleEnd.z);
		}
		else
		{
			ui.dockParticles->hide();
			ui.dockParticles->setEnabled(false);

			ui.editParticleCreateNum->setValue(0);
			ui.editParticleCreate->setValue(0);
			ui.editParticleFrameAppear->setValue(0);
			ui.editParticleFrameKeep->setValue(0);
			ui.editParticleFrameDisappear->setValue(0);
			ui.editParticleStartPosVar->setValue(0);
			ui.editParticleStartPosVarY->setValue(0);
			ui.editParticleYLow->setValue(0);
			ui.editParticleXZLow->setValue(0);
			ui.editParticleYHigh->setValue(0);
			ui.editParticleXZHigh->setValue(0);
			ui.editParticleAccelX->setValue(0);
			ui.editParticleAccelY->setValue(0);
			ui.editParticleAccelZ->setValue(0);
			ui.editParticleScaleX->setValue(0);
			ui.editParticleScaleY->setValue(0);
			ui.editParticleScaleZ->setValue(0);
			ui.editParticleScaleSpeedX->setValue(0);
			ui.editParticleScaleSpeedY->setValue(0);
			ui.editParticleScaleSpeedZ->setValue(0);
			ui.editParticleRotationLowX->setValue(0);
			ui.editParticleRotationLowY->setValue(0);
			ui.editParticleRotationLowZ->setValue(0);
			ui.editParticleRotationHighX->setValue(0);
			ui.editParticleRotationHighY->setValue(0);
			ui.editParticleRotationHighZ->setValue(0);
			ui.editParticleRepeatScale->setChecked(false);
			ui.editParticleRepeat->setChecked(false);
			ui.editParticleScalSpeedXLow->setValue(0);
			ui.editParticleScalSpeedYLow->setValue(0);
			ui.editParticleScalSpeedZLow->setValue(0);
			ui.editParticleScalSpeedXHigh->setValue(0);
			ui.editParticleScalSpeedYHigh->setValue(0);
			ui.editParticleScalSpeedZHigh->setValue(0);
			ui.editParticleScaleEndX->setValue(0);
			ui.editParticleScaleEndY->setValue(0);
			ui.editParticleScaleEndZ->setValue(0);
		}

		m_currentEditPart = editPart;
	}
	else
	{
		ui.editPartName->setText("");
		ui.editPartTexture->setText("");
		ui.editPartVisible->setChecked(true);
		ui.editPartTexFrame->setValue(1);
		ui.editPartTexLoop->setValue(1);
		ui.editPartBill->setCurrentText("Normal");
		ui.editPartBlend->setCurrentText("Blend");
		ui.editCustomMeshPointCount->setValue(20);
		ui.dockParticles->hide();
		ui.dockParticles->setEnabled(false);
		ui.editCustomMeshPointCount->setEnabled(false);
		ui.dockEditPart->setEnabled(false);
		ui.editParticleCreateNum->setValue(0);
		ui.editParticleCreate->setValue(0);
		ui.editParticleFrameAppear->setValue(0);
		ui.editParticleFrameKeep->setValue(0);
		ui.editParticleFrameDisappear->setValue(0);
		ui.editParticleStartPosVar->setValue(0);
		ui.editParticleStartPosVarY->setValue(0);
		ui.editParticleYLow->setValue(0);
		ui.editParticleXZLow->setValue(0);
		ui.editParticleYHigh->setValue(0);
		ui.editParticleXZHigh->setValue(0);
		ui.editParticleAccelX->setValue(0);
		ui.editParticleAccelY->setValue(0);
		ui.editParticleAccelZ->setValue(0);
		ui.editParticleScaleX->setValue(0);
		ui.editParticleScaleY->setValue(0);
		ui.editParticleScaleZ->setValue(0);
		ui.editParticleScaleSpeedX->setValue(0);
		ui.editParticleScaleSpeedY->setValue(0);
		ui.editParticleScaleSpeedZ->setValue(0);
		ui.editParticleRotationLowX->setValue(0);
		ui.editParticleRotationLowY->setValue(0);
		ui.editParticleRotationLowZ->setValue(0);
		ui.editParticleRotationHighX->setValue(0);
		ui.editParticleRotationHighY->setValue(0);
		ui.editParticleRotationHighZ->setValue(0);
		ui.editParticleRepeatScale->setChecked(false);
		ui.editParticleRepeat->setChecked(false);
		ui.editParticleScalSpeedXLow->setValue(0);
		ui.editParticleScalSpeedYLow->setValue(0);
		ui.editParticleScalSpeedZLow->setValue(0);
		ui.editParticleScalSpeedXHigh->setValue(0);
		ui.editParticleScalSpeedYHigh->setValue(0);
		ui.editParticleScalSpeedZHigh->setValue(0);
		ui.editParticleScaleEndX->setValue(0);
		ui.editParticleScaleEndY->setValue(0);
		ui.editParticleScaleEndZ->setValue(0);
	}
}

void CMainFrame::_editPart()
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

void CMainFrame::EditPartName(const QString& value)
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_name = value;

		const string name = MakePartName(part);
		m_timeline->RenameRow(m_currentEditPart, name);
		ui.listParts->item(m_currentEditPart)->setText(name);

		_editPart();
	}
}

void CMainFrame::EditPartVisible(bool value)
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_visible = value;

		_editPart();
	}
}

void CMainFrame::EditPartBill(const QString& value)
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];

		if (value == "Bill")
			part->m_billType = ESfxPartBillType::Bill;
		else if (value == "Normal")
			part->m_billType = ESfxPartBillType::Normal;
		else if (value == "Pole")
			part->m_billType = ESfxPartBillType::Pole;
		else if (value == "Bottom")
			part->m_billType = ESfxPartBillType::Bottom;

		_editPart();
	}
}

void CMainFrame::EditPartAlpha(const QString& value)
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];

		if (value == "Blend")
			part->m_alphaType = ESfxPartAlphaType::Blend;
		else if (value == "Glow")
			part->m_alphaType = ESfxPartAlphaType::Glow;

		_editPart();
	}
}

void CMainFrame::EditPartTextureName()
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];

		part->m_textureName = ui.editPartTexture->text();
		part->_setTexture();

		_editPart();
	}
}

void CMainFrame::EditPartTexture()
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];

		string oldFilename;
		if (GetExtension(part->m_textureName) == "o3d")
			oldFilename = "Model/";
		else
			oldFilename = "SFX/Texture/";
		oldFilename += part->m_textureName;

		const string filename = QFileDialog::getOpenFileName(this, tr("Charger une texture/model"), oldFilename, tr("Fichier texture") + " (*.dds *.tga *.bmp);; " + tr("Fichier 3D") + " (*.o3d)");

		if (!filename.isEmpty())
		{
			ui.editPartTexture->setText(QFileInfo(filename).fileName());

			part->m_textureName = ui.editPartTexture->text();
			part->_setTexture();

			_editPart();
		}
	}
}

void CMainFrame::EditPartTexFrame(int value)
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];

		if (part->m_texFrame != (ushort)value)
		{
			part->m_texFrame = (ushort)value;
			part->_setTexture();

			_editPart();
		}
	}
}

void CMainFrame::EditPartTexLoop(int value)
{
	if (m_currentEditPart > -1)
	{
		CSfxPart* part = m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_texLoop = (ushort)value;

		_editPart();
	}
}

void CMainFrame::EditPartPointCount(int value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::CustomMesh)
	{
		CSfxPartCustomMesh* part = (CSfxPartCustomMesh*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_pointCount = value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleCreate(int value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleCreate = (ushort)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleCreateNum(int value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleCreateNum = (ushort)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleFrameAppear(int value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleFrameAppear = (ushort)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleFrameKeep(int value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleFrameKeep = (ushort)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleFrameDisappear(int value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleFrameDisappear = (ushort)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleStartPosVar(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleStartPosVar = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleStartPosVarY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleStartPosVarY = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleYLow(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleYLow = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleYHigh(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleYHigh = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleXZLow(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleXZLow = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleXZHigh(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleXZHigh = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleAccelX(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleAccel.x = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleAccelY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleAccel.y = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleAccelZ(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_particleAccel.z = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleX(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scale.x = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scale.y = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleZ(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scale.z = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRotationLowX(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_rotationLow.x = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRotationLowY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_rotationLow.y = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRotationLowZ(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_rotationLow.z = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRotationHighX(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_rotationHigh.x = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRotationHighY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_rotationHigh.y = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRotationHighZ(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_rotationHigh.z = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScalSpeedXLow(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scalSpeedXLow = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScalSpeedYLow(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scalSpeedYLow = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScalSpeedZLow(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scalSpeedZLow = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScalSpeedXHigh(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scalSpeedXHigh = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScalSpeedYHigh(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scalSpeedYHigh = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScalSpeedZHigh(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scalSpeedZHigh = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRepeatScale(bool value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_repeatScal = value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleRepeat(bool value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_repeat = value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleEndX(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scaleEnd.x = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleEndY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scaleEnd.y = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleEndZ(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scaleEnd.z = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleSpeedX(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scaleSpeed.x = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleSpeedY(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scaleSpeed.y = (float)value;

		_editPart();
	}
}

void CMainFrame::EditPartParticleScaleSpeedZ(double value)
{
	if (m_currentEditPart > -1 && m_sfx->m_sfx->m_parts[m_currentEditPart]->GetType() == ESfxPartType::Particle)
	{
		CSfxPartParticle* part = (CSfxPartParticle*)m_sfx->m_sfx->m_parts[m_currentEditPart];
		part->m_scaleSpeed.z = (float)value;

		_editPart();
	}
}