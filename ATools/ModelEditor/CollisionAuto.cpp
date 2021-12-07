///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include <AnimatedMesh.h>
#include <Object3D.h>
#include <ModelViewer.h>

void CMainFrame::CollisionAuto()
{
	if (!m_mesh)
		return;

	CObject3D* obj3D = m_mesh->m_elements[0].obj;
	if (!obj3D)
		return;

	GMObject* collObj = &obj3D->m_collObj;
	LODGroup* group = obj3D->m_group;

	Release(collObj->VB);
	Release(collObj->IB);
	DeleteArray(collObj->materialBlocks);
	DeleteArray(collObj->materials);
	DeleteArray(collObj->vertices);
	DeleteArray(collObj->indices);
	DeleteArray(collObj->frames);
	DeleteArray(collObj->vertexList);
	DeleteArray(collObj->physiqueVertices);

	bool normalObj = false;
	int i, j;
	GMObject* obj;

	for (i = 0; i < group->objectCount; i++)
	{
		obj = &group->objects[i];
		if (obj->type == GMT_NORMAL && !obj->light)
		{
			normalObj = true;
			break;
		}
	}

	if (!normalObj)
		return;

	memset(collObj, 0, sizeof(GMObject));
	collObj->type = GMT_NORMAL;
	collObj->parentType = GMT_ERROR;
	collObj->parentID = -1;
	D3DXMatrixIdentity(&collObj->transform);

	for (i = 0; i < group->objectCount; i++)
	{
		obj = &group->objects[i];
		if (obj->type == GMT_NORMAL && !obj->light)
		{
			collObj->vertexCount += obj->vertexCount;
			collObj->vertexListCount += obj->vertexListCount;
			collObj->faceListCount += obj->faceListCount;
			collObj->indexCount += obj->indexCount;
		}
	}

	collObj->vertices = new NormalVertex[collObj->vertexCount];
	NormalVertex* vertices = (NormalVertex*)collObj->vertices;
	ushort* indices = collObj->indices = new ushort[collObj->indexCount + collObj->vertexCount];
	ushort* IIB = collObj->IIB = collObj->indices + collObj->indexCount;
	D3DXVECTOR3* vertexList = collObj->vertexList = new D3DXVECTOR3[collObj->vertexListCount];

	collObj->vertices;
	for (i = 0; i < group->objectCount; i++)
	{
		obj = &group->objects[i];
		if (obj->type == GMT_NORMAL && !obj->light)
		{
			for (j = 0; j < obj->vertexCount; j++)
				D3DXVec3TransformCoord(&vertices[j].p, &((NormalVertex*)obj->vertices)[j].p, &group->updates[i]);
			vertices += obj->vertexCount;

			memcpy(indices, obj->indices, obj->indexCount * sizeof(ushort));
			indices += obj->indexCount;

			memcpy(IIB, obj->IIB, obj->vertexCount * sizeof(ushort));
			IIB += obj->vertexCount;

			for (j = 0; j < obj->vertexListCount; j++)
				D3DXVec3TransformCoord(&vertexList[j], &obj->vertexList[j], &group->updates[i]);
			vertexList += obj->vertexListCount;
		}
	}

	collObj->materialBlockCount = 1;
	collObj->materialBlocks = new MaterialBlock[1];
	memset(&collObj->materialBlocks[0], 0, sizeof(MaterialBlock));
	collObj->materialBlocks[0].primitiveCount = collObj->faceListCount;

	D3DXVECTOR3 bbMin(FLT_MAX, FLT_MAX, FLT_MAX);
	D3DXVECTOR3 bbMax(FLT_MIN, FLT_MIN, FLT_MIN);

	for (i = 0; i < collObj->vertexListCount; i++)
	{
		const D3DXVECTOR3& v = collObj->vertexList[i];

		if (v.x < bbMin.x)
			bbMin.x = v.x;
		if (v.y < bbMin.y)
			bbMin.y = v.y;
		if (v.z < bbMin.z)
			bbMin.z = v.z;

		if (v.x > bbMax.x)
			bbMax.x = v.x;
		if (v.y > bbMax.y)
			bbMax.y = v.y;
		if (v.z > bbMax.z)
			bbMax.z = v.z;
	}

	collObj->bounds.Min = bbMin;
	collObj->bounds.Max = bbMax;

	LPDIRECT3DDEVICE9 device = obj3D->m_device;
	if (!device)
		return;

	int bufferSize = sizeof(CollisionVertex) * collObj->vertexCount;
	if (SUCCEEDED(device->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY, CollisionVertex::FVF, D3DPOOL_MANAGED, &collObj->VB, null)))
	{
		CollisionVertex* VB;
		if (SUCCEEDED(collObj->VB->Lock(0, bufferSize, (void**)&VB, 0)))
		{
			if (collObj->type == GMT_NORMAL)
			{
				NormalVertex* vertices = (NormalVertex*)collObj->vertices;
				for (i = 0; i < collObj->vertexCount; i++)
				{
					VB[i].p = vertices[i].p;
					VB[i].c = 0xffffffff;
				}
			}
			else
			{
				SkinVertex* vertices = (SkinVertex*)collObj->vertices;
				for (i = 0; i < collObj->vertexCount; i++)
				{
					VB[i].p = vertices[i].p;
					VB[i].c = 0xffffffff;
				}
			}
			collObj->VB->Unlock();
		}
	}

	bufferSize = sizeof(ushort) * collObj->indexCount;
	if (SUCCEEDED(device->CreateIndexBuffer(bufferSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &collObj->IB, null)))
	{
		void* data;
		if (SUCCEEDED(collObj->IB->Lock(0, bufferSize, &data, 0)))
		{
			memcpy(data, collObj->indices, bufferSize);
			collObj->IB->Unlock();
		}
	}

	ui.actionObjet_de_collision->setChecked(true);
	ShowCollObj(true);
}