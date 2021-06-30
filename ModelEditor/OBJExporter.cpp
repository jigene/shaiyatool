///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "OBJExporter.h"
#include <Object3D.h>
#include <Motion.h>

COBJExporter::COBJExporter(CAnimatedMesh* mesh)
	: CExporter(mesh)
{
}

bool COBJExporter::Export(const string& filename)
{
	m_objFile.setFileName(filename);
	if (!m_objFile.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;
	m_objOut.setDevice(&m_objFile);
	m_objOut.setRealNumberPrecision(4);
	m_objOut.setRealNumberNotation(QTextStream::FixedNotation);

	string mtlFilename = filename;
	mtlFilename.replace(".obj", ".mtl");

	m_mtlFile.setFileName(mtlFilename);
	if (!m_mtlFile.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;
	m_mtlOut.setDevice(&m_mtlFile);
	m_mtlOut.setRealNumberPrecision(4);
	m_mtlOut.setRealNumberNotation(QTextStream::FixedNotation);

	m_objOut << "# ATools v" << VERSION << '.' << SUB_VERSION << " OBJ Exporter by Aishiro" << endl << endl;
	m_objOut << "mtllib " << QFileInfo(mtlFilename).fileName() << endl << endl;

	string s;
	int vertexIndex = 0;

	for (auto it = m_objects.begin(); it != m_objects.end(); it++)
	{
		s = it.key();
		s.replace('-', '_');
		_writeGeometry(s, it.value(), vertexIndex);

		vertexIndex += it.value()->vertexCount;
	}

	m_mtlOut << "# ATools v" << VERSION << '.' << SUB_VERSION << " OBJ Exporter by Aishiro" << endl << endl;

	for (auto it = m_materials.begin(); it != m_materials.end(); it++)
	{
		s = it.key();
		s.replace('-', '_');
		_writeMaterial(s, it.value());
	}

	return true;
}

void COBJExporter::_writeGeometry(const string& name, GMObject* obj, int vertexIndex)
{
	m_objOut << "#" << endl;
	m_objOut << "# object " << name << endl;
	m_objOut << "#" << endl << endl;

	D3DXVECTOR3 v;
	int i, j;
	ushort u;

	if (obj->type == GMT_SKIN)
	{
		SkinVertex* vertices = (SkinVertex*)obj->vertices;

		for (i = 0; i < obj->vertexCount; i++)
		{
			v = vertices[i].p;
			m_objOut << "v  " << v.x << ' ' << v.y << ' ' << -v.z << endl;
		}
		m_objOut << "# " << obj->vertexCount << " vertices" << endl << endl;

		for (i = 0; i < obj->vertexCount; i++)
			m_objOut << "vn " << vertices[i].n.x << ' ' << vertices[i].n.y << ' ' << -vertices[i].n.z << endl;
		m_objOut << "# " << obj->vertexCount << " vertex normals" << endl << endl;

		for (i = 0; i < obj->vertexCount; i++)
			m_objOut << "vt " << vertices[i].t.x << ' ' << 1.0f - vertices[i].t.y << ' ' << 0.0f << endl;
		m_objOut << "# " << obj->vertexCount << " texture coords" << endl;
	}
	else
	{
		NormalVertex* vertices = (NormalVertex*)obj->vertices;

		for (i = 0; i < obj->vertexCount; i++)
		{
			v = vertices[i].p;
			D3DXVec3TransformCoord(&v, &v, &obj->transform);
			m_objOut << "v  " << v.x << ' ' << v.y << ' ' << -v.z << endl;
		}
		m_objOut << "# " << obj->vertexCount << " vertices" << endl << endl;

		for (i = 0; i < obj->vertexCount; i++)
			m_objOut << "vn " << vertices[i].n.x << ' ' << vertices[i].n.y << ' ' << -vertices[i].n.z << endl;
		m_objOut << "# " << obj->vertexCount << " vertex normals" << endl << endl;

		for (i = 0; i < obj->vertexCount; i++)
			m_objOut << "vt " << vertices[i].t.x << ' ' << 1.0f - vertices[i].t.y << ' ' << 0.0f << endl;
		m_objOut << "# " << obj->vertexCount << " texture coords" << endl;
	}

	m_objOut << endl << "g " << name << endl;

	MaterialBlock* block;
	int triCount = 0;
	string s;

	for (i = 0; i < obj->materialBlockCount; i++)
	{
		block = &obj->materialBlocks[i];

		if (obj->material)
		{
			s = _getMaterialID(&obj->materials[block->materialID]);
			s.replace('-', '_');

			m_objOut << "usemtl " << s << endl;
		}

		if (obj->materialBlockCount > 1)
			m_objOut << "o " << name << "_Block" << i << endl;
		else
			m_objOut << "o " << name << endl;

		m_objOut << "s off" << endl;

		for (j = 0; j < block->primitiveCount; j++)
		{
			u = obj->indices[block->startVertex + j * 3 + 0] + 1 + vertexIndex;
			m_objOut << "f " << u << '/' << u << '/' << u;

			u = obj->indices[block->startVertex + j * 3 + 1] + 1 + vertexIndex;
			m_objOut << " " << u << '/' << u << '/' << u;

			u = obj->indices[block->startVertex + j * 3 + 2] + 1 + vertexIndex;
			m_objOut << " " << u << '/' << u << '/' << u << endl;
		}

		triCount += block->primitiveCount;
	}

	m_objOut << "# 0 polygons - " << triCount << " triangles" << endl << endl;
}

void COBJExporter::_writeMaterial(const string& name, Material* mat)
{
	m_mtlOut << "newmtl " << name << endl
		<< "	Ns 56.4386" << endl
		<< "	Ni 1.5000" << endl
		<< "	d 1.0000" << endl
		<< "	Tr 0.0000" << endl
		<< "	Tf 1.0000 1.0000 1.0000" << endl
		<< "	illum 2" << endl
		<< "	Ka 0.0000 0.0000 0.0000" << endl
		<< "	Kd 0.8000 0.8000 0.8000" << endl
		<< "	Ks 0.5000 0.5000 0.5000" << endl
		<< "	Ke 0.0000 0.0000 0.0000" << endl
		<< "	Ks 0.5000 0.5000 0.5000" << endl
		<< "	Ke 0.0000 0.0000 0.0000" << endl
		<< "	map_Ka " << mat->textureName << endl
		<< "	map_Kd " << mat->textureName << endl
		<< endl;
}