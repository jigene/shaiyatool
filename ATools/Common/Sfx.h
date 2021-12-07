///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SFX_H
#define SFX_H

#define MAX_POINTS_SFX_CUSTOMMESH	180

class CSfxModel;
class CMesh;

struct SfxVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_TEX1 };
	D3DXVECTOR3 p;
	D3DXVECTOR2 t;
};

enum class ESfxPartType : int
{
	Bill = 1,
	Particle = 2,
	Mesh = 3,
	CustomMesh = 4,
};

enum class ESfxPartBillType : int
{
	Bill = 1,
	Bottom = 2,
	Pole = 3,
	Normal = 4,
};

enum class ESfxPartAlphaType : int
{
	Blend = 1,
	Glow = 2,
};

struct SfxKeyFrame
{
	ushort frame;
	D3DXVECTOR3 pos;
	D3DXVECTOR3 posRotate;
	D3DXVECTOR3 scale;
	D3DXVECTOR3 rotate;
	int alpha;
};

class CSfxPart
{
public:
	CSfxPart(LPDIRECT3DDEVICE9 device);
	virtual ~CSfxPart();

	virtual void Load(CFile& file, int version) = 0;
	virtual void Save(CFile& file) = 0;
	virtual ESfxPartType GetType() const = 0;
	virtual void Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale) = 0;

	SfxKeyFrame* AddKey(ushort frame);
	SfxKeyFrame* GetKey(ushort frame) const;
	SfxKeyFrame* GetFirstKey() const;
	SfxKeyFrame* GetLastKey() const;
	SfxKeyFrame* GetPrevKey(ushort frame) const;
	SfxKeyFrame* GetNextKey(ushort frame, bool skip = true) const;
	bool GetKey(ushort frame, SfxKeyFrame& key) const;
	void RemoveKey(ushort frame);
	void RemoveAllKeys();

	void RenderSelectionRectangle(ushort frame);

	bool IsVisible() const { return m_visible; }

protected:
	LPDIRECT3DDEVICE9 m_device;
	string m_name;
	string m_textureName;
	ESfxPartBillType m_billType;
	ESfxPartAlphaType m_alphaType;
	bool m_visible;
	ushort m_texFrame;
	ushort m_texLoop;
	CPtrArray<SfxKeyFrame> m_keys;
	CTexture* m_texture;
	CTexture** m_textures;

	virtual void _setTexture();

#ifdef SFX_EDITOR
	SFX_EDITOR_FRIENDS
#endif // SFX_EDITOR
};

class CSfxPartBill : public CSfxPart
{
public:
	CSfxPartBill(LPDIRECT3DDEVICE9 device);

	virtual void Load(CFile& file, int version);
	virtual void Save(CFile& file);
	virtual void Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale);

	virtual ESfxPartType GetType() const 
	{
		return ESfxPartType::Bill;
	}
};

class CSfxPartParticle : public CSfxPart
{
public:
	CSfxPartParticle(LPDIRECT3DDEVICE9 device);

	virtual void Load(CFile& file, int version);
	virtual void Save(CFile& file);
	virtual void Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale);

	virtual ESfxPartType GetType() const
	{
		return ESfxPartType::Particle;
	}

private:
	ushort m_particleCreate;
	ushort m_particleCreateNum;
	ushort m_particleFrameAppear;
	ushort m_particleFrameKeep;
	ushort m_particleFrameDisappear;
	float m_particleStartPosVar;
	float m_particleStartPosVarY;
	float m_particleYLow;
	float m_particleYHigh;
	float m_particleXZLow;
	float m_particleXZHigh;
	D3DXVECTOR3 m_particleAccel;
	D3DXVECTOR3 m_scale;
	D3DXVECTOR3 m_scaleSpeed;
	D3DXVECTOR3 m_rotation;
	D3DXVECTOR3 m_rotationLow;
	D3DXVECTOR3 m_rotationHigh;
	bool m_repeatScal;
	float m_scalSpeedXLow;
	float m_scalSpeedXHigh;
	float m_scalSpeedYLow;
	float m_scalSpeedYHigh;
	float m_scalSpeedZLow;
	float m_scalSpeedZHigh;
	D3DXVECTOR3 m_scaleSpeed2;
	D3DXVECTOR3 m_scaleEnd;
	bool m_repeat;

	friend class CSfxModel;

#ifdef SFX_EDITOR
	SFX_EDITOR_FRIENDS
#endif // SFX_EDITOR
};

class CSfxPartMesh : public CSfxPart
{
public:
	CSfxPartMesh(LPDIRECT3DDEVICE9 device);

	virtual void Load(CFile& file, int version);
	virtual void Save(CFile& file);
	virtual void Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale);

	virtual ESfxPartType GetType() const
	{
		return ESfxPartType::Mesh;
	}

protected:
	CMesh* m_mesh;

	virtual void _setTexture();
};

class CSfxPartCustomMesh : public CSfxPart
{
public:
	CSfxPartCustomMesh(LPDIRECT3DDEVICE9 device);

	virtual void Load(CFile& file, int version);
	virtual void Save(CFile& file);
	virtual void Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale);

	virtual ESfxPartType GetType() const
	{
		return ESfxPartType::CustomMesh;
	}

private:
	int m_pointCount;

#ifdef SFX_EDITOR
	SFX_EDITOR_FRIENDS
#endif // SFX_EDITOR
};

class CSfx
{
public:
	CSfx(LPDIRECT3DDEVICE9 device);
	~CSfx();

	bool Load(const string& filename);
	bool Save(const string& filename);

	CSfxPart* AddPart(ESfxPartType type);
	void RemovePart(int i);

private:
	LPDIRECT3DDEVICE9 m_device;
	CPtrArray<CSfxPart> m_parts;

	friend class CSfxModel;

#ifdef SFX_EDITOR
	SFX_EDITOR_FRIENDS
#endif // SFX_EDITOR
};

#endif // SFX_H