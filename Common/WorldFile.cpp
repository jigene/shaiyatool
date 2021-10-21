///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "World.h"
#include "TextFile.h"
#include "Object.h"
#include "SpawnObject.h"
#include "Region.h"
#include "Mover.h"
#include "Project.h"
#include "Landscape.h"
#include "Path.h"

bool CWorld::_loadWldFile(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename, true, true))
		return false;

	const string keys[] =
	{
		"size",
		"ambient",
		"revival",
		"indoor",
		"fly",
		"bgm",
		"heightmap",
		"diffuse",
		"lightmap",
		"camera",
		"fogsetting",
		"mpu",
		"sky",
		"cloud",
		"sun",
		"moon",
		"seacloud"
	};

	string tok;
	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		tok = file.Token().toLower();

		if (tok == keys[0])
		{
			m_height = m_width = file.GetLong();
		}
		else if (tok == keys[1])
			m_ambient = file.GetUInt();
		else if (tok == keys[2])
		{
			m_worldRevivalID = file.GetInt();
			m_revivalPlace = file.GetString();
		}
		else if (tok == keys[3])
			m_inDoor = file.GetBool();
		else if (tok == keys[4])
			m_canFly = file.GetBool();
		else if (tok == keys[5])
			m_bgmID = file.GetInt();
		else if (tok == keys[6]) {
			//m_modePK = file.GetInt();

			heightMap = file.GetMapHeight();
			heightMapTotal = heightMapLength = file.GetInt();
		}
		else if (tok == keys[7])
			m_diffuse = file.GetUInt();
		else if (tok == keys[8])
		{
			//m_lightDir.x = file.GetFloat();
			//m_lightDir.y = file.GetFloat();
			//m_lightDir.z = file.GetFloat();

			lightMap = file.GetLightMap();
			lightMapTotal = lightMapLength = file.GetInt();
		}
		else if (tok == keys[9])
		{
			m_cameraPos.x = file.GetFloat();
			m_cameraPos.y = file.GetFloat();
			m_cameraPos.z = file.GetFloat();

			m_cameraAngle.y = -fmod(file.GetFloat(), 6.0f) * 60.0f;
			m_cameraAngle.x = fmod(file.GetFloat(), 6.0f) * 60.0f;

			if (m_cameraAngle.y > 89.9f)
			{
				m_cameraAngle.y = 180.0f - m_cameraAngle.y;
				m_cameraAngle.x += 180.0f;
			}
			if (m_cameraAngle.y < -89.9f)
			{
				m_cameraAngle.y = -(180.0f + m_cameraAngle.y);
				m_cameraAngle.x += 180.0f;
			}
		}
		else if (tok == keys[10])
		{
			m_fogStart = file.GetFloat();
			m_fogEnd = file.GetFloat();
			m_fogDensity = file.GetFloat();
		}
		else if (tok == keys[11])
			m_MPU = file.GetInt();
		else if (tok == keys[12])
		{
			for (int i = 0; i < 3; i++) {
				m_skyTextureNames[i] = file.GetString();
				file.NextToken();
			}
		}
		else if (tok == keys[13])
		{
			for (int i = 0; i < 3; i++) {
				m_skyTextureNames[i] = file.GetString();
				file.NextToken();
			}
		}
		else if (tok == keys[14])
			m_sunTextureName = file.GetString();
		else if (tok == keys[15])
			m_moonTextureName = file.GetString();
		else if (tok == keys[16]) {
			//m_seaCloudTextureName = file.GetString();

			Project->LoadWater("Weather/" + std::string(file.GetString().toUtf8().constData()));

			//for (int i = 0; i < m_height; i++)
			//{
			//	for (int j = 0; j < m_width; j++)
			//	{

			//		CLandscape land = m_lands[i * m_width + j];
			//	}
			//}
		}

		file.NextToken();
	}

	file.Close();
	return true;
}

bool CWorld::_saveWldFile(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out.setRealNumberNotation(QTextStream::FixedNotation);

	out << "// World script" << endl << endl;
	out << "size " << m_width << ", " << m_height << endl;
	out << "indoor " << (m_inDoor ? 1 : 0) << endl;
	out << "ambient 0x" << hex << m_ambient << dec << endl;
	out << "bgColor 0xffe0e0ff" << endl;
	out << "fly " << (m_canFly ? 1 : 0) << endl;
	out << "camera " << m_cameraPos.x << ' ' << m_cameraPos.y << ' ' << m_cameraPos.z
		<< ' ' << (m_cameraAngle.y / -60.0f) << ' ' << (m_cameraAngle.x / 60.0f) << endl;
	out << "revival " << m_worldRevivalID << " \"" << m_revivalPlace << '"' << endl;
	out << "diffuse 0x" << hex << m_diffuse << dec << endl;
	out << "lightDir " << m_lightDir.x << ' ' << m_lightDir.y << ' ' << m_lightDir.z << endl;
	out << "fogSetting " << m_fogStart << ' ' << m_fogEnd << ' ' << m_fogDensity << endl;
	out << "bgm " << m_bgmID << endl;
	out << "pkmode " << m_modePK << endl;
	out << "MPU " << m_MPU << endl;
	out << "sky " << m_skyTextureNames[0] << ' ' << m_skyTextureNames[1] << ' ' << m_skyTextureNames[2] << endl;
	out << "cloud " << m_cloudTextureNames[0] << ' ' << m_cloudTextureNames[1] << ' ' << m_cloudTextureNames[2] << endl;
	out << "sun " << m_sunTextureName << endl;
	out << "moon " << m_moonTextureName << endl;
	out << "seacloud " << m_seaCloudTextureName << endl;

	file.close();
	return true;
}

bool CWorld::_loadDyoFile(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	CObject* obj;
	obj = CObject::CreateObject(file);
	while (obj != null)
	{
		if (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
		{
			CSpawnObject* dyna = ((CSpawnObject*)obj);
			dyna->m_rect = QRect(
				QPoint((int)(obj->m_pos.z - (float)MPU + 0.5f), (int)(obj->m_pos.x - (float)MPU + 0.5f)),
				QSize(4 * MPU, 4 * MPU)
				);
			if (dyna->m_rect.left() < 0)
				dyna->m_rect.setLeft(0);
			else if (dyna->m_rect.left() >= m_height * MAP_SIZE * MPU)
				dyna->m_rect.setLeft(m_height * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.right() < 0)
				dyna->m_rect.setRight(0);
			else if (dyna->m_rect.right() >= m_height * MAP_SIZE * MPU)
				dyna->m_rect.setRight(m_height * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.bottom() < 0)
				dyna->m_rect.setBottom(0);
			else if (dyna->m_rect.bottom() >= m_width * MAP_SIZE * MPU)
				dyna->m_rect.setBottom(m_width * MAP_SIZE * MPU - 1);
			if (dyna->m_rect.top() < 0)
				dyna->m_rect.setTop(0);
			else if (dyna->m_rect.top() >= m_width * MAP_SIZE * MPU)
				dyna->m_rect.setTop(m_height * MAP_SIZE * MPU - 1);
			dyna->m_rect = dyna->m_rect.normalized();
		}
		AddObject(obj);
		obj = CObject::CreateObject(file);
	}

	return true;
}

bool CWorld::_saveDyoFile(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	const D3DXVECTOR3 pos(0, 0, 0);

	int i;
	CSpawnObject* obj;
	for (i = 0; i < m_objects[OT_CTRL].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_CTRL].GetAt(i);
		if (obj->IsReal() && !obj->IsRespawn())
		{
			file.Write(obj->m_type);
			obj->Write(file, pos);
		}
	}

	for (i = 0; i < m_objects[OT_ITEM].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_ITEM].GetAt(i);
		if (obj->IsReal() && !obj->IsRespawn())
		{
			file.Write(obj->m_type);
			obj->Write(file, pos);
		}
	}

	for (i = 0; i < m_objects[OT_MOVER].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_MOVER].GetAt(i);
		if (obj->IsReal() && !obj->IsRespawn())
		{
			file.Write(obj->m_type);
			obj->Write(file, pos);
		}
	}

	file.Write((uint)4294967295);

	file.Close();
	return true;
}

bool CWorld::_loadCntFile(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	const string keys[] =
	{
		"Continent",
		"BEGIN",
		"END",
		"TOWN",
		"C_id",
		"VERTEX",
		"C_useEnvir",
		"C_useRealData",
		"C_name",
		"C_ambient",
		"C_diffuse",
		"C_fogstartend",
		"C_weather",
		"C_sky",
		"C_cloud",
		"C_sun",
		"C_moon"
	};

	Continent* continent = null;
	do
	{
		const string tok = file.GetString();

		if (tok == keys[0])
		{
			const string tok2 = file.GetString();
			if (tok2 == keys[1])
			{
				continent = new Continent();
				continent->town = false;
				continent->weather = 0;
				continent->useEnvir = false;
				continent->useRealData = false;
				continent->ambient = D3DXVECTOR3(0.5f, 0.5f, 0.5f);
				continent->diffuse = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				continent->fogStart = 70.0f;
				continent->fogEnd = 400.0f;
				continent->cloudTexture = STR_NO;
				continent->skyTexture = STR_NO;
				continent->moonTexture = STR_NO;
				continent->sunTexture = STR_NO;
			}
			else if (tok2 == keys[2])
			{
				m_continents.Append(continent);
			}
		}
		else if (tok == keys[3])
			continent->town = file.GetBool();
		else if (tok == keys[4])
			continent->ID = file.GetInt();
		else if (tok == keys[5])
		{
			D3DXVECTOR3 v;
			v.x = file.GetFloat();
			v.y = file.GetFloat();
			v.z = file.GetFloat();
			continent->vertices.push_back(v);
		}
		else if (tok == keys[6])
			continent->useEnvir = file.GetBool();
		else if (tok == keys[7])
			continent->useRealData = file.GetBool();
		else if (tok == keys[8])
			continent->name = file.GetString();
		else if (tok == keys[9])
		{
			continent->ambient.x = file.GetFloat();
			continent->ambient.y = file.GetFloat();
			continent->ambient.z = file.GetFloat();
		}
		else if (tok == keys[10])
		{
			continent->diffuse.x = file.GetFloat();
			continent->diffuse.y = file.GetFloat();
			continent->diffuse.z = file.GetFloat();
		}
		else if (tok == keys[11])
		{
			continent->fogStart = file.GetFloat();
			continent->fogEnd = file.GetFloat();
		}
		else if (tok == keys[12])
			continent->weather = file.GetInt();
		else if (tok == keys[13])
		{
			if (file.GetBool())
				continent->skyTexture = file.GetString();
			else
				file.NextToken();
		}
		else if (tok == keys[14])
		{
			if (file.GetBool())
				continent->cloudTexture = file.GetString();
			else
				file.NextToken();
		}
		else if (tok == keys[15])
		{
			if (file.GetBool())
				continent->sunTexture = file.GetString();
			else
				file.NextToken();
		}
		else if (tok == keys[16])
		{
			if (file.GetBool())
				continent->moonTexture = file.GetString();
			else
				file.NextToken();
		}

	} while (file.TokenType() != ETokenType::End);

	file.Close();
	return true;
}

bool CWorld::_saveCntFile(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out.setRealNumberNotation(QTextStream::FixedNotation);

	out << "WORLD " << QFileInfo(m_filename).baseName() + ".wld";

	Continent* continent;
	int j;
	for (int i = 0; i < m_continents.GetSize(); i++)
	{
		continent = m_continents[i];
		out << endl << endl << "Continent BEGIN" << endl;
		out << "TOWN " << (continent->town ? 1 : 0) << endl;
		out << "C_useEnvir " << (continent->useEnvir ? 1 : 0) << endl;
		out << "C_useRealData " << (continent->useRealData ? 1 : 0) << endl;
		if (!continent->name.isEmpty())
			out << "C_name " << continent->name << endl;
		out << "C_id " << continent->ID << endl;
		if (continent->useEnvir)
		{
			out << "C_ambient " << continent->ambient.x << ' ' << continent->ambient.y << ' ' << continent->ambient.z << endl;
			out << "C_diffuse " << continent->diffuse.x << ' ' << continent->diffuse.y << ' ' << continent->diffuse.z << endl;
			out << "C_fogstartend " << continent->fogStart << ' ' << continent->fogEnd << endl;
			out << "C_weather " << continent->weather << endl;
			out << "C_sky " << (continent->skyTexture == STR_NO ? 0 : 1) << ' ' << continent->skyTexture << endl;
			out << "C_cloud " << (continent->cloudTexture == STR_NO ? 0 : 1) << ' ' << continent->cloudTexture << endl;
			out << "C_sun " << (continent->sunTexture == STR_NO ? 0 : 1) << ' ' << continent->sunTexture << endl;
			out << "C_moon " << (continent->moonTexture == STR_NO ? 0 : 1) << ' ' << continent->moonTexture << endl;
		}
		for (j = 0; j < continent->vertices.size(); j++)
			out << "VERTEX " << continent->vertices[j].x << ' ' << continent->vertices[j].y << ' ' << continent->vertices[j].z << endl;
		out << "Continent END";
	}

	file.close();
	return true;
}

bool CWorld::_loadRgnFile(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	const string keys[] =
	{
		"region3",
		"respawn7"
	};

	CObject* obj;
	CRegion* reg;
	int type;
	QMap<string, string>::iterator it;

	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		const string tok = file.Token();

		if (tok == keys[1])
		{
			type = file.GetInt();
			obj = CObject::CreateObject((uint)type);
			if (obj)
			{
				((CSpawnObject*)obj)->ReadRespawn(&file);
				if (obj->Init())
				{
					if (obj->m_type == OT_MOVER)
					{
						CMover* mover = (CMover*)obj;
						mover->m_belligerence = mover->m_moverProp->belligerence;
						mover->m_AIInterface = mover->m_moverProp->AI;
						mover->m_name = mover->m_moverProp->name;
					}
					AddObject(obj);
				}
				else
					Delete(obj);
			}
		}
		else if (tok == keys[0])
		{
			type = file.GetInt();
			obj = CObject::CreateObject((uint)type);
			if (obj)
			{
				reg = (CRegion*)obj;
				reg->ReadRegion(&file);
				if (obj->Init())
				{
					obj->ResetScale();
					AddObject(obj);

					if (!reg->m_titleDefine.isEmpty())
					{
						it = m_texts.find(reg->m_titleDefine);
						if (it != m_texts.end())
							reg->m_title = it.value();
					}

					if (!reg->m_descDefine.isEmpty())
					{
						it = m_texts.find(reg->m_descDefine);
						if (it != m_texts.end())
							reg->m_desc = it.value();
					}
				}
				else
					Delete(obj);
			}
		}

		file.NextToken();
	}

	file.Close();
	return true;
}

bool CWorld::_saveRgnFile(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	const byte header[2] = { 0xff, 0xfe };
	file.write((const char*)header, (qint64)sizeof(header));
	QTextStream out(&file);
	out.setCodec("UTF-16LE");
	out.setRealNumberNotation(QTextStream::FixedNotation);

	out << "// Region Script File" << endl << endl;

	int i;
	CSpawnObject* obj;
	CRegion* reg;

	for (i = 0; i < m_objects[OT_CTRL].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_CTRL].GetAt(i);
		if (obj->IsReal() && obj->IsRespawn())
			obj->WriteRespawn(out);
	}

	for (i = 0; i < m_objects[OT_ITEM].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_ITEM].GetAt(i);
		if (obj->IsReal() && obj->IsRespawn())
			obj->WriteRespawn(out);
	}

	for (i = 0; i < m_objects[OT_MOVER].GetSize(); i++)
	{
		obj = (CSpawnObject*)m_objects[OT_MOVER].GetAt(i);
		if (obj->IsReal() && obj->IsRespawn())
			obj->WriteRespawn(out);
	}

	for (i = 0; i < m_objects[OT_REGION].GetSize(); i++)
	{
		reg = (CRegion*)m_objects[OT_REGION].GetAt(i);
		if (reg->IsReal())
			reg->WriteRegion(out);
	}

	file.close();
	return true;
}

void CWorld::_setRegionTexts()
{
	string pre(QFileInfo(m_filename).baseName().toUpper().replace(' ', '_').normalized(QString::NormalizationForm_D));
	pre = pre.replace(QRegExp("[^a-zA-Z\\s_]"), "");
	pre = "IDS_" + pre + "_WLD_";

	m_texts.clear();

	int ID = 0;
	string define;
	CRegion* reg;

	for (int i = 0; i < m_objects[OT_REGION].GetSize(); i++)
	{
		reg = (CRegion*)m_objects[OT_REGION].GetAt(i);
		if (reg->IsReal())
		{
			if (!reg->m_title.isEmpty())
			{
				define = pre + string().sprintf("%06d", ID);
				m_texts[define] = reg->m_title;
				reg->m_titleDefine = define;
				ID++;
			}

			if (!reg->m_desc.isEmpty())
			{
				define = pre + string().sprintf("%06d", ID);
				m_texts[define] = reg->m_desc;
				reg->m_descDefine = define;
				ID++;
			}
		}
	}
}

bool CWorld::_loadTxtTxtFile(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	file.NextToken(false);
	while (file.TokenType() != ETokenType::End)
	{
		const string id = file.Token();
		if (id.size() <= 9 || id[0] != 'I' || id[1] != 'D' || id[2] != 'S')
		{
			file.NextToken(false);
			continue;
		}

		m_texts[id] = file.GetLine();
		file.NextToken(false);
	}

	file.Close();
	return true;
}

bool CWorld::_saveTxtTxtFile(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	const byte header[2] = { 0xff, 0xfe };
	file.write((const char*)header, (qint64)sizeof(header));
	QTextStream out(&file);
	out.setCodec("UTF-16LE");
	out.setRealNumberNotation(QTextStream::FixedNotation);

	for (auto it = m_texts.begin(); it != m_texts.end(); it++)
		out << it.key() << '\t' << it.value() << endl;

	file.close();
	return true;
}

void CWorld::_loadLndFiles()
{
	int x, z, i, j;
	WorldPosToLand(m_cameraPos, x, z);

	for (i = z - m_visibilityLand; i <= z + m_visibilityLand; i++)
	{
		for (j = x - m_visibilityLand; j <= (x + m_visibilityLand); j++)
		{
			if (LandInWorld(j, i))
			{
				const int offset = i * m_width + j;
				if (!m_lands[offset])
				{
					if (!_loadLndFile(i, j))
						continue;
				}
			}
		}
	}
}

void CWorld::_cleanLndFiles()
{
	int x, z, i, j, k, l;
	WorldPosToLand(m_cameraPos, x, z);

	CLandscape* land;
	for (i = 0; i < m_height; i++)
	{
		for (j = 0; j < m_width; j++)
		{
			if (i >= z - m_visibilityLand && i <= z + m_visibilityLand
				&& j >= x - m_visibilityLand && j <= x + m_visibilityLand)
				continue;

			land = m_lands[i * m_width + j];
			if (land)
			{
				for (k = 0; k < MAX_OBJTYPE; k++)
					for (l = 0; l < land->m_objects[k].GetSize(); l++)
						land->m_objects[k].GetAt(l)->m_world = null;
				Delete(m_lands[i * m_width + j]);
			}
		}
	}
}

bool CWorld::_loadPatFile(const string& filename)
{
	CTextFile file;
	if (!file.Load(filename))
		return false;

	const string keys[] =
	{
		"INDEX"
	};

	D3DXVECTOR3 pos;
	CPath* path;
	int index, patIndex;

	file.NextToken();
	while (file.TokenType() != ETokenType::End)
	{
		if (file.Token() == keys[0])
		{
			patIndex = file.GetInt();
			m_paths[patIndex] = new CPtrArray<CPath>();

			file.NextToken(); // {
			index = file.GetInt();

			while (file.TokenType() != ETokenType::EndBlock && file.TokenType() != ETokenType::End)
			{
				path = (CPath*)CObject::CreateObject(OT_PATH);
				path->m_index = index;
				pos.x = file.GetFloat();
				pos.y = file.GetFloat();
				pos.z = file.GetFloat();
				path->SetPos(pos);

				if (path->m_index != 0)
				{
					file.GetFloat();
					file.GetFloat();
					file.GetFloat();
					path->m_motion = file.GetInt();
					path->m_delay = file.GetInt();
					file.GetFloat();
				}

				index = file.GetInt();

				if (path->Init())
				{
					AddObject(path);
					m_paths[patIndex]->Append(path);
				}
				else
					Delete(path);
			}

			file.NextToken();
		}
	}

	file.Close();
	return true;
}

bool CWorld::_savePatFile(const string& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical(("Cant create file '" + filename + "'").toLocal8Bit());
		return false;
	}
	QTextStream out(&file);
	out.setRealNumberNotation(QTextStream::FixedNotation);

	CPath* path;
	D3DXVECTOR3 dir;
	float len;

	for (auto it = m_paths.begin(); it != m_paths.end(); it++)
	{
		if (it.value()->GetSize() > 0)
		{
			out << "INDEX " << it.key() << endl << '{' << endl;

			for (int i = 0; i < it.value()->GetSize(); i++)
			{
				path = it.value()->GetAt(i);

				out << "	    " << path->m_index << "    " << path->m_pos.x << ' ' << path->m_pos.y << ' ' << path->m_pos.z << endl;

				if (i > 0)
				{
					dir = path->m_pos - it.value()->GetAt(i - 1)->m_pos;
					len = D3DXVec3Length(&dir);
					D3DXVec3Normalize(&dir, &dir);

					out << "		 " << dir.x << ' ' << dir.y << ' ' << dir.z << ' '
						<< path->m_motion << ' '
						<< path->m_delay << ' '
						<< len
						<< endl;
				}
			}

			out << '}' << endl << endl;
		}
	}

	file.close();
	return true;
}