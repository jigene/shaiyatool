///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Mover.h"
#include "Project.h"
#include "ModelMng.h"
#include "Font.h"
#include "World.h"
#include "AnimatedMesh.h"

CMover::CMover()
	: m_moverProp(null),
	m_character(null),
	m_motion(4294967295),
	m_AIInterface(AII_NONE),
	m_belligerence(BELLI_PEACEFUL)
{
}

void CMover::Read(CFile& file)
{
	CObject::Read(file);

	char buffer[64];
	file.Read(buffer, 64);
	m_name = buffer;
	file.Skip(32);
	file.Read(buffer, 32);
	m_characterKey = buffer;
	file.Read(m_belligerence);
	file.Skip(4);
}

void CMover::Write(CFile& file, const D3DXVECTOR3& posOffset)
{
	CObject::Write(file, posOffset);

	int i;
	const QByteArray name = m_name.normalized(QString::NormalizationForm_D).replace(QRegExp("[^a-zA-Z\\s_]"), "").toLocal8Bit();
	if (name.size() > 0)
		for (i = 0; i < name.size() && i < 63; i++)
			file.Write(name.at(i));
	if (name.size() < 63)
		for (i = 0; i < 63 - name.size(); i++)
			file.Write('\0');
	file.Write('\0');

	char unused[32];
	memset(unused, 0, 32);
	file.Write(unused, 32);

	const QByteArray characterKey = (m_character ? m_character->ID : "").toLocal8Bit();
	if (characterKey.size() > 0)
		for (i = 0; i < characterKey.size() && i < 31; i++)
			file.Write(characterKey.at(i));
	if (characterKey.size() < 31)
		for (int i = 0; i < 31 - characterKey.size(); i++)
			file.Write('\0');
	file.Write('\0');

	file.Write(m_belligerence);
	file.Write((uint)0);
}

bool CMover::Init()
{
	if (!CObject::Init())
		return false;

	m_moverProp = Project->GetMoverProp(m_modelID);
	if (!m_moverProp)
		return false;

	if (!m_characterKey.isEmpty())
		m_character = Project->GetCharacter(m_characterKey);

	ResetScale();
	return true;
}

void CMover::InitProperties()
{
	if (!m_moverProp)
		return;

	m_belligerence = m_moverProp->belligerence;
	m_AIInterface = m_moverProp->AI;
	m_name = m_moverProp->name;
}

void CMover::RenderName()
{
	if (!m_model)
		return;

	D3DXVECTOR3 out;
	D3DXVec3Project(&out, &D3DXVECTOR3(0.0f, m_model->GetBounds().Max.y, 0.0f), &g_global3D.viewport, &g_global3D.proj, &g_global3D.view, &m_TM);

	const string& name = m_character ? m_character->name : m_moverProp->name;

	CWorld::s_objNameFont->Render(name,
		QPoint(((int)out.x) - CWorld::s_objNameFont->GetSize(name).width() / 2 - 3, ((int)out.y) - 20),
		IsPeaceful() ? 0xffa0a0ff : 0xffffffa0);
}

void CMover::_loadModel()
{
	CObject::_loadModel();

	if (m_character && (m_character->moverID == MI_MALE || m_character->moverID == MI_FEMALE))
	{
		const int sex = m_character->moverID == MI_MALE ? SEX_MALE : SEX_FEMALE;

		CAnimatedMesh* mesh = (CAnimatedMesh*)m_model;
		for (int i = 0; i < m_character->partCount; i++)
		{
			ItemProp* item = Project->GetItemProp(m_character->parts[i]);
			if (item)
			{
				ModelProp* model = Project->GetModelProp(OT_ITEM, m_character->parts[i]);
				if (model && !model->part.isEmpty())
				{
					const int indexOfSlash = model->part.indexOf('/');
					if (indexOfSlash != -1)
					{
						if (m_character->moverID == MI_MALE)
							mesh->Load("part_" + model->part.left(indexOfSlash) + ".o3d", item->part);
						else if (m_character->moverID == MI_FEMALE)
							mesh->Load("part_" + model->part.right(indexOfSlash) + ".o3d", item->part);
					}
					else
						mesh->Load("part_" + model->part + ".o3d", item->part);
				}
			}
		}

		if (m_character->hair != 0)
			mesh->Load(string().sprintf(PARTSMESH_HAIR(sex), m_character->hair), PARTS_HAIR);
		else
			mesh->Load(string().sprintf(PARTSMESH_HAIR(sex), 1), PARTS_HAIR);

		if (m_character->head != 0)
			mesh->Load(string().sprintf(PARTSMESH_HEAD(sex), m_character->head), PARTS_HEAD);
		else
			mesh->Load(string().sprintf(PARTSMESH_HAIR(sex), 1), PARTS_HEAD);
	}
}

bool CMover::IsPeaceful() const
{
	return m_moverProp->belligerence == BELLI_PEACEFUL;
}