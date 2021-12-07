///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Region.h"
#include "TextFile.h"

CRegion::CRegion()
	: m_attributes(0),
	m_musicID(0),
	m_directMusic(false),
	m_teleportWorldID(WI_WORLD_NONE),
	m_teleportWorldPos(0, 0, 0),
	m_targetKey(false),
	m_itemID(-1),
	m_itemCount(-1),
	m_minLevel(-1),
	m_maxLevel(-1),
	m_questID(-1),
	m_questFlag(-1),
	m_jobID(-1),
	m_gender(-1),
	m_checkParty(false),
	m_checkGuild(false),
	m_chaoKey(false)
{
}

void CRegion::WriteRegion(QTextStream& file)
{
	const QRect rect = m_rect.normalized();

	file << "region3 "
		<< m_type << ' '
		<< m_modelID << ' '
		<< m_pos.x << ' '
		<< m_pos.y << ' '
		<< m_pos.z << ' '
		<< "0x" << hex << m_attributes << dec << ' '
		<< m_musicID << ' '
		<< (m_directMusic ? 1 : 0) << ' '
		<< '"' << m_script << "\" "
		<< '"' << m_sound << "\" "
		<< m_teleportWorldID << ' '
		<< m_teleportWorldPos.x << ' '
		<< m_teleportWorldPos.y << ' '
		<< m_teleportWorldPos.z << ' '
		<< rect.top() << ' '
		<< rect.left() << ' '
		<< rect.bottom() << ' '
		<< rect.right() << ' '
		<< '"' << m_key << "\" "
		<< (m_targetKey ? 1 : 0) << ' '
		<< m_itemID << ' '
		<< m_itemCount << ' '
		<< m_minLevel << ' '
		<< m_maxLevel << ' '
		<< m_questID << ' '
		<< m_questFlag << ' '
		<< m_jobID << ' '
		<< m_gender << ' '
		<< (m_checkParty ? 1 : 0) << ' '
		<< (m_checkGuild ? 1 : 0) << ' '
		<< (m_chaoKey ? 1 : 0) << endl;

	const bool hasTitle = !m_title.isEmpty();

	file << "title " << (hasTitle ? 1 : 0) << endl;
	if (hasTitle)
		file << '{' << endl << m_titleDefine << endl << '}' << endl;

	const bool hasDesc = !m_desc.isEmpty();

	file << "desc " << (hasDesc ? 1 : 0) << endl;
	if (hasDesc)
		file << '{' << endl << m_descDefine << endl << '}' << endl;
}

void CRegion::ReadRegion(CTextFile* file)
{
	m_modelID = file->GetInt();
	D3DXVECTOR3 pos;
	pos.x = file->GetFloat();
	pos.y = file->GetFloat();
	pos.z = file->GetFloat();
	SetPos(pos);
	m_attributes = file->GetUInt();
	m_musicID = file->GetInt();
	m_directMusic = file->GetBool();
	m_script = file->GetString();
	m_sound = file->GetString();
	m_teleportWorldID = file->GetInt();
	m_teleportWorldPos.x = file->GetFloat();
	m_teleportWorldPos.y = file->GetFloat();
	m_teleportWorldPos.z = file->GetFloat();
	m_rect = QRect(QPoint(file->GetInt(), file->GetInt()), QPoint(file->GetInt(), file->GetInt()));
	m_key = file->GetString();
	m_targetKey = file->GetBool();
	m_itemID = file->GetInt();
	m_itemCount = file->GetInt();
	m_minLevel = file->GetInt();
	m_maxLevel = file->GetInt();
	m_questID = file->GetInt();
	m_questFlag = file->GetInt();
	m_jobID = file->GetInt();
	m_gender = file->GetInt();
	m_checkParty = file->GetBool();
	m_checkGuild = file->GetBool();
	m_chaoKey = file->GetBool();

	file->NextToken();
	const bool hasTitle = file->GetBool();
	if (hasTitle)
	{
		file->NextToken();
		m_titleDefine = file->GetString(false);
		file->NextToken();
	}

	file->NextToken();
	const bool hasDesc = file->GetBool();
	if (hasDesc)
	{
		file->NextToken();
		m_descDefine = file->GetString(false);
		file->NextToken();
	}
}