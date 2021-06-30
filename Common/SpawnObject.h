///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SPAWNOBJECT_H
#define SPAWNOBJECT_H

#include "Object.h"

enum EAIState
{
	STATE_INIT = 1,
	STATE_IDLE,
	STATE_WANDER,
	STATE_PURSUE,
	STATE_EVADE,
	STATE_RUNAWAY,
	STATE_RAGE,
	STATE_STAND,
	STATE_PATROL,
	STATE_RAGE_PATROL
};

class CSpawnObject : public CObject
{
public:
	CSpawnObject();

	void ReadRespawn(CTextFile* file);
	void WriteRespawn(QTextStream& file);

	bool IsRespawn() const {
		return m_isRespawn;
	}
	virtual DWORD GetRectColor() {
		return 0xffffff00; // item
	}
	const QRect& GetRect() const {
		return m_rect;
	}

	void SetRect(const QRect& rect) {
		m_rect = rect;
	}

protected:
	bool m_isRespawn;
	int m_count;
	int m_time;
	int m_attackCount;
	QRect m_rect;
	int m_dayMin;
	int m_dayMax;
	int m_hourMin;
	int m_hourMax;
	int m_itemMin;
	int m_itemMax;
	int m_aiState;
	int m_patrolIndex;
	int m_patrolCycle;
	CSpawnObject* m_owner;

	friend class CWorld;

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR
};

#endif // SPAWNOBJECT_H