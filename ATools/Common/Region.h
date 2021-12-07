///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef REGION_H
#define REGION_H

#include "Object.h"

class CRegion : public CObject
{
public:
	CRegion();

	void ReadRegion(CTextFile* file);
	void WriteRegion(QTextStream& file);

	virtual DWORD GetRectColor() {
		switch (m_modelID)
		{
		case RI_TRIGGER:
			return 0xff0a0af7;
		case RI_ATTRIBUTE:
			return 0xff03fb04;
		case RI_REVIVAL:
			return m_targetKey ? 0xffa0ff40 : 0;
		default:
			return 0;
		}
	}
	const QRect& GetRect() const {
		return m_rect;
	}

	void SetRect(const QRect& rect) {
		m_rect = rect;
	}

protected:
	uint m_attributes;
	int m_musicID;
	bool m_directMusic;
	string m_script;
	string m_sound;
	int m_teleportWorldID;
	D3DXVECTOR3 m_teleportWorldPos;
	QRect m_rect;
	string m_key;
	bool m_targetKey;
	int m_itemID;
	int m_itemCount;
	int m_minLevel;
	int m_maxLevel;
	int m_questID;
	int m_questFlag;
	int m_jobID;
	int m_gender;
	bool m_checkParty;
	bool m_checkGuild;
	bool m_chaoKey;
	string m_titleDefine;
	string m_title;
	string m_descDefine;
	string m_desc;

	friend class CWorld;

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR
};

#endif // REGION_H