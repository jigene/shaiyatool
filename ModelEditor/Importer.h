///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef IMPORTER_H
#define IMPORTER_H

class CAnimatedMesh;
class CObject3D;
struct GMObject;
struct Bone;
struct TMAnimation;

enum EAxis
{
	Z_UP,
	Y_UP,
	X_UP
};

class CImporter
{
public:
	CImporter(CAnimatedMesh* mesh, float scaleFactor);
	~CImporter();

	bool Import(const string& filename);

private:
	CAnimatedMesh* m_mesh;
	CObject3D* m_obj3D;
	bool m_externBones;
	Bone* m_bones;
	int m_frameCount;

	float m_scaleFactor;
	D3DXMATRIX m_rootTransform;
	EAxis m_rootAxis;
	D3DXMATRIX m_rootBoneTransform;

	QMap<string, GMObject*> m_objects;
	QMap<GMObject*, D3DXVECTOR3> m_objectScale;

	aiScene* m_aiScene;
	QVector<aiNode*> m_aiObjects;
	QVector<aiNode*> m_aiBones;

private:
	void _importScene();
	void _scanNode(aiNode* node);
	void _calculateBounds();
	void _createRootTransform();

	void _createGMObjects();
	void _createBones();

	void _createAnimations();
	TMAnimation* _createAnimation(aiNodeAnim* anim, double ticksPerSecond, D3DXVECTOR3& scale);
	void _transformSkinAnimation(CMotion* motion, const D3DXVECTOR3& scale);
	void _transformObjAnimation(TMAnimation* frames);

	void _fillGMObject(GMObject* obj, aiNode* node);
	void _setMaterials(GMObject* obj, aiNode* node);
	void _setMaterialBlocks(GMObject* obj, aiNode* node);
	void _setBones(GMObject* obj, aiNode* node);
	void _setPhysiqueVertices(GMObject* obj);
	void _preTransformVertices(GMObject* obj);

	aiNode* _getObjectNode(const string& name);
};

#endif // IMPORTER_H