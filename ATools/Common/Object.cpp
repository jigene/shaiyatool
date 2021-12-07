///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Object.h"
#include "Project.h"
#include "ModelMng.h"
#include "Model.h"
#include "World.h"
#include "Model.h"
#include "Collision.h"
#include "Mover.h"
#include "Ctrl.h"
#include "Region.h"
#include "Path.h"

static const string g_root[MAX_OBJTYPE] =
{
	"obj",
	"ani",
	"ctrl",
	"sfx",
	"item",
	"mvr",
	"region",
	"obj",		// ship
	"path"
};

CObject* CObject::CreateObject(uint type, CObject* clone)
{
	if (type >= MAX_OBJTYPE)
		return null;

	CObject* obj;
	switch (type)
	{
	case OT_MOVER:
		obj = new CMover();
		if (clone)
			*(CMover*)obj = *(CMover*)clone;
		break;
	case OT_CTRL:
		obj = new CCtrl();
		if (clone)
			*(CCtrl*)obj = *(CCtrl*)clone;
		break;
	case OT_ITEM:
		obj = new CSpawnObject();
		if (clone)
			*(CSpawnObject*)obj = *(CSpawnObject*)clone;
		break;
	case OT_REGION:
		obj = new CRegion();
		if (clone)
			*(CRegion*)obj = *(CRegion*)clone;
		break;
	case OT_PATH:
		obj = new CPath();
		if (clone)
			*(CPath*)obj = *(CPath*)clone;
		break;
	default:
		obj = new CObject();
		if (clone)
			*obj = *clone;
		break;
	}

	if (clone)
	{
		obj->m_model = null;
		obj->m_world = null;
	}
	else
	{
		obj->SetType(type);
	}

	return obj;
}

CObject* CObject::CreateObject(CFile& file)
{
	uint type = 0;
	file.Read(type);

	CObject* obj = CreateObject(type);

	if (obj)
	{
		obj->Read(file);

		if (!obj->Init())
			Delete(obj);
	}

	return obj;
}

bool ObjSortFarToNear(const CObject* obj1, const CObject* obj2)
{
	return obj1->m_distToCamera > obj2->m_distToCamera;
}

CObject::CObject()
	: m_isReal(true),
	m_type(0),
	m_rot(0, 0, 0),
	m_pos(0, 0, 0),
	m_scale(1, 1, 1),
	m_modelID(0),
	m_modelProp(null),
	m_model(null),
	m_updateMatrix(true),
	m_world(null),
	m_visible(false),
	m_distToCamera(0.0f),
	m_isUnvisible(false),
	m_ID(0)
{
	memset(&m_bounds, 0, sizeof(m_bounds));
}

CObject::~CObject()
{
	if (m_modelProp && m_modelProp->modelType != MODELTYPE_MESH)
		Delete(m_model);
}

void CObject::Read(CFile& file)
{
	file.Read(m_rot.y);
	file.Read(m_rot.x);
	file.Skip(4);
	file.Read(m_rot.z);
	file.Read(m_pos);
	m_pos.x *= 4.0f;
	m_pos.z *= 4.0f;
	file.Read(m_scale);
	file.Read(m_type);

	if (m_type & FOA_Invisible_Collision)
		m_type &= ~FOA_Invisible_Collision;

	if (m_type & FOA_Invisible)
		m_type &= ~FOA_Invisible;

	file.Read(m_modelID);

	uint motion, AIInterface, AI2;
	file.Read(motion);
	file.Read(AIInterface);
	file.Read(AI2);

	if (m_type == OT_MOVER)
	{
		CMover* mover = (CMover*)this;
		mover->m_motion = motion;
		mover->m_AIInterface = AIInterface;
		mover->m_aiState = (int)AI2;
	}
}

void CObject::Write(CFile& file, const D3DXVECTOR3& posOffset)
{
	file.Write(m_rot.y);
	file.Write(m_rot.x);
	file.Write(0.0f);
	file.Write(m_rot.z);
	D3DXVECTOR3 pos = m_pos;
	pos -= posOffset;
	pos.x /= 4.0f;
	pos.z /= 4.0f;
	file.Write(pos);
	file.Write(m_scale);
	file.Write(m_type);
	file.Write(m_modelID);

	if (m_type != OT_MOVER)
	{
		file.Write((uint)4294967295);
		file.Write((uint)0);
		file.Write((uint)0);
	}
	else
	{
		CMover* mover = (CMover*)this;
		file.Write(mover->m_motion);
		file.Write(mover->m_AIInterface);
		file.Write((uint)mover->m_aiState);
	}
}

bool CObject::Init()
{
	m_modelProp = Project->GetModelProp(m_type, m_modelID);

	if (!m_modelProp)
		return false;

	if (m_modelProp->modelType != MODELTYPE_MESH
		&& m_modelProp->modelType != MODELTYPE_ANIMATED_MESH
		&& m_modelProp->modelType != MODELTYPE_SFX)
		return false;

	return true;
}

void CObject::ResetScale()
{
	m_scale.x = m_scale.y = m_scale.z = m_modelProp->scale;
	m_updateMatrix = true;
}

void CObject::SetPos(const D3DXVECTOR3& pos)
{
	if (m_pos != pos)
	{
		m_pos = pos;
		m_updateMatrix = true;
	}
}

void CObject::SetRot(const D3DXVECTOR3& rot)
{
	if (m_rot != rot)
	{
		m_rot = rot;
		m_updateMatrix = true;
	}
}

void CObject::SetScale(const D3DXVECTOR3& scale)
{
	if (m_scale != scale)
	{
		m_scale = scale;
		m_updateMatrix = true;
	}
}

void CObject::Render()
{
	if (!m_model)
		return;

	if (g_global3D.animate && m_modelProp->modelType != MODELTYPE_MESH
		&& (m_type == OT_OBJ || m_type == OT_SFX))
		m_model->MoveFrame();

	if (m_modelProp->modelType == MODELTYPE_SFX)
		m_model->Render(m_pos + D3DXVECTOR3(0.0f, 0.2f, 0.0f), m_rot, m_scale);
	else
	{
		int lod = 0;

		if (!m_world->m_inDoor)
		{
			lod = (int)(m_distToCamera / 50.0f);
			if (lod < 0)
				lod = 0;
			if (lod >= 2)
				lod = 2;
		}

		m_model->Render(&m_TM, lod);
	}
}

void CObject::RenderName()
{
}

string CObject::GetModelFilename(ModelProp* prop)
{
	if (!prop)
		return "";

	string filename;
	if (prop->type == OT_SFX && prop->filename.contains('_'))
		filename = prop->filename;
	else
		filename = g_root[prop->type] + '_' + prop->filename;

	if (prop->modelType == MODELTYPE_SFX)
		return filename + ".sfx";
	else
		return filename + ".o3d";
}

void CObject::_loadModel()
{
	m_model = ModelMng->GetModel(m_modelProp->modelType, GetModelFilename());
	if (!m_model)
		return;

	if (m_modelID != MI_MALE && m_modelID != MI_FEMALE)
		m_model->SetTextureEx(m_modelProp->textureEx);
}

void CObject::Cull()
{
	if (!m_model)
		_loadModel();

	if (m_updateMatrix)
		_updateMatrix();

	byte outside[8];
	memset(outside, 0, sizeof(outside));
	int plane;
	for (int point = 0; point < 8; point++)
	{
		for (plane = 0; plane < 6; plane++)
		{
			if (g_planeFrustum[plane].a * m_bounds[point].x +
				g_planeFrustum[plane].b * m_bounds[point].y +
				g_planeFrustum[plane].c * m_bounds[point].z +
				g_planeFrustum[plane].d < 0)
			{
				outside[point] |= (1 << plane);
			}
		}
		if (outside[point] == 0)
		{
			m_visible = true;
			return;
		}
	}
	if ((outside[0] & outside[1] & outside[2] & outside[3] &
		outside[4] & outside[5] & outside[6] & outside[7]) != 0)
	{
		m_visible = false;
		return;
	}
	m_visible = true;
}

void CObject::_updateMatrix()
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&m_TM);
	D3DXMatrixScaling(&mat, m_scale.x, m_scale.y, m_scale.z);
	D3DXMatrixMultiply(&m_TM, &m_TM, &mat);
	D3DXMatrixRotationYawPitchRoll(&mat, D3DXToRadian(-m_rot.y), D3DXToRadian(-m_rot.x), D3DXToRadian(m_rot.z));
	D3DXMatrixMultiply(&m_TM, &m_TM, &mat);
	D3DXMatrixTranslation(&mat, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_TM, &m_TM, &mat);

	if (m_model)
	{
		const Bounds& bounds = m_model->GetBounds();
		m_bounds[0].x = bounds.Min.x;
		m_bounds[0].y = bounds.Max.y;
		m_bounds[0].z = bounds.Min.z;

		m_bounds[1].x = bounds.Max.x;
		m_bounds[1].y = bounds.Max.y;
		m_bounds[1].z = bounds.Min.z;

		m_bounds[2].x = bounds.Max.x;
		m_bounds[2].y = bounds.Max.y;
		m_bounds[2].z = bounds.Max.z;

		m_bounds[3].x = bounds.Min.x;
		m_bounds[3].y = bounds.Max.y;
		m_bounds[3].z = bounds.Max.z;

		m_bounds[4].x = bounds.Min.x;
		m_bounds[4].y = bounds.Min.y;
		m_bounds[4].z = bounds.Min.z;

		m_bounds[5].x = bounds.Max.x;
		m_bounds[5].y = bounds.Min.y;
		m_bounds[5].z = bounds.Min.z;

		m_bounds[6].x = bounds.Max.x;
		m_bounds[6].y = bounds.Min.y;
		m_bounds[6].z = bounds.Max.z;

		m_bounds[7].x = bounds.Min.x;
		m_bounds[7].y = bounds.Min.y;
		m_bounds[7].z = bounds.Max.z;

		for (int i = 0; i < 8; i++)
			D3DXVec3TransformCoord(&m_bounds[i], &m_bounds[i], &m_TM);
	}

	m_updateMatrix = false;
}

bool CObject::Pick(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist)
{
	static int indexTable[12 * 3] =
	{
		3, 2, 6, 3, 6, 7,
		0, 3, 7, 0, 7, 4,
		1, 2, 6, 1, 6, 5,
		0, 1, 2, 0, 2, 3,
		0, 4, 5, 0, 5, 1,
		4, 5, 6, 4, 6, 7
	};

	bool pick = false;
	float tempDist;
	dist = 65535.0f;

	for (int i = 0; i < 12 * 3; i += 3)
	{
		if (IntersectTriangle(m_bounds[indexTable[i]], m_bounds[indexTable[i + 1]], m_bounds[indexTable[i + 2]], origin, dir, tempDist) && tempDist < dist)
		{
			dist = tempDist;
			pick = true;
		}
	}

	if (pick)
	{
		if (m_modelProp->modelType == MODELTYPE_SFX)
			return true;
		else
			return m_model->Pick(m_TM, origin, dir, dist);
	}

	return false;
}