///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef NEWWORLD_H
#define NEWWORLD_H

#include "ui_NewWorld.h"
#include <D3DWidget.h>

class CWorld;

class CTextureWidget : public CD3DWidget
{
	Q_OBJECT

public:
	CTextureWidget(QWidget* parent = null, Qt::WindowFlags flags = 0);
	~CTextureWidget();

	virtual bool InitDeviceObjects();
	virtual void DeleteDeviceObjects();

	virtual bool Render();
	void SetTexture(const string& filename);

private:
	CTexture* m_texture;
};

class CDialogNewWorld : public QDialog
{
	Q_OBJECT

public:
	CDialogNewWorld(QWidget *parent = 0);
	~CDialogNewWorld();

	void CreateWorld(CWorld* world);

public slots:
	void SelectTexture(const QString& texture);
	void OpenBitmap();

private:
	Ui::NewWorldDialog ui;
	CTextureWidget* m_textureWidget;
	int m_terrainTexture;
};

#endif // NEWWORLD_H