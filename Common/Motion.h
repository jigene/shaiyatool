///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MOTION_H
#define MOTION_H

#define	VER_BONE 4
#define	VER_MOTION	10

#define MAX_MDL_EVENT	8

#define	MA_HIT		0x1
#define	MA_SOUND	0x2
#define	MA_QUAKE	0x4

struct TMAnimation
{
	D3DXQUATERNION rot;
	D3DXVECTOR3 pos;
};

struct MotionAttribute
{
	uint type;
	int soundID;
	float frame;
};

struct BoneFrame
{
	TMAnimation* frames;
	D3DXMATRIX TM;
};

struct Bone
{
	int parentID;
	char name[32];
	D3DXMATRIX localTM;
	D3DXMATRIX TM;
	D3DXMATRIX inverseTM;
};

class CMotion
{
public:
	CMotion();
	~CMotion();

	bool Load(const string& filename);
	void ReadTM(CFile& file, int boneCount, int frameCount);

	bool Save(const string& filename);
	void SaveTM(CFile& file);

	MotionAttribute* GetAttributes() const;
	int GetFrameCount();

	void Animate(D3DXMATRIX* bones, float currentFrame, int nextFrame);

private:
	int m_ID;
	float m_perSlerp;
	int m_boneCount;
	int m_frameCount;
	D3DXVECTOR3* m_paths;
	MotionAttribute* m_attributes;
	D3DXVECTOR3 m_events[4];
	int m_eventCount;
	Bone* m_bones;
	TMAnimation* m_animations;
	BoneFrame* m_frames;

#ifdef MODEL_EDITOR
	MODEL_EDITOR_FRIENDS
#endif // MODEL_EDITOR
};

class CSkeleton
{
public:
	CSkeleton();
	~CSkeleton();

	bool Load(const string& filename);
	bool Save(const string& filename);

	void ResetBones(D3DXMATRIX* bones, D3DXMATRIX* invBones);

	int GetBoneCount() const { return m_boneCount; }
	bool MustSendVS() const { return m_sendVS; }

private:
	int m_ID;
	int m_boneCount;
	Bone* m_bones;
	bool m_sendVS;
	D3DXMATRIX m_localRH;
	D3DXMATRIX m_localLH;
	D3DXMATRIX m_localShield;
	D3DXMATRIX m_localKnuckle;
	D3DXVECTOR3	m_events[MAX_MDL_EVENT];
	int	m_eventParentIDs[MAX_MDL_EVENT];

#ifdef MODEL_EDITOR
	MODEL_EDITOR_FRIENDS
#endif // MODEL_EDITOR
};

#endif // MOTION_H