///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef CTRL_H
#define CTRL_H

#include "SpawnObject.h"

#define MAX_CTRLDROPITEM		4
#define MAX_CTRLDROPMOB			3
#define	MAX_TRAP                3
#define MAX_KEY					64

struct CtrlElem
{
	uint dwSet;
	uint dwSetItem;
	uint dwSetLevel;
	uint dwSetQuestNum;
	uint dwSetFlagNum;
	uint dwSetGender;
	int bSetJob[MAX_JOB];
	uint dwSetEndu;
	uint dwMinItemNum;
	uint dwMaxiItemNum;
	uint dwInsideItemKind[MAX_CTRLDROPITEM];
	uint dwInsideItemPer[MAX_CTRLDROPITEM];
	uint dwMonResKind[MAX_CTRLDROPMOB];
	uint dwMonResNum[MAX_CTRLDROPMOB];
	uint dwMonActAttack[MAX_CTRLDROPMOB];
	uint dwTrapOperType;
	uint dwTrapRandomPer;
	uint dwTrapDelay;
	uint dwTrapKind[MAX_TRAP];
	uint dwTrapLevel[MAX_TRAP];
	char strLinkCtrlKey[MAX_KEY];
	char strCtrlKey[MAX_KEY];
	uint dwSetQuestNum1;
	uint dwSetFlagNum1;
	uint dwSetQuestNum2;
	uint dwSetFlagNum2;
	uint dwSetItemCount;
	uint dwTeleWorldId;
	uint dwTeleX;
	uint dwTeleY;
	uint dwTeleZ;
};

class CCtrl : public CSpawnObject
{
public:
	CCtrl();

	virtual void Read(CFile& file);
	virtual void Write(CFile& file, const D3DXVECTOR3& posOffset);

	virtual DWORD GetRectColor() {
		return 0xff00ffff;
	}

protected:
	CtrlElem m_elem;

	friend class CSpawnObject;

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR
};

#endif // CTRL_H