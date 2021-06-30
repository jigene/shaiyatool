///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "NewWorld.h"
#include <TextureMng.h>
#include <Project.h>
#include <World.h>

CTextureWidget::CTextureWidget(QWidget* parent, Qt::WindowFlags flags)
	: CD3DWidget(parent, flags)
{
	setMinimumSize(QSize(140, 140));
	setMaximumSize(QSize(140, 140));
}

CTextureWidget::~CTextureWidget()
{
	Destroy();
}

bool CTextureWidget::InitDeviceObjects()
{
	m_texture = new CTexture(m_device);
	return true;
}

void CTextureWidget::DeleteDeviceObjects()
{
	Delete(m_texture);
}

struct TextureVertex
{
	enum { FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 };
	D3DXVECTOR2 p;
	float z = 0.5f;
	float rhw = 1.0f;
	D3DXVECTOR2 t;
};

bool CTextureWidget::Render()
{
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 50, 50, 50), 1.0f, 0L);

	if (FAILED(m_device->BeginScene()))
		return false;

	const float left = (float)-0.5f;
	const float top = (float)-0.5f;
	const float right = left + (float)width();
	const float bottom = top + (float)height();

	TextureVertex vertices[4];
	vertices[0].p = D3DXVECTOR2(left, top);
	vertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].p = D3DXVECTOR2(left, bottom);
	vertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
	vertices[2].p = D3DXVECTOR2(right, top);
	vertices[2].t = D3DXVECTOR2(1.0f, 0.0f);
	vertices[3].p = D3DXVECTOR2(right, bottom);
	vertices[3].t = D3DXVECTOR2(1.0f, 1.0f);

	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_device->SetFVF(TextureVertex::FVF);
	m_device->SetTexture(0, *m_texture);
	m_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(TextureVertex));

	m_device->EndScene();
	return true;
}

void CTextureWidget::SetTexture(const string& filename)
{
	if (QFileInfo("World/TextureMid/" + filename).exists())
		m_texture->Load("World/TextureMid/" + filename);
	else if (QFileInfo("World/Texture/" + filename).exists())
		m_texture->Load("World/Texture/" + filename);
	else if (QFileInfo("World/TextureLow/" + filename).exists())
		m_texture->Load("World/TextureLow/" + filename);
	else
		m_texture->Release();
}

CDialogNewWorld::CDialogNewWorld(QWidget *parent)
	: QDialog(parent),
	m_terrainTexture(0)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_textureWidget = new CTextureWidget(this);
	ui.horizontalLayout->addWidget(m_textureWidget);

	if (m_textureWidget->CreateEnvironment())
		m_textureWidget->SetAutoRefresh(true);

	connect(ui.comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(SelectTexture(const QString&)));
	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(OpenBitmap()));
	Project->FillTerrainComboBox(ui.comboBox);
}

void CDialogNewWorld::SelectTexture(const QString& texture)
{
	m_textureWidget->SetTexture(texture);
	m_terrainTexture = ui.comboBox->itemData(ui.comboBox->currentIndex()).toInt();
}

void CDialogNewWorld::OpenBitmap()
{
	const string filename = QFileDialog::getOpenFileName(this, tr("Ouvrir une image"), "", tr("Fichier image") + " (*.bmp *.png)");
	ui.lineEdit->setText(filename);
}

void CDialogNewWorld::CreateWorld(CWorld* world)
{
	world->Create(ui.spinBox_2->value(), ui.spinBox_3->value(), m_terrainTexture,
		(float)ui.spinBox->value(), ui.spinBox_4->value(), ui.checkBox->isChecked(), ui.lineEdit->text());
}

CDialogNewWorld::~CDialogNewWorld()
{
	Delete(m_textureWidget);
}