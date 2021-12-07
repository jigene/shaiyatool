///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "SpawnObject.h"
#include "TextFile.h"
#include "Ctrl.h"

CSpawnObject::CSpawnObject()
	: m_isRespawn(false),
	m_owner(null),
	m_count(1),
	m_time(120),
	m_attackCount(0),
	m_dayMin(1),
	m_dayMax(30),
	m_hourMin(1),
	m_hourMax(24),
	m_itemMin(1),
	m_itemMax(1),
	m_aiState(STATE_IDLE),
	m_patrolIndex(-1),
	m_patrolCycle(0)
{
}

void CSpawnObject::ReadRespawn(CTextFile* file)
{
	m_isRespawn = true;

	m_modelID = file->GetInt();
	D3DXVECTOR3 pos;
	pos.x = file->GetFloat();
	pos.y = file->GetFloat();
	pos.z = file->GetFloat();
	SetPos(pos);
	m_count = file->GetInt();
	m_time = file->GetInt();
	m_attackCount = file->GetInt();
	m_rect = QRect(QPoint(file->GetInt(), file->GetInt()), QPoint(file->GetInt(), file->GetInt()));
	m_dayMin = file->GetInt();
	m_dayMax = file->GetInt();
	m_hourMin = file->GetInt();
	m_hourMax = file->GetInt();
	m_itemMin = file->GetInt();
	m_itemMax = file->GetInt();
	m_aiState = file->GetInt();
	const float angle = file->GetFloat();
	SetRot(D3DXVECTOR3(0.0f, angle, 0.0f));
	m_patrolIndex = file->GetInt();
	m_patrolCycle = file->GetInt();
	const int control = file->GetInt();

	if (control)
	{
		CtrlElem elem;
		memset(&elem, 0, sizeof(elem));

		int i;
		elem.dwSet = file->GetInt();
		elem.dwSetItem = file->GetInt();
		if (control == 2)
			elem.dwSetItemCount = file->GetInt();
		else
			elem.dwSetItemCount = 1;
		elem.dwSetLevel = file->GetInt();
		elem.dwSetQuestNum = file->GetInt();
		elem.dwSetFlagNum = file->GetInt();
		if (control == 2)
		{
			elem.dwSetQuestNum1 = file->GetInt();
			elem.dwSetFlagNum1 = file->GetInt();
			elem.dwSetQuestNum2 = file->GetInt();
			elem.dwSetFlagNum2 = file->GetInt();
		}
		elem.dwSetGender = file->GetInt();
		for (i = 0; i < MAX_JOB; i++)
			elem.bSetJob[i] = file->GetInt();
		elem.dwSetEndu = file->GetInt();
		elem.dwMinItemNum = file->GetInt();
		elem.dwMaxiItemNum = file->GetInt();
		for (i = 0; i < MAX_CTRLDROPITEM; i++)
			elem.dwInsideItemKind[i] = file->GetInt();
		for (i = 0; i < MAX_CTRLDROPITEM; i++)
			elem.dwInsideItemPer[i] = file->GetInt();
		for (i = 0; i < MAX_CTRLDROPMOB; i++)
			elem.dwMonResKind[i] = file->GetInt();
		for (i = 0; i < MAX_CTRLDROPMOB; i++)
			elem.dwMonResNum[i] = file->GetInt();
		for (i = 0; i < MAX_CTRLDROPMOB; i++)
			elem.dwMonActAttack[i] = file->GetInt();
		elem.dwTrapOperType = file->GetInt();
		elem.dwTrapRandomPer = file->GetInt();
		elem.dwTrapDelay = file->GetInt();
		for (i = 0; i < MAX_TRAP; i++)
			elem.dwTrapKind[i] = file->GetInt();
		for (i = 0; i < MAX_TRAP; i++)
			elem.dwTrapLevel[i] = file->GetInt();
		if (control == 2)
		{
			elem.dwTeleWorldId = file->GetInt();
			elem.dwTeleX = file->GetInt();
			elem.dwTeleY = file->GetInt();
			elem.dwTeleZ = file->GetInt();
		}

		if (m_type == OT_CTRL)
		{
			CCtrl* ctrl = (CCtrl*)this;
			memcpy(&ctrl->m_elem, &elem, sizeof(elem));
		}
	}
}

void CSpawnObject::WriteRespawn(QTextStream& file)
{
	const QRect rect = m_rect.normalized();

	file << "respawn7 "
			<< m_type << ' '
			<< m_modelID << ' '
			<< m_pos.x << ' '
			<< m_pos.y << ' '
			<< m_pos.z << ' '
			<< m_count << ' '
			<< m_time << ' '
			<< m_attackCount << ' '
			<< rect.top() << ' '
			<< rect.left() << ' '
			<< rect.bottom() << ' '
			<< rect.right() << ' '
			<< m_dayMin << ' '
			<< m_dayMax << ' '
			<< m_hourMin << ' '
			<< m_hourMax << ' '
			<< m_itemMin << ' '
			<< m_itemMax << ' '
			<< m_aiState << ' '
			<< m_rot.y << ' '
			<< m_patrolIndex << ' '
			<< m_patrolCycle << ' ';

	if (m_type == OT_CTRL)
	{
		CCtrl* ctrl = (CCtrl*)this;
		CtrlElem& elem = ctrl->m_elem;
		int i;

		file << 2 << ' '
			<< elem.dwSet << ' '
			<< elem.dwSetItem << ' '
			<< elem.dwSetItemCount << ' '
			<< elem.dwSetLevel << ' '
			<< elem.dwSetQuestNum << ' '
			<< elem.dwSetFlagNum << ' '
			<< elem.dwSetQuestNum1 << ' '
			<< elem.dwSetFlagNum1 << ' '
			<< elem.dwSetQuestNum2 << ' '
			<< elem.dwSetFlagNum2 << ' '
			<< elem.dwSetGender << ' ';
		for (i = 0; i < MAX_JOB; i++)
			file << elem.bSetJob[i] << ' ';
		file << elem.dwSetEndu << ' '
			<< elem.dwMinItemNum << ' '
			<< elem.dwMaxiItemNum << ' ';
		for (i = 0; i < MAX_CTRLDROPITEM; i++)
			file << elem.dwInsideItemKind[i] << ' ';
		for (i = 0; i < MAX_CTRLDROPITEM; i++)
			file << elem.dwInsideItemPer[i] << ' ';
		for (i = 0; i < MAX_CTRLDROPMOB; i++)
			file << elem.dwMonResKind[i] << ' ';
		for (i = 0; i < MAX_CTRLDROPMOB; i++)
			file << elem.dwMonResNum[i] << ' ';
		for (i = 0; i < MAX_CTRLDROPMOB; i++)
			file << elem.dwMonActAttack[i] << ' ';
		file << elem.dwTrapOperType << ' '
			<< elem.dwTrapRandomPer << ' '
			<< elem.dwTrapDelay << ' ';
		for (i = 0; i < MAX_TRAP; i++)
			file << elem.dwTrapKind[i] << ' ';
		for (i = 0; i < MAX_TRAP; i++)
			file << elem.dwTrapLevel[i] << ' ';
		file << elem.dwTeleWorldId << ' '
			<< elem.dwTeleX << ' '
			<< elem.dwTeleY << ' '
			<< elem.dwTeleZ << endl;
	}
	else
		file << 0 << endl;
}