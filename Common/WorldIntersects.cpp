///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "World.h"
#include "Collision.h"
#include "Landscape.h"
#include "Object.h"
#include "ModelMng.h"

bool CWorld::PickTerrain(const QPoint& mousePos, D3DXVECTOR3& out)
{
	D3DXVECTOR3 origin, dir, cur;
	GetPickRay(mousePos, origin, dir);

	dir.x *= (float)m_MPU * 2.0f;
	dir.z *= (float)m_MPU * 2.0f;
	dir.y *= (float)m_MPU * 2.0f;
	cur = origin;

	if (D3DXVec3Length(&dir) <= 0.000001f)
		return false;

	CLandscape* land;
	int z, x;
	D3DXVECTOR3 v1, v2, v3, v4, length, intersect, normal, ray, line1, line2;
	float dist;
	float nearDist = g_global3D.farPlane;
	bool pick = false;

	do
	{
		cur += dir;
		if (VecInWorld(cur))
		{
			land = GetLand(cur);
			if (land  && land->m_visible)
			{
				for (z = -m_MPU; z <= m_MPU; z += m_MPU)
				{
					for (x = -m_MPU; x <= m_MPU; x += m_MPU)
					{
						const int tempx = (int)(cur.x - x);
						const int tempy = (int)(cur.z - z);
						if (tempx < 0 || tempy < 0)
							continue;

						v1 = D3DXVECTOR3((float)(tempx), GetHeight((float)tempx, (float)tempy), (float)(tempy));
						v2 = D3DXVECTOR3((float)(tempx + m_MPU), GetHeight((float)tempx + m_MPU, (float)tempy), (float)(tempy));
						v3 = D3DXVECTOR3((float)(tempx), GetHeight((float)tempx, (float)tempy + m_MPU), (float)(tempy + m_MPU));
						v4 = D3DXVECTOR3((float)(tempx + m_MPU), GetHeight((float)tempx + m_MPU, (float)tempy + m_MPU), (float)(tempy + m_MPU));
						if (IntersectTriangle(v1, v2, v3, origin, dir, intersect, dist))
						{
							if (dist < nearDist)
							{
								line1 = v1 - v3;
								line2 = v2 - v3;
								D3DXVec3Cross(&normal, &line1, &line2);
								D3DXVec3Normalize(&normal, &normal);
								D3DXVec3Normalize(&ray, &dir);
								if (D3DXVec3Dot(&normal, &ray) < 0.0f)
									break;
								nearDist = dist;
								out = intersect;
								pick = true;
							}
						}
						else if (IntersectTriangle(v2, v4, v3, origin, dir, intersect, dist))
						{
							if (dist < nearDist)
							{
								line1 = v2 - v3;
								line2 = v4 - v3;
								D3DXVec3Cross(&normal, &line1, &line2);
								D3DXVec3Normalize(&normal, &normal);
								D3DXVec3Normalize(&ray, &dir);
								if (D3DXVec3Dot(&normal, &ray) < 0.0f)
									break;
								nearDist = dist;
								out = intersect;
								pick = true;
							}
						}
					}
				}
			}
		}
		length = origin - cur;
	} while (D3DXVec3Length(&length) < g_global3D.farPlane);

	if (out.x < 0.0f)
		out.x = 0.0f;
	else if (out.x >(float)(m_width * MAP_SIZE * MPU))
		out.x = (float)(m_width * MAP_SIZE * MPU);
	if (out.z < 0.0f)
		out.z = 0.0f;
	else if (out.z >(float)(m_height * MAP_SIZE * MPU))
		out.z = (float)(m_height * MAP_SIZE * MPU);

	return pick;
}

CObject* CWorld::PickObject(const QPoint& mousePos)
{
	D3DXVECTOR3 origin, dir;
	GetPickRay(mousePos, origin, dir);

	CObject* obj = null;
	float dist = g_global3D.farPlane, tempDist;
	int i;

	for (i = 0; i < m_cullObjCount; i++)
	{
		if (m_objCull[i]->m_isReal && !m_objCull[i]->m_isUnvisible && m_objCull[i]->Pick(origin, dir, tempDist) && tempDist < dist)
		{
			dist = tempDist;
			obj = m_objCull[i];
		}
	}

	for (i = 0; i < m_cullSfxCount; i++)
	{
		if (m_sfxCull[i]->m_isReal && !m_sfxCull[i]->m_isUnvisible && m_sfxCull[i]->Pick(origin, dir, tempDist) && tempDist < dist)
		{
			dist = tempDist;
			obj = m_sfxCull[i];
		}
	}

	return obj;
}

void CWorld::SelectObjects(const QRect& rect)
{
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);

	CObject* obj;
	D3DXVECTOR3 out;
	int i;
	for (i = 0; i < m_cullObjCount; i++)
	{
		obj = m_objCull[i];
		if (obj->m_isReal && !m_objCull[i]->m_isUnvisible)
		{
			D3DXVec3Project(&out, &obj->m_pos, &g_global3D.viewport, &g_global3D.proj, &g_global3D.view, &identity);
			if (rect.contains(QPoint((int)out.x, (int)out.y)))
			{
				if (s_selection.Find(obj) == -1)
					s_selection.Append(obj);
			}
		}
	}

	for (i = 0; i < m_cullSfxCount; i++)
	{
		obj = m_sfxCull[i];
		if (obj->m_isReal && !m_sfxCull[i]->m_isUnvisible)
		{
			D3DXVec3Project(&out, &obj->m_pos, &g_global3D.viewport, &g_global3D.proj, &g_global3D.view, &identity);
			if (rect.contains(QPoint((int)out.x, (int)out.y)))
			{
				if (s_selection.Find(obj) == -1)
					s_selection.Append(obj);
			}
		}
	}
}