///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WORLD_INL
#define WORLD_INL

inline void CWorld::WorldPosToLand(const D3DXVECTOR3& pos, int& x, int& z) const
{
	x = int(pos.x) / (MAP_SIZE * m_MPU);
	z = int(pos.z) / (MAP_SIZE * m_MPU);
	if (pos.x == m_width * MAP_SIZE)
		x--;
	if (pos.z == m_height * MAP_SIZE * MPU)
		z--;
}

inline bool CWorld::LandInWorld(int x, int z) const
{
	return (x >= 0) && (z >= 0) && (x < m_width) && (z < m_height);
}

inline CLandscape* CWorld::GetLand(int x, int z) const
{
	return m_lands[x + z * m_width];
}

inline CLandscape* CWorld::GetLand(const D3DXVECTOR3& v) const
{
	int mX = int(v.x / (MAP_SIZE * m_MPU));
	int mZ = int(v.z / (MAP_SIZE * m_MPU));
	if (v.x == m_width * MAP_SIZE * MPU)
		mX--;
	if (v.z == m_height * MAP_SIZE * MPU)
		mZ--;
	const int offset = mX + mZ * m_width;
	if (offset < 0 || offset >= m_width * m_height)
		return null;
	return m_lands[offset];
}

inline bool CWorld::VecInWorld(const D3DXVECTOR3& v) const
{
	return (v.x >= 0.0f) && (v.z >= 0.0f) && (v.x <= m_width * MAP_SIZE * MPU) && (v.z <= m_height * MAP_SIZE * MPU);
}

inline bool CWorld::VecInWorld(float x, float z) const
{
	return (x >= 0.0f) && (z >= 0.0f) && (x <= m_width * MAP_SIZE * MPU) && (z <= m_height * MAP_SIZE * MPU);
}

inline CTexture* CWorld::GetSeacloudTexture() const
{
	return m_seacloudTexture;
}

inline const D3DXVECTOR2& CWorld::GetSeacloudPos() const
{
	return m_seacloudPos;
}

inline const D3DXVECTOR3& CWorld::GetCameraPos() const
{
	return m_cameraPos;
}

inline int CWorld::GetWidth() const
{
	return m_width;
}

inline int CWorld::GetHeight() const
{
	return m_height;
}

#endif // WORLD_INL