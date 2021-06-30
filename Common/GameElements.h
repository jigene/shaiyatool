///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef GAMEELEMENTS_H
#define GAMEELEMENTS_H

enum EGameElementType
{
	GAMELE_ROOTFOLDER = 1001,
	GAMEELE_FOLDER,
	GAMEELE_TERRAIN,
	GAMEELE_MODEL
};

struct ModelProp;

class CFolderElement : public QStandardItem
{
public:
	CFolderElement() { }
	virtual int type() const {
		return GAMEELE_FOLDER;
	}
};

class CRootFolderElement : public CFolderElement
{
public:
	CRootFolderElement() { setSelectable(false); }
	virtual int type() const {
		return GAMELE_ROOTFOLDER;
	}
};

class CModelElement : public QStandardItem
{
public:
	CModelElement() { }
	virtual int type() const {
		return GAMEELE_MODEL;
	}

	void SetModel(ModelProp* prop) {
		QVariant var(QVariant::Type::ULongLong);
		var.setValue<qulonglong>((qulonglong)prop);
		setData(var);
	}

	ModelProp* GetModel() const {
		return (ModelProp*)data().toULongLong();
	}
};

class CTerrainElement : public QStandardItem
{
public:
	CTerrainElement() { }
	virtual int type() const {
		return GAMEELE_TERRAIN;
	}

	void SetTerrain(int id) {
		QVariant var(QVariant::Type::Int);
		var.setValue<int>(id);
		setData(var);
	}

	int GetTerrain() const {
		return data().toInt();
	}
};

#endif // GAMEELEMENTS_H