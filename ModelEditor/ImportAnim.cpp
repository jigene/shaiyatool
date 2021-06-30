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

void CImporter::_transformSkinAnimation(CMotion* motion, const D3DXVECTOR3& scale)
{
	const int boneCount = motion->m_boneCount;
	const int frameCount = motion->m_frameCount;

	TMAnimation* frames;
	int i, j;
	D3DXMATRIX mat, matRot, matPos, matScale;
	D3DXVECTOR3 pos;
	D3DXQUATERNION rot;
	Bone* bone;

	D3DXVECTOR3* scales = new D3DXVECTOR3[boneCount];

	for (j = 0; j < frameCount; j++)
	{
		for (i = 0; i < boneCount; i++)
		{
			bone = &motion->m_bones[i];

			if (motion->m_frames[i].frames)
			{
				frames = motion->m_frames[i].frames;

				pos = frames[j].pos;
				rot = frames[j].rot;

				if (bone->parentID == -1)
				{
					D3DXMatrixIdentity(&mat);
					D3DXMatrixRotationQuaternion(&matRot, &rot);
					D3DXMatrixMultiply(&mat, &mat, &matRot);
					D3DXMatrixTranslation(&matPos, pos.x, pos.y, pos.z);
					D3DXMatrixMultiply(&mat, &mat, &matPos);

					mat *= m_rootTransform;
					D3DXMatrixDecompose(&scales[i], &rot, &pos, &mat);

					scales[i].x *= scale.x;
					scales[i].y *= scale.y;
					scales[i].z *= scale.z;
				}
				else
				{
					pos.x *= scales[bone->parentID].x;
					pos.y *= scales[bone->parentID].y;
					pos.z *= scales[bone->parentID].z;

					scales[i] = scales[bone->parentID];
				}

				frames[j].pos = pos;
				frames[j].rot = rot;
			}
			else
			{
				if (bone->parentID == -1)
				{
					D3DXMatrixDecompose(&scales[i], &rot, &pos, &bone->TM);

					scales[i].x *= scale.x;
					scales[i].y *= scale.y;
					scales[i].z *= scale.z;
				}
				else
					scales[i] = scales[bone->parentID];
			}
		}
	}

	DeleteArray(scales);
}

void CImporter::_transformObjAnimation(TMAnimation* frames)
{
	if (!frames)
		return;

	D3DXMATRIX mat, matRot, matPos, matScale;
	D3DXVECTOR3 pos, scale;
	D3DXQUATERNION rot;

	for (int i = 0; i < m_frameCount; i++)
	{
		pos = frames[i].pos;
		rot = frames[i].rot;

		D3DXMatrixIdentity(&mat);
		D3DXMatrixRotationQuaternion(&matRot, &rot);
		D3DXMatrixMultiply(&mat, &mat, &matRot);
		D3DXMatrixTranslation(&matPos, pos.x, pos.y, pos.z);
		D3DXMatrixMultiply(&mat, &mat, &matPos);

		D3DXMatrixMultiply(&mat, &mat, &m_rootTransform);
		D3DXMatrixDecompose(&scale, &rot, &pos, &mat);

		frames[i].pos = pos;
		frames[i].rot = rot;
	}
}

void CImporter::_createAnimations()
{
	aiAnimation* anim = null;
	uint i;

	for (i = 0; i < m_aiScene->mNumAnimations; i++)
	{
		if (!anim)
			anim = m_aiScene->mAnimations[i];
		else
		{
			if (m_aiScene->mAnimations[i]->mDuration > anim->mDuration)
				anim = m_aiScene->mAnimations[i];
		}
	}

	if (!anim || !anim->mNumChannels)
		return;

	double ticksPerSecond = 1.0;
	if (anim->mTicksPerSecond > 0.0)
		ticksPerSecond = anim->mTicksPerSecond;

	m_frameCount = (int)((anim->mDuration / ticksPerSecond) * 30.0 + 1.5);

	if (!m_frameCount)
		return;

	QMap<int, TMAnimation*> bonesAni;
	bool found;
	aiNodeAnim* nodeAnim;
	int j;
	QMap<string, GMObject*>::iterator it;
	D3DXVECTOR3 scale, boneScale(1.0f, 1.0f, 1.0f);

	for (i = 0; i < anim->mNumChannels; i++)
	{
		nodeAnim = anim->mChannels[i];
		TMAnimation* frames = _createAnimation(nodeAnim, ticksPerSecond, scale);

		const string name = ToString(nodeAnim->mNodeName);

		it = m_objects.find(name);
		if (it != m_objects.end())
		{
			it.value()->frames = frames;
			m_obj3D->m_frameCount = m_frameCount;
			m_objectScale[it.value()] = scale;

			_transformObjAnimation(frames);
			continue;
		}

		found = false;
		if (m_bones)
		{
			for (j = 0; j < m_aiBones.size(); j++)
			{
				if (name == m_bones[j].name)
				{
					bonesAni[j] = frames;
					found = true;
					if (m_bones[j].parentID == -1)
						boneScale = scale;
					break;
				}
			}
		}

		if (!found)
			DeleteArray(frames);
	}

	m_mesh->m_frameCount = m_frameCount;

	if (bonesAni.size() > 0)
	{
		CMotion* motion = new CMotion();
		motion->m_ID = GenerateRandomID();
		motion->m_boneCount = m_aiBones.size();
		motion->m_bones = m_bones;
		motion->m_frameCount = m_frameCount;
		motion->m_attributes = new MotionAttribute[motion->m_frameCount];
		memset(motion->m_attributes, 0, sizeof(MotionAttribute) * motion->m_frameCount);
		motion->m_animations = new TMAnimation[bonesAni.size() * m_frameCount];
		memset(motion->m_animations, 0, sizeof(TMAnimation) * bonesAni.size() * m_frameCount);
		motion->m_frames = new BoneFrame[motion->m_boneCount];
		memset(motion->m_frames, 0, sizeof(BoneFrame) * motion->m_boneCount);

		TMAnimation* frames = motion->m_animations;
		for (int i = 0; i < motion->m_boneCount; i++)
		{
			if (bonesAni.find(i) != bonesAni.end())
			{
				motion->m_frames[i].frames = frames;
				memcpy(frames, bonesAni[i], sizeof(TMAnimation) * m_frameCount);
				frames += m_frameCount;
			}
			else
			{
				motion->m_frames[i].frames = null;
				motion->m_frames[i].TM = m_bones[i].localTM;
			}
		}

		if (m_externBones)
		{
			m_mesh->m_motion = motion;
			m_mesh->m_skeleton->m_bones = new Bone[m_mesh->m_skeleton->m_boneCount];
			memcpy(m_mesh->m_skeleton->m_bones, m_bones, sizeof(Bone) * m_mesh->m_skeleton->m_boneCount);
		}
		else
		{
			m_obj3D->m_motion = motion;
			m_obj3D->m_frameCount = m_frameCount;
		}

		_transformSkinAnimation(motion, boneScale);
	}

	if (bonesAni.size() > 0)
	{
		for (auto it2 = bonesAni.begin(); it2 != bonesAni.end(); it2++)
			DeleteArray(it2.value());
	}

	if (m_obj3D->m_frameCount)
	{
		m_obj3D->m_attributes = new MotionAttribute[m_obj3D->m_frameCount];
		memset(m_obj3D->m_attributes, 0, sizeof(MotionAttribute) * m_obj3D->m_frameCount);
	}
}

TMAnimation* CImporter::_createAnimation(aiNodeAnim* anim, double ticksPerSecond, D3DXVECTOR3& scale)
{
	TMAnimation* frames = new TMAnimation[m_frameCount];
	memset(frames, 0, sizeof(TMAnimation) * m_frameCount);

	uint j, k;
	aiVectorKey* firstPos, *lastPos;
	double firstTime = 99999.0, lastTime = -1.0;
	for (j = 0; j < anim->mNumPositionKeys; j++)
	{
		if (anim->mPositionKeys[j].mTime < firstTime)
		{
			firstPos = &anim->mPositionKeys[j];
			firstTime = anim->mPositionKeys[j].mTime;
		}
		if (anim->mPositionKeys[j].mTime > lastTime)
		{
			lastPos = &anim->mPositionKeys[j];
			lastTime = anim->mPositionKeys[j].mTime;
		}
	}

	firstTime = 99999.0; lastTime = -1.0;
	aiQuatKey* firstRot, *lastRot;
	for (j = 0; j < anim->mNumRotationKeys; j++)
	{
		if (anim->mRotationKeys[j].mTime < firstTime)
		{
			firstRot = &anim->mRotationKeys[j];
			firstTime = anim->mRotationKeys[j].mTime;
		}
		if (anim->mRotationKeys[j].mTime > lastTime)
		{
			lastRot = &anim->mRotationKeys[j];
			lastTime = anim->mRotationKeys[j].mTime;
		}
	}

	D3DXVECTOR3 prevVec, nextVec, lerp;
	D3DXQUATERNION prevQuat, nextQuat, slerp;

	aiVectorKey* prevPos, *nextPos;
	aiQuatKey* prevRot, *nextRot;
	for (int i = 0; i < m_frameCount; i++)
	{
		const double time = ((double)i * ticksPerSecond * (1.0 / 30.0));
		prevPos = null;
		nextPos = null;
		firstTime = -1.0; lastTime = 99999.0;

		for (j = 0; j < anim->mNumPositionKeys; j++)
		{
			if (anim->mPositionKeys[j].mTime <= time
				&& anim->mPositionKeys[j].mTime > firstTime)
			{
				prevPos = &anim->mPositionKeys[j];
				firstTime = anim->mPositionKeys[j].mTime;
			}
			if (anim->mPositionKeys[j].mTime >= time
				&& anim->mPositionKeys[j].mTime < lastTime)
			{
				nextPos = &anim->mPositionKeys[j];
				lastTime = anim->mPositionKeys[j].mTime;
			}
		}

		if (!prevPos)
			prevPos = firstPos;
		if (!nextPos)
			nextPos = lastPos;

		prevRot = null;
		nextRot = null;
		firstTime = -1.0; lastTime = 99999.0;

		for (j = 0; j < anim->mNumRotationKeys; j++)
		{
			if (anim->mRotationKeys[j].mTime <= time
				&& anim->mRotationKeys[j].mTime > firstTime)
			{
				prevRot = &anim->mRotationKeys[j];
				firstTime = anim->mRotationKeys[j].mTime;
			}
			if (anim->mRotationKeys[j].mTime >= time
				&& anim->mRotationKeys[j].mTime < lastTime)
			{
				nextRot = &anim->mRotationKeys[j];
				lastTime = anim->mRotationKeys[j].mTime;
			}
		}

		if (!prevRot)
			prevRot = firstRot;
		if (!nextRot)
			nextRot = lastRot;

		prevVec = D3DXVECTOR3(prevPos->mValue.x, prevPos->mValue.y, prevPos->mValue.z);
		nextVec = D3DXVECTOR3(nextPos->mValue.x, nextPos->mValue.y, nextPos->mValue.z);
		prevQuat = D3DXQUATERNION(prevRot->mValue.x, prevRot->mValue.y, prevRot->mValue.z, prevRot->mValue.w);
		nextQuat = D3DXQUATERNION(nextRot->mValue.x, nextRot->mValue.y, nextRot->mValue.z, nextRot->mValue.w);

		if ((nextPos->mTime - prevPos->mTime) < 0.000001)
			lerp = prevVec;
		else
		{
			const double lerpSlp = (time - prevPos->mTime) / (nextPos->mTime - prevPos->mTime);

			if (lerpSlp < 0.000001)
				lerp = prevVec;
			else
				D3DXVec3Lerp(&lerp, &prevVec, &nextVec, (float)lerpSlp);
		}

		if ((nextRot->mTime - prevRot->mTime) < 0.000001)
			slerp = prevQuat;
		else
		{
			const double slerpSlp = (time - prevRot->mTime) / (nextRot->mTime - prevRot->mTime);

			if (slerpSlp < 0.000001)
				slerp = prevQuat;
			else
				D3DXQuaternionSlerp(&slerp, &prevQuat, &nextQuat, (float)slerpSlp);
		}

		frames[i].pos = lerp;
		frames[i].rot = slerp;
	}

	if (anim->mNumScalingKeys > 0)
	{
		scale = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		for (j = 0; j < anim->mNumScalingKeys; j++)
			scale += D3DXVECTOR3(anim->mScalingKeys[j].mValue.x, anim->mScalingKeys[j].mValue.y, anim->mScalingKeys[j].mValue.z);
		scale /= (float)anim->mNumScalingKeys;
	}
	else
		scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

	return frames;
}