///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "DialogNewPart.h"

CDialogNewPart::CDialogNewPart(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(SetTextureName()));
}

ESfxPartType CDialogNewPart::GetSfxType() const
{
	string text = ui.comboBox->currentText();
	if (text == "Billboard")
		return ESfxPartType::Bill;
	else if (text == "Particle")
		return ESfxPartType::Particle;
	else if (text == "CustomMesh")
		return ESfxPartType::CustomMesh;
	else if (text == "Mesh")
		return ESfxPartType::Mesh;
	return ESfxPartType::Bill;
}

void CDialogNewPart::SetPart(CSfxPart* part)
{
	part->m_name = ui.lineEdit->text();
	part->m_textureName = ui.lineEdit_2->text();

	if (!part->m_textureName.isEmpty())
		part->_setTexture();

	string text = ui.comboBox_2->currentText();
	if (text == "Blend")
		part->m_alphaType = ESfxPartAlphaType::Blend;
	else if (text == "Glow")
		part->m_alphaType = ESfxPartAlphaType::Glow;

	text = ui.comboBox_3->currentText();
	if (text == "Normal")
		part->m_billType = ESfxPartBillType::Normal;
	else if (text == "Bill")
		part->m_billType = ESfxPartBillType::Bill;
	else if (text == "Bottom")
		part->m_billType = ESfxPartBillType::Bottom;
	else if (text == "Pole")
		part->m_billType = ESfxPartBillType::Pole;
}

void CDialogNewPart::SetTextureName()
{
	string oldFilename;
	if (GetExtension(ui.lineEdit_2->text()) == "o3d")
		oldFilename = "Model/";
	else
		oldFilename = "SFX/Texture/";
	oldFilename += ui.lineEdit_2->text();

	const string filename = QFileDialog::getOpenFileName(this, tr("Charger une texture/model"), oldFilename, tr("Fichier texture") + " (*.dds *.tga *.bmp);; " + tr("Fichier 3D") + " (*.o3d)");

	if (!filename.isEmpty())
		ui.lineEdit_2->setText(QFileInfo(filename).fileName());
}