///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Collision.h"
#include "ModelMng.h"

void GetPickRay(const QPoint& mousePos, D3DXVECTOR3& origin, D3DXVECTOR3& dir)
{
	D3DXVECTOR3 v;
	v.x = (((2.0f * (float)mousePos.x()) / (float)g_global3D.viewport.Width) - 1.0f) / g_global3D.proj._11;
	v.y = -(((2.0f * (float)mousePos.y()) / (float)g_global3D.viewport.Height) - 1.0f) / g_global3D.proj._22;
	v.z = 1.0f;

	D3DXMATRIX m;
	D3DXMatrixInverse(&m, null, &g_global3D.view);

	dir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
	dir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
	dir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
	origin.x = m._41;
	origin.y = m._42;
	origin.z = m._43;
}