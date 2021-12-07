///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef COLLISION_H
#define COLLISION_H

void GetPickRay(const QPoint& mousePos, D3DXVECTOR3& origin, D3DXVECTOR3& dir);

inline bool IntersectTriangle(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, D3DXVECTOR3& intersect, float& dist)
{
	float u, v;
	if (D3DXIntersectTri(&v0, &v1, &v2, &origin, &dir, &u, &v, &dist) == TRUE)
	{
		intersect = origin + dist * dir;
		return true;
	}
	return false;
}

inline bool IntersectTriangle(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist)
{
	float u, v;
	if (D3DXIntersectTri(&v0, &v1, &v2, &origin, &dir, &u, &v, &dist) == TRUE)
		return true;
	return false;
}

#endif // COLLISION_H