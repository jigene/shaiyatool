///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Object3D.h"
#include "ModelMng.h"
#include "Motion.h"
#include "TextureMng.h"
#include "resource.h"

#ifdef WORLD_EDITOR
#include "Collision.h"
#endif // WORLD_EDITOR

LPDIRECT3DVERTEXSHADER9 CObject3D::s_skinVS = null;
LPDIRECT3DVERTEXDECLARATION9 CObject3D::s_skinVertexDeclaration = null;
CTexture CObject3D::s_reflectTexture;

bool CObject3D::InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device)
{
	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ 0, 20, D3DDECLTYPE_SHORT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	if (FAILED(device->CreateVertexDeclaration(decl, &s_skinVertexDeclaration)))
	{
		qCritical("Can't create skin vertex declaration. You must (re)install the lastest DirectX9 runtime on http://www.microsoft.com/en-us/download/details.aspx?id=8109");
		return false;
	}

	LPD3DXBUFFER code;
	if (FAILED(D3DXAssembleShaderFromResource(NULL, MAKEINTRESOURCE(IDR_SKINVS), NULL, NULL, 0, &code, NULL)))
	{
		qCritical("Can't assemble skin vertex shader from resource. You must (re)install the lastest DirectX9 runtime on http://www.microsoft.com/en-us/download/details.aspx?id=8109");
		return false;
	}

	if (FAILED(device->CreateVertexShader((DWORD*)code->GetBufferPointer(), &s_skinVS)))
	{
		qCritical("Can't create skin vertex shader. You must (re)install the lastest DirectX9 runtime on http://www.microsoft.com/en-us/download/details.aspx?id=8109");
		Release(code);
		return false;
	}
	Release(code);

	s_reflectTexture.SetDevice(device);
	if (!s_reflectTexture.Load("Model/Texture/etc_reflect.tga"))
	{
		qCritical("Can't load Model/Texture/etc_reflect.tga");
		return false;
	}

	return true;
}

void CObject3D::DeleteStaticDeviceObjects()
{
	Release(s_skinVertexDeclaration);
	Release(s_skinVS);
	s_reflectTexture.Release();
}

CObject3D::CObject3D(LPDIRECT3DDEVICE9 device)
	: m_device(device),
	m_ID(-1),
	m_scrlU(0.0f),
	m_scrlV(0.0f),
	m_perSlerp(0.5f),
	m_frameCount(0),
	m_LOD(false),
	m_boneCount(0),
	m_attributes(null),
	m_baseBones(null),
	m_baseInvBones(null),
	m_sendVS(false),
	m_motion(null),
	m_externBones(null),
	m_externInvBones(null),
	m_group(null),
	m_eventCount(0),
	m_hasSkin(false)
{
	memset(&m_bounds, 0, sizeof(m_bounds));
	memset(m_forces, 0, sizeof(m_forces));
	memset(m_groups, 0, sizeof(m_groups));
	memset(&m_collObj, 0, sizeof(m_collObj));
	memset(m_events, 0, sizeof(m_events));
	m_collObj.type = GMT_ERROR;
	m_collObj.parentID = -1;
	m_collObj.parentType = GMT_ERROR;
	D3DXMatrixIdentity(&m_collObj.transform);

	for (int i = 0; i < MAX_GROUP; i++)
		m_groups[i].currentTextureEx = -1;
}

CObject3D::~CObject3D()
{
	GMObject* obj;
	int j;
	for (int i = 0; i < MAX_GROUP; i++)
	{
		for (j = 0; j < m_groups[i].objectCount; j++)
		{
			obj = &m_groups[i].objects[j];
			DeleteArray(obj->frames);
			DeleteArray(obj->materialBlocks);
			DeleteArray(obj->materials);
			DeleteArray(obj->vertices);
			DeleteArray(obj->indices);
			DeleteArray(obj->vertexList);
			DeleteArray(obj->physiqueVertices);
			Release(obj->VB);
			Release(obj->IB);
		}
	}

	Release(m_collObj.VB);
	Release(m_collObj.IB);
	DeleteArray(m_collObj.materialBlocks);
	DeleteArray(m_collObj.materials);
	DeleteArray(m_collObj.vertices);
	DeleteArray(m_collObj.indices);
	DeleteArray(m_collObj.frames);
	DeleteArray(m_collObj.vertexList);
	DeleteArray(m_collObj.physiqueVertices);

	DeleteArray(m_attributes);
	DeleteArray(m_groups[0].objects);
	DeleteArray(m_groups[0].updates);

	DeleteArray(m_baseBones);
	Delete(m_motion);
}

void CObject3D::Render(const D3DXMATRIX* world, int lod, float currentFrame, int nextFrame, int textureEx, int alpha)
{
	m_group = m_LOD ? &m_groups[lod] : &m_groups[0];

	D3DXMATRIX mat;
	int i;

	if (textureEx != m_group->currentTextureEx)
		_setTextureEx(textureEx);

	if (m_group->updates)
		_animate(currentFrame, nextFrame);

	if (m_boneCount && m_baseBones)
	{
		if (m_motion)
			m_motion->Animate(m_baseBones, currentFrame, nextFrame);

		if (m_sendVS)
		{
			for (i = 0; i < m_boneCount; i++)
			{
				mat = m_baseInvBones[i] * m_baseBones[i];
				D3DXMatrixTranspose(&mat, &mat);
				m_device->SetVertexShaderConstantF(i * 3, (float*)&mat, 3);
			}
		}

		mat = *world * g_global3D.view * g_global3D.proj;
		D3DXMatrixTranspose(&mat, &mat);
		m_device->SetVertexShaderConstantF(84, (float*)&mat, 4);

		D3DXVECTOR4 vLight;
		D3DXMatrixInverse(&mat, null, world);
		D3DXVec4Transform(&vLight, &g_global3D.lightVec, &mat);
		D3DXVec4Normalize(&vLight, &vLight);
		m_device->SetVertexShaderConstantF(92, (float*)&vLight, 1);
	}

	for (i = 0; i < m_group->objectCount; i++)
	{
		const GMObject& obj = m_group->objects[i];

		if (obj.type == GMT_SKIN)
			_renderSkin(obj, world, alpha);
		else
		{
			if (!g_global3D.night && obj.light)
				continue;

			mat = m_group->updates[i] * *world;
			_renderNormal(obj, &mat, alpha);
		}
	}
}

void CObject3D::_setTextureEx(int textureEx)
{
	int j;
	Material* mat;
	char buffer[128], buffer2[6], ext[5];
	sprintf(buffer2, "-et%02d", textureEx);

	for (int i = 0; i < m_group->objectCount; i++)
	{
		if (m_group->objects[i].material)
		{
			const int matCount = m_group->objects[i].materialCount;
			for (j = 0; j < matCount; j++)
			{
				mat = &m_group->objects[i].materials[j];
				const char* textureName = mat->textureName;
				const int len = strlen(textureName);

				if (len > 0 && !mat->textures[textureEx])
				{
					if (textureEx > 0)
					{
						strcpy(ext, textureName + (len - 4));
						strncpy(buffer, textureName, len - 4);
						buffer[len - 4] = '\0';
						strcat(buffer, buffer2);
						strcat(buffer, ext);

						mat->textures[textureEx] = TextureMng->GetModelTexture(buffer);
					}
					else
					{
						mat->textures[textureEx] = TextureMng->GetModelTexture(textureName);
					}
				}
			}
		}
	}

	m_group->currentTextureEx = textureEx;
}

void CObject3D::_animate(float currentFrame_, int nextFrame)
{
	TMAnimation* frame = null;
	TMAnimation* next = null;
	D3DXQUATERNION slerp;
	D3DXVECTOR3 lerp;
	D3DXMATRIX	m1, m2;
	D3DXMATRIX* updates = m_group->updates;
	D3DXMATRIX* parent;
	GMObject* obj;

	const int currentFrame = (int)currentFrame_;
	const float slp = currentFrame_ - (float)currentFrame;

	for (int i = 0; i < m_group->objectCount; i++)
	{
		obj = &m_group->objects[i];
		if (obj->parentType == GMT_BONE)
		{
			if (m_externBones)
				parent = m_externBones;
			else
				parent = m_baseBones;
		}
		else
			parent = updates;

		if (m_frameCount > 0)
		{
			if (obj->frames)
			{
				frame = &obj->frames[currentFrame];
				next = &obj->frames[nextFrame];

				D3DXQuaternionSlerp(&slerp, &frame->rot, &next->rot, slp);
				D3DXVec3Lerp(&lerp, &frame->pos, &next->pos, slp);
				D3DXMatrixTranslation(&m1, lerp.x, lerp.y, lerp.z);
				D3DXMatrixRotationQuaternion(&m2, &slerp);
				updates[i] = m2 * m1;

				if (obj->parentID != -1)
					updates[i] *= parent[obj->parentID];
			}
			else
			{
				if (obj->parentID != -1)
					updates[i] = obj->transform * parent[obj->parentID];
				else
					updates[i] = obj->transform;
			}
		}
		else
		{
			if (obj->parentID != -1)
				updates[i] = obj->transform * parent[obj->parentID];
			else
				updates[i] = obj->transform;
		}
	}
}

void CObject3D::_renderSkin(const GMObject& obj, const D3DXMATRIX* world, int alpha)
{
	D3DXMATRIX* bones, *invBones;
	D3DXMATRIX mat;
	int i, nID, j;

	if (m_externBones)
	{
		bones = m_externBones;
		invBones = m_externInvBones;
	}
	else
	{
		bones = m_baseBones;
		invBones = m_baseInvBones;
	}

	if (obj.usedBoneCount)
	{
		for (i = 0; i < obj.usedBoneCount; i++)
		{
			nID = obj.usedBones[i];
			mat = invBones[nID] * bones[nID];
			D3DXMatrixTranspose(&mat, &mat);
			m_device->SetVertexShaderConstantF(i * 3, (float*)&mat, 3);
		}
	}

	m_device->SetVertexDeclaration(s_skinVertexDeclaration);
	m_device->SetVertexShader(s_skinVS);
	m_device->SetStreamSource(0, obj.VB, 0, sizeof(SkinVertex));
	m_device->SetIndices(obj.IB);
	m_device->SetTransform(D3DTS_WORLD, world);

	for (i = 0; i < obj.materialBlockCount; i++)
	{
		const MaterialBlock& block = obj.materialBlocks[i];

		if (block.usedBoneCount)
		{
			for (j = 0; j < block.usedBoneCount; j++)
			{
				nID = block.usedBones[j];
				mat = invBones[nID] * bones[nID];
				D3DXMatrixTranspose(&mat, &mat);
				m_device->SetVertexShaderConstantF(j * 3, (float*)&mat, 3);
			}
		}

		_setState(block, alpha);

		if (obj.material)
		{
			const CTexture* texture = obj.materials[block.materialID].textures[m_group->currentTextureEx];
			if (texture)
				m_device->SetTexture(0, *texture);
			else
				m_device->SetTexture(0, null);
		}
		else
			m_device->SetTexture(0, null);

		m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, obj.vertexCount, block.startVertex, block.primitiveCount);

		_resetState(block, alpha);
	}

	m_device->SetVertexShader(null);
	m_device->SetVertexDeclaration(null);
}

void CObject3D::_renderNormal(const GMObject& obj, const D3DXMATRIX* world, int alpha)
{
	m_device->SetFVF(NormalVertex::FVF);
	m_device->SetStreamSource(0, obj.VB, 0, sizeof(NormalVertex));
	m_device->SetIndices(obj.IB);
	m_device->SetTransform(D3DTS_WORLD, world);

	for (int i = 0; i < obj.materialBlockCount; i++)
	{
		const MaterialBlock& block = obj.materialBlocks[i];

		if (obj.light)
		{
			if (g_global3D.obj3Deffects)
			{
				if (g_global3D.light)
					m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

				m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			}
		}
		else
			_setState(block, alpha);

		if (obj.material)
		{
			const CTexture* texture = obj.materials[block.materialID].textures[m_group->currentTextureEx];
			if (texture)
				m_device->SetTexture(0, *texture);
			else
				m_device->SetTexture(0, null);
		}
		else
			m_device->SetTexture(0, null);

		m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, obj.vertexCount, block.startVertex, block.primitiveCount);

		if (obj.light)
		{
			if (g_global3D.obj3Deffects)
			{
				if (g_global3D.light)
					m_device->SetRenderState(D3DRS_LIGHTING, TRUE);

				m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
		}
		else
			_resetState(block, alpha);
	}
}

void CObject3D::_setState(const MaterialBlock& block, int alpha)
{
	const int effect = block.effect;

	if (effect & XE_2SIDE)
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	if (g_global3D.obj3Deffects)
	{
		if (effect & XE_SELF_ILLUMINATE && g_global3D.light)
			m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

		if (effect & XE_HIGHLIGHT_OBJ)
		{
			if (g_global3D.light)
				m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
			if (!(effect & XE_2SIDE))
				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

			m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		}

		if (effect & XE_REFLECT)
		{
			m_device->SetTexture(1, s_reflectTexture);
			const D3DXMATRIX reflect(0.50f, 0.00f, 0.00f, 0.00f,
				0.00f, -0.50f, 0.00f, 0.00f,
				0.00f, 0.00f, 1.00f, 0.00f,
				0.50f, 0.50f, 0.00f, 1.00f);
			m_device->SetTransform(D3DTS_TEXTURE1, &reflect);
			m_device->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
			m_device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
			m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED2X);
		}

		if (effect & XE_OPACITY)
		{
			m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			if (block.amount != 255)
			{
				m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				m_device->SetRenderState(D3DRS_ALPHAREF, 0);
				m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(block.amount, 0, 0, 0));
			}
		}
	}

	if (alpha < 255)
	{
		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ALPHAREF, 0);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		if (g_global3D.obj3Deffects && (effect & XE_OPACITY) && block.amount != 255)
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB((alpha + block.amount) / 2, 0, 0, 0));
		else
			m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, 0, 0, 0));
	}
}

void CObject3D::_resetState(const MaterialBlock& block, int alpha)
{
	const int effect = block.effect;

	if (effect & XE_2SIDE)
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	if (g_global3D.obj3Deffects)
	{
		if (effect & XE_SELF_ILLUMINATE && g_global3D.light)
			m_device->SetRenderState(D3DRS_LIGHTING, TRUE);

		if (effect & XE_HIGHLIGHT_OBJ)
		{
			if (g_global3D.light)
				m_device->SetRenderState(D3DRS_LIGHTING, TRUE);
			if (!(effect & XE_2SIDE))
				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

			m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		if (effect & XE_REFLECT)
		{
			m_device->SetTexture(1, null);
			D3DXMATRIX identity;
			D3DXMatrixIdentity(&identity);
			m_device->SetTransform(D3DTS_TEXTURE1, &identity);
			m_device->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			m_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		}

		if (effect & XE_OPACITY)
		{
			m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			if (block.amount != 255)
			{
				m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				m_device->SetRenderState(D3DRS_ALPHAREF, 0xb0);
				m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
			}
		}
	}

	if (alpha < 255)
	{
		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ALPHAREF, 0xb0);
		m_device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(255, 0, 0, 0));
	}
}

bool CObject3D::InitDeviceObjects()
{
	GMObject* obj;
	int vertexSize, bufferSize, i, j, poolSize = 0;
	uint FVF;
	bool normalObj = false;
	void* data;

	for (i = 0; i < (m_LOD ? MAX_GROUP : 1); i++)
	{
		for (j = 0; j < m_groups[i].objectCount; j++)
		{
			obj = &m_groups[i].objects[j];
			poolSize++;

			if (obj->type == GMT_SKIN)
			{
				vertexSize = sizeof(SkinVertex);
				FVF = SkinVertex::FVF;

				m_hasSkin = true;
			}
			else
			{
				vertexSize = sizeof(NormalVertex);
				FVF = NormalVertex::FVF;
				normalObj = true;
			}

			bufferSize = vertexSize * obj->vertexCount;
			if (SUCCEEDED(m_device->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY, FVF, D3DPOOL_MANAGED, &obj->VB, NULL)))
			{
				if (SUCCEEDED(obj->VB->Lock(0, bufferSize, &data, 0)))
				{
					memcpy(data, obj->vertices, bufferSize);
					obj->VB->Unlock();
				}
				else
					return false;
			}
			else
				return false;

			bufferSize = sizeof(ushort) * obj->indexCount;
			if (SUCCEEDED(m_device->CreateIndexBuffer(bufferSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &obj->IB, NULL)))
			{
				if (SUCCEEDED(obj->IB->Lock(0, bufferSize, &data, 0)))
				{
					memcpy(data, obj->indices, bufferSize);
					obj->IB->Unlock();
				}
				else
					return false;
			}
			else
				return false;
		}
	}

	if (normalObj)
	{
		D3DXMATRIX* matPool = new D3DXMATRIX[poolSize];
		for (i = 0; i < (m_LOD ? MAX_GROUP : 1); i++)
		{
			m_groups[i].updates = matPool;
			matPool += m_groups[i].objectCount;
			for (j = 0; j < m_groups[i].objectCount; j++)
				D3DXMatrixIdentity(&m_groups[i].updates[j]);
		}
	}

#if defined(WORLD_EDITOR) || defined(MODEL_EDITOR)
	if (m_collObj.type != GMT_ERROR
		&& m_collObj.vertexCount > 0
		&& m_collObj.indexCount > 0)
	{
		bufferSize = sizeof(CollisionVertex) * m_collObj.vertexCount;
		if (SUCCEEDED(m_device->CreateVertexBuffer(bufferSize, D3DUSAGE_WRITEONLY, CollisionVertex::FVF, D3DPOOL_MANAGED, &m_collObj.VB, null)))
		{
			CollisionVertex* VB;
			if (SUCCEEDED(m_collObj.VB->Lock(0, bufferSize, (void**)&VB, 0)))
			{
				if (m_collObj.type == GMT_NORMAL)
				{
					NormalVertex* vertices = (NormalVertex*)m_collObj.vertices;
					for (i = 0; i < m_collObj.vertexCount; i++)
					{
						VB[i].p = vertices[i].p;
						VB[i].c = 0xffffffff;
					}
				}
				else
				{
					SkinVertex* vertices = (SkinVertex*)m_collObj.vertices;
					for (i = 0; i < m_collObj.vertexCount; i++)
					{
						VB[i].p = vertices[i].p;
						VB[i].c = 0xffffffff;
					}
				}
				m_collObj.VB->Unlock();
			}
			else
				return false;
		}
		else
			return false;

		bufferSize = sizeof(ushort) * m_collObj.indexCount;
		if (SUCCEEDED(m_device->CreateIndexBuffer(bufferSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_collObj.IB, null)))
		{
			if (SUCCEEDED(m_collObj.IB->Lock(0, bufferSize, &data, 0)))
			{
				memcpy(data, m_collObj.indices, bufferSize);
				m_collObj.IB->Unlock();
			}
			else
				return false;
		}
		else
			return false;
	}
#endif // defined(WORLD_EDITOR) || defined(MODEL_EDITOR)

	return true;
}

bool CObject3D::Load(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::ReadOnly))
		return false;

	char name[260];
	char nameLen;
	file.Read(nameLen);
	file.Read(name, nameLen);
	for (int i = 0; i < nameLen; i++)
		name[i] = name[i] ^ (char)0xcd;

	if (nameLen > 64)
	{
		qCritical(("Internal filename too long in '" + filename + "'").toLocal8Bit());
		file.Close();
		return false;
	}

	name[nameLen] = '\0';
	if (strcmpi(QFileInfo(filename).fileName().toLocal8Bit().data(), name) != 0)
	{
		qCritical(("External filename isn't correct in '" + filename + "'").toLocal8Bit());
		file.Close();
		return false;
	}

	int ver;
	file.Read(ver);
	if (ver < VER_MESH)
	{
		qCritical(("Object3D version no more supported in '" + filename + "'").toLocal8Bit());
		file.Close();
		return false;
	}

	file.Read(m_ID);
	file.Read(m_forces[0]);
	file.Read(m_forces[1]);
	if (ver >= 22)
	{
		file.Read(m_forces[2]);
		file.Read(m_forces[3]);
	}
	file.Read(m_scrlU);
	file.Read(m_scrlV);
	file.Skip(16);

	file.Read(m_bounds);
	file.Read(m_perSlerp);
	file.Read(m_frameCount);

	file.Read(m_eventCount);
	if (m_eventCount > 0)
		file.Read(m_events, m_eventCount);

	int temp;
	file.Read(temp);
	if (temp)
	{
		m_collObj.type = GMT_NORMAL;
		ReadGMObject(file, m_collObj);
	}

	file.Read(temp);
	m_LOD = temp != 0;

	file.Read(m_boneCount);
	if (m_boneCount > 0)
	{
		m_baseBones = new D3DXMATRIX[m_boneCount * 2];
		m_baseInvBones = m_baseBones + m_boneCount;
		file.Read(m_baseBones, m_boneCount);
		file.Read(m_baseInvBones, m_boneCount);

		if (m_frameCount > 0)
		{
			m_motion = new CMotion();
			m_motion->ReadTM(file, m_boneCount, m_frameCount);
		}

		file.Read(temp);
		m_sendVS = temp != 0;
	}

	const int groupCount = m_LOD ? MAX_GROUP : 1;
	int poolSize, debugSize = 0, j, type;
	LODGroup* group = null;
	file.Read(poolSize);
	GMObject* pool = new GMObject[poolSize];
	memset(pool, 0, sizeof(GMObject) * poolSize);
	GMObject* obj;

	for (int i = 0; i < groupCount; i++)
	{
		group = &m_groups[i];
		file.Read(group->objectCount);
		group->objects = pool;
		pool += group->objectCount;
		debugSize += group->objectCount;

		if (debugSize > poolSize)
			qWarning(("LoadObject debug " + QString::number(debugSize) + " and pool "  + QString::number(poolSize) + " size problem").toLocal8Bit());

		for (j = 0; j < group->objectCount; j++)
			group->objects[j].ID = -1;

		for (j = 0; j < group->objectCount; j++)
		{
			obj = &group->objects[j];

			file.Read(type);
			obj->type = (EGMType)(type & 0xffff);
			if (type & 0x80000000)
				obj->light = true;

			file.Read(obj->usedBoneCount);
			if (obj->usedBoneCount > 0)
				file.Read(obj->usedBones, obj->usedBoneCount);

			file.Read(obj->ID);
			file.Read(obj->parentID);

			if (obj->parentID != -1)
				file.Read(obj->parentType);

			file.Read(obj->transform);
			ReadGMObject(file, *obj);

			if (obj->type == GMT_NORMAL && m_frameCount > 0)
			{
				int frame;
				file.Read(frame);
				if (frame)
				{
					obj->frames = new TMAnimation[m_frameCount];
					file.Read(obj->frames, m_frameCount);
				}
			}
		}
	}

	if (ver >= 21)
	{
		int attr;
		file.Read(attr);
		if (attr == m_frameCount)
		{
			if (m_frameCount > 0)
			{
				m_attributes = new MotionAttribute[m_frameCount];
				file.Read(m_attributes, m_frameCount);
			}
		}
		else if (m_frameCount > 0)
		{
			m_attributes = new MotionAttribute[m_frameCount];
			memset(m_attributes, 0, sizeof(MotionAttribute) * m_frameCount);
		}
	}
	else if (m_frameCount > 0)
	{
		m_attributes = new MotionAttribute[m_frameCount];
		memset(m_attributes, 0, sizeof(MotionAttribute) * m_frameCount);
	}

	file.Close();
	return true;
}

bool CObject3D::Save(const string& filename)
{
	CFile file;
	if (!file.Open(filename, QIODevice::WriteOnly))
		return false;

	const QByteArray name = QFileInfo(filename).fileName().toLocal8Bit();

	file.Write((char)name.size());
	for (int i = 0; i < name.size(); i++)
		file.Write((char)(name[i] ^ (char)0xcd));

	file.Write(22);
	file.Write(m_ID);
	file.Write(m_forces, 4);
	file.Write(m_scrlU);
	file.Write(m_scrlV);

	char unused[16];
	memset(unused, 0, 16);
	file.Write(unused, 16);

	file.Write(m_bounds);
	file.Write(m_perSlerp);
	file.Write(m_frameCount);

	file.Write(m_eventCount);
	if (m_eventCount > 0)
		file.Write(m_events, m_eventCount);

	int temp = (m_collObj.type != GMT_ERROR) ? 1 : 0;
	file.Write(temp);
	if (temp)
		SaveGMObject(file, m_collObj);

	temp = m_LOD ? 1 : 0;
	file.Write(temp);

	file.Write(m_boneCount);
	if (m_boneCount > 0)
	{
		file.Write(m_baseBones, m_boneCount);
		file.Write(m_baseInvBones, m_boneCount);

		if (m_frameCount > 0)
			m_motion->SaveTM(file);

		temp = m_sendVS ? 1 : 0;
		file.Write(temp);
	}

	int poolSize = 0;
	for (int i = 0; i < (m_LOD ? MAX_GROUP : 1); i++)
		poolSize += m_groups[i].objectCount;

	file.Write(poolSize);

	LODGroup* group;
	GMObject* obj;
	int type;
	for (int i = 0; i < (m_LOD ? MAX_GROUP : 1); i++)
	{
		group = &m_groups[i];
		file.Write(group->objectCount);

		for (int j = 0; j < group->objectCount; j++)
		{
			obj = &group->objects[j];

			type = obj->type & 0xffff;
			if (obj->light)
				type |= 0x80000000;

			file.Write(type);

			file.Write(obj->usedBoneCount);
			if (obj->usedBoneCount > 0)
				file.Write(obj->usedBones, obj->usedBoneCount);

			file.Write(obj->ID);
			file.Write(obj->parentID);

			if (obj->parentID != -1)
				file.Write(obj->parentType);

			file.Write(obj->transform);
			SaveGMObject(file, *obj);

			if (obj->type == GMT_NORMAL && m_frameCount > 0)
			{
				const int frame = (obj->frames != null) ? 1 : 0;
				file.Write(frame);
				if (frame)
					file.Write(obj->frames, m_frameCount);
			}
		}
	}

	temp = (m_frameCount > 0) ? m_frameCount : 0;
	file.Write(temp);
	if (temp)
		file.Write(m_attributes, m_frameCount);

	file.Close();
	return true;
}

void CObject3D::ReadGMObject(CFile& file, GMObject& obj)
{
	file.Read(obj.bounds);

	int temp;
	file.Read(temp);
	obj.opacity = temp != 0;
	file.Read(temp);
	obj.bump = temp != 0;
	file.Read(temp);
	obj.rigid = temp != 0;

	file.Skip(28);

	file.Read(obj.vertexListCount);
	file.Read(obj.vertexCount);
	file.Read(obj.faceListCount);
	file.Read(obj.indexCount);

	obj.vertexList = new D3DXVECTOR3[obj.vertexListCount];
	file.Read(obj.vertexList, obj.vertexListCount);

	if (obj.type == GMT_SKIN)
	{
		obj.vertices = new SkinVertex[obj.vertexCount];
		file.Read((SkinVertex*)obj.vertices, obj.vertexCount);
	}
	else
	{
		obj.vertices = new NormalVertex[obj.vertexCount];
		file.Read((NormalVertex*)obj.vertices, obj.vertexCount);
	}

	obj.indices = new ushort[obj.indexCount + obj.vertexCount];
	obj.IIB = obj.indices + obj.indexCount;

	file.Read(obj.indices, obj.indexCount);
	file.Read(obj.IIB, obj.vertexCount);

	file.Read(temp);
	if (temp)
	{
		obj.physiqueVertices = new int[obj.vertexListCount];
		file.Read(obj.physiqueVertices, obj.vertexListCount);
	}

	file.Read(temp);
	obj.material = temp != 0;
	if (obj.material)
	{
		file.Read(obj.materialCount);

		if (obj.materialCount == 0)
			obj.materialCount = 1;

		Material* mat;
		int len;
		obj.materials = new Material[obj.materialCount];
		memset(obj.materials, 0, sizeof(Material) * obj.materialCount);
		for (int i = 0; i < obj.materialCount; i++)
		{
			mat = &obj.materials[i];
			file.Read(mat->material);
			file.Read(len);
			file.Read(mat->textureName, len);
		}
	}

	file.Read(obj.materialBlockCount);
	if (obj.materialBlockCount > 0)
	{
		obj.materialBlocks = new MaterialBlock[obj.materialBlockCount];
		file.Read(obj.materialBlocks, obj.materialBlockCount);
	}
}

void CObject3D::SaveGMObject(CFile& file, GMObject& obj)
{
	file.Write(obj.bounds);

	int temp = obj.opacity ? 1 : 0;
	file.Write(temp);
	temp = obj.bump ? 1 : 0;
	file.Write(temp);
	temp = obj.rigid ? 1 : 0;
	file.Write(temp);

	char unused[28];
	memset(unused, 0, 28);
	file.Write(unused, 28);

	file.Write(obj.vertexListCount);
	file.Write(obj.vertexCount);
	file.Write(obj.faceListCount);
	file.Write(obj.indexCount);

	file.Write(obj.vertexList, obj.vertexListCount);

	if (obj.type == GMT_SKIN)
		file.Write((SkinVertex*)obj.vertices, obj.vertexCount);
	else
		file.Write((NormalVertex*)obj.vertices, obj.vertexCount);

	file.Write(obj.indices, obj.indexCount);
	file.Write(obj.IIB, obj.vertexCount);

	temp = (obj.physiqueVertices != null) ? 1 : 0;
	file.Write(temp);
	if (temp)
		file.Write(obj.physiqueVertices, obj.vertexListCount);

	temp = obj.material ? 1 : 0;
	file.Write(temp);
	if (temp)
	{
		file.Write(obj.materialCount);

		Material* mat;
		for (int i = 0; i < obj.materialCount; i++)
		{
			mat = &obj.materials[i];
			file.Write(mat->material);

			const int len = strlen(mat->textureName);
			file.Write(len + 1);
			file.Write(mat->textureName, len);
			file.Write('\0');
		}
	}

	file.Write(obj.materialBlockCount);
	if (obj.materialBlockCount > 0)
		file.Write(obj.materialBlocks, obj.materialBlockCount);
}

int CObject3D::GetFrameCount() const
{
	return m_frameCount;
}

MotionAttribute* CObject3D::GetAttributes() const
{
	return m_attributes;
}

const Bounds& CObject3D::GetBounds() const
{
	return m_bounds;
}

void CObject3D::RenderCollision(const D3DXMATRIX* world)
{
#if defined(WORLD_EDITOR) || defined(MODEL_EDITOR)
	if (m_collObj.type == GMT_ERROR || !m_collObj.VB)
		return;

	m_device->SetFVF(CollisionVertex::FVF);
	m_device->SetStreamSource(0, m_collObj.VB, 0, sizeof(CollisionVertex));
	m_device->SetIndices(m_collObj.IB);
	m_device->SetTransform(D3DTS_WORLD, world);
	m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_collObj.vertexCount, 0, m_collObj.indexCount / 3);
#endif // defined(WORLD_EDITOR) || defined(MODEL_EDITOR)
}

bool CObject3D::Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist)
{
#ifdef WORLD_EDITOR
	float tempDist;
	bool pick = false;
	dist = 65535.0f;
	D3DXMATRIX invTM, TM;
	D3DXVECTOR3 invOrigin, invDir;
	GMObject* obj;
	ushort* IB, *IIB;
	int faceCount, i, j, boneID;
	D3DXMATRIX* bones, *invBones;

	for (i = 0; i < m_groups[0].objectCount; i++)
	{
		obj = &m_groups[0].objects[i];
		if (obj->type == GMT_SKIN)
		{
			D3DXMatrixInverse(&invTM, null, &world);
			D3DXVec3TransformCoord(&invOrigin, &origin, &invTM);
			invTM._41 = 0;	invTM._42 = 0;	invTM._43 = 0;
			D3DXVec3TransformCoord(&invDir, &dir, &invTM);

			const SkinVertex* VB = (SkinVertex*)obj->vertices;
			IB = obj->indices;
			faceCount = obj->faceListCount;
			IIB = obj->IIB;

			if (m_externBones)
			{
				bones = m_externBones;
				invBones = m_externInvBones;
			}
			else
			{
				bones = m_baseBones;
				invBones = m_baseInvBones;
			}

			const int vertexCount = obj->vertexCount;
			if (vertexCount > 4096)
				continue;

			const int* physique = obj->physiqueVertices;
			static D3DXVECTOR3 temp[4096];

			for (j = 0; j < vertexCount; j++)
			{
				boneID = physique[*IIB++];
				TM = invBones[boneID] * bones[boneID];
				D3DXVec3TransformCoord(&temp[j], &VB[j].p, &TM);
			}

			for (j = 0; j < faceCount; j++)
			{
				const D3DXVECTOR3& v1 = temp[*IB++];
				const D3DXVECTOR3& v2 = temp[*IB++];
				const D3DXVECTOR3& v3 = temp[*IB++];

				if (IntersectTriangle(v1, v2, v3, invOrigin, invDir, tempDist) && tempDist < dist)
				{
					pick = true;
					dist = tempDist;
				}
			}
		}
		else
		{
			if (m_groups[0].updates)
			{
				TM = m_groups[0].updates[i] * world;
				D3DXMatrixInverse(&invTM, null, &TM);
				D3DXVec3TransformCoord(&invOrigin, &origin, &invTM);
				invTM._41 = 0;	invTM._42 = 0;	invTM._43 = 0;
				D3DXVec3TransformCoord(&invDir, &dir, &invTM);

				const NormalVertex* VB = (NormalVertex*)obj->vertices;
				IB = obj->indices;
				faceCount = obj->faceListCount;

				for (j = 0; j < faceCount; j++)
				{
					const D3DXVECTOR3& v1 = VB[*IB++].p;
					const D3DXVECTOR3& v2 = VB[*IB++].p;
					const D3DXVECTOR3& v3 = VB[*IB++].p;

					if (IntersectTriangle(v1, v2, v3, invOrigin, invDir, tempDist) && tempDist < dist)
					{
						pick = true;
						dist = tempDist;
					}
				}
			}
		}
	}

	return pick;
#else // WORLD_EDITOR
	return false;
#endif // WORLD_EDITOR
}