///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "Exporter.h"
#include <AnimatedMesh.h>
#include <Object3D.h>
#include <Motion.h>

CExporter::CExporter(CAnimatedMesh* mesh)
{
	CObject3D* obj3D = null;
	for (int i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		if (mesh->m_elements[i].obj)
		{
			obj3D = mesh->m_elements[i].obj;
			break;
		}
	}

	if (!obj3D)
		return;

	CMotion* motion = null;
	int boneCount = 0;
	Bone* bones = null;

	if (mesh->m_motion)
		motion = mesh->m_motion;
	if (obj3D->m_motion)
		motion = obj3D->m_motion;

	if (mesh->m_skeleton)
	{
		boneCount = mesh->m_skeleton->m_boneCount;
		bones = mesh->m_skeleton->m_bones;
	}
	else if (motion)
	{
		boneCount = motion->m_boneCount;
		bones = motion->m_bones;
	}

	int matID = 0;
	if (obj3D->m_collObj.type != GMT_ERROR)
	{
		if (obj3D->m_collObj.material)
		{
			for (int i = 0; i < obj3D->m_collObj.materialCount; i++)
			{
				m_materials["Material" + string::number(matID) + "CollObj"] = &obj3D->m_collObj.materials[i];
				matID++;
			}
		}

		if (obj3D->m_collObj.materialBlockCount == 0
			&& obj3D->m_collObj.indexCount > 0)
		{
			obj3D->m_collObj.materialBlockCount = 1;
			obj3D->m_collObj.materialBlocks = new MaterialBlock[1];
			memset(obj3D->m_collObj.materialBlocks, 0, sizeof(MaterialBlock));

			if (obj3D->m_collObj.material)
				obj3D->m_collObj.materialBlocks[0].materialID = 0;
			else
				obj3D->m_collObj.materialBlocks[0].materialID = -1;

			obj3D->m_collObj.materialBlocks[0].primitiveCount = obj3D->m_collObj.indexCount / 3;
		}

		m_objects["CollisionObject-coll"] = &obj3D->m_collObj;
		m_objectLODs[&obj3D->m_collObj] = -1;
	}

	int obj3DCount = 0;
	for (int i = 0; i < MAX_ANIMATED_ELEMENTS; i++)
	{
		if (mesh->m_elements[i].obj)
			obj3DCount++;
	}

	if (obj3DCount == 1)
	{
		int gmID = 0;
		GMObject* obj = null;
		for (int i = 0; i < (obj3D->m_LOD ? MAX_GROUP : 1); i++)
		{
			gmID = 0;
			for (int j = 0; j < obj3D->m_groups[i].objectCount; j++)
			{
				obj = &obj3D->m_groups[i].objects[j];

				string objName = "";
				if (i > 0)
					objName += "LOD" + string::number(i) + '-';
				objName += "GMObject" + string::number(gmID);
				if (obj->light)
					objName += "-light";

				m_objects[objName] = obj;
				m_objectIDs[obj] = j;
				m_objectLODs[obj] = i;

				if (obj->material)
				{
					matID = 0;
					for (int k = 0; k < obj->materialCount; k++)
					{
						string matName = "Material" + string::number(matID);
						if (i > 0)
							matName += "LOD" + string::number(i) + '-';
						matName += "GMObj" + string::number(gmID);
						m_materials[matName] = &obj->materials[k];
						matID++;

						for (int l = 0; l < obj->materialBlockCount; l++)
							if (obj->materialBlocks[l].materialID == k)
								m_materialBlocks[&obj->materials[k]] = &obj->materialBlocks[l];
					}
				}

				gmID++;
			}
		}
	}
	else
	{
		CObject3D* curObj3D = null;
		int gmIDs[MAX_GROUP];
		memset(gmIDs, 0, sizeof(gmIDs));
		GMObject* obj = null;

		for (int objIndex = 0; objIndex < MAX_ANIMATED_ELEMENTS; objIndex++)
		{
			if (mesh->m_elements[objIndex].obj)
			{
				curObj3D = mesh->m_elements[objIndex].obj;

				for (int i = 0; i < (curObj3D->m_LOD ? MAX_GROUP : 1); i++)
				{
					for (int j = 0; j < curObj3D->m_groups[i].objectCount; j++)
					{
						obj = &curObj3D->m_groups[i].objects[j];

						string objName = "";
						if (i > 0)
							objName += "LOD" + string::number(i) + '-';
						objName += "GMObject" + string::number(gmIDs[i]);
						if (obj->light)
							objName += "-light";

						m_objects[objName] = obj;
						m_objectIDs[obj] = j;
						m_objectLODs[obj] = i;

						if (obj->material)
						{
							matID = 0;
							for (int k = 0; k < obj->materialCount; k++)
							{
								string matName = "Material" + string::number(matID);
								if (i > 0)
									matName += "LOD" + string::number(i) + '-';
								matName += "GMObj" + string::number(gmIDs[i]);
								m_materials[matName] = &obj->materials[k];
								matID++;

								for (int l = 0; l < obj->materialBlockCount; l++)
									if (obj->materialBlocks[l].materialID == k)
										m_materialBlocks[&obj->materials[k]] = &obj->materialBlocks[l];
							}
						}

						gmIDs[i]++;
					}
				}
			}
		}
	}

	m_rootBoneID = "";
	if (bones)
	{
		for (int i = 0; i < boneCount; i++)
		{
			m_bones.push_back(&bones[i]);
			m_boneIDs[&bones[i]] = i;

			if (bones[i].parentID == -1)
				m_rootBoneID = string(bones[i].name).toLower().replace('.', '_').replace('-', '_').replace(' ', '_');
		}
	}

	m_frameCount = 0;
	if (obj3D->m_frameCount > 0)
		m_frameCount = obj3D->m_frameCount;
	else if (motion && motion->m_frameCount > 0)
		m_frameCount = motion->m_frameCount;

	if (obj3D->m_frameCount > 0)
	{
		for (auto it = m_objects.begin(); it != m_objects.end(); it++)
		{
			if (it.value()->frames)
				m_animations[it.key() + "-transform"] = it.value()->frames;
		}
	}

	if (motion)
	{
		for (int i = 0; i < boneCount; i++)
		{
			const string boneID = string(bones[i].name).toLower().replace('.', '_').replace('-', '_').replace(' ', '_');
			if (motion->m_frames[i].frames)
				m_animations[boneID + "-transform"] = motion->m_frames[i].frames;
			else
				m_boneAnimTMs[m_bones[i]] = motion->m_frames[i].TM;
		}
	}
}

string CExporter::_getMaterialID(Material* mat)
{
	for (auto it = m_materials.begin(); it != m_materials.end(); it++)
	{
		if (it.value() == mat)
			return it.key();
	}
	return "";
}

MaterialBlock* CExporter::_getMaterialBlock(Material* mat)
{
	auto it = m_materialBlocks.find(mat);
	if (it != m_materialBlocks.end())
		return it.value();
	return null;
}

D3DXMATRIX CExporter::_getRotationMatrix(const D3DXQUATERNION& quat) const
{
	D3DXMATRIX mat;
	D3DXMatrixRotationQuaternion(&mat, &quat);
	return mat;
}