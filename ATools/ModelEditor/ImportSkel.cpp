///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "Importer.h"
#include <Motion.h>
#include <Object3D.h>
#include <AnimatedMesh.h>
#include "ImportUtils.h"

void CImporter::_createBones()
{
	const uint boneCount = m_aiBones.size();
	m_bones = new Bone[boneCount];
	memset(m_bones, 0, sizeof(Bone) * boneCount);

	Bone* bone;
	aiNode* node;
	uint i, j, k, l;

	for (i = 0; i < boneCount; i++)
	{
		bone = &m_bones[i];
		node = m_aiBones[i];

		strcpy(bone->name, node->mName.C_Str());
		bone->localTM = ToD3DMatrix(node->mTransformation);

		if (!node->mParent || node->mParent == m_aiScene->mRootNode)
		{
			bone->parentID = -1;
			bone->localTM *= m_rootTransform;
		}
		else
		{
			for (j = 0; j < boneCount; j++)
			{
				if (m_aiBones[j] == node->mParent)
				{
					bone->parentID = (int)j;
					break;
				}
			}
		}
	}

	for (i = 0; i < boneCount; i++)
	{
		m_bones[i].TM = m_bones[i].localTM;
		if (m_bones[i].parentID != -1)
			m_bones[i].TM *= m_bones[m_bones[i].parentID].TM;
	}

	aiMesh* mesh;
	bool findRootBone = false;

	for (i = 0; i < m_aiObjects.size(); i++)
	{
		for (j = 0; j < (int)m_aiObjects[i]->mNumMeshes; j++)
		{
			mesh = m_aiScene->mMeshes[m_aiObjects[i]->mMeshes[j]];
			if (mesh->HasBones())
			{
				for (k = 0; k < (int)mesh->mNumBones; k++)
				{
					bone = null;

					for (l = 0; l < boneCount; l++)
					{
						if (strcmp(m_bones[l].name, mesh->mBones[k]->mName.C_Str()) == 0)
						{
							bone = &m_bones[l];
							break;
						}
					}

					if (bone)
					{
						bone->inverseTM = ToD3DMatrix(mesh->mBones[k]->mOffsetMatrix);

						if (bone->parentID == -1)
						{
							m_rootBoneTransform = bone->inverseTM * bone->TM;
							findRootBone = true;
						}
					}
				}
			}
		}
	}

	if (!findRootBone)
		D3DXMatrixScaling(&m_rootBoneTransform, m_scaleFactor, m_scaleFactor, m_scaleFactor);

	D3DXMATRIX mat, matRot, matPos;
	D3DXVECTOR3 pos;
	D3DXQUATERNION rot;

	D3DXVECTOR3* scales = new D3DXVECTOR3[boneCount];

	for (i = 0; i < boneCount; i++)
	{
		bone = &m_bones[i];

		D3DXMatrixDecompose(&scales[i], &rot, &pos, &bone->localTM);

		if (bone->parentID != -1)
		{
			pos.x *= scales[bone->parentID].x;
			pos.y *= scales[bone->parentID].y;
			pos.z *= scales[bone->parentID].z;

			scales[i] = scales[bone->parentID];
		}

		D3DXMatrixIdentity(&mat);
		D3DXMatrixRotationQuaternion(&matRot, &rot);
		D3DXMatrixMultiply(&mat, &mat, &matRot);
		D3DXMatrixTranslation(&matPos, pos.x, pos.y, pos.z);
		D3DXMatrixMultiply(&mat, &mat, &matPos);

		bone->localTM = mat;
	}

	DeleteArray(scales);

	for (i = 0; i < boneCount; i++)
	{
		m_bones[i].TM = m_bones[i].localTM;
		if (m_bones[i].parentID != -1)
			m_bones[i].TM *= m_bones[m_bones[i].parentID].TM;

		D3DXMatrixInverse(&m_bones[i].inverseTM, null, &m_bones[i].TM);
	}

	if (m_externBones)
	{
		CSkeleton* skel = m_mesh->m_skeleton = new CSkeleton();
		skel->m_ID = GenerateRandomID();
		skel->m_boneCount = (int)boneCount;
		skel->m_bones = m_bones;

		if (boneCount <= MAX_BONES)
			skel->m_sendVS = true;

		m_mesh->m_bones = new D3DXMATRIX[boneCount * 2];
		m_mesh->m_invBones = m_mesh->m_bones + boneCount;
		skel->ResetBones(m_mesh->m_bones, m_mesh->m_invBones);
	}
	else
	{
		m_obj3D->m_boneCount = (int)boneCount;

		if (boneCount <= MAX_BONES)
			m_obj3D->m_sendVS = true;

		m_obj3D->m_baseBones = new D3DXMATRIX[boneCount * 2];
		m_obj3D->m_baseInvBones = m_obj3D->m_baseBones + boneCount;

		for (i = 0; i < boneCount; i++)
		{
			m_obj3D->m_baseBones[i] = m_bones[i].TM;
			m_obj3D->m_baseInvBones[i] = m_bones[i].inverseTM;
		}
	}
}

void CImporter::_setBones(GMObject* obj, aiNode* node)
{
	bool sendVS = false;
	bool objUsedBones = false;
	uint i, j, k;
	aiMesh* mesh;

	if (m_aiBones.size() <= MAX_BONES)
		sendVS = true;
	else
	{
		QVector<string> usedBones;
		for (i = 0; i < node->mNumMeshes; i++)
		{
			mesh = m_aiScene->mMeshes[node->mMeshes[i]];
			if (mesh->HasBones())
			{
				for (j = 0; j < mesh->mNumBones; j++)
				{
					const string name = ToString(mesh->mBones[j]->mName);
					if (!usedBones.contains(name))
						usedBones.push_back(name);
				}
			}
		}
		if (usedBones.size() <= MAX_BONES)
			objUsedBones = true;
	}

	QMap<string, int> boneIDs;
	if (sendVS)
	{
		for (i = 0; i < m_aiBones.size(); i++)
			boneIDs[m_bones[i].name] = i;
	}
	else if (objUsedBones)
	{
		QVector<string> usedBones;
		for (i = 0; i < node->mNumMeshes; i++)
		{
			mesh = m_aiScene->mMeshes[node->mMeshes[i]];
			if (mesh->HasBones())
			{
				for (j = 0; j < mesh->mNumBones; j++)
				{
					const string name = ToString(mesh->mBones[j]->mName);
					if (!usedBones.contains(name))
						usedBones.push_back(name);
				}
			}
		}

		obj->usedBoneCount = usedBones.size();
		for (j = 0; j < usedBones.size(); j++)
		{
			const string name(usedBones[j]);
			int boneID = -1;

			for (k = 0; k < m_aiBones.size(); k++)
			{
				if (m_bones[k].name == name)
				{
					boneID = k;
					break;
				}
			}

			if (boneID != -1)
			{
				obj->usedBones[j] = boneID;
				boneIDs[name] = j;
			}
		}
	}

	aiBone* bone;
	SkinVertex* vertices = (SkinVertex*)obj->vertices;

	for (i = 0; i < node->mNumMeshes; i++)
	{
		mesh = m_aiScene->mMeshes[node->mMeshes[i]];

		if (mesh->HasBones())
		{
			if (!sendVS && !objUsedBones)
			{
				boneIDs.clear();

				MaterialBlock* block = &obj->materialBlocks[i];
				block->usedBoneCount = (int)mesh->mNumBones;

				for (j = 0; j < mesh->mNumBones; j++)
				{
					bone = mesh->mBones[j];
					const string name = ToString(bone->mName);
					int boneID = -1;

					for (k = 0; k < m_aiBones.size(); k++)
					{
						if (m_bones[k].name == name)
						{
							boneID = k;
							break;
						}
					}

					if (boneID != -1)
					{
						block->usedBones[j] = boneID;
						boneIDs[name] = j;
					}
				}
			}

			for (j = 0; j < mesh->mNumBones; j++)
			{
				bone = mesh->mBones[j];
				const string name = ToString(bone->mName);
				if (boneIDs.find(name) != boneIDs.end())
				{
					const ushort boneID = (ushort)boneIDs[name] * 3;
					for (k = 0; k < bone->mNumWeights; k++)
					{
						const aiVertexWeight w = bone->mWeights[k];
						if (vertices[w.mVertexId].w1 > 0.0f)
						{
							vertices[w.mVertexId].id2 = boneID;
							vertices[w.mVertexId].w2 = w.mWeight;
						}
						else
						{
							vertices[w.mVertexId].id1 = boneID;
							vertices[w.mVertexId].w1 = w.mWeight;
						}
					}
				}
			}
		}

		vertices += mesh->mNumVertices;
	}
}