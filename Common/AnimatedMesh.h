///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef ANIMATEDMESH_H
#define ANIMATEDMESH_H

#include "Model.h"

#define MAX_ANIMATED_ELEMENTS	21

struct MotionAttribute;
class CSkeleton;
class CMotion;
class CObject3D;

class CAnimatedMesh : public CModel
{
public:
	CAnimatedMesh(LPDIRECT3DDEVICE9 device);
	virtual ~CAnimatedMesh();

	virtual void Render(const D3DXMATRIX* world, int lod, int alpha = 255);
	virtual bool Load(const string& filename)
	{
		return Load(filename, 0);
	}
	virtual bool Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist);

	bool Load(const string& filename, int part);
	void SetMotion(const string& filename);
	MotionAttribute* GetAttributes() const;

	virtual void RenderCollision(const D3DXMATRIX* world);

private:
	struct Element
	{
		CObject3D* obj;
	};

	Element m_elements[MAX_ANIMATED_ELEMENTS];
	CSkeleton* m_skeleton;
	D3DXMATRIX* m_bones;
	D3DXMATRIX* m_invBones;
	CMotion* m_motion;

#ifdef MODEL_EDITOR
	MODEL_EDITOR_FRIENDS
#endif // MODEL_EDITOR
};

#endif // ANIMATEDMESH_H