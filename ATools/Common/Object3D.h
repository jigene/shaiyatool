///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef OBJECT3D_H
#define OBJECT3D_H

#define VER_MESH	20
#define MAX_BONES	28
#define	MAX_GROUP	3

#define XE_NONE				0
#define XE_REFLECT			0x1
#define XE_OPACITY			0x2
#define XE_2SIDE			0x4
#define XE_SELF_ILLUMINATE	0x8
#define XE_HIGHLIGHT_OBJ	0x00002000

struct MotionAttribute;
struct TMAnimation;
class CMotion;

struct SkinVertex
{
	enum { FVF = D3DFVF_XYZB3 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1 };
	D3DXVECTOR3 p;
	float w1, w2;
	ushort id1, id2;
	D3DXVECTOR3 n;
	D3DXVECTOR2 t;
};

struct NormalVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 };
	D3DXVECTOR3 p;
	D3DXVECTOR3 n;
	D3DXVECTOR2 t;
};

struct CollisionVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
	D3DXVECTOR3 p;
	DWORD c;
};

struct Material
{
	CTexture* textures[8];
	D3DMATERIAL9 material;
	char textureName[256];
};

struct MaterialBlock
{
	int startVertex;
	int primitiveCount;
	int materialID;
	uint effect;
	int amount;
	int usedBoneCount;
	int usedBones[MAX_BONES];
};

enum EGMType
{
	GMT_ERROR = -1,
	GMT_NORMAL,
	GMT_SKIN,
	GMT_BONE
};

struct GMObject
{
	int ID;
	EGMType type;
	int usedBoneCount;
	int usedBones[MAX_BONES];
	int parentID;
	EGMType parentType;
	D3DXMATRIX transform;
	Bounds bounds;
	bool opacity, bump, rigid;
	int vertexCount, vertexListCount, faceListCount, indexCount;
	D3DXVECTOR3* vertexList;
	void* vertices;
	ushort* indices, *IIB;
	int* physiqueVertices;
	bool material;
	int materialCount;
	Material* materials;
	int materialBlockCount;
	MaterialBlock* materialBlocks;
	TMAnimation* frames;
	bool light;
	LPDIRECT3DVERTEXBUFFER9 VB;
	LPDIRECT3DINDEXBUFFER9 IB;
};

struct LODGroup
{
	int objectCount;
	GMObject* objects;
	D3DXMATRIX* updates;
	int currentTextureEx;
};

class CObject3D
{
public:
	CObject3D(LPDIRECT3DDEVICE9 device);
	~CObject3D();

	bool Load(const string& filename);
	bool Save(const string& filename);

	void ReadGMObject(CFile& file, GMObject& obj);
	void SaveGMObject(CFile& file, GMObject& obj);

	bool InitDeviceObjects();

	int GetFrameCount() const;
	MotionAttribute* GetAttributes() const;
	const Bounds& GetBounds() const;

	void Render(const D3DXMATRIX* world, int lod, float currentFrame, int nextFrame, int textureEx, int alpha = 255);
	void RenderCollision(const D3DXMATRIX* world);

	bool Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist);

	void SetExternBones(D3DXMATRIX* bones, D3DXMATRIX* invBones)
	{
		if (!m_baseBones)
		{
			m_externBones = bones;
			m_externInvBones = invBones;
		}
	}

	bool HasSkin() const {
		return m_hasSkin;
	}

private:
	LPDIRECT3DDEVICE9 m_device;
	int m_ID;
	D3DXVECTOR3 m_forces[4];
	float m_scrlU, m_scrlV;
	Bounds m_bounds;
	float m_perSlerp;
	int m_frameCount;
	int m_eventCount;
	D3DXVECTOR3 m_events[8];
	GMObject m_collObj;
	bool m_LOD;
	int m_boneCount;
	LODGroup m_groups[MAX_GROUP];
	MotionAttribute* m_attributes;
	D3DXMATRIX* m_baseBones;
	D3DXMATRIX* m_baseInvBones;
	D3DXMATRIX* m_externBones;
	D3DXMATRIX* m_externInvBones;
	bool m_sendVS;
	CMotion* m_motion;
	LODGroup* m_group;
	bool m_hasSkin;

private:
	void _setTextureEx(int textureEx);
	void _animate(float currentFrame, int nextFrame);
	void _renderSkin(const GMObject& obj, const D3DXMATRIX* world, int alpha);
	void _renderNormal(const GMObject& obj, const D3DXMATRIX* world, int alpha);
	void _setState(const MaterialBlock& block, int alpha);
	void _resetState(const MaterialBlock& block, int alpha);

public:
	static bool InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device);
	static void DeleteStaticDeviceObjects();

private:
	static LPDIRECT3DVERTEXSHADER9 s_skinVS;
	static LPDIRECT3DVERTEXDECLARATION9 s_skinVertexDeclaration;
	static CTexture s_reflectTexture;

#ifdef MODEL_EDITOR
	MODEL_EDITOR_FRIENDS
#endif // MODEL_EDITOR
};

#endif // OBJECT3D_H