///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WORLD_H
#define WORLD_H

#define MAP_SIZE	128
#define NUM_PATCHES_PER_SIDE	16
#define PATCH_SIZE (MAP_SIZE/NUM_PATCHES_PER_SIDE)

#define LIGHTMAP_SIZE ((PATCH_SIZE - 1) * NUM_PATCHES_PER_SIDE)
#define LIGHTMAP_UNITY ((float)(MAP_SIZE * MPU) / (float)LIGHTMAP_SIZE)

#define MASK_WATERFRAME	0xfc

#define WTYPE_NONE  0x00
#define WTYPE_CLOUD 0x01
#define WTYPE_WATER 0x02

#define HGT_NOWALK 1000.0f
#define HGT_NOFLY  2000.0f
#define HGT_NOMOVE 3000.0f
#define HGT_DIE    4000.0f

#define MAX_DISPLAYOBJ			5000
#define MAX_DISPLAYSFX			500
#define MAX_CONTINENT_VERTICES	100
#define MAX_PATH_VERTICES		500

#define SEACLOUD_DEFAULT_TEXTURE_ID	10

extern int MPU;
extern D3DXPLANE g_planeFrustum[6];
extern string STR_DEFAULT;
extern string STR_NO;

class CLandscape;
class CSkybox;
class CObject;
class CTextFile;
class CFont;
struct WaterHeight;
class CSpawnObject;
class CPath;

typedef CPtrArray<CObject> CObjectArray[MAX_OBJTYPE];
typedef QMap<int, CPtrArray<CPath>*> CPathArray;

struct TerrainVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 };
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	float tu1, tv1;
	float tu2, tv2;
};

struct WaterVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 };
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	float u, v;
};

struct Color3DVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
	D3DXVECTOR3 p;
	DWORD c;
};

struct UnderWaterVertex
{
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 };
	D3DXVECTOR4 p;
	D3DXVECTOR2 t;
};

struct LightColor
{
	float r1, g1, b1, r2, g2, b2;
};

struct Continent
{
	bool town;
	int ID;
	string name;
	bool useEnvir;
	bool useRealData;
	D3DXVECTOR3 ambient;
	D3DXVECTOR3 diffuse;
	float fogStart;
	float fogEnd;
	int weather;
	string skyTexture;
	string cloudTexture;
	string moonTexture;
	string sunTexture;
	QVector<D3DXVECTOR3> vertices;
};

class CWorld
{
public:
	CWorld(LPDIRECT3DDEVICE9 device);
	~CWorld();

	void Create(int width, int height, int textureID, float heightMap, int MPU, bool indoor, const string& bitmap);
	bool Load(const string& filename);
	bool Save(const string& filename);

	void UpdateContinent();
	void Render();

	void WorldPosToLand(const D3DXVECTOR3& pos, int& x, int& z) const;
	bool LandInWorld(int x, int z) const;
	CLandscape* GetLand(int x, int z) const;
	CLandscape* GetLand(const D3DXVECTOR3& v) const;
	bool VecInWorld(const D3DXVECTOR3& v) const;
	bool VecInWorld(float x, float z) const;

	CTexture* GetSeacloudTexture() const;
	const D3DXVECTOR2& GetSeacloudPos() const;
	const D3DXVECTOR3& GetCameraPos() const;
	int GetWidth() const;
	int GetHeight() const;
	float GetHeight(float x, float z) const;
	float GetHeight_fast(float x, float z) const;
	float GetHeightAttribute(float x, float z) const;
	WaterHeight* GetWaterHeight(float x, float z);

	void AddObject(CObject* obj);
	void DeleteObject(CObject* obj);
	CObject* GetObject(objid id) const;
	CObject* GetObject(objid id, uint type) const;
	CPtrArray<CPath>* GetPath(int ID) const;
	const CPtrArray<CObject>& GetObjects(uint type) const;
	void SpawnObject(CObject* obj, bool addSpawn);
	void UpdateSpawns();
	void MoveObject(CObject* obj, const D3DXVECTOR3& newPos);
	bool PickTerrain(const QPoint& mousePos, D3DXVECTOR3& out);
	CObject* PickObject(const QPoint& mousePos);
	void SelectObjects(const QRect& rect);

private:
	LPDIRECT3DDEVICE9 m_device;
	int m_width;
	int m_height;
	DWORD m_ambient;
	int m_worldRevivalID;
	string m_revivalPlace;
	bool m_inDoor;
	bool m_canFly;
	int m_bgmID;
	int m_modePK;
	DWORD m_diffuse;
	D3DXVECTOR3 m_lightDir;
	D3DXVECTOR3 m_cameraPos;
	D3DXVECTOR2 m_cameraAngle;
	float m_fogStart;
	float m_fogEnd;
	float m_fogDensity;
	int m_MPU;
	string m_skyTextureNames[3];
	string m_cloudTextureNames[3];
	string m_sunTextureName;
	string m_moonTextureName;
	string m_seaCloudTextureName;
	QMap<string, string> m_texts;
	string m_filename;
	CLandscape** m_lands;
	int m_visibilityLand;
	CTexture* m_seacloudTexture;
	D3DXVECTOR2 m_seacloudPos;
	DWORD m_fogColor;
	CSkybox* m_skybox;
	int m_cullObjCount;
	int m_cullSfxCount;
	CObject* m_objCull[MAX_DISPLAYOBJ];
	CObject* m_sfxCull[MAX_DISPLAYSFX];
	Continent* m_continent;
	CObjectArray m_objects;
	CPtrArray<Continent> m_continents;
	CPathArray m_paths;
	objid m_nextObjectID;

	void _initialize();
	bool _loadWldFile(const string& filename);
	bool _loadTxtTxtFile(const string& filename);
	bool _loadRgnFile(const string& filename);
	bool _loadDyoFile(const string& filename);
	bool _loadCntFile(const string& filename);
	bool _loadPatFile(const string& filename);
	bool _loadLndFile(int i, int j);
	void _cullObjects();
	void _loadLndFiles();
	void _cleanLndFiles();

	bool _saveWldFile(const string& filename);
	bool _saveTxtTxtFile(const string& filename);
	bool _saveRgnFile(const string& filename);
	bool _saveDyoFile(const string& filename);
	bool _saveCntFile(const string& filename);
	bool _savePatFile(const string& filename);
	void _setRegionTexts();

	void _applyCamera();
	void _setLight(bool sendToDevice);
	void _setFog();
	void _renderTerrain();
	void _renderWater();
	void _renderTerrainAttributes();
	void _renderGrid();
	void _renderGrids(const QRect& _rect, DWORD color, int dx = 0);
	void _renderWorldGrids(int wx, int wy, const QPoint& ptLT, const QPoint& ptRB, int dx, DWORD color);
	void _renderBB(CObject* obj, DWORD c = 0xffffffff);
	void _renderContinentLines();
	void _renderGridPoints(const QRect& _rect);
	void _renderPaths();
	void _renderPath(int index, const D3DXVECTOR3& origin);

	friend class CSkybox;
	friend class CObject;

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR

public:
	static bool InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device);
	static void DeleteStaticDeviceObjects();
	static bool RestoreStaticDeviceObjects(LPDIRECT3DDEVICE9 device);
	static void InvalidateStaticDeviceObjects();

	static CFont* s_objNameFont;
	static CPtrArray<CObject> s_selection;

private:
	static LPDIRECT3DINDEXBUFFER9 s_IB;
	static LPDIRECT3DINDEXBUFFER9 s_gridIB;
	static LPDIRECT3DVERTEXBUFFER9 s_bbVB;
	static LightColor s_light[24];
};

#include "World.inl"

#endif // WORLD_H