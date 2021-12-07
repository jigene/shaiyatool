///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "AnimatedMesh.h"
#include "Motion.h"
#include "Object3D.h"
#include "ModelMng.h"

#ifdef WORLD_EDITOR
#include "Mover.h"
#endif // WORLD_EDITOR

CAnimatedMesh::CAnimatedMesh(LPDIRECT3DDEVICE9 device)
	: CModel(device),
	m_skeleton(null),
	m_bones(null),
	m_invBones(null),
	m_motion(null)
{
	m_bounds.Max = D3DXVECTOR3(-65535.0f, -65535.0f, -65535.0f);
	m_bounds.Min = D3DXVECTOR3(65535.0f, 65535.0f, 65535.0f);
	memset(&m_elements, 0, sizeof(m_elements));
}

CAnimatedMesh::~CAnimatedMesh()
{
	DeleteArray(m_bones);
}

void CAnimatedMesh::Render(const D3DXMATRIX* world, int lod, int alpha)
{
	D3DXMATRIX mat;
	int i;
	const int nextFrame = GetNextFrame();

	if (m_motion)
		m_motion->Animate(m_bones, m_currentFrame, nextFrame);

	if (m_bones)
	{
		if (m_skeleton->MustSendVS())
		{
			const int boneCount = m_skeleton->GetBoneCount();

			for (i = 0; i < boneCount; i++)
			{
				mat = m_invBones[i] * m_bones[i];
				D3DXMatrixTranspose(&mat, &mat);
				m_device->SetVertexShaderConstantF(i * 3, (float*)&mat, 3);
			}
		}

		// WVP
		mat = *world * g_global3D.view * g_global3D.proj;
		D3DXMatrixTranspose(&mat, &mat);
		m_device->SetVertexShaderConstantF(84, (float*)&mat, 4);

		D3DXVECTOR4 vLight;
		D3DXMatrixInverse(&mat, null, world);
		D3DXVec4Transform(&vLight, &g_global3D.lightVec, &mat);
		D3DXVec4Normalize(&vLight, &vLight);
		m_device->SetVertexShaderConstantF(92, (float*)&vLight, 1);
	}

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_LIGHTING, g_global3D.light ? TRUE : FALSE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	for (i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		const Element& element = m_elements[i];

		if (!element.obj)
			continue;

		if (m_bones)
			element.obj->SetExternBones(m_bones, m_invBones);

		element.obj->Render(world, lod, m_currentFrame, nextFrame, m_textureEx, alpha);
	}
}

bool CAnimatedMesh::Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist)
{
	bool pick = false;
	float tempDist;
	dist = 65535.0f;

	for (int i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		const Element& element = m_elements[i];
		if (element.obj)
		{
			if (m_bones)
				element.obj->SetExternBones(m_bones, m_invBones);

			if (element.obj->Pick(world, origin, dir, tempDist) && tempDist < dist)
			{
				dist = tempDist;
				pick = true;
			}
		}
	}

	return pick;
}

bool CAnimatedMesh::Load(const string& filename, int part)
{
	if (part >= MAX_ANIMATED_ELEMENTS)
		return false;

#ifdef WORLD_EDITOR
	static const string humans[] = { "mvr_male.o3d", "mvr_female.o3d" };
	const string temp = filename.toLower();
	if (temp == humans[0])
	{
		m_skeleton = ModelMng->GetSkeleton("mvr_male.chr");
		if (!m_skeleton)
			return false;
		return Load(PARTSMESH_UPPER(SEX_MALE), PARTS_UPPER_BODY)
			&& Load(PARTSMESH_LOWER(SEX_MALE), PARTS_LOWER_BODY)
			&& Load(PARTSMESH_HAND(SEX_MALE), PARTS_HAND)
			&& Load(PARTSMESH_FOOT(SEX_MALE), PARTS_FOOT)
			&& Load(string().sprintf(PARTSMESH_HEAD(SEX_MALE), 1), PARTS_HEAD)
			&& Load(string().sprintf(PARTSMESH_HAIR(SEX_MALE), 1), PARTS_HAIR);
	}
	else if (temp == humans[1])
	{
		m_skeleton = ModelMng->GetSkeleton("mvr_female.chr");
		if (!m_skeleton)
			return false;
		return Load(PARTSMESH_UPPER(SEX_FEMALE), PARTS_UPPER_BODY)
			&& Load(PARTSMESH_LOWER(SEX_FEMALE), PARTS_LOWER_BODY)
			&& Load(PARTSMESH_HAND(SEX_FEMALE), PARTS_HAND)
			&& Load(PARTSMESH_FOOT(SEX_FEMALE), PARTS_FOOT)
			&& Load(string().sprintf(PARTSMESH_HEAD(SEX_FEMALE), 1), PARTS_HEAD)
			&& Load(string().sprintf(PARTSMESH_HAIR(SEX_FEMALE), 1), PARTS_HAIR);
	}
#endif // WORLD_EDITOR
	m_elements[part].obj = ModelMng->GetObject3D(filename);

	if (!m_elements[part].obj)
		return false;

	const Bounds bounds = m_elements[part].obj->GetBounds();
	if (m_bounds.Min.x > bounds.Min.x) m_bounds.Min.x = bounds.Min.x;
	if (m_bounds.Min.y > bounds.Min.y) m_bounds.Min.y = bounds.Min.y;
	if (m_bounds.Min.z > bounds.Min.z) m_bounds.Min.z = bounds.Min.z;
	if (m_bounds.Max.x < bounds.Max.x) m_bounds.Max.x = bounds.Max.x;
	if (m_bounds.Max.y < bounds.Max.y) m_bounds.Max.y = bounds.Max.y;
	if (m_bounds.Max.z < bounds.Max.z) m_bounds.Max.z = bounds.Max.z;

	if (m_elements[part].obj->HasSkin())
		m_hasSkin = true;

	if (!m_frameCount)
	{
		const string filenameToLower = filename.toLower();
		if (filenameToLower.startsWith("mvr"))
		{
			string skelname = filename;
			int lastIndex = skelname.lastIndexOf('.');
			skelname.remove(lastIndex, skelname.size() - lastIndex);
			skelname.append(".chr");

			m_skeleton = ModelMng->GetSkeleton(skelname);
			if (!m_skeleton)
				return false;
		}
		else if (filenameToLower.startsWith("item"))
		{
			string skelname = filename;
			int lastIndex = skelname.lastIndexOf('.');
			skelname.remove(lastIndex, skelname.size() - lastIndex);
			skelname.append(".chr");

			if (ModelMng->SkeletonExists(skelname))
			{
				m_skeleton = ModelMng->GetSkeleton(skelname);
				if (!m_skeleton)
					return false;
			}
		}
		else if (filenameToLower.startsWith("part"))
		{
			if (filename.size() > 6)
			{
				const QChar sex = filename[5].toLower();
				if (sex == 'm')
					m_skeleton = ModelMng->GetSkeleton("mvr_male.chr");
				else if (sex == 'f')
					m_skeleton = ModelMng->GetSkeleton("mvr_female.chr");

				if (!m_skeleton)
					return false;
			}
		}
		else
		{
			m_frameCount = m_elements[part].obj->GetFrameCount();
		}

		if (m_skeleton)
		{
			m_bones = new D3DXMATRIX[m_skeleton->GetBoneCount() * 2];
			m_invBones = m_bones + m_skeleton->GetBoneCount();
			m_skeleton->ResetBones(m_bones, m_invBones);
		}
	}

	return true;
}

void CAnimatedMesh::SetMotion(const string& filename)
{
	if (!m_skeleton)
		return;

	m_currentFrame = 0.0f;

	if (filename.isEmpty())
	{
		if (m_motion)
		{
			m_motion = null;
			m_frameCount = 0;
			m_skeleton->ResetBones(m_bones, m_invBones);
		}
	}
	else
	{
		m_motion = ModelMng->GetMotion(filename);

		if (m_motion)
			m_frameCount = m_motion->GetFrameCount();
		else
			m_frameCount = 0;
	}
}

MotionAttribute* CAnimatedMesh::GetAttributes() const
{
	if (m_motion)
		return m_motion->GetAttributes();
	else if (m_elements[0].obj && m_elements[0].obj->GetFrameCount() > 0)
		return m_elements[0].obj->GetAttributes();
	else
		return null;
}

void CAnimatedMesh::RenderCollision(const D3DXMATRIX* world)
{
	for (int i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		if (m_elements[i].obj)
			m_elements[i].obj->RenderCollision(world);
	}
}