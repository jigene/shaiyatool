///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef TEXTUREMNG_H
#define TEXTUREMNG_H

class CTexture
{
public:
	CTexture();
	CTexture(LPDIRECT3DDEVICE9 device);
	~CTexture();

	void SetDevice(LPDIRECT3DDEVICE9 device);
	ulong Release();

	bool Load(const string& filename,
		uint mipLevels = 1,
		D3DCOLOR colorKey = 0,
		bool powerOfTwo = true);

	operator LPDIRECT3DTEXTURE9() const {
		return m_texture;
	}

	LPDIRECT3DTEXTURE9 GetTexture() const {
		return m_texture;
	}

	uint GetWidth() const {
		return m_width;
	}

	uint GetHeight() const {
		return m_height;
	}

private:
	LPDIRECT3DDEVICE9 m_device;
	LPDIRECT3DTEXTURE9 m_texture;
	uint m_width;
	uint m_height;
};

class CTextureMng
{
public:
	static CTextureMng* Instance;

	CTextureMng(LPDIRECT3DDEVICE9 device);
	~CTextureMng();

	CTexture* GetGUITexture(const string& filename);
	void SetModelTexturePath(const string& path);
	CTexture* GetModelTexture(const string& filename);
	void SetSfxTexturePath(const string& path);
	string GetSfxTexturePath() const;
	CTexture* GetSfxTexture(const string& filename);
	CTexture* GetTerrainTexture(const string& filename);
	CTexture* GetWeatherTexture(const string& filename);

private:
	LPDIRECT3DDEVICE9 m_device;
	string m_modelTexturePath;
	string m_sfxTexturePath;
	QMap<string, CTexture*> m_guiTextures;
	QMap<string, CTexture*> m_modelTextures;
	QMap<string, CTexture*> m_sfxTextures;
	QMap<string, CTexture*> m_terrainTextures;
	QMap<string, CTexture*> m_weatherTextures;
};

#define TextureMng	CTextureMng::Instance

#endif // TEXTUREMNG_H