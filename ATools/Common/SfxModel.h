///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SFXMODEL_H
#define SFXMODEL_H

#include "Model.h"

class CSfx;
class CSfxPartParticle;

struct Particle
{
	ushort frame;
	D3DXVECTOR3 pos;
	D3DXVECTOR3 speed;
	D3DXVECTOR3 scale;
	D3DXVECTOR3 rotation;
	D3DXVECTOR3 scaleStart;
	D3DXVECTOR3 scaleEnd;
	D3DXVECTOR3 scaleSpeed;
	bool swScal;
};

class CSfxModel : public CModel
{
public:
	CSfxModel(LPDIRECT3DDEVICE9 device);
	virtual ~CSfxModel();

	virtual bool Load(const string& filename);
	virtual void MoveFrame();
	virtual void Render(const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale);
	virtual void SetFrame(float currentFrame);

private:
	CSfx* m_sfx;
	CPtrArray<CPtrArray<Particle>> m_particles;

	bool _nextFrame();
	void _renderParticles(CPtrArray<Particle>* particles, const CSfxPartParticle* part, const D3DXVECTOR3& pos, const D3DXVECTOR3& angle, const D3DXVECTOR3& scale);

public:
	static bool InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device);
	static void DeleteStaticDeviceObjects();

private:
	static LPDIRECT3DVERTEXBUFFER9 s_VB;

#ifdef SFX_EDITOR
	SFX_EDITOR_FRIENDS
#endif // SFX_EDITOR
};

#endif // SFXMODEL_H