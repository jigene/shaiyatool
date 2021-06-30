///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Motion.h"

CMotion::CMotion()
	: m_attributes(null),
	m_ID(-1),
	m_perSlerp(0.5f),
	m_paths(null),
	m_eventCount(0),
	m_bones(null),
	m_animations(null),
	m_frames(null)
{
	memset(&m_events, 0, sizeof(m_events));
}

CMotion::~CMotion()
{
	DeleteArray(m_attributes);
	DeleteArray(m_paths);
	DeleteArray(m_bones);
	DeleteArray(m_animations);
	DeleteArray(m_frames);
}

void CMotion::Animate(D3DXMATRIX* bones, float currentFrame_, int nextFrame)
{
	TMAnimation* frame = null;
	TMAnimation* next = null;
	D3DXQUATERNION slerp;
	D3DXVECTOR3 lerp;
	D3DXMATRIX	m1, m2;
	const int currentFrame = (int)currentFrame_;
	const float slp = currentFrame_ - (float)currentFrame;

	for (int i = 0; i < m_boneCount; i++)
	{
		if (m_frames[i].frames)
		{
			frame = &m_frames[i].frames[currentFrame];
			next = &m_frames[i].frames[nextFrame];

			D3DXQuaternionSlerp(&slerp, &frame->rot, &next->rot, slp);
			D3DXVec3Lerp(&lerp, &frame->pos, &next->pos, slp);
			D3DXMatrixTranslation(&m1, lerp.x, lerp.y, lerp.z);
			D3DXMatrixRotationQuaternion(&m2, &slerp);
			m2 *= m1;

			if (m_bones[i].parentID != -1)
				m2 *= bones[m_bones[i].parentID];
		}
		else
		{
			m2 = m_frames[i].TM;

			if (m_bones[i].parentID != -1)
				m2 *= bones[m_bones[i].parentID];
		}

		bones[i] = m2;
	}
}

bool CMotion::Load(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	int ver;
	file.Read(ver);
	if (ver < VER_MOTION)
	{
		qWarning(("Motion version no more supported in '" + filename + "'").toLocal8Bit());
		file.Close();
		return false;
	}

	file.Read(m_ID);
	file.Read(m_perSlerp);
	file.Skip(32);

	file.Read(m_boneCount);
	file.Read(m_frameCount);

	int temp;
	file.Read(temp);
	if (temp)
	{
		m_paths = new D3DXVECTOR3[m_frameCount];
		file.Read(m_paths, m_frameCount);
	}

	ReadTM(file, m_boneCount, m_frameCount);

	file.Read(m_attributes, m_frameCount);

	file.Read(m_eventCount);
	if (m_eventCount > 0)
		file.Read(m_events, m_eventCount);

	file.Close();
	return true;
}

bool CMotion::Save(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	file.Write(VER_MOTION);
	file.Write(m_ID);
	file.Write(m_perSlerp);

	char unused[32];
	memset(unused, 0, 32);
	file.Write(unused, 32);

	file.Write(m_boneCount);
	file.Write(m_frameCount);

	const int temp = (m_paths != null) ? 1 : 0;
	file.Write(temp);
	if (temp)
		file.Write(m_paths, m_frameCount);

	SaveTM(file);

	file.Write(m_attributes, m_frameCount);

	file.Write(m_eventCount);
	if (m_eventCount > 0)
		file.Write(m_events, m_eventCount);

	file.Close();
	return true;
}

void CMotion::ReadTM(CFile& file, int boneCount, int frameCount)
{
	m_boneCount = boneCount;
	m_frameCount = frameCount;

	m_bones = new Bone[boneCount];
	memset(m_bones, 0, sizeof(Bone) * m_boneCount);

	int len;
	Bone* bone;
	for (int i = 0; i < boneCount; i++)
	{
		bone = &m_bones[i];
		file.Read(len);
		file.Read(bone->name, len);
		file.Read(bone->inverseTM);
		file.Read(bone->localTM);
		file.Read(bone->parentID);
	}

	int aniCount, debug = 0;
	file.Read(aniCount);

	m_animations = new TMAnimation[aniCount];
	m_attributes = new MotionAttribute[m_frameCount];
	memset(m_attributes, 0, sizeof(MotionAttribute) * m_frameCount);
	m_frames = new BoneFrame[m_boneCount];

	int frame;
	TMAnimation* ani = m_animations;
	for (int i = 0; i < m_boneCount; i++)
	{
		file.Read(frame);
		if (frame == 1)
		{
			m_frames[i].frames = ani;
			file.Read(ani, m_frameCount);
			ani += m_frameCount;
			debug += m_frameCount;
		}
		else
		{
			m_frames[i].frames = null;
			file.Read(m_frames[i].TM);
		}
	}

	if (debug != aniCount)
		qWarning(("ReadTM debug " + QString::number(debug) + " and ani count "  + QString::number(aniCount) + " size problem").toLocal8Bit());
}

void CMotion::SaveTM(CFile& file)
{
	Bone* bone;
	for (int i = 0; i < m_boneCount; i++)
	{
		bone = &m_bones[i];
		const int len = strlen(bone->name);
		file.Write(len + 1);
		file.Write(bone->name, len);
		file.Write('\0');
		file.Write(bone->inverseTM);
		file.Write(bone->localTM);
		file.Write(bone->parentID);
	}

	int aniCount = 0;
	for (int i = 0; i < m_boneCount; i++)
		if (m_frames[i].frames)
			aniCount += m_frameCount;

	file.Write(aniCount);

	TMAnimation* ani = m_animations;
	for (int i = 0; i < m_boneCount; i++)
	{
		const int frame = (m_frames[i].frames != null) ? 1 : 0;
		file.Write(frame);
		if (frame == 1)
		{
			file.Write(ani, m_frameCount);
			ani += m_frameCount;
		}
		else
			file.Write(m_frames[i].TM);
	}
}

MotionAttribute* CMotion::GetAttributes() const
{
	return m_attributes;
}

int CMotion::GetFrameCount()
{
	return m_frameCount;
}

CSkeleton::CSkeleton()
	: m_boneCount(0),
	m_bones(null),
	m_sendVS(false),
	m_ID(-1)
{
	D3DXMatrixIdentity(&m_localRH);
	D3DXMatrixIdentity(&m_localLH);
	D3DXMatrixIdentity(&m_localShield);
	D3DXMatrixIdentity(&m_localKnuckle);
	memset(m_events, 0, sizeof(m_events));
	memset(m_eventParentIDs, 0, sizeof(m_eventParentIDs));
}

CSkeleton::~CSkeleton()
{
	DeleteArray(m_bones);
}

void CSkeleton::ResetBones(D3DXMATRIX* bones, D3DXMATRIX* invBones)
{
	for (int i = 0; i < m_boneCount; i++)
	{
		bones[i] = m_bones[i].TM;
		invBones[i] = m_bones[i].inverseTM;
	}
}

bool CSkeleton::Load(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	int ver;
	file.Read(ver);
	if (ver < VER_BONE)
	{
		qWarning(("Skeleton version no more supported in '" + filename + "'").toLocal8Bit());
		file.Close();
		return false;
	}

	file.Read(m_ID);
	file.Read(m_boneCount);

	int len;
	Bone* bone;
	m_bones = new Bone[m_boneCount];
	for (int i = 0; i < m_boneCount; i++)
	{
		bone = &m_bones[i];
		file.Read(len);
		file.Read(bone->name, len);
		file.Read(bone->TM);
		file.Read(bone->inverseTM);
		file.Read(bone->localTM);
		file.Read(bone->parentID);
	}

	int temp;
	file.Read(temp);
	m_sendVS = temp != 0;

	file.Read(m_localRH);
	file.Read(m_localShield);
	file.Read(m_localKnuckle);

	if (ver == 5)
	{
		file.Read(m_events, 4);
		file.Read(m_eventParentIDs, 4);
	}
	else if (ver >= 6)
	{
		file.Read(m_events, MAX_MDL_EVENT);
		file.Read(m_eventParentIDs, MAX_MDL_EVENT);
	}

	if (ver == 7)
		file.Read(m_localLH);

	file.Close();
	return true;
}

bool CSkeleton::Save(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	file.Write(VER_BONE);
	file.Write(m_ID);
	file.Write(m_boneCount);

	Bone* bone;
	for (int i = 0; i < m_boneCount; i++)
	{
		bone = &m_bones[i];
		const int len = (int)strlen(bone->name);
		file.Write(len + 1);
		file.Write(bone->name, len);
		file.Write('\0');
		file.Write(bone->TM);
		file.Write(bone->inverseTM);
		file.Write(bone->localTM);
		file.Write(bone->parentID);
	}

	const int sendVS = m_sendVS ? 1 : 0;
	file.Write(sendVS);

	file.Write(m_localRH);
	file.Write(m_localShield);
	file.Write(m_localKnuckle);
	file.Write(m_events, MAX_MDL_EVENT);
	file.Write(m_eventParentIDs, MAX_MDL_EVENT);
	file.Write(m_localLH);

	file.Close();
	return true;
}