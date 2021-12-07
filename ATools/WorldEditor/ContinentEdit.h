///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef CONTINENTEDIT_H
#define CONTINENTEDIT_H

#include "ui_ContinentEdit.h"

class CWorld;

class CDialogContinentEdit : public QDialog
{
	Q_OBJECT

public:
	CDialogContinentEdit(CWorld* world, QWidget *parent = 0);

	void UpdateVertexList();

	public slots:
	void SelectContinent(int index);
	void NewContinent();
	void DeleteContinent();
	void TeleportToVertex(QListWidgetItem* item);
	void SetName();
	void SetID(int id);
	void SetTown(bool town);
	void UseEnvir(bool use);
	void SetWeather(int index);
	void SetFogStart(double value);
	void SetFogEnd(double value);
	void SetSkyTexture();
	void SetCloudTexture();
	void SetSunTexture();
	void SetMoonTexture();
	void SetAmbient();
	void SetDiffuse();

private:
	Ui::ContinentEditDialog ui;
	CWorld* m_world;

	void _updateContinentList();
};

#endif // CONTINENTEDIT_H