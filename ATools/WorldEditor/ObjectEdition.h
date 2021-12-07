///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef OBJECTEDITION_H
#define OBJECTEDITION_H

#include "EditCommand.h"

class CObject;
class CObjectTransformCommand;

class CObjectHideCommand : public CEditCommand
{
public:
	CObjectHideCommand(CWorld* world)
		: CEditCommand(world), m_hide(false) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_objects.empty();
	}

	void Hide();
	void ShowAll();
	void HideUpstair();

private:
	std::vector<objid> m_objects;
	bool m_hide;
};

class CObjectDeleteCommand : public CEditCommand
{
public:
	CObjectDeleteCommand(CWorld* world)
		: CEditCommand(world), m_create(false), m_deleteAll(false), m_pathID(-1) {}

	virtual ~CObjectDeleteCommand();
	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_objects.GetSize() <= 0;
	}

	void DeleteSelection();
	void DeleteAll();
	void AddCreateObject(CObject* obj);

private:
	CPtrArray<CObject> m_objects;
	bool m_create;
	bool m_deleteAll;
	int m_pathID;

	friend class CObjectTransformCommand;
};

class CObjectTransformCommand : public CEditCommand
{
public:
	CObjectTransformCommand(CWorld* world)
		: CEditCommand(world) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_objects.empty();
	}

	void SetRotate(CObject* obj, const D3DXVECTOR3& rot);
	void SetTranslate(CObject* obj, const D3DXVECTOR3& pos);
	void SetScale(CObject* obj, const D3DXVECTOR3& scale);

	void EditRotate(int axis, float factor);
	void EditScale(int axis, float factor);
	void EditTranslate(int axis, const D3DXVECTOR3& globalMove, float factor);

	void SetOnGrid();

	void Apply(CObjectDeleteCommand* command);

private:
	struct ObjectEntry
	{
		objid objID;
		D3DXVECTOR3 originalScale;
		D3DXVECTOR3 scale;
		D3DXVECTOR3 originalPos;
		D3DXVECTOR3 pos;
		D3DXVECTOR3 tempPos;
		D3DXVECTOR3 originalRot;
		D3DXVECTOR3 rot;
	};
	std::vector<ObjectEntry> m_objects;

	ObjectEntry* _getEntry(CObject* obj);
};

class CObjectCreateCommand : public CEditCommand
{
public:
	CObjectCreateCommand(CWorld* world)
		: CEditCommand(world) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_objects.empty();
	}

	void AddCreateObject(CObject* obj, int pathID = -1);

private:
	struct ObjectEntry
	{
		objid objID;
		uint type;
		uint modelID;
		D3DXVECTOR3 pos;
		float rot;
		float scale;
		QRect rect;
		int pathID;
	};
	std::vector<ObjectEntry> m_objects;
};

class CObjectEditRectCommand : public CEditCommand
{
public:
	CObjectEditRectCommand(CWorld* world, CObject* obj, byte editVertex);

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return !m_objID || m_origin == m_rect;
	}

	void Edit(const D3DXVECTOR3& pos);

private:
	objid m_objID;
	QRect m_origin;
	QRect m_rect;
	byte m_editVertex;
};

class CPathDeleteCommand : public CEditCommand
{
public:
	CPathDeleteCommand(CWorld* world)
		: CEditCommand(world), m_ID(-1), m_create(false) { }
	virtual ~CPathDeleteCommand();

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_ID == -1;
	}

	void CreatePath();
	void RemovePath(int ID);

private:
	bool m_create;
	int m_ID;
	CPtrArray<CObject> m_objects;
};

#endif // OBJECTEDITION_H