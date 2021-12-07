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

void CImporter::_createGMObjects()
{
	int poolSize = 0, i, k;
	aiNode* node;

	for (i = 0; i < m_aiObjects.size(); i++)
	{
		node = m_aiObjects[i];
		const string name = ToString(node->mName).toLower();

		if (!name.endsWith("coll"))
		{
			if (name.startsWith("lod1"))
			{
				m_obj3D->m_LOD = true;
				m_obj3D->m_groups[1].objectCount++;
			}
			else if (name.startsWith("lod2"))
			{
				m_obj3D->m_LOD = true;
				m_obj3D->m_groups[2].objectCount++;
			}
			else
				m_obj3D->m_groups[0].objectCount++;
			poolSize++;
		}
	}

	if (!poolSize)
		return;

	GMObject* pool = new GMObject[poolSize];
	memset(pool, 0, sizeof(GMObject) * poolSize);
	for (i = 0; i < (m_obj3D->m_LOD ? MAX_GROUP : 1); i++)
	{
		m_obj3D->m_groups[i].objects = pool;
		pool += m_obj3D->m_groups[i].objectCount;
	}

	int IDs[MAX_GROUP];
	memset(IDs, 0, sizeof(int) * MAX_GROUP);

	int lod = 0;
	uint j;
	GMObject* obj;
	QMap<aiNode*, int> objectsIDWithLOD;

	for (i = 0; i < m_aiObjects.size(); i++)
	{
		node = m_aiObjects[i];

		const string name = ToString(node->mName).toLower();

		if (name.endsWith("coll"))
		{
			obj = &m_obj3D->m_collObj;
			objectsIDWithLOD[node] = 0;
		}
		else
		{
			if (name.startsWith("lod1"))
				lod = 1;
			else if (name.startsWith("lod2"))
				lod = 2;
			else
				lod = 0;

			obj = &m_obj3D->m_groups[lod].objects[IDs[lod]];
			objectsIDWithLOD[node] = IDs[lod];
			IDs[lod]++;
		}

		obj->ID = i;
		obj->type = GMT_NORMAL;
		for (j = 0; j < node->mNumMeshes; j++)
		{
			if (m_aiScene->mMeshes[node->mMeshes[j]]->HasBones())
			{
				obj->type = GMT_SKIN;
				break;
			}
		}

		if (name.endsWith("light"))
			obj->light = true;
		else
			obj->light = false;

		obj->parentID = -1;
		obj->parentType = GMT_ERROR;

		if (node->mParent && node->mParent != m_aiScene->mRootNode)
		{
			if (node->mParent->mNumMeshes > 0)
			{
				obj->parentType = GMT_NORMAL;
				obj->parentID = objectsIDWithLOD[node->mParent];
			}
			else
			{
				obj->parentType = GMT_BONE;
				for (k = 0; k < m_aiBones.size(); k++)
				{
					if (m_aiBones[k] == node->mParent)
					{
						obj->parentID = k;
						break;
					}
				}
			}
		}

		obj->transform = ToD3DMatrix(node->mTransformation);

		if (obj->parentType == GMT_ERROR)
			obj->transform *= m_rootTransform;

		m_objects[ToString(node->mName)] = obj;
	}
}

void CImporter::_fillGMObject(GMObject* obj, aiNode* node)
{
	_setMaterials(obj, node);
	_setMaterialBlocks(obj, node);

	if (obj->type == GMT_SKIN)
	{
		_setBones(obj, node);
		_setPhysiqueVertices(obj);
	}
}

void CImporter::_setMaterialBlocks(GMObject* obj, aiNode* node)
{
	obj->materialBlockCount = (int)node->mNumMeshes;
	obj->materialBlocks = new MaterialBlock[obj->materialBlockCount];
	memset(obj->materialBlocks, 0, sizeof(MaterialBlock) * obj->materialBlockCount);

	obj->indexCount = 0;
	obj->faceListCount = 0;
	obj->vertexCount = 0;

	aiMesh* mesh;
	MaterialBlock* block;
	QMap<uint, int> materialIDs;
	int materialID = 0, i;
	aiMaterial* aiMat;

	for (i = 0; i < node->mNumMeshes; i++)
	{
		block = &obj->materialBlocks[i];
		mesh = m_aiScene->mMeshes[node->mMeshes[i]];

		block->primitiveCount = mesh->mNumFaces;
		block->startVertex = obj->indexCount;

		obj->indexCount += mesh->mNumFaces * 3;
		obj->vertexCount += mesh->mNumVertices;
		obj->faceListCount += mesh->mNumFaces;

		if (obj->material)
		{
			if (materialIDs.find(mesh->mMaterialIndex) == materialIDs.end())
			{
				materialIDs[mesh->mMaterialIndex] = materialID;
				materialID++;
			}

			block->materialID = materialIDs[mesh->mMaterialIndex];
			aiMat = m_aiScene->mMaterials[mesh->mMaterialIndex];

			float opacity;
			aiMat->Get(AI_MATKEY_OPACITY, opacity);
			block->amount = (int)(opacity * 255.0f);
			if (block->amount < 255)
				block->effect |= XE_OPACITY;

			int twoSided;
			aiMat->Get(AI_MATKEY_TWOSIDED, twoSided);
			if (twoSided)
				block->effect |= XE_2SIDE;

			if (aiMat->GetTextureCount(aiTextureType_OPACITY) > 0)
			{
				aiString textureName;
				aiMat->GetTexture(aiTextureType_OPACITY, 0, &textureName);

				if (strcmp(textureName.C_Str(), obj->materials[block->materialID].textureName) == 0)
				{
					block->effect |= XE_OPACITY;
					block->effect |= XE_2SIDE;
				}
			}
		}
		else
			block->materialID = 0;
	}

	obj->indices = new ushort[obj->indexCount + obj->vertexCount];
	obj->IIB = obj->indices + obj->indexCount;
	memset(obj->indices, 0, sizeof(ushort) * (obj->indexCount + obj->vertexCount));

	if (obj->type == GMT_SKIN)
	{
		obj->vertices = new SkinVertex[obj->vertexCount];
		memset(obj->vertices, 0, sizeof(SkinVertex) * obj->vertexCount);
	}
	else
	{
		obj->vertices = new NormalVertex[obj->vertexCount];
		memset(obj->vertices, 0, sizeof(NormalVertex) * obj->vertexCount);
	}

	QVector<D3DXVECTOR3> vertexList;

	int indexIndex = 0,
		vertexIndex = 0,
		j, k,
		texCoordsIndex;

	D3DXVECTOR3 scale(1.0f, 1.0f, 1.0f), v;

	if (obj->frames)
	{
		auto it = m_objectScale.find(obj);
		if (it != m_objectScale.end())
		{
			scale = it.value();
			scale *= m_scaleFactor;

			aiNode* children;
			D3DXMATRIX matScale;
			D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);

			for (j = 0; j < node->mNumChildren; j++)
			{
				children = node->mChildren[j];
				if (children->mNumMeshes > 0)
				{
					auto it2 = m_objects.find(ToString(children->mName));
					if (it2 != m_objects.end())
						D3DXMatrixMultiply(&it2.value()->transform, &it2.value()->transform, &matScale);
				}
			}
		}
	}

	if (obj->parentType == GMT_BONE)
	{
		D3DXVECTOR3 outScale, outPos;
		D3DXQUATERNION outRot;
		D3DXMatrixDecompose(&outScale, &outRot, &outPos, &m_rootBoneTransform);

		scale.x *= outScale.x;
		scale.y *= outScale.y;
		scale.z *= outScale.z;
	}

	for (i = 0; i < node->mNumMeshes; i++)
	{
		mesh = m_aiScene->mMeshes[node->mMeshes[i]];

		for (j = 0; j < mesh->mNumFaces; j++)
		{
			obj->indices[indexIndex + 0] = vertexIndex + mesh->mFaces[j].mIndices[0];
			obj->indices[indexIndex + 1] = vertexIndex + mesh->mFaces[j].mIndices[1];
			obj->indices[indexIndex + 2] = vertexIndex + mesh->mFaces[j].mIndices[2];
			indexIndex += 3;
		}

		texCoordsIndex = 0;
		for (k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++)
		{
			if (mesh->HasTextureCoords(texCoordsIndex))
			{
				texCoordsIndex = k;
				break;
			}
		}

		if (obj->type == GMT_SKIN)
		{
			SkinVertex* vertices = (SkinVertex*)obj->vertices;
			for (j = 0; j < mesh->mNumVertices; j++)
			{
				if (mesh->HasPositions())
				{
					v = *(D3DXVECTOR3*)&mesh->mVertices[j];
					D3DXVec3TransformCoord(&v, &v, &m_rootBoneTransform);
					vertices[vertexIndex].p = v;
				}
				if (mesh->HasNormals())
				{
					v = *(D3DXVECTOR3*)&mesh->mNormals[j];
					D3DXVec3TransformCoord(&v, &v, &m_rootBoneTransform);
					D3DXVec3Normalize(&v, &v);
					vertices[vertexIndex].n = v;
				}
				if (mesh->HasTextureCoords(texCoordsIndex))
				{
					vertices[vertexIndex].t.x = mesh->mTextureCoords[texCoordsIndex][j].x;
					vertices[vertexIndex].t.y = mesh->mTextureCoords[texCoordsIndex][j].y;
				}
				if (!vertexList.contains(vertices[vertexIndex].p))
					vertexList.push_back(vertices[vertexIndex].p);
				vertexIndex++;
			}
		}
		else
		{
			NormalVertex* vertices = (NormalVertex*)obj->vertices;
			for (j = 0; j < mesh->mNumVertices; j++)
			{
				if (mesh->HasPositions())
				{
					v = *(D3DXVECTOR3*)&mesh->mVertices[j];
					v.x *= scale.x;
					v.y *= scale.y;
					v.z *= scale.z;
					vertices[vertexIndex].p = v;
				}
				if (mesh->HasNormals())
				{
					v = *(D3DXVECTOR3*)&mesh->mNormals[j];
					v.x *= scale.x;
					v.y *= scale.y;
					v.z *= scale.z;
					D3DXVec3Normalize(&v, &v);
					vertices[vertexIndex].n = v;
				}
				if (mesh->HasTextureCoords(texCoordsIndex))
				{
					vertices[vertexIndex].t.x = mesh->mTextureCoords[texCoordsIndex][j].x;
					vertices[vertexIndex].t.y = mesh->mTextureCoords[texCoordsIndex][j].y;
				}
				if (!vertexList.contains(vertices[vertexIndex].p))
					vertexList.push_back(vertices[vertexIndex].p);
				vertexIndex++;
			}
		}
	}

	D3DXVECTOR3 bbMin(FLT_MAX, FLT_MAX, FLT_MAX);
	D3DXVECTOR3 bbMax(FLT_MIN, FLT_MIN, FLT_MIN);

	obj->vertexListCount = vertexList.size();
	obj->vertexList = new D3DXVECTOR3[obj->vertexListCount];

	for (i = 0; i < obj->vertexListCount; i++)
	{
		v = vertexList[i];
		obj->vertexList[i] = v;

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

	obj->bounds.Min = bbMin;
	obj->bounds.Max = bbMax;

	for (i = 0; i < obj->vertexCount; i++)
	{
		if (obj->type == GMT_SKIN)
			v = ((SkinVertex*)obj->vertices)[i].p;
		else
			v = ((NormalVertex*)obj->vertices)[i].p;

		for (j = 0; j < obj->vertexListCount; j++)
		{
			if (obj->vertexList[j] == v)
			{
				obj->IIB[i] = j;
				break;
			}
		}
	}
}

void CImporter::_setPhysiqueVertices(GMObject* obj)
{
	obj->physiqueVertices = new int[obj->vertexListCount];
	memset(obj->physiqueVertices, 0, sizeof(int) * obj->vertexListCount);

	int boneIDs[MAX_BONES];
	int i, j, k;

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

	SkinVertex* vertices = (SkinVertex*)obj->vertices;
	SkinVertex* vertex = null;
	int boneID;
	D3DXVECTOR3 v;
	MaterialBlock* block;
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
	}
}

void CImporter::_preTransformVertices(GMObject* obj)
{
	int i;

	if (obj->type == GMT_SKIN)
	{
		SkinVertex* vertices = (SkinVertex*)obj->vertices;
		for (i = 0; i < obj->vertexCount; i++)
		{
			D3DXVec3TransformCoord(&vertices[i].p, &vertices[i].p, &obj->transform);
			D3DXVec3TransformCoord(&vertices[i].n, &vertices[i].n, &obj->transform);
			D3DXVec3Normalize(&vertices[i].n, &vertices[i].n);
		}
	}
	else
	{
		NormalVertex* vertices = (NormalVertex*)obj->vertices;
		for (i = 0; i < obj->vertexCount; i++)
		{
			D3DXVec3TransformCoord(&vertices[i].p, &vertices[i].p, &obj->transform);
			D3DXVec3TransformCoord(&vertices[i].n, &vertices[i].n, &obj->transform);
			D3DXVec3Normalize(&vertices[i].n, &vertices[i].n);
		}
	}

	for (i = 0; i < obj->vertexListCount; i++)
		D3DXVec3TransformCoord(&obj->vertexList[i], &obj->vertexList[i], &obj->transform);

	D3DXMatrixIdentity(&obj->transform);

	D3DXVECTOR3 bbMin(FLT_MAX, FLT_MAX, FLT_MAX);
	D3DXVECTOR3 bbMax(FLT_MIN, FLT_MIN, FLT_MIN);

	D3DXVECTOR3 v;
	for (i = 0; i < obj->vertexListCount; i++)
	{
		v = obj->vertexList[i];
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

	obj->bounds.Min = bbMin;
	obj->bounds.Max = bbMax;
}

void CImporter::_calculateBounds()
{
	LODGroup* group = &m_obj3D->m_groups[0];

	bool normalObj = false;
	int i, j;

	for (i = 0; i < group->objectCount; i++)
	{
		if (group->objects[i].type != GMT_SKIN)
		{
			normalObj = true;
			break;
		}
	}

	D3DXMATRIX* updates = null;
	GMObject* obj;
	D3DXMATRIX m1, m2;

	if (normalObj)
	{
		updates = new D3DXMATRIX[group->objectCount];

		for (i = 0; i < group->objectCount; i++)
			D3DXMatrixIdentity(&updates[i]);

		for (i = 0; i < group->objectCount; i++)
		{
			obj = &group->objects[i];

			if (obj->frames)
			{
				D3DXMatrixTranslation(&m1, obj->frames[0].pos.x, obj->frames[0].pos.y, obj->frames[0].pos.z);
				D3DXMatrixRotationQuaternion(&m2, &obj->frames[0].rot);
				updates[i] = m2 * m1;

				if (obj->parentID != -1)
				{
					if (obj->parentType == GMT_BONE)
						updates[i] *= m_bones[obj->parentID].TM;
					else
						updates[i] *= updates[obj->parentID];
				}
			}
			else
			{
				if (obj->parentID == -1)
					updates[i] = obj->transform;
				else
				{
					if (obj->parentType == GMT_BONE)
						updates[i] = obj->transform * m_bones[obj->parentID].TM;
					else
						updates[i] = obj->transform * updates[obj->parentID];
				}
			}
		}
	}

	D3DXVECTOR3 bbMin(FLT_MAX, FLT_MAX, FLT_MAX);
	D3DXVECTOR3 bbMax(FLT_MIN, FLT_MIN, FLT_MIN);

	D3DXMATRIX mat;
	D3DXVECTOR3 vecs[2];

	D3DXVECTOR3 v;
	for (i = 0; i < group->objectCount; i++)
	{
		obj = &group->objects[i];

		if (obj->type == GMT_SKIN)
			D3DXMatrixIdentity(&mat);
		else
			mat = updates[i];

		D3DXVec3TransformCoord(&vecs[0], &obj->bounds.Min, &mat);
		D3DXVec3TransformCoord(&vecs[1], &obj->bounds.Max, &mat);

		for (j = 0; j < 2; j++)
		{
			v = vecs[j];

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
	}

	DeleteArray(updates);

	m_obj3D->m_bounds.Min = bbMin;
	m_obj3D->m_bounds.Max = bbMax;

	m_mesh->m_bounds.Min = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_mesh->m_bounds.Max = D3DXVECTOR3(FLT_MIN, FLT_MIN, FLT_MIN);

	if (m_mesh->m_bounds.Min.x > bbMin.x) m_mesh->m_bounds.Min.x = bbMin.x;
	if (m_mesh->m_bounds.Min.y > bbMin.y) m_mesh->m_bounds.Min.y = bbMin.y;
	if (m_mesh->m_bounds.Min.z > bbMin.z) m_mesh->m_bounds.Min.z = bbMin.z;
	if (m_mesh->m_bounds.Max.x < bbMax.x) m_mesh->m_bounds.Max.x = bbMax.x;
	if (m_mesh->m_bounds.Max.y < bbMax.y) m_mesh->m_bounds.Max.y = bbMax.y;
	if (m_mesh->m_bounds.Max.z < bbMax.z) m_mesh->m_bounds.Max.z = bbMax.z;
}