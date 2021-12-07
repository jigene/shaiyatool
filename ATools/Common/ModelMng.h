///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MODELMNG_H
#define MODELMNG_H

struct CGlobal3D
{
	bool light = false;
	bool night = true;
	bool animate = false;
	D3DVIEWPORT9 viewport;
	D3DXVECTOR4 lightVec = D3DXVECTOR4(0, 0, 0, 1);
	int hour = 12, min = 0, sec = 0;
	D3DXMATRIX view, proj, invView;
	bool grid = true;
	bool obj3Deffects = true;
	DWORD backgroundColor = D3DCOLOR_ARGB(255, 50, 50, 50);
	D3DXVECTOR3 cameraPos;
	float farPlane = 1024.0f;

	bool renderTerrain = true;
	bool terrainLOD = false;
	bool renderWater = true;
	bool renderTerrainAttributes = true;
	bool gridUnity = false;
	bool gridPatch = false;
	bool gridLand = false;
	bool fog = true;
	bool skybox = true;
	bool renderObjects = true;
	bool renderObj = true;
	bool renderSFX = true;
	bool renderItem = true;
	bool renderCtrl = true;
	bool renderRegions = true;
	bool renderNPC = true;
	bool renderMonster = true;
	bool renderSpawns = true;
	bool renderMoverNames = true;
	bool objectLOD = true;
	bool spawnAllMovers = false;
	bool renderMinimap = false;
	bool renderCollisions = false;
	bool showBones = false;
	bool editionLight = false;
	bool continentVertices = false;
	D3DFILLMODE fillMode = D3DFILL_SOLID;
};

extern CGlobal3D g_global3D;

class CModel;
class CModel;
class CMesh;

class CModelMng
{
public:
	static CModelMng* Instance;

	CModelMng(LPDIRECT3DDEVICE9 device);
	~CModelMng();

	void SetModelPath(const string& path);
	const string& GetModelPath() const;
	CObject3D* GetObject3D(const string& filename);
	CSkeleton* GetSkeleton(const string& filename);
	CMotion* GetMotion(const string& filename);

	bool SkeletonExists(const string& filename);
	int GetModelPart(const string& filename);

#ifndef MODEL_EDITOR
	void SetSfxPath(const string& path);
	CSfx* GetSfx(const string& filename);

	CModel* GetModel(unsigned char modelType, const string& filename);
#endif // MODEL_EDITOR

private:
	LPDIRECT3DDEVICE9 m_device;
	string m_modelPath;
	QMap<string, CObject3D*> m_objects3D;
	QMap<string, CSkeleton*> m_skeletons;
	QMap<string, CMotion*> m_motions;

#ifndef MODEL_EDITOR
	string m_sfxPath;
	QMap<string, CSfx*> m_sfxs;

	QMap<string, CMesh*> m_meshes;
#endif // MODEL_EDITOR
};

#define ModelMng	CModelMng::Instance

#endif // MODELMNG_H