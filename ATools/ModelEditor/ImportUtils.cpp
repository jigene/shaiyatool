///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "ImportUtils.h"

D3DXMATRIX ToD3DMatrix(const aiMatrix4x4& mat)
{
	return D3DXMATRIX(
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
		);
}

int GenerateRandomID()
{
	const int id = rand();
	if (id < 0)
		return -id;
	else if (id == 0)
		return 1;
	else
		return id;
}

string ToString(const aiString& s)
{
	return string(s.C_Str());
}