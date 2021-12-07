///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "TerrainEdition.h"
#include "ObjectEdition.h"
#include "MainFrame.h"
#include <World.h>
#include <Object.h>
#include <Region.h>
#include <WorldEditor.h>
#include <SpawnObject.h>
#include <Path.h>
#include <Mover.h>
#include <ModelMng.h>

#include "ui_Random.h"

class CCustomDoubleSpinBox : public QDoubleSpinBox
{
public:
	CCustomDoubleSpinBox(QWidget* parent = null) : QDoubleSpinBox(parent) { }

protected:
	virtual void keyPressEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
		{
			((QDialog*)parent())->accept();
			event->accept();
			return;
		}
		QDoubleSpinBox::keyPressEvent(event);
	}
};

void CMainFrame::OptimizeWater()
{
	if (!m_world)
		return;

	CEditWaterCommand* command = new CEditWaterCommand(m_world);
	command->Optimize();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::FillAllMapWithWater()
{
	if (!m_world)
		return;

	CEditWaterCommand* command = new CEditWaterCommand(m_world);
	command->FillAllMap();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::DeleteLandLayer()
{
	if (!m_world || !m_currentLand || ui.editTerrainLayerList->count() <= 0)
		return;

	auto selection = ui.editTerrainLayerList->selectedItems();
	if (selection.count() <= 0)
		return;

	const int index = ui.editTerrainLayerList->row(selection[0]);
	if (index < 0 || index >= m_currentLand->m_layers.GetSize())
		return;

	CDeleteTerrainLayerCommand* command = new CDeleteTerrainLayerCommand(m_world, m_currentLand, m_currentLand->m_layers[index]->textureID);
	command->Apply();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::HideObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	CObjectHideCommand* command = new CObjectHideCommand(m_world);
	command->Hide();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::ShowAllObjects()
{
	if (!m_world)
		return;

	CObjectHideCommand* command = new CObjectHideCommand(m_world);
	command->ShowAll();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::HideUpstairObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	CObjectHideCommand* command = new CObjectHideCommand(m_world);
	command->HideUpstair();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::DeleteObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	CObjectDeleteCommand* command = new CObjectDeleteCommand(m_world);
	command->DeleteSelection();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::DeleteAllObjects()
{
	if (!m_world)
		return;

	CObjectDeleteCommand* command = new CObjectDeleteCommand(m_world);
	command->DeleteAll();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::CopyObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	for (int i = 0; i < m_clipboardObjects.GetSize(); i++)
		Delete(m_clipboardObjects[i]);
	m_clipboardObjects.RemoveAll();

	D3DXVECTOR3 center(0, 0, 0);
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
		center += CWorld::s_selection[i]->m_pos;
	center /= (float)CWorld::s_selection.GetSize();

	CObject* obj, *newObj;
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		obj = CWorld::s_selection[i];
		newObj = CObject::CreateObject(obj->m_type, obj);
		newObj->SetPos(obj->m_pos - center);

		if (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
		{
			((CSpawnObject*)newObj)->m_rect = QRect(
				((CSpawnObject*)obj)->m_rect.topLeft() - QPoint((int)(obj->m_pos.z + 0.5f), (int)(obj->m_pos.x + 0.5f)),
				((CSpawnObject*)obj)->m_rect.size()
				);
		}
		else if (obj->m_type == OT_REGION)
		{
			((CRegion*)newObj)->m_rect = QRect(
				((CRegion*)obj)->m_rect.topLeft() - QPoint((int)(obj->m_pos.z + 0.5f), (int)(obj->m_pos.x + 0.5f)),
				((CRegion*)obj)->m_rect.size()
				);
		}

		m_clipboardObjects.Append(newObj);
	}
}

void CMainFrame::PasteObjects()
{
	if (!m_world || m_clipboardObjects.GetSize() < 1 || !m_editor->IsPickingTerrain())
		return;

	const D3DXVECTOR3 terrainPos = m_editor->GetTerrainPos();

	CObjectDeleteCommand* command = new CObjectDeleteCommand(m_world);

	CObject* obj, *newObj;
	for (int i = 0; i < m_clipboardObjects.GetSize(); i++)
	{
		obj = m_clipboardObjects[i];
		if (obj->m_type == OT_PATH && MainFrame->GetCurrentPatrol() == -1)
			continue;

		newObj = CObject::CreateObject(obj->m_type, obj);
		newObj->m_ID = 0;
		newObj->SetPos(obj->m_pos + terrainPos);

		if (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
		{
			CSpawnObject* dyna = ((CSpawnObject*)newObj);
			dyna->m_rect = QRect(
				QPoint((int)(newObj->m_pos.z + 0.5f), (int)(newObj->m_pos.x + 0.5f)) + dyna->m_rect.topLeft(),
				dyna->m_rect.size()
				);

			if (dyna->m_rect.left() < 0)
				dyna->m_rect.setLeft(0);
			else if (dyna->m_rect.left() >= m_world->m_height * MAP_SIZE * MPU)
				dyna->m_rect.setLeft(m_world->m_height * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.right() < 0)
				dyna->m_rect.setRight(0);
			else if (dyna->m_rect.right() >= m_world->m_height * MAP_SIZE * MPU)
				dyna->m_rect.setRight(m_world->m_height * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.bottom() < 0)
				dyna->m_rect.setBottom(0);
			else if (dyna->m_rect.bottom() >= m_world->m_width * MAP_SIZE * MPU)
				dyna->m_rect.setBottom(m_world->m_width * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.top() < 0)
				dyna->m_rect.setTop(0);
			else if (dyna->m_rect.top() >= m_world->m_width * MAP_SIZE * MPU)
				dyna->m_rect.setTop(m_world->m_height * MAP_SIZE * MPU - 1);
			dyna->m_rect = dyna->m_rect.normalized();
		}
		else if (obj->m_type == OT_REGION)
		{
			CRegion* dyna = ((CRegion*)newObj);
			dyna->m_rect = QRect(
				QPoint((int)(newObj->m_pos.z + 0.5f), (int)(newObj->m_pos.x + 0.5f)) + dyna->m_rect.topLeft(),
				dyna->m_rect.size()
				);

			if (dyna->m_rect.left() < 0)
				dyna->m_rect.setLeft(0);
			else if (dyna->m_rect.left() >= m_world->m_height * MAP_SIZE * MPU)
				dyna->m_rect.setLeft(m_world->m_height * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.right() < 0)
				dyna->m_rect.setRight(0);
			else if (dyna->m_rect.right() >= m_world->m_height * MAP_SIZE * MPU)
				dyna->m_rect.setRight(m_world->m_height * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.bottom() < 0)
				dyna->m_rect.setBottom(0);
			else if (dyna->m_rect.bottom() >= m_world->m_width * MAP_SIZE * MPU)
				dyna->m_rect.setBottom(m_world->m_width * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.top() < 0)
				dyna->m_rect.setTop(0);
			else if (dyna->m_rect.top() >= m_world->m_width * MAP_SIZE * MPU)
				dyna->m_rect.setTop(m_world->m_height * MAP_SIZE * MPU - 1);
			dyna->m_rect = dyna->m_rect.normalized();
		}

		command->AddCreateObject(newObj);
	}

	command->Apply();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::CutObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	CopyObjects();
	DeleteObjects();
}

void CMainFrame::keyPressEvent(QKeyEvent* event)
{
	if (m_editor && m_world && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right))
	{
		RotateObjects(event->key());
		event->accept();
	}
}

void CMainFrame::RotateObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	HideDialogs();

	QDialog* dialog = new QDialog(this, Qt::Window);
	dialog->setModal(true);
	dialog->setWindowTitle(tr("Rotation"));
	QBoxLayout * layout = new QBoxLayout(QBoxLayout::LeftToRight, dialog);
	dialog->setLayout(layout);
	CCustomDoubleSpinBox* spinX = new CCustomDoubleSpinBox(dialog);
	spinX->setMinimum(-360.0);
	spinX->setMaximum(360.0);
	spinX->setSingleStep(10.0);
	spinX->setDecimals(3);
	spinX->setPrefix(tr("X: "));
	spinX->setValue(fmod(CWorld::s_selection[0]->GetRot().x, 360.0f));
	layout->addWidget(spinX);
	CCustomDoubleSpinBox* spinY = new CCustomDoubleSpinBox(dialog);
	spinY->setMinimum(-360.0);
	spinY->setMaximum(360.0);
	spinY->setSingleStep(10.0);
	spinY->setDecimals(3);
	spinY->setPrefix(tr("Y: "));
	spinY->setValue(fmod(CWorld::s_selection[0]->GetRot().y, 360.0f));
	layout->addWidget(spinY);
	CCustomDoubleSpinBox* spinZ = new CCustomDoubleSpinBox(dialog);
	spinZ->setMinimum(-360.0);
	spinZ->setMaximum(360.0);
	spinZ->setSingleStep(10.0);
	spinZ->setDecimals(3);
	spinZ->setPrefix(tr("Z: "));
	spinZ->setValue(fmod(CWorld::s_selection[0]->GetRot().z, 360.0f));
	layout->addWidget(spinZ);
	spinY->setFocus();

	if (dialog->exec() == QDialog::Accepted)
	{
		CObjectTransformCommand* command = new CObjectTransformCommand(m_world);

		const D3DXVECTOR3 rot((float)spinX->value(),
			(float)spinY->value(),
			(float)spinZ->value());

		for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
			command->SetRotate(CWorld::s_selection[i], rot);

		command->Apply();
		AddCommand(command);
		UpdateWorldEditor();
	}

	Delete(dialog);
	ShowDialogs();
}

void CMainFrame::TranslateObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	HideDialogs();

	D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
		center += CWorld::s_selection[i]->GetPos();
	center /= (float)CWorld::s_selection.GetSize();

	QDialog* dialog = new QDialog(this, Qt::Window);
	dialog->setModal(true);
	dialog->setWindowTitle(tr("Translation"));
	QBoxLayout * layout = new QBoxLayout(QBoxLayout::LeftToRight, dialog);
	dialog->setLayout(layout);
	CCustomDoubleSpinBox* spinX = new CCustomDoubleSpinBox(dialog);
	spinX->setMinimum(0.0);
	spinX->setMaximum(m_world->m_width * MAP_SIZE * MPU);
	spinX->setSingleStep(10.0);
	spinX->setDecimals(3);
	spinX->setPrefix(tr("X: "));
	spinX->setValue((float)center.x);
	layout->addWidget(spinX);
	CCustomDoubleSpinBox* spinY = new CCustomDoubleSpinBox(dialog);
	spinY->setMinimum(0.0);
	spinY->setMaximum(9999.0);
	spinY->setSingleStep(10.0);
	spinY->setDecimals(3);
	spinY->setPrefix(tr("Y: "));
	spinY->setValue((float)center.y);
	layout->addWidget(spinY);
	CCustomDoubleSpinBox* spinZ = new CCustomDoubleSpinBox(dialog);
	spinZ->setMinimum(0.0);
	spinZ->setMaximum(m_world->m_height * MAP_SIZE * MPU);
	spinZ->setSingleStep(10.0);
	spinZ->setDecimals(3);
	spinZ->setPrefix(tr("Z: "));
	spinZ->setValue((float)center.z);
	layout->addWidget(spinZ);
	spinX->setFocus();

	if (dialog->exec() == QDialog::Accepted)
	{
		CObjectTransformCommand* command = new CObjectTransformCommand(m_world);

		center.x -= (float)spinX->value();
		center.y -= (float)spinY->value();
		center.z -= (float)spinZ->value();

		for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
			command->SetTranslate(CWorld::s_selection[i], CWorld::s_selection[i]->GetPos() - center);

		command->Apply();
		AddCommand(command);
		UpdateWorldEditor();
	}

	Delete(dialog);
	ShowDialogs();
}

void CMainFrame::ScaleObjects()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	HideDialogs();

	QDialog* dialog = new QDialog(this, Qt::Window);
	dialog->setModal(true);
	dialog->setWindowTitle(tr("Redimension"));
	QBoxLayout * layout = new QBoxLayout(QBoxLayout::LeftToRight, dialog);
	dialog->setLayout(layout);
	CCustomDoubleSpinBox* spinX = new CCustomDoubleSpinBox(dialog);
	spinX->setMinimum(-9999999.0);
	spinX->setMaximum(9999999.0);
	spinX->setSingleStep(0.1);
	spinX->setDecimals(3);
	spinX->setPrefix(tr("X: "));
	spinX->setValue(fmod(CWorld::s_selection[0]->GetScale().x, 360.0f));
	layout->addWidget(spinX);
	CCustomDoubleSpinBox* spinY = new CCustomDoubleSpinBox(dialog);
	spinY->setMinimum(-9999999.0);
	spinY->setMaximum(9999999.0);
	spinY->setSingleStep(0.1);
	spinY->setDecimals(3);
	spinY->setPrefix(tr("Y: "));
	spinY->setValue(fmod(CWorld::s_selection[0]->GetScale().y, 360.0f));
	layout->addWidget(spinY);
	CCustomDoubleSpinBox* spinZ = new CCustomDoubleSpinBox(dialog);
	spinZ->setMinimum(-9999999.0);
	spinZ->setMaximum(9999999.0);
	spinZ->setSingleStep(0.1);
	spinZ->setDecimals(3);
	spinZ->setPrefix(tr("Z: "));
	spinZ->setValue(fmod(CWorld::s_selection[0]->GetScale().z, 360.0f));
	layout->addWidget(spinZ);
	spinY->setFocus();

	if (dialog->exec() == QDialog::Accepted)
	{
		CObjectTransformCommand* command = new CObjectTransformCommand(m_world);

		const D3DXVECTOR3 scale((float)spinX->value(),
			(float)spinY->value(),
			(float)spinZ->value());

		for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
			command->SetScale(CWorld::s_selection[i], scale);

		command->Apply();
		AddCommand(command);
		UpdateWorldEditor();
	}

	Delete(dialog);
	ShowDialogs();
}

void CMainFrame::SetOnLand()
{
	if (!m_world)
		return;

	CObjectTransformCommand* command = new CObjectTransformCommand(m_world);

	D3DXVECTOR3 temp;
	CObject* obj;
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		obj = CWorld::s_selection[i];
		temp = obj->m_pos;
		temp.y = m_world->GetHeight(temp.x, temp.z);
		if (temp.y != obj->m_pos.y)
			command->SetTranslate(obj, temp);
	}

	command->Apply();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::RandomScaleAndSize()
{
	if (!m_world || CWorld::s_selection.GetSize() <= 0)
		return;

	HideDialogs();
	QDialog dialog(this);
	Ui::RandomDialog ui;
	ui.setupUi(&dialog);

	if (dialog.exec() == QDialog::Accepted)
	{
		D3DXVECTOR3 temp;
		CObject* obj;
		float min, max, temp2;

		CObjectTransformCommand* command = new CObjectTransformCommand(m_world);

		if (ui.scale->isChecked())
		{
			min = (float)ui.minScale->value();
			max = (float)ui.maxScale->value();
			if (min > max)
			{
				const float temp2 = min;
				min = max;
				max = temp2;
			}

			for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
			{
				obj = CWorld::s_selection[i];
				if (min > max)
					temp2 = max + (float)(rand()) / ((float)RAND_MAX / (min - max));
				else
					temp2 = min + (float)(rand()) / ((float)RAND_MAX / (max - min));
				temp.x = temp2;
				temp.y = temp2;
				temp.z = temp2;
				command->SetScale(obj, temp);
			}
		}

		if (ui.rotation->isChecked())
		{
			min = (float)ui.minRot->value();
			max = (float)ui.maxRot->value();

			for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
			{
				obj = CWorld::s_selection[i];
				if (min > max)
					temp2 = max + (float)(rand()) / ((float)RAND_MAX / (min - max));
				else
					temp2 = min + (float)(rand()) / ((float)RAND_MAX / (max - min));
				temp.x = obj->m_rot.x;
				temp.y = temp2;
				temp.z = obj->m_rot.z;
				command->SetRotate(obj, temp);
			}
		}

		command->Apply();
		AddCommand(command);
	}

	ShowDialogs();
	UpdateWorldEditor();
}

void CMainFrame::RotateObjects(int key)
{
	if (!m_editor || !m_world || g_global3D.renderMinimap || CWorld::s_selection.GetSize() <= 0)
		return;

	CObjectTransformCommand* command = new CObjectTransformCommand(m_world);

	if (key == Qt::Key_Left)
		command->EditRotate(EDIT_Y, 22.5f);
	if (key == Qt::Key_Right)
		command->EditRotate(EDIT_Y, -22.5f);

	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::AddPath()
{
	if (!m_world)
		return;

	CPathDeleteCommand* command = new CPathDeleteCommand(m_world);
	command->CreatePath();
	AddCommand(command);
	UpdateWorldEditor();
}

void CMainFrame::RemovePath()
{
	if (!m_world)
		return;

	m_currentPatrol = -1;
	const int current = ui.patrolList->currentRow();
	if (current == -1)
		return;

	QListWidgetItem* item = ui.patrolList->takeItem(current);
	if (!item)
		return;

	const int index = item->data(Qt::UserRole + 1).toInt();
	Delete(item);

	if (index != -1)
	{
		CPathDeleteCommand* command = new CPathDeleteCommand(m_world);
		command->RemovePath(index);
		AddCommand(command);
	}

	UpdateWorldEditor();
}

void CMainFrame::SetCurrentPath(int row)
{
	if (!m_world)
		return;

	QListWidgetItem* item = ui.patrolList->item(row);
	if (!item)
		return;

	const int index = item->data(Qt::UserRole + 1).toInt();

	auto it = m_world->m_paths.find(index);
	if (it != m_world->m_paths.end())
		m_currentPatrol = index;
	else
		m_currentPatrol = -1;
}

void CMainFrame::SetGravityEnabled(bool gravity)
{
	if (gravity && m_world && CWorld::s_selection.GetSize() > 0)
		SetOnLand();
}

void CMainFrame::SetOnGridEnabled(bool grid)
{
	if (grid && m_world && CWorld::s_selection.GetSize() > 0)
	{
		CObjectTransformCommand* command = new CObjectTransformCommand(m_world);
		command->SetOnGrid();
		AddCommand(command);
		UpdateWorldEditor();
	}
}