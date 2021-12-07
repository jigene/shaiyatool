///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef IMPORTUTILS_H
#define IMPORTUTILS_H

D3DXMATRIX ToD3DMatrix(const aiMatrix4x4& mat);

int GenerateRandomID();

string ToString(const aiString& s);

#endif // IMPORTUTILS_H