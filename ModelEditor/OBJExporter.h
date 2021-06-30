///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef OBJEXPORTER_H
#define OBJEXPORTER_H

#include "Exporter.h"

class COBJExporter : public CExporter
{
public:
	COBJExporter(CAnimatedMesh* mesh);

	virtual bool Export(const string& filename);

private:
	QFile m_objFile;
	QTextStream m_objOut;
	QFile m_mtlFile;
	QTextStream m_mtlOut;

	void _writeGeometry(const string& name, GMObject* obj, int vertexIndex);
	void _writeMaterial(const string& name, Material* mat);
};

#endif // OBJEXPORTER_H