///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

struct ControlData
{
	string define;
	int ID;
	int type;
	string texture;
	bool tiles;
	QRect rect;
	int flags;
	bool visible;
	bool disabled;
	bool tabStop;
	bool group;
	string text;
	string textID;
	string tooltip;
	string tooltipID;
#if __VER >= 19
	QColor color;
#endif
};

struct WindowData
{
	string define;
	int ID;
	string texture;
	bool tiles;
	QSize size;
	int flags;
	D3DFORMAT format;
	string title;
	string titleID;
	string tooltip;
	string tooltipID;
	CPtrArray<ControlData> controls;
#if __VER >= 19
	string icon;
#endif

	ControlData* GetControl(int ID) const
	{
		for (int i = 0; i < controls.GetSize(); i++)
			if (controls[i]->ID == ID)
				return controls[i];
		return null;
	}
};

class CDataManager
{
public:
	static CDataManager* Instance;

	CDataManager(QComboBox* controlList);
	~CDataManager();

	bool Load();
	bool Save();

	void FillWindowList(QStringListModel* list);
	void FillControlList();

	WindowData* GetWindow(int ID) const;
	WindowData* GetWindow(const string& ID) const;

	int GetWindowID(const string& ID) const;

	int GetNewWindowID() const;
	bool WindowExists(const string& ID);
	void AddNewWindow(const string& name, int ID);

	string GetNewText(const string& text);
	void RemoveText(const string& ID);
	void SetText(const string& ID, const string& text);

	void RemoveWindow(int ID);
	void RemoveControl(int windowID, int controlID);

	int GetControlID(const string& ID);

private:
	QMap<int, WindowData*> m_windows;
	QMap<string, int> m_windowIDs;
	QMap<string, int> m_newWindowIDs;
	QMap<string, int> m_controlIDs;
	QMap<string, string> m_texts;
	QMap<string, string> m_newTexts;
	QComboBox* m_controlList;

	bool _loadScript(const string& filename);
	bool _loadDefine(const string& filename);
	bool _loadText(const string& filename);

	bool _saveScript(const string& filename);
	bool _saveDefine(const string& filename);
	bool _saveText(const string& filename);
};

#define DataMng CDataManager::Instance

#endif // DATAMANAGER_H