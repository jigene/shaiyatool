///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "WorldEditor.h"
#include "ObjectEdition.h"
#include <World.h>
#include <Object.h>
#include <ModelMng.h>
#include <SpawnObject.h>
#include <Region.h>
#include <Mover.h>
#include <Project.h>
#include <Path.h>

void CObjectHideCommand::Hide()
{
	m_hide = true;

	CObject* obj;
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		obj = CWorld::s_selection[i];
		if (!obj->IsUnvisible())
			m_objects.push_back(obj->GetID());
	}
	CWorld::s_selection.RemoveAll();
	Apply();
}

void CObjectHideCommand::ShowAll()
{
	m_hide = false;

	CObject* obj;
	uint i;
	int j;
	for (i = 0; i < MAX_OBJTYPE; i++)
	{
		const CPtrArray<CObject>& objects = m_world->GetObjects(i);
		for (j = 0; j < objects.GetSize(); j++)
		{
			obj = objects[j];
			if (obj->IsUnvisible())
				m_objects.push_back(obj->GetID());
		}
	}
	Apply();
}

void CObjectHideCommand::HideUpstair()
{
	m_hide = true;

	D3DXVECTOR3 center(0, 0, 0);
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
		center += CWorld::s_selection[i]->GetPos();
	center /= (float)CWorld::s_selection.GetSize();

	CWorld::s_selection.RemoveAll();

	CObject* obj;
	uint i;
	int j;
	for (i = 0; i < MAX_OBJTYPE; i++)
	{
		const CPtrArray<CObject>& objects = m_world->GetObjects(i);
		for (j = 0; j < objects.GetSize(); j++)
		{
			obj = objects[j];
			if (!obj->IsUnvisible())
			{
				const D3DXVECTOR3& pos = obj->GetPos();

				if (pos.y < center.y)
					continue;

				if (D3DXVec2Length(&(D3DXVECTOR2(pos.x, pos.z) - D3DXVECTOR2(center.x, center.z))) > 100.0f)
					continue;

				m_objects.push_back(obj->GetID());
			}
		}
	}
	Apply();
}

void CObjectHideCommand::Apply(bool undo)
{
	CObject* obj;
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		obj = m_world->GetObject(m_objects[i]);
		if (obj)
		{
			obj->SetUnvisible(undo ? !m_hide : m_hide);

			if (g_global3D.spawnAllMovers
				&& (obj->GetType() == OT_MOVER || obj->GetType() == OT_ITEM || obj->GetType() == OT_CTRL)
				&& ((CSpawnObject*)obj)->IsRespawn())
				m_world->SpawnObject(obj, undo ? m_hide : !m_hide);

			const int find = CWorld::s_selection.Find(obj);
			if (find != -1)
				CWorld::s_selection.RemoveAt(find);
		}
	}
}

CObjectDeleteCommand::~CObjectDeleteCommand()
{
	for (int i = 0; i < m_objects.GetSize(); i++)
		Delete(m_objects[i]);
}

void CObjectDeleteCommand::DeleteSelection()
{
	CObject* obj;
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		obj = CWorld::s_selection[i];
		if (obj)
			m_objects.Append(CObject::CreateObject(obj->GetType(), obj));
	}
	CWorld::s_selection.RemoveAll();

	Apply();
}

void CObjectDeleteCommand::DeleteAll()
{
	m_deleteAll = true;

	CWorld::s_selection.RemoveAll();

	CObject* obj;
	int j;
	for (uint i = 0; i < MAX_OBJTYPE; i++)
	{
		const CPtrArray<CObject>& objects = m_world->GetObjects(i);
		for (j = 0; j < objects.GetSize(); j++)
		{
			obj = objects[j];
			if (obj && obj->IsReal())
				m_objects.Append(CObject::CreateObject(obj->GetType(), obj));
		}
	}

	Apply();
}

void CObjectDeleteCommand::AddCreateObject(CObject* obj)
{
	m_create = true;
	m_objects.Append(obj);

	if (obj->GetType() == OT_PATH)
		m_pathID = MainFrame->GetCurrentPatrol();
}

void CObjectDeleteCommand::Apply(bool undo)
{
	if (IsEmpty())
		return;

	CObject* obj;
	if ((!m_create && undo) || (m_create && !undo))
	{
		CWorld::s_selection.RemoveAll();

		for (int i = 0; i < m_objects.GetSize(); i++)
		{
			obj = CObject::CreateObject(m_objects[i]->GetType(), m_objects[i]);
			m_world->AddObject(obj);

			if (m_create)
				m_objects[i]->SetID(obj->GetID());

			if (!m_deleteAll)
			{
				if (CWorld::s_selection.Find(obj) == -1)
					CWorld::s_selection.Append(obj);
			}

			obj->Cull();

			if (obj->GetType() == OT_PATH)
			{
				CPtrArray<CPath>* path = m_world->GetPath(m_pathID);
				if (path)
				{
					CPath* objPath = (CPath*)obj;
					objPath->SetIndex(path->GetSize());
					path->Append(objPath);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < m_objects.GetSize(); i++)
		{
			obj = m_world->GetObject(m_objects[i]->GetID());
			if (obj)
			{
				m_world->DeleteObject(obj);

				const int find = CWorld::s_selection.Find(obj);
				if (find != -1)
					CWorld::s_selection.RemoveAt(find);
			}
		}
	}
}

void CObjectTransformCommand::SetRotate(CObject* obj, const D3DXVECTOR3& rot)
{
	ObjectEntry* entry = _getEntry(obj);
	if (!entry)
		return;

	entry->rot = rot;
}

void CObjectTransformCommand::EditRotate(int axis, float factor)
{
	const int objCount = CWorld::s_selection.GetSize();
	if (!objCount)
		return;

	D3DXVECTOR3 temp, temp2;
	const bool gravity = MainFrame->UseGravity();
	CObject* obj;
	const float s = sin(D3DXToRadian(factor));
	const float c = cos(D3DXToRadian(factor));
	float x, z;
	ObjectEntry* entry;

	D3DXVECTOR3 centroid(0, 0, 0);
	if (objCount > 1)
	{
		for (int i = 0; i < objCount; i++)
			centroid += CWorld::s_selection[i]->GetPos();
		centroid /= (float)objCount;
	}

	for (int i = 0; i < objCount; i++)
	{
		obj = CWorld::s_selection[i];
		entry = _getEntry(obj);
		if (!entry)
			continue;

		temp = obj->GetPos();
		temp2 = obj->GetRot();

		if (objCount > 1)
		{
			temp2.y += factor;
			temp.x -= centroid.x;
			temp.z -= centroid.z;
			x = temp.x * c - temp.z * s;
			z = temp.x * s + temp.z * c;
			temp.x = x + centroid.x;
			temp.z = z + centroid.z;

			if (gravity)
				temp.y = m_world->GetHeight(temp.x, temp.z);

			entry->pos = temp;
		}
		else
		{
			switch (axis)
			{
			case EDIT_X:
				temp2.x += factor;
				break;
			case EDIT_XZ:
			case EDIT_Y:
				temp2.y += factor;
				break;
			case EDIT_Z:
				temp2.z += factor;
				break;
			}
		}

		temp2.x = fmod(temp2.x, 360.0f);
		temp2.y = fmod(temp2.y, 360.0f);
		temp2.z = fmod(temp2.z, 360.0f);
		entry->rot = temp2;
	}

	Apply();
}

void CObjectTransformCommand::SetTranslate(CObject* obj, const D3DXVECTOR3& pos)
{
	ObjectEntry* entry = _getEntry(obj);
	if (!entry)
		return;

	entry->pos = pos;
}

void CObjectTransformCommand::EditTranslate(int axis, const D3DXVECTOR3& globalMove, float factor)
{
	const bool gravity = MainFrame->UseGravity();
	const bool useGrid = MainFrame->UseGrid();
	const float gridSize = MainFrame->GetGridSize();
	const float worldWidth = m_world->GetWidth() * MAP_SIZE * MPU;
	const float worldHeight = m_world->GetHeight() * MAP_SIZE * MPU;

	ObjectEntry* entry;
	D3DXVECTOR3 temp;
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		entry = _getEntry(CWorld::s_selection[i]);
		if (!entry)
			continue;

		temp = entry->tempPos;

		switch (axis)
		{
		case EDIT_XZ:
			temp.x -= globalMove.x;
			temp.z -= globalMove.z;
			break;
		case  EDIT_X:
			temp.x -= globalMove.x;
			break;
		case EDIT_Z:
			temp.z -= globalMove.z;
			break;
		case EDIT_Y:
			temp.y += factor;
			break;
		}

		if (temp.x < 0.0f)
			temp.x = 0.0f;
		else if (temp.x >= worldWidth)
			temp.x = worldWidth - 0.0001f;
		if (temp.z < 0.0f)
			temp.z = 0.0f;
		else if (temp.z >= worldHeight)
			temp.z = worldHeight - 0.0001f;

		entry->tempPos = temp;

		if (useGrid)
		{
			switch (axis)
			{
			case EDIT_XZ:
				temp.x = RoundFloat(entry->tempPos.x, gridSize);
				temp.z = RoundFloat(entry->tempPos.z, gridSize);
				break;
			case  EDIT_X:
				temp.x = RoundFloat(entry->tempPos.x, gridSize);
				break;
			case EDIT_Z:
				temp.z = RoundFloat(entry->tempPos.z, gridSize);
				break;
			case EDIT_Y:
				temp.y = RoundFloat(entry->tempPos.y, gridSize);
				break;
			}
		}

		if (gravity)
		{
			temp.y = m_world->GetHeight(temp.x, temp.z);
			if (useGrid)
				entry->tempPos.y = temp.y;
		}

		entry->pos = temp;
	}

	Apply();
}

void CObjectTransformCommand::SetOnGrid()
{
	const bool useGrid = MainFrame->UseGrid();
	const float gridSize = MainFrame->GetGridSize();

	CObject* obj;
	ObjectEntry* entry;
	D3DXVECTOR3 temp;
	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		obj = CWorld::s_selection[i];

		temp = obj->GetPos();

		temp.x = RoundFloat(temp.x, gridSize);
		temp.y = RoundFloat(temp.y, gridSize);
		temp.z = RoundFloat(temp.z, gridSize);

		if (temp != obj->GetPos())
		{
			entry = _getEntry(obj);
			if (entry)
				entry->pos = temp;
		}
	}

	Apply();
}

void CObjectTransformCommand::SetScale(CObject* obj, const D3DXVECTOR3& scale)
{
	ObjectEntry* entry = _getEntry(obj);
	if (!entry)
		return;

	entry->scale = scale;
}

void CObjectTransformCommand::EditScale(int axis, float factor)
{
	ObjectEntry* entry;
	D3DXVECTOR3 temp;

	for (int i = 0; i < CWorld::s_selection.GetSize(); i++)
	{
		entry = _getEntry(CWorld::s_selection[i]);
		if (!entry)
			continue;

		temp = entry->scale;

		switch (axis)
		{
		case EDIT_XZ:
			temp = entry->originalScale * factor;
			break;
		case  EDIT_X:
			temp.x = entry->originalScale.x * factor;
			break;
		case EDIT_Z:
			temp.z = entry->originalScale.z * factor;
			break;
		case EDIT_Y:
			temp.y = entry->originalScale.y * factor;
			break;
		}

		entry->scale = temp;
	}

	Apply();
}

void CObjectTransformCommand::Apply(CObjectDeleteCommand* command)
{
	if (!command)
		return;

	CObject* obj;
	ObjectEntry* entry;
	for (int i = 0; i < command->m_objects.GetSize(); i++)
	{
		obj = command->m_objects[i];
		if (!obj)
			continue;

		entry = _getEntry(obj);
		if (!entry)
			continue;

		obj->SetPos(entry->pos);
		obj->SetRot(entry->rot);
		obj->SetScale(entry->scale);
	}
}

CObjectTransformCommand::ObjectEntry* CObjectTransformCommand::_getEntry(CObject* obj)
{
	if (!obj)
		return null;

	const objid objID = obj->GetID();
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		if (m_objects[i].objID == objID)
			return &m_objects[i];
	}

	ObjectEntry entry;
	entry.objID = objID;
	entry.tempPos = entry.pos = entry.originalPos = obj->GetPos();
	entry.rot = entry.originalRot = obj->GetRot();
	entry.scale = entry.originalScale = obj->GetScale();
	m_objects.push_back(entry);
	return &m_objects[m_objects.size() - 1];
}

void CObjectTransformCommand::Apply(bool undo)
{
	CObject* obj;
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		const ObjectEntry& entry = m_objects[i];

		obj = m_world->GetObject(entry.objID);
		if (!obj)
			continue;

		m_world->MoveObject(obj, undo ? entry.originalPos : entry.pos);
		obj->SetRot(undo ? entry.originalRot : entry.rot);
		obj->SetScale(undo ? entry.originalScale : entry.scale);
	}
}

void CObjectCreateCommand::Apply(bool undo)
{
	CObject* obj;

	if (undo)
	{
		for (size_t i = 0; i < m_objects.size(); i++)
		{
			obj = m_world->GetObject(m_objects[i].objID);

			if (obj)
			{
				const int find = CWorld::s_selection.Find(obj);
				if (find != -1)
					CWorld::s_selection.RemoveAt(find);

				m_world->DeleteObject(obj);
			}
		}
	}
	else
	{
		CWorld::s_selection.RemoveAll();
		for (size_t i = 0; i < m_objects.size(); i++)
		{
			const ObjectEntry& entry = m_objects[i];

			obj = CObject::CreateObject(entry.type);
			obj->SetModelID(entry.modelID);
			obj->SetPos(entry.pos);
			obj->SetRot(D3DXVECTOR3(0.0f, entry.rot, 0.0f));
			if (obj->Init())
			{
				if (entry.scale != 1.0f)
					obj->SetScale(D3DXVECTOR3(entry.scale, entry.scale, entry.scale));
				else
					obj->ResetScale();

				if (entry.objID)
					obj->SetID(entry.objID);

				m_world->AddObject(obj);

				m_objects[i].objID = obj->GetID();

				if (CWorld::s_selection.Find(obj) == -1)
					CWorld::s_selection.Append(obj);

				if (entry.type == OT_MOVER || entry.type == OT_ITEM || entry.type == OT_CTRL)
					((CSpawnObject*)obj)->SetRect(entry.rect);
				else if (entry.type == OT_REGION)
					((CRegion*)obj)->SetRect(entry.rect);

				if (entry.type == OT_MOVER)
					((CMover*)obj)->InitProperties();

				obj->Cull();

				if (entry.type == OT_PATH)
				{
					CPtrArray<CPath>* path = m_world->GetPath(entry.pathID);
					if (path)
					{
						CPath* objPath = (CPath*)obj;
						objPath->SetIndex(path->GetSize());
						path->Append(objPath);
					}
				}
			}
			else
				Delete(obj);
		}
	}
}

void CObjectCreateCommand::AddCreateObject(CObject* obj, int pathID)
{
	if (!obj)
		return;

	ObjectEntry entry;
	entry.objID = 0;
	entry.type = obj->GetType();
	entry.pos = obj->GetPos();
	entry.rot = obj->GetRot().y;
	entry.scale = 1.0f;
	entry.modelID = obj->GetModelID();
	entry.pathID = pathID;

	if (entry.type == OT_MOVER || entry.type == OT_ITEM || entry.type == OT_CTRL)
		entry.rect = ((CSpawnObject*)obj)->GetRect();
	else if (entry.type == OT_REGION)
		entry.rect = ((CRegion*)obj)->GetRect();

	if (entry.type != OT_REGION && entry.type != OT_PATH)
	{
		bool rot, scale;
		float minScale, maxScale, minRot, maxRot;
		MainFrame->GetAddObjSettings(rot, minRot, maxRot, scale, minScale, maxScale);

		if (rot)
			entry.rot = minRot + (float)(rand()) / ((float)RAND_MAX / (maxRot - minRot));

		if (scale)
		{
			entry.scale = minScale + (float)(rand()) / ((float)RAND_MAX / (maxScale - minScale));

			const ModelProp* prop = Project->GetModelProp(entry.type, entry.modelID);
			if (prop)
				entry.scale *= prop->scale;
		}
	}

	m_objects.push_back(entry);
}

CObjectEditRectCommand::CObjectEditRectCommand(CWorld* world, CObject* obj, byte editVertex)
	: CEditCommand(world),
	m_editVertex(editVertex)
{
	m_objID = obj->GetID();

	if (obj->GetType() == OT_REGION)
		m_origin = m_rect = ((CRegion*)obj)->GetRect();
	else if (obj->GetType() == OT_MOVER || obj->GetType() == OT_ITEM || obj->GetType() == OT_CTRL)
		m_origin = m_rect = ((CSpawnObject*)obj)->GetRect();
}

void CObjectEditRectCommand::Edit(const D3DXVECTOR3& pos)
{
	const QPoint realPos((int)(pos.z + ((float)MPU / 2.0f)) / MPU * MPU,
		(int)(pos.x + ((float)MPU / 2.0f)) / MPU * MPU);

	m_rect = m_rect.normalized();

	switch (m_editVertex)
	{
	case 0:
		m_rect.setBottomRight(realPos);
		break;
	case 1:
		m_rect.setBottomLeft(realPos);
		break;
	case 2:
		m_rect.setTopRight(realPos);
		break;
	case 3:
		m_rect.setTopLeft(realPos);
		break;
	}

	m_rect = m_rect.normalized();
	Apply();
}

void CObjectEditRectCommand::Apply(bool undo)
{
	CObject* obj = m_world->GetObject(m_objID);
	if (!obj)
		return;

	if (obj->GetType() == OT_REGION)
		((CRegion*)obj)->SetRect(undo ? m_origin : m_rect);
	else if (obj->GetType() == OT_MOVER || obj->GetType() == OT_ITEM || obj->GetType() == OT_CTRL)
		((CSpawnObject*)obj)->SetRect(undo ? m_origin : m_rect);
}

CPathDeleteCommand::~CPathDeleteCommand()
{
	for (int i = 0; i < m_objects.GetSize(); i++)
		Delete(m_objects[i]);
}

void CPathDeleteCommand::Apply(bool undo)
{
	if ((undo && !m_create) || (!undo && m_create))
	{
		m_world->m_paths[m_ID] = new CPtrArray<CPath>();

		if (m_objects.GetSize() > 0)
		{
			CWorld::s_selection.RemoveAll();

			CObject* obj;
			for (int i = 0; i < m_objects.GetSize(); i++)
			{
				obj = CObject::CreateObject(m_objects[i]->GetType(), m_objects[i]);
				m_world->AddObject(obj);

				if (CWorld::s_selection.Find(obj) == -1)
					CWorld::s_selection.Append(obj);

				obj->Cull();

				m_world->m_paths[m_ID]->Append((CPath*)obj);
			}
		}
	}
	else
	{
		auto it = m_world->m_paths.find(m_ID);
		if (it != m_world->m_paths.end())
		{
			CObject* obj;
			while (it.value()->GetSize() > 0)
			{
				obj = (CObject*)it.value()->GetAt(0);
				const int find = CWorld::s_selection.Find(obj);
				if (find != -1)
					CWorld::s_selection.RemoveAt(find);
				m_world->DeleteObject(obj);
			}
			Delete(m_world->m_paths[m_ID]);
			m_world->m_paths.remove(m_ID);

			CMover* mover;
			for (int i = 0; i < m_world->m_objects[OT_MOVER].GetSize(); i++)
			{
				mover = (CMover*)m_world->m_objects[OT_MOVER].GetAt(i);
				if (mover->m_patrolIndex == m_ID)
					mover->m_patrolIndex = -1;
			}
		}
	}

	MainFrame->UpdatePatrolList();
}

void CPathDeleteCommand::CreatePath()
{
	m_create = true;

	for (m_ID = 0; m_ID < INT_MAX; m_ID++)
		if (m_world->m_paths.find(m_ID) == m_world->m_paths.end())
			break;

	Apply();
}

void CPathDeleteCommand::RemovePath(int ID)
{
	m_ID = ID;

	auto it = m_world->m_paths.find(ID);
	if (it != m_world->m_paths.end())
	{
		CObject* obj;
		for (int i = 0; i < it.value()->GetSize(); i++)
		{
			obj = it.value()->GetAt(i);
			if (obj)
				m_objects.Append(CObject::CreateObject(obj->GetType(), obj));
		}
	}

	Apply();
}