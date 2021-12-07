///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SKYBOX_H
#define SKYBOX_H

class CWorld;

class CSkybox
{
public:
	CSkybox(LPDIRECT3DDEVICE9 device, CWorld* world);
	~CSkybox();

	void LoadTextures();
	void Render();

private:
	LPDIRECT3DDEVICE9 m_device;
	CWorld* m_world;
	CTexture* m_moonTexture;
	CTexture* m_sunTexture;
	CTexture* m_topTextures[3];
	CTexture* m_cloudTextures[3];
	bool m_threeTexturesSky;
	bool m_threeTexturesCloud;
	float m_cloudPos;

public:
	static bool InitStaticDeviceObjects(LPDIRECT3DDEVICE9 device);
	static void DeleteStaticDeviceObjects();

private:
	static LPDIRECT3DVERTEXBUFFER9 s_topVB;
	static LPDIRECT3DVERTEXBUFFER9 s_cloudVB;
};

#endif // SKYBOX_H