///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef OBJECT_H
#define OBJECT_H

struct ModelProp;
class CModel;
class CWorld;
class CObject;

enum FD_OBJ_ATTR
{
	FOA_Invisible_Collision = 0x00010000,
	FOA_Invisible = 0x00020000,
};

bool ObjSortFarToNear(const CObject*, const CObject*);

class CObject
{
public:
	static CObject* CreateObject(CFile& file);
	static CObject* CreateObject(uint type, CObject* clone = null);

	static string GetModelFilename(ModelProp* prop);

public:
	CObject();
	virtual ~CObject();

	virtual void Read(CFile& file);
	virtual void Write(CFile& file, const D3DXVECTOR3& posOffset);
	virtual bool Init();
	virtual void RenderName();

	virtual DWORD GetRectColor() {
		return 0x00000000;
	}

	void Render();
	void Cull();
	bool Pick(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist);

	void SetPos(const D3DXVECTOR3& pos);
	void SetRot(const D3DXVECTOR3& rot);
	void SetScale(const D3DXVECTOR3& scale);
	virtual void ResetScale();

	void SetType(uint type) {
		m_type = type;
	}
	void SetUnvisible(bool unvisible) {
		m_isUnvisible = unvisible;
	}
	void SetID(objid id) {
		m_ID = id;
	}
	void SetModelID(uint id) {
		m_modelID = id;
	}

	objid GetID() const {
		return m_ID;
	}
	uint GetType() const {
		return m_type;
	}
	const D3DXVECTOR3& GetPos() const {
		return m_pos;
	}
	const D3DXVECTOR3& GetRot() const {
		return m_rot;
	}
	const D3DXVECTOR3& GetScale() const {
		return m_scale;
	}
	bool IsReal() const {
		return m_isReal;
	}
	string GetModelFilename() const {
		return GetModelFilename(m_modelProp);
	}
	bool IsUnvisible() const {
		return m_isUnvisible;
	}
	uint GetModelID() const {
		return m_modelID;
	}

protected:
	ulong m_ID;
	uint m_type;
	D3DXVECTOR3 m_rot;
	D3DXVECTOR3 m_pos;
	D3DXVECTOR3 m_scale;
	uint m_modelID;
	ModelProp* m_modelProp;
	CModel* m_model;
	bool m_updateMatrix;
	bool m_visible;
	D3DXVECTOR3 m_bounds[8];
	D3DXMATRIX m_TM;
	CWorld* m_world;
	float m_distToCamera;
	bool m_isReal;
	bool m_isUnvisible;

	void _updateMatrix();
	virtual void _loadModel();

	friend class CWorld;
	friend bool ObjSortFarToNear(const CObject*, const CObject*);

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR
};

#endif // OBJECT_H