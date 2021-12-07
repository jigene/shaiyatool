///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "ModelMng.h"
#include "Object3D.h"
#include "Motion.h"
#include "AnimatedMesh.h"
#ifndef MODEL_EDITOR
#include "Sfx.h"
#include "Mesh.h"
#include "SfxModel.h"
#endif // MODEL_EDITOR

CGlobal3D g_global3D;
CModelMng* ModelMng = null;

CModelMng::CModelMng(LPDIRECT3DDEVICE9 device)
	: m_device(device)
{
	ModelMng = this;
	m_modelPath = "Model/";
#ifndef MODEL_EDITOR
	m_sfxPath = "SFX/";
#endif // MODEL_EDITOR
}

CModelMng::~CModelMng()
{
	for (auto it = m_objects3D.begin(); it != m_objects3D.end(); it++)
		Delete(it.value());
	for (auto it = m_skeletons.begin(); it != m_skeletons.end(); it++)
		Delete(it.value());
	for (auto it = m_motions.begin(); it != m_motions.end(); it++)
		Delete(it.value());
#ifndef MODEL_EDITOR
	for (auto it = m_sfxs.begin(); it != m_sfxs.end(); it++)
		Delete(it.value());
	for (auto it = m_meshes.begin(); it != m_meshes.end(); it++)
		Delete(it.value());
#endif // MODEL_EDITOR

	ModelMng = null;
}

void CModelMng::SetModelPath(const string& path)
{
	m_modelPath = path;
}

const string& CModelMng::GetModelPath() const
{
	return m_modelPath;
}

CObject3D* CModelMng::GetObject3D(const string& filename)
{
	const string name = filename.toLower();

	auto it = m_objects3D.find(name);
	if (it != m_objects3D.end())
		return it.value();

	CObject3D* obj3D = new CObject3D(m_device);

	if (!obj3D->Load(m_modelPath + filename))
		Delete(obj3D);

	if (obj3D)
	{
		if (!obj3D->InitDeviceObjects())
		{
			qCritical(("Can't init object3D device objects '" + filename + "'").toLocal8Bit().data());
			Delete(obj3D);
		}
	}

#ifndef MODEL_EDITOR
	m_objects3D[name] = obj3D;
#endif // MODEL_EDITOR
	return obj3D;
}

CSkeleton* CModelMng::GetSkeleton(const string& filename)
{
	const string name = filename.toLower();

	auto it = m_skeletons.find(name);
	if (it != m_skeletons.end())
		return it.value();

	CSkeleton* skel = new CSkeleton();

	if (!skel->Load(m_modelPath + filename))
		Delete(skel);
	
#ifndef MODEL_EDITOR
	m_skeletons[name] = skel;
#endif // MODEL_EDITOR
	return skel;
}

bool CModelMng::SkeletonExists(const string& filename)
{
	return QFileInfo(m_modelPath + filename).exists();
}

CMotion* CModelMng::GetMotion(const string& filename)
{
	const string name = filename.toLower();

	auto it = m_motions.find(name);
	if (it != m_motions.end())
		return it.value();

	CMotion* motion = new CMotion();

	if (!motion->Load(m_modelPath + filename))
		Delete(motion);

#ifndef MODEL_EDITOR
	m_motions[name] = motion;
#endif // MODEL_EDITOR
	return motion;
}

int CModelMng::GetModelPart(const string& filename)
{
	int part = 0;

	string partName = filename.toLower();
	partName.remove(0, 6);
	partName.remove(partName.size() - 4, 4);

	if (partName.endsWith("hand") || partName.endsWith("gloves"))
		part = PARTS_HAND;
	else if (partName.endsWith("cap") || partName.endsWith("hat"))
		part = PARTS_CAP;
	else if (partName.endsWith("upper") || partName.endsWith("suit"))
		part = PARTS_UPPER_BODY;
	else if (partName.endsWith("lower"))
		part = PARTS_LOWER_BODY;
	else if (partName.endsWith("foot") || partName.endsWith("shoes") || partName.endsWith("boots"))
		part = PARTS_FOOT;
	else if (partName.startsWith("mas"))
		part = PARTS_MASK;
	else if (partName.endsWith("cape") || partName.contains("cloak"))
		part = PARTS_CLOAK;
	else if (partName.contains("head"))
		part = PARTS_HEAD;
	else if (partName.contains("hair"))
		part = PARTS_HAIR;

	return part;
}

#ifndef MODEL_EDITOR
void CModelMng::SetSfxPath(const string& path)
{
	m_sfxPath = path;
}

CSfx* CModelMng::GetSfx(const string& filename)
{
	const string name = filename.toLower();

	auto it = m_sfxs.find(name);
	if (it != m_sfxs.end())
		return it.value();

	CSfx* sfx = new CSfx(m_device);

	if (!sfx->Load(m_sfxPath + filename))
		Delete(sfx);

#ifndef SFX_EDITOR
	m_sfxs[name] = sfx;
#endif // SFX_EDITOR
	return sfx;
}

CModel* CModelMng::GetModel(unsigned char modelType, const string& filename)
{
	CModel* model = null;

	switch (modelType)
	{
	case MODELTYPE_SFX:
		model = new CSfxModel(m_device);
		if (!model->Load(filename))
			Delete(model);
		break;

	case MODELTYPE_ANIMATED_MESH:
		model = new CAnimatedMesh(m_device);
		if (!model->Load(filename))
			Delete(model);
		break;

	case MODELTYPE_MESH:
	{
		const string name = filename.toLower();
		auto it = m_meshes.find(name);
		if (it != m_meshes.end())
			model = it.value();
		else
		{
			model = new CMesh(m_device);
			if (!model->Load(filename))
				Delete(model);
			m_meshes[name] = (CMesh*)model;
		}
	}
	break;
	}

	return model;
}
#endif // MODEL_EDITOR