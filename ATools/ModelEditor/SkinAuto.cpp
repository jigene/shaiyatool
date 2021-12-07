///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "ModelViewer.h"
#include <AnimatedMesh.h>
#include <Object3D.h>
#include "SkinAuto.h"
#include <ModelMng.h>
#include <Motion.h>

void CMainFrame::SkinAuto()
{
	if (!m_mesh)
		return;

	bool useful = false;

	CObject3D* obj3D;
	int i, j, k;
	for (i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		if (m_mesh->m_elements[i].obj)
		{
			obj3D = m_mesh->m_elements[i].obj;
			if (obj3D)
			{
				for (j = 0; j < (obj3D->m_LOD ? MAX_GROUP : 1); j++)
					for (k = 0; k < obj3D->m_groups[j].objectCount; k++)
						if (obj3D->m_groups[j].objects[k].type == GMT_NORMAL)
							useful = true;
			}
		}
	}

	if (!useful)
		return;

	CDialogSkinAuto(m_mesh, this).exec();

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

CDialogSkinAuto::CDialogSkinAuto(CAnimatedMesh* mesh, QWidget* parent)
	: QDialog(parent),
	m_mesh(mesh)
{
	ui.setupUi(this);

	connect(ui.loadSkel, SIGNAL(clicked()), this, SLOT(LoadSkel()));
	connect(ui.okSkel, SIGNAL(clicked()), this, SLOT(SkinSkel()));
}

CDialogSkinAuto::~CDialogSkinAuto()
{
}

void CDialogSkinAuto::LoadSkel()
{
	const string skel = QFileDialog::getOpenFileName(this, tr("Charger un squelette"),
		"Model/", tr("Fichier squelette") + " (*.chr)");

	if (!skel.isEmpty())
	{
		QFileInfo fileInfo(skel);
		ModelMng->SetModelPath(fileInfo.path() + '/');
		CSkeleton* skl = ModelMng->GetSkeleton(fileInfo.fileName());

		if (skl)
		{
			Delete(m_mesh->m_skeleton);
			m_mesh->m_skeleton = skl;
			DeleteArray(m_mesh->m_bones);
			ui.bonesList->clear();
			m_items.clear();

			ui.skelName->setText(fileInfo.fileName());

			m_mesh->m_bones = new D3DXMATRIX[skl->GetBoneCount() * 2];
			m_mesh->m_invBones = m_mesh->m_bones + skl->GetBoneCount();
			skl->ResetBones(m_mesh->m_bones, m_mesh->m_invBones);

			QList<QTreeWidgetItem*> items;

			QTreeWidgetItem* item;
			Bone* bone;
			for (int i = 0; i < skl->GetBoneCount(); i++)
			{
				bone = &skl->m_bones[i];
				item = new QTreeWidgetItem(QStringList(bone->name));
				item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
				item->setCheckState(0, Qt::Unchecked);
				item->setData(0, Qt::UserRole + 1, QVariant(i));
				m_items[i] = item;

				if (bone->parentID == -1)
					items.append(item);
				else
					m_items[bone->parentID]->addChild(item);
			}

			ui.bonesList->insertTopLevelItems(0, items);
			ui.bonesList->expandAll();

			ui.okSkel->setEnabled(true);
		}
	}
}

void CDialogSkinAuto::SkinSkel()
{
	QList<int> bones;
	for (auto it = m_items.begin(); it != m_items.end(); it++)
	{
		if (it.value()->checkState(0) & Qt::Checked)
			bones.append(it.key());
	}

	if (bones.size() <= 0)
		return;

	if (bones.size() > MAX_BONES)
	{
		QMessageBox::warning(this, tr("Attention"), tr("Vous ne devez pas sélectionner plus de") + ' ' + string::number(MAX_BONES) + ' ' + tr("os..."));
		return;
	}

	QList<D3DXVECTOR3> bonePos;
	D3DXVECTOR3 v;
	int i, j, k, l, m, n, boneID, bufferSize;

	for (i = 0; i < bones.size(); i++)
	{
		D3DXVec3TransformCoord(&v, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &m_mesh->m_skeleton->m_bones[bones[i]].TM);
		bonePos.append(v);
	}

	QList<GMObject*> objs;
	CObject3D* obj3D;
	for (i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		if (m_mesh->m_elements[i].obj)
		{
			obj3D = m_mesh->m_elements[i].obj;
			if (obj3D)
			{
				for (j = 0; j < (obj3D->m_LOD ? MAX_GROUP : 1); j++)
					for (k = 0; k < obj3D->m_groups[j].objectCount; k++)
						if (obj3D->m_groups[j].objects[k].type == GMT_NORMAL)
						{
							objs.append(&obj3D->m_groups[j].objects[k]);
							obj3D->m_hasSkin = true;
						}
			}
		}
	}

	GMObject* obj;
	SkinVertex* vertices;
	NormalVertex* oldVertices;
	SkinVertex* vertex;
	MaterialBlock* block;
	int boneIDs[MAX_BONES];
	void* data;
	QMap<float, int> vertexBones;

	for (l = 0; l < objs.size(); l++)
	{
		obj = objs[l];
		obj->type = GMT_SKIN;

		oldVertices = (NormalVertex*)obj->vertices;
		vertices = new SkinVertex[obj->vertexCount];

		for (m = 0; m < obj->vertexCount; m++)
		{
			v = oldVertices[m].p;
			vertices[m].p = v;
			vertices[m].t = oldVertices[m].t;
			vertices[m].n = oldVertices[m].n;

			vertexBones.clear();
			for (n = 0; n < bonePos.size(); n++)
				vertexBones[D3DXVec3Length(&(bonePos[n] - v))] = n;

			vertices[m].w1 = 1.0f;
			vertices[m].w2 = 0.0f;
			vertices[m].id1 = (ushort)(vertexBones.begin().value() * 3);
			vertices[m].id2 = 0;
		}

		obj->usedBoneCount = bones.size();
		for (m = 0; m < obj->usedBoneCount; m++)
			obj->usedBones[m] = bones[m];

		DeleteArray(obj->vertices);
		obj->vertices = vertices;

		obj->physiqueVertices = new int[obj->vertexListCount];
		memset(obj->physiqueVertices, 0, sizeof(int) * obj->vertexListCount);
		memset(boneIDs, sizeof(boneIDs), 0);

		if (obj->usedBoneCount > 0)
		{
			for (i = 0; i < obj->usedBoneCount; i++)
				boneIDs[i] = obj->usedBones[i];
		}
		else
		{
			for (i = 0; i < MAX_BONES; i++)
				boneIDs[i] = i;
		}

		for (i = 0; i < obj->materialBlockCount; i++)
		{
			block = &obj->materialBlocks[i];

			if (block->usedBoneCount > 0)
			{
				for (j = 0; j < block->usedBoneCount; j++)
					boneIDs[j] = block->usedBones[j];
			}

			for (j = 0; j < block->primitiveCount * 3; j++)
			{
				vertex = &vertices[obj->indices[block->startVertex + j]];

				if (vertex->w1 >= vertex->w2)
					boneID = boneIDs[vertex->id1 / 3];
				else
					boneID = boneIDs[vertex->id2 / 3];

				v = vertex->p;

				for (k = 0; k < obj->vertexListCount; k++)
				{
					if (obj->vertexList[k] == v)
					{
						obj->physiqueVertices[k] = boneID;
						break;
					}
				}
			}

			Release(obj->VB);

			bufferSize = sizeof(SkinVertex) * obj->vertexCount;
			if (SUCCEEDED(m_mesh->m_device->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY, SkinVertex::FVF, D3DPOOL_MANAGED, &obj->VB, null)))
			{
				if (SUCCEEDED(obj->VB->Lock(0, bufferSize, &data, 0)))
				{
					memcpy(data, obj->vertices, bufferSize);
					obj->VB->Unlock();
				}
			}
		}
	}

	close();
}