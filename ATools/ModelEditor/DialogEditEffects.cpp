///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "DialogEditEffects.h"
#include <Object3D.h>
#include "ModelViewer.h"
#include <TextureMng.h>

CDialogEditEffects::CDialogEditEffects(CModelViewer* modelViewer, CObject3D* obj3D, QWidget *parent)
	: QDialog(parent),
	m_modelViewer(modelViewer),
	m_obj3D(obj3D),
	m_oldBlocks(null),
	m_editing(null),
	m_editingObj(null),
	m_editingGroup(null)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui.spinOpacity->setEnabled(false);
	connect(ui.check2Sides, SIGNAL(toggled(bool)), this, SLOT(Set2Sides(bool)));
	connect(ui.checkHighlight, SIGNAL(toggled(bool)), this, SLOT(SetHighlight(bool)));
	connect(ui.checkOpacity, SIGNAL(toggled(bool)), this, SLOT(SetOpacity(bool)));
	connect(ui.checkReflect, SIGNAL(toggled(bool)), this, SLOT(SetReflect(bool)));
	connect(ui.checkSelfIlluminate, SIGNAL(toggled(bool)), this, SLOT(SetSelfIlluminate(bool)));
	connect(ui.spinOpacity, SIGNAL(valueChanged(int)), this, SLOT(SetAmount(int)));
	connect(ui.changeTexture, SIGNAL(clicked()), this, SLOT(ChangeTexture()));
	connect(ui.CWOrCCW, SIGNAL(clicked()), this, SLOT(ChangeCW()));

	int materialBlockCount = 0;

	for (int i = 0; i < (m_obj3D->m_LOD ? MAX_GROUP : 1); i++)
		for (int j = 0; j < m_obj3D->m_groups[i].objectCount; j++)
			for (int k = 0; k < m_obj3D->m_groups[i].objects[j].materialBlockCount; k++)
				materialBlockCount++;

	m_oldBlocks = new MaterialBlock[materialBlockCount];
	_setTree();

	connect(ui.tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(CurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}

CDialogEditEffects::~CDialogEditEffects()
{
	DeleteArray(m_oldBlocks);
}

void CDialogEditEffects::ResetBlocks()
{
	int id = 0;
	for (int i = 0; i < (m_obj3D->m_LOD ? MAX_GROUP : 1); i++)
	{
		for (int j = 0; j < m_obj3D->m_groups[i].objectCount; j++)
		{
			for (int k = 0; k < m_obj3D->m_groups[i].objects[j].materialBlockCount; k++)
			{
				m_obj3D->m_groups[i].objects[j].materialBlocks[k] = m_oldBlocks[id];
				id++;
			}
		}
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::CurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	m_editing = null;
	m_editingObj = null;
	m_editingGroup = null;

	ui.checkReflect->setEnabled(false);
	ui.checkOpacity->setEnabled(false);
	ui.check2Sides->setEnabled(false);
	ui.checkSelfIlluminate->setEnabled(false);
	ui.spinOpacity->setEnabled(false);
	ui.checkHighlight->setEnabled(false);
	ui.changeTexture->setEnabled(false);
	ui.checkReflect->setChecked(false);
	ui.checkOpacity->setChecked(false);
	ui.check2Sides->setChecked(false);
	ui.checkSelfIlluminate->setChecked(false);
	ui.spinOpacity->setValue(255);
	ui.checkHighlight->setChecked(false);
	ui.CWOrCCW->setEnabled(false);

	if (current && current->type() > 0)
	{
		int id = 1;
		for (int i = 0; i < (m_obj3D->m_LOD ? MAX_GROUP : 1); i++)
		{
			for (int j = 0; j < m_obj3D->m_groups[i].objectCount; j++)
			{
				for (int k = 0; k < m_obj3D->m_groups[i].objects[j].materialBlockCount; k++)
				{
					if (id == current->type())
					{
						MaterialBlock* editing = &m_obj3D->m_groups[i].objects[j].materialBlocks[k];

						ui.checkReflect->setEnabled(true);
						ui.checkOpacity->setEnabled(true);
						ui.check2Sides->setEnabled(true);
						ui.checkSelfIlluminate->setEnabled(true);
						ui.checkHighlight->setEnabled(true);
						ui.CWOrCCW->setEnabled(true);

						ui.checkReflect->setChecked(editing->effect & XE_REFLECT);
						ui.checkHighlight->setChecked(editing->effect & XE_HIGHLIGHT_OBJ);
						ui.checkOpacity->setChecked(editing->effect & XE_OPACITY);
						ui.check2Sides->setChecked(editing->effect & XE_2SIDE);
						ui.checkSelfIlluminate->setChecked(editing->effect & XE_SELF_ILLUMINATE);

						if (ui.checkOpacity->isChecked())
						{
							ui.spinOpacity->setValue(editing->amount);
							ui.spinOpacity->setEnabled(true);
						}

						if (m_obj3D->m_groups[i].objects[j].material)
							ui.changeTexture->setEnabled(true);

						m_editing = editing;
						m_editingObj = &m_obj3D->m_groups[i].objects[j];
						m_editingGroup = &m_obj3D->m_groups[i];
						return;
					}
					id++;
				}
			}
		}
	}
}

void CDialogEditEffects::_setTree()
{
	ui.tree->clear();
	m_items.clear();

	QTreeWidgetItem* item, *item2, *item3;
	QList<QTreeWidgetItem*> items;
	int id = 0;
	for (int i = 0; i < (m_obj3D->m_LOD ? MAX_GROUP : 1); i++)
	{
		if (m_obj3D->m_LOD)
			item3 = new QTreeWidgetItem(QStringList(string("LOD %1").arg(i)));

		for (int j = 0; j < m_obj3D->m_groups[i].objectCount; j++)
		{
			item = new QTreeWidgetItem(QStringList(string("GMObject %1").arg(j + 1)));
			for (int k = 0; k < m_obj3D->m_groups[i].objects[j].materialBlockCount; k++)
			{
				m_oldBlocks[id] = m_obj3D->m_groups[i].objects[j].materialBlocks[k];

				if (m_obj3D->m_groups[i].objects[j].material)
				{
					item2 = new QTreeWidgetItem(QStringList(string(
						string(m_obj3D->m_groups[i].objects[j].materials[m_obj3D->m_groups[i].objects[j].materialBlocks[k].materialID].textureName) + " %1").arg(
						k + 1)), id + 1);
				}
				else
				{
					item2 = new QTreeWidgetItem(QStringList(string(
						string("- %1")).arg(
						k + 1)), id + 1);
				}

				item->addChild(item2);

				m_items[&m_obj3D->m_groups[i].objects[j].materialBlocks[k]] = item2;

				id++;
			}

			if (m_obj3D->m_LOD)
				item3->addChild(item);
			else
				items.append(item);
		}

		if (m_obj3D->m_LOD)
			items.append(item3);
	}

	ui.tree->insertTopLevelItems(0, items);
	ui.tree->expandAll();
}

void CDialogEditEffects::Set2Sides(bool b)
{
	const int effect = XE_2SIDE;
	if (m_editing)
	{
		if (b)
			m_editing->effect |= effect;
		else
			m_editing->effect &= ~effect;
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::SetOpacity(bool b)
{
	const int effect = XE_OPACITY;
	if (m_editing)
	{
		if (b)
		{
			ui.spinOpacity->setEnabled(true);

			if (m_editing->amount == 0)
			{
				m_editing->amount = 255;
				ui.spinOpacity->setValue(255);
			}

			m_editing->effect |= effect;
		}
		else
		{
			m_editing->effect &= ~effect;
			ui.spinOpacity->setEnabled(false);
		}
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::SetReflect(bool b)
{
	const int effect = XE_REFLECT;
	if (m_editing)
	{
		if (b)
			m_editing->effect |= effect;
		else
			m_editing->effect &= ~effect;
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::SetSelfIlluminate(bool b)
{
	const int effect = XE_SELF_ILLUMINATE;
	if (m_editing)
	{
		if (b)
			m_editing->effect |= effect;
		else
			m_editing->effect &= ~effect;
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::SetHighlight(bool b)
{
	const int effect = XE_HIGHLIGHT_OBJ;
	if (m_editing)
	{
		if (b)
			m_editing->effect |= effect;
		else
			m_editing->effect &= ~effect;
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::SetAmount(int v)
{
	if (m_editing)
		m_editing->amount = v;

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::ChangeTexture()
{
	if (m_editing && m_editingObj && m_editingGroup)
	{
		Material* mat = &m_editingObj->materials[m_editing->materialID];

		const string texture = QFileDialog::getOpenFileName(this, tr("Charger une texture"),
			"Model/Texture/" + string(mat->textureName), tr("Fichier texture") + " (*.dds *.tga *.bmp *.png)");

		if (!texture.isEmpty())
		{
			QFileInfo fileInfo(texture);
			const string text = fileInfo.fileName().left(255).toLower();
			TextureMng->SetModelTexturePath(fileInfo.path() + '/');

			const QByteArray data = text.toLocal8Bit();
			char* textureName = mat->textureName;

			memcpy(textureName, data.constData(), data.size());
			textureName[data.size()] = '\0';

			memset(mat->textures, 0, sizeof(CTexture*) * 8);
			m_editingGroup->currentTextureEx = -1;

			MaterialBlock* oldEditing = m_editing;
			_setTree();
			auto it = m_items.find(oldEditing);
			if (it != m_items.end())
				ui.tree->setCurrentItem(it.value());
		}
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CDialogEditEffects::ChangeCW()
{
	if (m_editing && m_editingObj && m_editingGroup)
	{
		const int startIndex = m_editing->startVertex;
		const int primitiveCount = m_editing->primitiveCount;

		for (int j = 0; j < primitiveCount; j++)
			std::swap(m_editingObj->indices[startIndex + j * 3], m_editingObj->indices[startIndex + j * 3 + 2]);

		if (m_editingObj->IB)
		{
			const int bufferSize = sizeof(ushort) * m_editingObj->indexCount;
			void* data;
			if (SUCCEEDED(m_editingObj->IB->Lock(0, bufferSize, &data, 0)))
			{
				memcpy(data, m_editingObj->indices, bufferSize);
				m_editingObj->IB->Unlock();
			}
		}
	}

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}