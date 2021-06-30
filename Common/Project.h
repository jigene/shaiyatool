///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef PROJECT_H
#define PROJECT_H

class CRootFolderElement;

#define MAX_HUMAN_PARTS 31

struct WaterList
{
	int textureCount;
	int* textureIDs;
	CTexture** textures;
	float frame;
	string name;
};

struct ModelProp
{
	byte type;
	uint ID;
	string filename;
	byte modelType;
	string part;
	bool fly;
	byte distant;
	bool pick;
	float scale;
	bool trans;
	bool shadow;
	byte textureEx;
	bool renderFlag;

	int motionCount;
	char* motions;

	char* GetMotion(int i) const
	{
		if (i < 0 || i >= motionCount)
			i = 0;
		if (motions)
			return &motions[i * 32];
		return null;
	}
};

struct MoverProp
{
	string name;
	uint AI;
	uint belligerence;
	bool fly;
};

struct ItemProp
{
	string name;
	int part;
};

struct Character
{
	string ID;
	string name;
	uint moverID;
	int hair;
	uint hairColor;
	int head;
	uint parts[MAX_HUMAN_PARTS];
	int partCount;
};

extern int g_waterCount;

class CProject
{
public:
	static CProject* Instance;

	CProject();
	~CProject();

	bool Load(const string& filename);

	void FillTreeView(QTreeView* treeView);
	void FillTerrainComboBox(QComboBox* comboBox);
	void FillWaterComboBox(QComboBox* comboBox);
	void FillCharacterComboBox(QComboBox* comboBox, const string& current);

	CTexture* GetTerrain(int terrainID) const;
	string GetTerrainFilename(int terrainID) const;
	void MoveWaterFrame();
	CTexture* GetWaterTexture(int index);

	ModelProp* GetModelProp(uint type, uint index);
	MoverProp* GetMoverProp(uint index);
	Character* GetCharacter(const string& key);
	ItemProp* GetItemProp(uint index);

	QStandardItem* CreateFavorite(QDataStream& data);

private:
	string m_name;
	QMap<int, string> m_terrains;
	WaterList* m_waterLists;
	int m_waterListCount;
	QMap<int, ModelProp> m_models[MAX_OBJTYPE];
	QMap<int, MoverProp> m_moverProps;
	QMap<int, ItemProp> m_itemProps;
	QMap<string, Character> m_characters;

	QStandardItemModel m_gameElements;
	CRootFolderElement* m_objFolder;
	CRootFolderElement* m_terrainFolder;
	QIcon m_icons[6];

	bool _loadModel(const string& filename);
	bool _loadTerrain(const string& filename);
	bool _loadPropMover(const string& filename);
	bool _loadPropItem(const string& filename);
	bool _loadCharacter(const string& filename);
};

#define Project	CProject::Instance

#endif // PROJECT_H