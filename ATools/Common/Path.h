///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef PATH_H
#define PATH_H

#include "Object.h"

class CPath : public CObject
{
public:
	CPath();

	virtual bool Init();
	virtual void ResetScale();

	void SetIndex(int index) {
		m_index = index;
	}

private:
	int m_index;
	int m_motion;
	int m_delay;

	friend class CWorld;

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR
};

#endif // PATH_H