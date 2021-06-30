///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MESH_H
#define MESH_H

#include "Model.h"

class CObject3D;

class CMesh : public CModel
{
public:
	CMesh(LPDIRECT3DDEVICE9 device);

	virtual bool Load(const string& filename);
	virtual void Render(const D3DXMATRIX* world, int lod, int alpha = 255);
	virtual bool Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist);

	virtual void RenderCollision(const D3DXMATRIX* world);

private:
	CObject3D* m_object3D;
};

#endif // MESH_H