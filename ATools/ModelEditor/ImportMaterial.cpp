///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "Importer.h"
#include "ImportUtils.h"
#include <Object3D.h>

void CImporter::_setMaterials(GMObject* obj, aiNode* node)
{
	QVector<uint> materials;

	for (uint i = 0; i < node->mNumMeshes; i++)
	{
		const uint id = m_aiScene->mMeshes[node->mMeshes[i]]->mMaterialIndex;

		if (m_aiScene->mMaterials[id]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			if (!materials.contains(id))
				materials.push_back(id);
		}
	}

	if (materials.size() == 0)
	{
		obj->materialCount = 0;
		obj->materials = null;
		obj->material = false;
		return;
	}
	else
		obj->material = true;

	obj->materialCount = materials.size();
	obj->materials = new Material[obj->materialCount];
	memset(obj->materials, 0, sizeof(Material) * obj->materialCount);

	Material* mat;
	aiMaterial* aiMat;
	for (int i = 0; i < obj->materialCount; i++)
	{
		mat = &obj->materials[i];
		aiMat = m_aiScene->mMaterials[materials[i]];

		aiString path;
		aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		const string texture = ToString(path);
		const string textureFilename = QFileInfo(texture).fileName();
		const QByteArray buffer = textureFilename.toLocal8Bit();
		strcpy(mat->textureName, buffer.constData());

		aiColor3D ambiant;
		aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambiant);
		mat->material.Ambient.a = 1.0f;
		mat->material.Ambient.r = ambiant.r;
		mat->material.Ambient.g = ambiant.g;
		mat->material.Ambient.b = ambiant.b;

		aiColor3D diffuse;
		aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		mat->material.Diffuse.a = 1.0f;
		mat->material.Diffuse.r = diffuse.r;
		mat->material.Diffuse.g = diffuse.g;
		mat->material.Diffuse.b = diffuse.b;

		aiColor3D specular;
		aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
		mat->material.Specular.a = 1.0f;
		mat->material.Specular.r = specular.r;
		mat->material.Specular.g = specular.g;
		mat->material.Specular.b = specular.b;

		aiColor3D emissive;
		aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
		mat->material.Emissive.a = 1.0f;
		mat->material.Emissive.r = emissive.r;
		mat->material.Emissive.g = emissive.g;
		mat->material.Emissive.b = emissive.b;

		float power;
		aiMat->Get(AI_MATKEY_SHININESS_STRENGTH, power);
		mat->material.Power = power;
	}
}