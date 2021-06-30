///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Sfx.h"
#include "TextureMng.h"
#include "ModelMng.h"
#include "Mesh.h"

struct SelectVertex
{
	enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE };
	D3DXVECTOR3 p;
	uint c;
};

CSfx::CSfx(LPDIRECT3DDEVICE9 device)
	: m_device(device)
{
}

CSfx::~CSfx()
{
	for (int i = 0; i < m_parts.GetSize(); i++)
		Delete(m_parts[i]);
}

bool CSfx::Load(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	char versionName[9];
	file.Read(versionName, 8);
	versionName[8] = '\0';

	int version = 0;
	if (strcmp(versionName, "SFX0.1  ") == 0)
		version = 1;
	else if (strcmp(versionName, "SFX0.2  ") == 0)
		version = 2;
	else if (strcmp(versionName, "SFX0.3  ") == 0)
		version = 3;
	else
		file.SetPos(0);

	int partCount;
	file.Read(partCount);

	ESfxPartType type;
	CSfxPart* part;
	for (int i = 0; i < partCount; i++)
	{
		file.Read(type);
		part = AddPart(type);
		part->Load(file, version);
	}

	file.Close();
	return true;
}

bool CSfx::Save(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	const char version[9] = "SFX0.3  ";
	file.Write(version, 8);
	file.Write(m_parts.GetSize());

	for (int i = 0; i < m_parts.GetSize(); i++)
	{
		file.Write(m_parts[i]->GetType());
		m_parts[i]->Save(file);
	}

	file.Close();
	return true;
}

CSfxPart* CSfx::AddPart(ESfxPartType type)
{
	CSfxPart* part = null;
	switch (type)
	{
	case ESfxPartType::Bill:
		part = new CSfxPartBill(m_device);
		break;
	case ESfxPartType::Particle:
		part = new CSfxPartParticle(m_device);
		break;
	case ESfxPartType::Mesh:
		part = new CSfxPartMesh(m_device);
		break;
	case ESfxPartType::CustomMesh:
		part = new CSfxPartCustomMesh(m_device);
		break;
	default:
		return null;
	}

	m_parts.Append(part);
	return part;
}

void CSfx::RemovePart(int i)
{
	if (i >= m_parts.GetSize())
		return;

	Delete(m_parts[i]);
	m_parts.RemoveAt(i);
}

CSfxPart::CSfxPart(LPDIRECT3DDEVICE9 device)
	: m_device(device),
	m_billType(ESfxPartBillType::Bill),
	m_alphaType(ESfxPartAlphaType::Blend),
	m_visible(true),
	m_texFrame(1),
	m_texLoop(1),
	m_texture(null),
	m_textures(null)
{
}

CSfxPart::~CSfxPart()
{
	for (int i = 0; i < m_keys.GetSize(); i++)
		Delete(m_keys[i]);
	DeleteArray(m_textures);
}

SfxKeyFrame* CSfxPart::AddKey(ushort frame)
{
	SfxKeyFrame* existingKey = GetKey(frame);
	if (existingKey)
		return existingKey;

	SfxKeyFrame* key = new SfxKeyFrame();
	const SfxKeyFrame* prevKey = GetPrevKey(frame);
	const SfxKeyFrame* nextKey = GetNextKey(frame);
	if (prevKey == null || nextKey == null)
	{
		key->pos = D3DXVECTOR3(0, 0, 0);
		key->posRotate = D3DXVECTOR3(0, 0, 0);
		key->scale = D3DXVECTOR3(1, 1, 1);
		key->rotate = D3DXVECTOR3(0, 0, 0);
		key->alpha = 255;
		if (GetType() == ESfxPartType::CustomMesh)
			key->posRotate.z = 1.0f;
	}
	else
	{
		const int deltaFrame = nextKey->frame - prevKey->frame;
		*key = *prevKey;
		if (deltaFrame != 0)
		{
			key->pos += (nextKey->pos - prevKey->pos) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
			key->posRotate = (nextKey->posRotate - prevKey->posRotate) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
			key->rotate += (nextKey->rotate - prevKey->rotate) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
			key->scale += (nextKey->scale - prevKey->scale) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
			key->alpha += (nextKey->alpha - prevKey->alpha) * (frame - prevKey->frame) / deltaFrame;
		}
	}

	int pos = -1;
	if (m_keys.GetSize() > 0)
	{
		for (int i = 0; i < m_keys.GetSize(); i++)
		{
			if (m_keys[i]->frame > frame)
			{
				pos = i;
				break;
			}
		}
	}

	key->frame = frame;
	m_keys.Append(key, pos);
	return key;
}

SfxKeyFrame* CSfxPart::GetKey(ushort frame) const
{
	for (int i = 0; i < m_keys.GetSize(); i++)
		if (m_keys[i]->frame == frame)
			return m_keys[i];
	return null;
}

SfxKeyFrame* CSfxPart::GetFirstKey() const
{
	if (m_keys.GetSize() > 0)
		return m_keys[0];
	return null;
}

SfxKeyFrame* CSfxPart::GetLastKey() const
{
	if (m_keys.GetSize() > 0)
		return m_keys[m_keys.GetSize() - 1];
	return null;
}

SfxKeyFrame* CSfxPart::GetPrevKey(ushort frame) const
{
	SfxKeyFrame* key = null;
	for (int i = 0; i < m_keys.GetSize(); i++)
	{
		if (m_keys[i]->frame > frame)
			break;
		key = m_keys[i];
	}
	return key;
}

SfxKeyFrame* CSfxPart::GetNextKey(ushort frame, bool skip) const
{
	SfxKeyFrame* key = null;
	if (skip)
	{
		for (int i = 0; i < m_keys.GetSize(); i++)
		{
			if (m_keys[i]->frame >= frame)
			{
				key = m_keys[i];
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_keys.GetSize(); i++)
		{
			if (m_keys[i]->frame > frame)
			{
				key = m_keys[i];
				break;
			}
		}

	}
	return key;
}

bool CSfxPart::GetKey(ushort frame, SfxKeyFrame& key) const
{
	const SfxKeyFrame* prevKey = GetPrevKey(frame);
	const SfxKeyFrame* nextKey = GetNextKey(frame);
	if (prevKey == null || nextKey == null)
		return false;

	const int deltaFrame = nextKey->frame - prevKey->frame;
	key = *prevKey;
	if (deltaFrame != 0)
	{
		key.pos += (nextKey->pos - prevKey->pos) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.posRotate += (nextKey->posRotate - prevKey->posRotate) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.rotate += (nextKey->rotate - prevKey->rotate) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.scale += (nextKey->scale - prevKey->scale) * (float)((frame - prevKey->frame)) / (float)(deltaFrame);
		key.alpha += (nextKey->alpha - prevKey->alpha) * (frame - prevKey->frame) / deltaFrame;
		key.frame = deltaFrame;
	}
	return true;
}

void CSfxPart::RemoveKey(ushort frame)
{
	for (int i = 0; i < m_keys.GetSize(); i++)
	{
		if (m_keys[i]->frame == frame)
		{
			Delete(m_keys[i]);
			m_keys.RemoveAt(i);
			break;
		}
	}
}

void CSfxPart::RemoveAllKeys()
{
	for (int i = 0; i < m_keys.GetSize(); i++)
		Delete(m_keys[i]);
	m_keys.RemoveAll();
}

void CSfxPart::_setTexture()
{
	if (m_texFrame > 1)
	{
		DeleteArray(m_textures);
		m_textures = new CTexture*[m_texFrame];
		memset(m_textures, 0, sizeof(CTexture*) * m_texFrame);

		if (m_textureName.size() > 0)
		{
			string temp = m_textureName.left(m_textureName.size() - 4);
			temp = temp.right(2);
			const int numStart = temp.toInt();
			const string name = m_textureName.left(m_textureName.size() - 6);
			const string ext = m_textureName.right(4);
			for (int i = 0; i < m_texFrame; i++)
			{
				temp = name + string().sprintf("%02d", numStart + i) + ext;
				m_textures[i] = TextureMng->GetSfxTexture(temp);
			}
		}
	}
	else if (m_textureName.size() > 0)
		m_texture = TextureMng->GetSfxTexture(m_textureName);
}

void CSfxPart::RenderSelectionRectangle(ushort frame)
{
	SfxKeyFrame key;
	if (!GetKey(frame, key))
		return;

	const D3DXVECTOR3 scale(1, 1, 1);
	const D3DXVECTOR3 angle(0, 0, 0);
	const D3DXVECTOR3 pos(0, 0, 0);
	const D3DXVECTOR3 scaleTemp(
		scale.x * key.scale.x,
		scale.y * key.scale.y,
		scale.z * key.scale.z
		);
	const D3DXVECTOR3 rot = D3DXToRadian(angle);
	D3DXMATRIX matScale, matAngle, matTemp, matRot, matTemp2;
	D3DXMatrixScaling(&matScale, scaleTemp.x, scaleTemp.y, scaleTemp.z);
	D3DXMatrixRotationYawPitchRoll(&matRot, rot.y, rot.x, rot.z);

	switch (m_billType)
	{
	case ESfxPartBillType::Pole:
	case ESfxPartBillType::Bill:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		matTemp = matScale * matAngle * g_global3D.invView;
		break;

	case ESfxPartBillType::Bottom:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z - angle.z));
		D3DXMatrixRotationX(&matTemp, D3DXToRadian(90));
		matTemp = matScale * matAngle * matTemp;
		break;

	case ESfxPartBillType::Normal:
		const D3DXVECTOR3 temp = D3DXToRadian(key.rotate);
		D3DXMatrixRotationYawPitchRoll(&matAngle, temp.y, temp.x, temp.z);
		matTemp = matScale * matAngle * matRot;
		break;
	}

	const D3DXVECTOR3 temp = D3DXToRadian(key.posRotate);
	D3DXMatrixRotationYawPitchRoll(&matTemp2, temp.y, temp.x, temp.z);

	D3DXVECTOR3 ptPos;
	D3DXVec3TransformCoord(&ptPos, &key.pos, &matTemp2);

	matTemp2 = matRot * matTemp2;
	D3DXVECTOR3 temp2;
	D3DXVec3TransformCoord(&temp2, &key.pos, &matTemp2);
	D3DXMatrixTranslation(&matTemp2,
		pos.x + (temp2.x * scale.x),
		pos.y + (temp2.y * scale.y),
		pos.z + (temp2.z * scale.z)
		);
	const D3DXMATRIX world = matTemp * matTemp2;

	SelectVertex vertices[22];
	for (int i = 0; i < 22; i++)
		vertices[i].c = 0xff00e1e4;
	vertices[0].p = D3DXVECTOR3(-0.5f, 0.5f, 0.0f);
	vertices[1].p = D3DXVECTOR3(0.5f, 0.5f, 0.0f);
	vertices[2].p = D3DXVECTOR3(0.5f, 0.5f, 0.0f);
	vertices[3].p = D3DXVECTOR3(0.5f, -0.5f, 0.0f);
	vertices[4].p = D3DXVECTOR3(0.5f, -0.5f, 0.0f);
	vertices[5].p = D3DXVECTOR3(-0.5f, -0.5f, 0.0f);
	vertices[6].p = D3DXVECTOR3(-0.5f, -0.5f, 0.0f);
	vertices[7].p = D3DXVECTOR3(-0.5f, 0.5f, 0.0f);
	vertices[8].p = D3DXVECTOR3(0.0f, 0.0f, ptPos.z);
	vertices[9].p = D3DXVECTOR3(ptPos.x, 0.0f, ptPos.z);
	vertices[10].p = D3DXVECTOR3(0.0f, 0.0f, ptPos.z);
	vertices[11].p = D3DXVECTOR3(0.0f, ptPos.y, ptPos.z);
	vertices[12].p = D3DXVECTOR3(0.0f, ptPos.y, ptPos.z);
	vertices[13].p = D3DXVECTOR3(ptPos.x, ptPos.y, ptPos.z);
	vertices[14].p = D3DXVECTOR3(ptPos.x, 0.0f, ptPos.z);
	vertices[15].p = D3DXVECTOR3(ptPos.x, ptPos.y, ptPos.z);
	vertices[16].p = D3DXVECTOR3(ptPos.x, 0.0f, 0.0f);
	vertices[17].p = D3DXVECTOR3(ptPos.x, 0.0f, ptPos.z);
	vertices[18].p = D3DXVECTOR3(ptPos.x, 0.0f, 0.0f);
	vertices[19].p = D3DXVECTOR3(ptPos.x, ptPos.y, 0.0f);
	vertices[20].p = D3DXVECTOR3(ptPos.x, ptPos.y, ptPos.z);
	vertices[21].p = D3DXVECTOR3(ptPos.x, ptPos.y, 0.0f);

	m_device->SetTexture(0, null);
	m_device->SetFVF(SelectVertex::FVF);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);

	m_device->SetTransform(D3DTS_WORLD, &world);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 4, vertices, sizeof(SelectVertex));

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ZENABLE, TRUE);

	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	m_device->SetTransform(D3DTS_WORLD, &identity);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 7, &vertices[8], sizeof(SelectVertex));
}

CSfxPartBill::CSfxPartBill(LPDIRECT3DDEVICE9 device)
	: CSfxPart(device)
{
}

void CSfxPartBill::Load(CFile& file, int version)
{
	int len;
	char temp[255];
	file.Read(len);
	file.Read(temp, len);
	temp[len] = '\0';
	m_name = temp;

	if (version >= 2)
	{
		file.Read(len);
		file.Read(temp, len);
		temp[len] = '\0';
		m_textureName = temp;
	}
	else
		m_textureName = m_name;

	if (version >= 1)
	{
		file.Read(m_texFrame);
		file.Read(m_texLoop);

		if (version >= 2)
		{
			int visible;
			file.Read(visible);
			m_visible = visible != 0;
		}
	}

	file.Read(m_billType);
	file.Read(m_alphaType);

	int keyCount;
	file.Read(keyCount);

	SfxKeyFrame* key;
	ushort frame;
	for (int i = 0; i < keyCount; i++)
	{
		file.Read(frame);
		key = AddKey(frame);
		file.Read(key->pos);
		file.Read(key->posRotate);
		file.Read(key->scale);
		file.Read(key->rotate);
		file.Read(key->alpha);
	}

	_setTexture();
}

void CSfxPartBill::Save(CFile& file)
{
	const QByteArray name = m_name.toLocal8Bit();
	file.Write(name.size());
	file.Write(name.constData(), name.size());

	const QByteArray textureName = m_textureName.toLocal8Bit();
	file.Write(textureName.size());
	file.Write(textureName.constData(), textureName.size());

	file.Write(m_texFrame);
	file.Write(m_texLoop);

	const int visible = m_visible ? 1 : 0;
	file.Write(visible);

	file.Write(m_billType);
	file.Write(m_alphaType);

	file.Write(m_keys.GetSize());
	for (int i = 0; i < m_keys.GetSize(); i++)
	{
		const SfxKeyFrame* key = m_keys[i];
		file.Write(key->frame);
		file.Write(key->pos);
		file.Write(key->posRotate);
		file.Write(key->scale);
		file.Write(key->rotate);
		file.Write(key->alpha);
	}
}

void CSfxPartBill::Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale)
{
	if (m_texFrame > 1)
	{
		SfxKeyFrame* firstKey = GetNextKey(0);
		if (!firstKey || frame < firstKey->frame)
			return;
		const int texFrame = (m_texFrame * (frame - firstKey->frame) / m_texLoop) + m_texFrame;
		if (m_textures[texFrame])
			m_device->SetTexture(0, *m_textures[texFrame]);
		else
			m_device->SetTexture(0, null);
	}
	else if (m_texture)
		m_device->SetTexture(0, *m_texture);
	else
		m_device->SetTexture(0, null);

	SfxKeyFrame key;
	if (!GetKey(frame, key))
		return;

	const D3DXVECTOR3 scaleTemp(
		scale.x * key.scale.x,
		scale.y * key.scale.y,
		scale.z * key.scale.z
		);
	const D3DXVECTOR3 rot = D3DXToRadian(angle);
	D3DXMATRIX matScale, matAngle, matTemp, matRot, matTemp2;
	D3DXMatrixScaling(&matScale, scaleTemp.x, scaleTemp.y, scaleTemp.z);
	D3DXMatrixRotationYawPitchRoll(&matRot, rot.y, rot.x, rot.z);

	switch (m_billType)
	{
	case ESfxPartBillType::Pole:
	case ESfxPartBillType::Bill:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		matTemp = matScale * matAngle * g_global3D.invView;
		break;

	case ESfxPartBillType::Bottom:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z - angle.z));
		D3DXMatrixRotationX(&matTemp, D3DXToRadian(90));
		matTemp = matScale * matAngle * matTemp;
		break;

	case ESfxPartBillType::Normal:
	{
		const D3DXVECTOR3 temp = D3DXToRadian(key.rotate);
		D3DXMatrixRotationYawPitchRoll(&matAngle, temp.y, temp.x, temp.z);
		matTemp = matScale * matAngle * matRot;
		break;
	}
	}

	const D3DXVECTOR3 temp = D3DXToRadian(key.posRotate);
	D3DXMatrixRotationYawPitchRoll(&matTemp2, temp.y, temp.x, temp.z);
	matTemp2 = matRot * matTemp2;
	D3DXVECTOR3 temp2;
	D3DXVec3TransformCoord(&temp2, &key.pos, &matTemp2);
	D3DXMatrixTranslation(&matTemp2,
		pos.x + (temp2.x * scale.x),
		pos.y + (temp2.y * scale.y),
		pos.z + (temp2.z * scale.z)
		);
	const D3DXMATRIX world = matTemp * matTemp2;

	if (m_alphaType == ESfxPartAlphaType::Glow)
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	m_device->SetTransform(D3DTS_WORLD, &world);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, key.alpha << 24 | 0x404040);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

	if (m_alphaType == ESfxPartAlphaType::Glow)
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

CSfxPartParticle::CSfxPartParticle(LPDIRECT3DDEVICE9 device)
	: CSfxPart(device),
	m_particleCreate(0),
	m_particleCreateNum(5),
	m_particleFrameAppear(0),
	m_particleFrameKeep(10),
	m_particleFrameDisappear(20),
	m_particleStartPosVar(0.0f),
	m_particleStartPosVarY(0.0f),
	m_particleYLow(0.01f),
	m_particleYHigh(0.1f),
	m_particleXZLow(0.01f),
	m_particleXZHigh(0.1f),
	m_particleAccel(0.0f, 0.0f, 0.0f),
	m_scale(0.1f, 0.1f, 0.1f),
	m_scaleSpeed(0.0f, 0.0f, 0.0f),
	m_rotation(0.0f, 0.0f, 0.0f),
	m_rotationLow(0.0f, 0.0f, 0.0f),
	m_rotationHigh(0.0f, 0.0f, 0.0f),
	m_repeatScal(false),
	m_repeat(false),
	m_scalSpeedXLow(0),
	m_scalSpeedXHigh(0),
	m_scalSpeedYLow(0),
	m_scalSpeedYHigh(0),
	m_scalSpeedZLow(0),
	m_scalSpeedZHigh(0),
	m_scaleSpeed2(0.0f, 0.0f, 0.0f),
	m_scaleEnd(0.0f, 0.0f, 0.0f)
{
}

void CSfxPartParticle::Load(CFile& file, int version)
{
	int len;
	char temp[255];
	file.Read(len);
	file.Read(temp, len);
	temp[len] = '\0';
	m_name = temp;

	if (version >= 2)
	{
		file.Read(len);
		file.Read(temp, len);
		temp[len] = '\0';
		m_textureName = temp;
	}
	else
		m_textureName = m_name;

	if (version >= 1)
	{
		file.Read(m_texFrame);
		file.Read(m_texLoop);

		if (version >= 2)
		{
			int visible;
			file.Read(visible);
			m_visible = visible != 0;
		}
	}

	file.Read(m_billType);
	file.Read(m_alphaType);

	file.Read(m_particleCreate);
	file.Read(m_particleCreateNum);
	file.Read(m_particleFrameAppear);
	file.Read(m_particleFrameKeep);
	file.Read(m_particleFrameDisappear);

	file.Read(m_particleStartPosVar);
	file.Read(m_particleStartPosVarY);

	file.Read(m_particleYLow);
	file.Read(m_particleYHigh);
	file.Read(m_particleXZLow);
	file.Read(m_particleXZHigh);

	file.Read(m_particleAccel);

	file.Read(m_scale);
	file.Read(m_scaleSpeed);

	if (version >= 2)
	{
		file.Read(m_rotation);
		file.Read(m_rotationLow);
		file.Read(m_rotationHigh);

		int tempBool;
		file.Read(tempBool);
		m_repeatScal = tempBool != 0;

		if (version >= 3)
		{
			file.Read(tempBool);
			m_repeat = tempBool != 0;
		}

		file.Read(m_scalSpeedXLow);
		file.Read(m_scalSpeedXHigh);
		file.Read(m_scalSpeedYLow);
		file.Read(m_scalSpeedYHigh);
		file.Read(m_scalSpeedZLow);
		file.Read(m_scalSpeedZHigh);
		file.Read(m_scaleSpeed2);
		file.Read(m_scaleEnd);
	}

	int keyCount;
	file.Read(keyCount);

	SfxKeyFrame* key;
	ushort frame;
	for (int i = 0; i < keyCount; i++)
	{
		file.Read(frame);
		key = AddKey(frame);
		file.Read(key->pos);
		file.Read(key->posRotate);
		file.Read(key->scale);
		file.Read(key->rotate);
		file.Read(key->alpha);
	}

	_setTexture();
}

void CSfxPartParticle::Save(CFile& file)
{
	const QByteArray name = m_name.toLocal8Bit();
	file.Write(name.size());
	file.Write(name.constData(), name.size());

	const QByteArray textureName = m_textureName.toLocal8Bit();
	file.Write(textureName.size());
	file.Write(textureName.constData(), textureName.size());

	file.Write(m_texFrame);
	file.Write(m_texLoop);

	const int visible = m_visible ? 1 : 0;
	file.Write(visible);

	file.Write(m_billType);
	file.Write(m_alphaType);

	file.Write(m_particleCreate);
	file.Write(m_particleCreateNum);
	file.Write(m_particleFrameAppear);
	file.Write(m_particleFrameKeep);
	file.Write(m_particleFrameDisappear);

	file.Write(m_particleStartPosVar);
	file.Write(m_particleStartPosVarY);

	file.Write(m_particleYLow);
	file.Write(m_particleYHigh);
	file.Write(m_particleXZLow);
	file.Write(m_particleXZHigh);

	file.Write(m_particleAccel);

	file.Write(m_scale);
	file.Write(m_scaleSpeed);

	file.Write(m_rotation);
	file.Write(m_rotationLow);
	file.Write(m_rotationHigh);

	const int repeatScal = m_repeatScal ? 1 : 0;
	file.Write(repeatScal);

	const int repeat = m_repeat ? 1 : 0;
	file.Write(repeat);

	file.Write(m_scalSpeedXLow);
	file.Write(m_scalSpeedXHigh);
	file.Write(m_scalSpeedYLow);
	file.Write(m_scalSpeedYHigh);
	file.Write(m_scalSpeedZLow);
	file.Write(m_scalSpeedZHigh);
	file.Write(m_scaleSpeed2);
	file.Write(m_scaleEnd);

	file.Write(m_keys.GetSize());
	for (int i = 0; i < m_keys.GetSize(); i++)
	{
		const SfxKeyFrame* key = m_keys[i];
		file.Write(key->frame);
		file.Write(key->pos);
		file.Write(key->posRotate);
		file.Write(key->scale);
		file.Write(key->rotate);
		file.Write(key->alpha);
	}
}

void CSfxPartParticle::Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale)
{
}

CSfxPartMesh::CSfxPartMesh(LPDIRECT3DDEVICE9 device)
	: CSfxPart(device),
	m_mesh(null)
{
}

void CSfxPartMesh::Load(CFile& file, int version)
{
	int len;
	char temp[255];
	file.Read(len);
	file.Read(temp, len);
	temp[len] = '\0';
	m_name = temp;

	if (version >= 2)
	{
		file.Read(len);
		file.Read(temp, len);
		temp[len] = '\0';
		m_textureName = temp;
	}
	else
		m_textureName = m_name;

	if (version >= 1)
	{
		file.Read(m_texFrame);
		file.Read(m_texLoop);

		if (version >= 2)
		{
			int visible;
			file.Read(visible);
			m_visible = visible != 0;
		}
	}

	file.Read(m_billType);
	file.Read(m_alphaType);

	int keyCount;
	file.Read(keyCount);

	SfxKeyFrame* key;
	ushort frame;
	for (int i = 0; i < keyCount; i++)
	{
		file.Read(frame);
		key = AddKey(frame);
		file.Read(key->pos);
		file.Read(key->posRotate);
		file.Read(key->scale);
		file.Read(key->rotate);
		file.Read(key->alpha);
	}

	_setTexture();
}

void CSfxPartMesh::Save(CFile& file)
{
	const QByteArray name = m_name.toLocal8Bit();
	file.Write(name.size());
	file.Write(name.constData(), name.size());

	const QByteArray textureName = m_textureName.toLocal8Bit();
	file.Write(textureName.size());
	file.Write(textureName.constData(), textureName.size());

	file.Write(m_texFrame);
	file.Write(m_texLoop);

	const int visible = m_visible ? 1 : 0;
	file.Write(visible);

	file.Write(m_billType);
	file.Write(m_alphaType);

	file.Write(m_keys.GetSize());
	for (int i = 0; i < m_keys.GetSize(); i++)
	{
		const SfxKeyFrame* key = m_keys[i];
		file.Write(key->frame);
		file.Write(key->pos);
		file.Write(key->posRotate);
		file.Write(key->scale);
		file.Write(key->rotate);
		file.Write(key->alpha);
	}
}

void CSfxPartMesh::_setTexture()
{
	if (m_textureName.isEmpty())
		m_mesh = null;
	else
		m_mesh = (CMesh*)ModelMng->GetModel(MODELTYPE_MESH, m_textureName);
}

void CSfxPartMesh::Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale)
{
	if (!m_mesh)
		return;

	SfxKeyFrame key;
	if (!GetKey(frame, key))
		return;

	const D3DXVECTOR3 scaleTemp(
		scale.x * key.scale.x,
		scale.y * key.scale.y,
		scale.z * key.scale.z
		);
	const D3DXVECTOR3 rot = D3DXToRadian(key.rotate + D3DXVECTOR3(0.0f, angle.y, 0.0f));
	const D3DXVECTOR3 posRot = D3DXToRadian(key.posRotate + D3DXVECTOR3(0.0f, angle.y, 0.0f));
	D3DXMATRIX matScale, matTemp, matRot;
	D3DXMatrixScaling(&matScale, scaleTemp.x, scaleTemp.y, scaleTemp.z);
	D3DXMatrixRotationYawPitchRoll(&matRot, rot.y, rot.x, rot.z);
	D3DXMatrixRotationYawPitchRoll(&matTemp, posRot.y, posRot.x, posRot.z);

	D3DXVECTOR3 temp;
	D3DXVec3TransformCoord(&temp, &key.pos, &matTemp);
	D3DXMatrixTranslation(&matTemp,
		pos.x + (temp.x * scale.x),
		pos.y + (temp.y * scale.y),
		pos.z + (temp.z * scale.z)
		);

	const D3DXMATRIX world = matScale * matRot * matTemp;

	if (m_alphaType == ESfxPartAlphaType::Glow)
	{
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		g_global3D.obj3Deffects = false;
	}

	m_mesh->Render(&world, 0, key.alpha);

	if (m_alphaType == ESfxPartAlphaType::Glow)
	{
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		g_global3D.obj3Deffects = true;
	}
}

CSfxPartCustomMesh::CSfxPartCustomMesh(LPDIRECT3DDEVICE9 device)
	: CSfxPart(device),
	m_pointCount(20)
{
}

void CSfxPartCustomMesh::Load(CFile& file, int version)
{
	int len;
	char temp[255];
	file.Read(len);
	file.Read(temp, len);
	temp[len] = '\0';
	m_name = temp;

	if (version >= 2)
	{
		file.Read(len);
		file.Read(temp, len);
		temp[len] = '\0';
		m_textureName = temp;
	}
	else
		m_textureName = m_name;

	if (version >= 1)
	{
		file.Read(m_texFrame);
		file.Read(m_texLoop);

		if (version >= 2)
		{
			int visible;
			file.Read(visible);
			m_visible = visible != 0;
		}
	}

	file.Read(m_billType);
	file.Read(m_alphaType);

	int keyCount;
	file.Read(keyCount);

	SfxKeyFrame* key;
	ushort frame;
	for (int i = 0; i < keyCount; i++)
	{
		file.Read(frame);
		key = AddKey(frame);
		file.Read(key->pos);
		file.Read(key->posRotate);
		file.Read(key->scale);
		file.Read(key->rotate);
		file.Read(key->alpha);
	}

	file.Read(m_pointCount);

	_setTexture();
}

void CSfxPartCustomMesh::Save(CFile& file)
{
	const QByteArray name = m_name.toLocal8Bit();
	file.Write(name.size());
	file.Write(name.constData(), name.size());

	const QByteArray textureName = m_textureName.toLocal8Bit();
	file.Write(textureName.size());
	file.Write(textureName.constData(), textureName.size());

	file.Write(m_texFrame);
	file.Write(m_texLoop);

	const int visible = m_visible ? 1 : 0;
	file.Write(visible);

	file.Write(m_billType);
	file.Write(m_alphaType);

	file.Write(m_keys.GetSize());
	for (int i = 0; i < m_keys.GetSize(); i++)
	{
		const SfxKeyFrame* key = m_keys[i];
		file.Write(key->frame);
		file.Write(key->pos);
		file.Write(key->posRotate);
		file.Write(key->scale);
		file.Write(key->rotate);
		file.Write(key->alpha);
	}

	file.Write(m_pointCount);
}

void CSfxPartCustomMesh::Render(ushort frame, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale)
{
	if (m_texFrame > 1)
	{
		SfxKeyFrame* firstKey = GetNextKey(0);
		if (!firstKey || frame < firstKey->frame)
			return;
		const int texFrame = (m_texFrame * (frame - firstKey->frame) / m_texLoop) + m_texFrame;
		if (m_textures[texFrame])
			m_device->SetTexture(0, *m_textures[texFrame]);
		else
			m_device->SetTexture(0, null);
	}
	else if (m_texture)
		m_device->SetTexture(0, *m_texture);
	else
		m_device->SetTexture(0, null);

	SfxKeyFrame key;
	if (!GetKey(frame, key))
		return;

	const D3DXVECTOR3 rot = D3DXToRadian(angle);
	D3DXMATRIX matRot;
	D3DXMatrixRotationYawPitchRoll(&matRot, rot.y, rot.x, rot.z);

	D3DXVECTOR3 temp;
	D3DXMATRIX matAngle, matTemp, matTemp2;
	switch (m_billType)
	{
	case ESfxPartBillType::Bill:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z - angle.z));
		matTemp = g_global3D.view;	matTemp._41 = matTemp._42 = matTemp._43 = 0.0f;
		D3DXMatrixInverse(&matTemp, null, &matTemp2);
		matTemp = matAngle * matTemp;
		break;

	case ESfxPartBillType::Bottom:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		D3DXMatrixRotationX(&matTemp, D3DXToRadian(90));
		matTemp = matAngle * matTemp;
		break;

	case ESfxPartBillType::Pole:
		D3DXMatrixRotationZ(&matAngle, D3DXToRadian(key.rotate.z));
		matTemp = g_global3D.view;	matTemp._41 = matTemp._42 = matTemp._43 = 0.0f;
		D3DXMatrixInverse(&matTemp, null, &matTemp);
		matTemp = matAngle * matTemp;
		break;

	case ESfxPartBillType::Normal:
		temp = D3DXToRadian(key.rotate);
		D3DXMatrixRotationYawPitchRoll(&matAngle, temp.y, temp.x, temp.z);
		matTemp = matAngle * matRot;
		break;
	}

	D3DXVec3TransformCoord(&temp, &key.pos, &matRot);
	D3DXMatrixTranslation(&matTemp2,
		pos.x + (temp.x * scale.x),
		pos.y + (temp.y * scale.y),
		pos.z + (temp.z * scale.z));

	const D3DXMATRIX world = matTemp * matTemp2;

	static SfxVertex vertices[((MAX_POINTS_SFX_CUSTOMMESH * 2) + 1) * 2];

	for (int i = 0; i < (m_pointCount * 2) + 1; i++)
	{
		vertices[i * 2].p = D3DXVECTOR3(sin(D3DXToRadian(360 / (m_pointCount * 2)*i))*key.scale.x, key.scale.y, cos(D3DXToRadian(360 / (m_pointCount * 2)*i))*key.scale.x);
		vertices[i * 2 + 1].p = D3DXVECTOR3(sin(D3DXToRadian(360 / (m_pointCount * 2)*i))*key.scale.z, 0, cos(D3DXToRadian(360 / (m_pointCount * 2)*i))*key.scale.z);
		vertices[i * 2].p.x *= scale.x;
		vertices[i * 2].p.y *= scale.y;
		vertices[i * 2].p.z *= scale.z;
		vertices[i * 2 + 1].p.x *= scale.x;
		vertices[i * 2 + 1].p.y *= scale.y;
		vertices[i * 2 + 1].p.z *= scale.z;

		vertices[i * 2].t.y = key.posRotate.y + 0.1f; vertices[i * 2 + 1].t.y = key.posRotate.z;
		if (key.posRotate.x == .0f)
		{
			if (i / 2 * 2 == i)
			{
				vertices[i * 2].t.x = .0f; vertices[i * 2 + 1].t.x = .0f;
			}
			else
			{
				vertices[i * 2].t.x = 1.0f; vertices[i * 2 + 1].t.x = 1.0f;
			}
		}
		else
		{
			vertices[i * 2].t.x = key.posRotate.x*((float)i) / (m_pointCount * 2); vertices[i * 2 + 1].t.x = key.posRotate.x*((float)i) / (m_pointCount * 2);
		}
	}

	if (m_alphaType == ESfxPartAlphaType::Glow)
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	m_device->SetTransform(D3DTS_WORLD, &world);
	m_device->SetRenderState(D3DRS_TEXTUREFACTOR, key.alpha << 24 | 0x404040);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_pointCount * 4, vertices, sizeof(SfxVertex));
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_pointCount * 4, vertices, sizeof(SfxVertex));

	if (m_alphaType == ESfxPartAlphaType::Glow)
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}