///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MODEL_H
#define MODEL_H

class CModel
{
public:
	CModel(LPDIRECT3DDEVICE9 device);
	virtual bool Load(const string& filename) = 0;
	virtual void Render(const D3DXMATRIX* world, int lod, int alpha = 255);
	virtual void Render(const D3DXVECTOR3& pos, const D3DXVECTOR3& rotate, const D3DXVECTOR3& scale);
	virtual void MoveFrame();
	virtual void SetFrame(float currentFrame);
	virtual void RenderCollision(const D3DXMATRIX* world) { }
	virtual bool Pick(const D3DXMATRIX& world, const D3DXVECTOR3& origin, const D3DXVECTOR3& dir, float& dist) { return false; }

	int GetNextFrame() const;
	const Bounds& GetBounds() const;
	int GetFrameCount() const;
	float GetCurrentFrame() const;
	void SetTextureEx(int ex);
	int GetTextureEx() const;

	bool HasSkin() const {
		return m_hasSkin;
	}

protected:
	LPDIRECT3DDEVICE9 m_device;
	Bounds m_bounds;
	int m_frameCount;
	float m_currentFrame;
	int m_textureEx;
	float m_perSlerp;
	bool m_hasSkin;
};

#endif // MODEL_H