///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef WORLDPROPERTIES_H
#define	WORLDPROPERTIES_H

#include "ui_WorldProperties.h"

class CWorld;

class CDialogWorldProperties : public QDialog
{
	Q_OBJECT

public:
	CDialogWorldProperties(CWorld* world, QWidget *parent = 0);

	public slots:
	void SetCanFly(bool canFly);
	void SetBGM(int index);
	void SetModePK(int index);
	void SetWorldRevivalID(int index);
	void SetRevivalPlace();
	void SetFogStart(double value);
	void SetFogEnd(double value);
	void SetFogDensity(double value);
	void SetSkyTexture1();
	void SetSkyTexture2();
	void SetSkyTexture3();
	void SetMoonTexture();
	void SetSunTexture();
	void SetCloudTexture1();
	void SetCloudTexture2();
	void SetCloudTexture3();
	void SetSeacloudTexture();
	void SetInDoor(bool inDoor);
	void SetAmbient();
	void SetDiffuse();
	void SetLightDirX(double value);
	void SetLightDirY(double value);
	void SetLightDirZ(double value);
	void ResetProperties();

private:
	Ui::WorldPropertiesDialog ui;
	CWorld* m_world;
	bool m_canFly;
	int m_bgmID;
	int m_modePK;
	int m_worldRevivalID;
	string m_revivalPlace;
	float m_fogStart, m_fogEnd, m_fogDensity;
	string m_skyTextureNames[3];
	string m_sunTextureName;
	string m_moonTextureName;
	string m_cloudTextureNames[3];
	string m_seaCloudTextureName;
	bool m_inDoor;
	DWORD m_ambient;
	DWORD m_diffuse;
	D3DXVECTOR3 m_lightDir;
};

#endif // WORLDPROPERTIES_H