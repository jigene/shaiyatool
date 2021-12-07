///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "Importer.h"
#include "ImportUtils.h"
#include <AnimatedMesh.h>
#include <Object3D.h>
#include <Motion.h>

using namespace Assimp;

CImporter::CImporter(CAnimatedMesh* mesh, float scaleFactor)
{
	m_rootAxis = Y_UP;
	m_mesh = mesh;
	m_obj3D = null;
	m_externBones = false;
	m_bones = null;
	m_frameCount = 0;
	m_scaleFactor = scaleFactor;
	D3DXMatrixIdentity(&m_rootTransform);
	m_aiScene = null;
	D3DXMatrixIdentity(&m_rootBoneTransform);
}

CImporter::~CImporter()
{
	if (!m_mesh->m_skeleton
		&& !m_mesh->m_motion
		&& m_obj3D
		&& !m_obj3D->m_motion)
	{
		DeleteArray(m_bones);
	}
}

bool CImporter::Import(const string& filename)
{
	const uint flags = aiProcess_JoinIdenticalVertices
		| aiProcess_FlipUVs
		| aiProcess_Triangulate
		| aiProcess_GenSmoothNormals
		| aiProcess_LimitBoneWeights
		| aiProcess_ValidateDataStructure
		| aiProcess_ImproveCacheLocality
		| aiProcess_RemoveRedundantMaterials
		| aiProcess_FixInfacingNormals
		| aiProcess_FindInvalidData
		| aiProcess_OptimizeMeshes
		| aiProcess_SplitByBoneCount
		| aiProcess_TransformUVCoords
		| aiProcess_SortByPType
		| aiProcess_RemoveComponent
		| aiProcess_MakeLeftHanded;

	Importer importer;
	if (!importer.ValidateFlags(flags))
	{
		qCritical("Import flags not supported");
		return false;
	}

	if (!importer.IsExtensionSupported(('.' + GetExtension(filename)).toLocal8Bit().constData()))
	{
		qCritical("This file format isn't supported");
		return false;
	}

	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 2);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, MAX_BONES);
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_TANGENTS_AND_BITANGENTS
		| aiComponent_COLORS
		| aiComponent_TEXTURES
		| aiComponent_LIGHTS
		| aiComponent_CAMERAS);

	m_aiScene = (aiScene*)importer.ReadFile(filename.toLocal8Bit().constData(), flags);
	if (!m_aiScene)
	{
		qCritical(importer.GetErrorString());
		return false;
	}

	if (!m_aiScene->HasMeshes()
		|| !m_aiScene->mRootNode
		|| m_aiScene->mRootNode->mNumChildren == 0)
	{
		qCritical("No meshes in this scene");
		return false;
	}

	m_obj3D = m_mesh->m_elements[0].obj = new CObject3D(m_mesh->m_device);
	m_obj3D->m_ID = GenerateRandomID();

	const string filenameToLower = QFileInfo(filename).fileName().toLower();

	if (filenameToLower.startsWith("mvr")
		|| filenameToLower.startsWith("part")
		|| filenameToLower.startsWith("item"))
		m_externBones = true;

	_importScene();

	if (!m_obj3D->InitDeviceObjects())
	{
		qCritical(("Can't init object3D device objects '" + filename + "'").toLocal8Bit().data());
		return false;
	}

	importer.FreeScene();
	return true;
}

void CImporter::_importScene()
{
	_createRootTransform();
	_scanNode(m_aiScene->mRootNode);

	if (m_aiBones.size() > 0)
	{
		for (int i = 0; i < m_aiObjects.size(); i++)
		{
			for (uint j = 0; j < m_aiObjects[i]->mNumMeshes; j++)
			{
				if (m_aiScene->mMeshes[m_aiObjects[i]->mMeshes[j]]->HasBones())
				{
					_createBones();
					break;
				}
			}
		}
	}

	_createGMObjects();

	if (m_aiScene->HasAnimations())
		_createAnimations();

	aiNode* node;
	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		node = _getObjectNode(it.key());
		if (node)
			_fillGMObject(it.value(), node);
	}

	if (m_obj3D->m_collObj.type != GMT_ERROR)
		_preTransformVertices(&m_obj3D->m_collObj);

	_calculateBounds();
}

void CImporter::_scanNode(aiNode* node)
{
	aiNode* children;
	for (uint i = 0; i < node->mNumChildren; i++)
	{
		children = node->mChildren[i];
		if (children->mNumMeshes > 0)
			m_aiObjects.push_back(children);
		else
			m_aiBones.push_back(children);
		if (children->mNumChildren > 0)
			_scanNode(children);
	}
}

aiNode* CImporter::_getObjectNode(const string& name)
{
	for (int i = 0; i < m_aiObjects.size(); i++)
		if (ToString(m_aiObjects[i]->mName) == name)
			return m_aiObjects[i];
	return null;
}

void CImporter::_createRootTransform()
{
	m_rootTransform = ToD3DMatrix(m_aiScene->mRootNode->mTransformation);

	float temp;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp = m_rootTransform.m[i][j];
			if (temp < 0.0f)
				temp = -1.0f;
			else if (temp > 0.0f)
				temp = 1.0f;
			else
				temp = 0.0f;
			m_rootTransform.m[i][j] = temp;
		}
	}

	const D3DXMATRIX xUp(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	const D3DXMATRIX zUp(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	if (m_rootTransform == xUp)
		m_rootAxis = X_UP;
	else if (m_rootTransform == zUp)
		m_rootAxis = Z_UP;
	else
		m_rootAxis = Y_UP;

	D3DXMATRIX matScale;
	D3DXMatrixScaling(&matScale, m_scaleFactor, m_scaleFactor, m_scaleFactor);
	D3DXMatrixMultiply(&m_rootTransform, &m_rootTransform, &matScale);
}