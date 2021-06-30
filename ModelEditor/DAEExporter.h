///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef DAEEXPORTER_H
#define DAEEXPORTER_H

#include "Exporter.h"

class CDAEExporter : public CExporter
{
public:
	CDAEExporter(CAnimatedMesh* mesh);

	virtual bool Export(const string& filename);

private:
	struct VertexWeight
	{
		int boneIds[2];
		int weightIDs[2];
	};

	QFile m_file;
	QDomDocument m_doc;
	QDomElement m_colladaNode;

	void _writeAsset();
	void _writeCameras();
	void _writeLights();
	void _writeImages();
	void _writeEffects();
	void _writeMaterials();
	void _writeGeometries();
	void _writeAnimations();
	void _writeControllers();
	void _writeVisualScenes();
	void _writeScene();

	void _writeNode(QDomElement* parent, const string& name, GMObject* obj);
	void _writeNode(QDomElement* parent, const string& name, Bone* bone);

	string _matToString(const D3DXMATRIX& mat);
	string _identityMat();
};

#endif // DAEEXPORTER_H