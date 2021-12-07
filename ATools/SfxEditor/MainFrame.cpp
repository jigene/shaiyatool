///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "ModelViewer.h"
#include <TextureMng.h>
#include <ModelMng.h>
#include <AnimatedMesh.h>
#include <SfxModel.h>
#include <Timeline.h>
#include <Sfx.h>
#include <AboutDialog.h>
#include <ShortcutsMng.h>

CMainFrame::CMainFrame(QWidget *parent)
	: QMainWindow(parent),
	m_mesh(null),
	m_meshSex(SEX_SEXLESS),
	m_sfx(null),
	m_timeline(null),
	m_currentEditPart(-1),
	m_currentEditKey(null),
	m_language(LANG_FRE),
	m_languageActionGroup(null),
	m_modelRot(null),
	m_modelRotX(null),
	m_modelRotY(null),
	m_modelRotZ(null),
	m_modelViewer(null),
	m_status(null)
{
}

CMainFrame::~CMainFrame()
{
	Delete(m_modelRotZ);
	Delete(m_modelRotY);
	Delete(m_modelRotX);
	Delete(m_modelRot);
	Delete(m_status);
	Delete(m_timeline);
	Delete(m_mesh);
	Delete(m_sfx);
	Delete(m_modelViewer);
	Delete(m_languageActionGroup);
}

bool CMainFrame::Initialize()
{
	ui.setupUi(this);

	m_modelViewer = new CModelViewer(this);
	setCentralWidget(m_modelViewer);

	if (!m_modelViewer->CreateEnvironment())
		return false;

	m_status = new QLabel(tr("Prêt"));
	m_status->setStyleSheet("color: white;");
	ui.statusBar->addWidget(m_status);
	qApp->setStyleSheet("QStatusBar::item { border: none; }");
	
	ui.dockParticles->setFloating(true);
	ui.dockParticles->move(QPoint(30, 30));
	ui.dockParticles->hide();

	m_timeline = new CTimeline(this);
	ui.dockTimeline->setWidget(m_timeline);
	m_timeline->SetFrameCount(-1);
	m_timeline->setMinimumHeight(160);

	m_modelRot = new QLabel(tr("Rotation du model : "), ui.mainToolBar);
	m_modelRotX = new QDoubleSpinBox(ui.mainToolBar);
	m_modelRotX->setPrefix("X: ");
	m_modelRotX->setDecimals(2);
	m_modelRotX->setMaximum(999.0);
	m_modelRotX->setMinimum(-999.0);
	m_modelRotX->setSingleStep(10.0);
	m_modelRotY = new QDoubleSpinBox(ui.mainToolBar);
	m_modelRotY->setPrefix("Y: ");
	m_modelRotY->setDecimals(2);
	m_modelRotY->setMaximum(999.0);
	m_modelRotY->setMinimum(-999.0);
	m_modelRotY->setSingleStep(10.0);
	m_modelRotZ = new QDoubleSpinBox(ui.mainToolBar);
	m_modelRotZ->setPrefix("Z: ");
	m_modelRotZ->setDecimals(2);
	m_modelRotZ->setMaximum(999.0);
	m_modelRotZ->setMinimum(-999.0);
	m_modelRotZ->setSingleStep(10.0);
	ui.mainToolBar->addWidget(m_modelRot);
	ui.mainToolBar->addWidget(m_modelRotX);
	ui.mainToolBar->addWidget(m_modelRotY);
	ui.mainToolBar->addWidget(m_modelRotZ);

	connect(m_modelRotX, SIGNAL(valueChanged(double)), m_modelViewer, SLOT(SetModelRotX(double)));
	connect(m_modelRotY, SIGNAL(valueChanged(double)), m_modelViewer, SLOT(SetModelRotY(double)));
	connect(m_modelRotZ, SIGNAL(valueChanged(double)), m_modelViewer, SLOT(SetModelRotZ(double)));

	ui.menuFen_tres->addAction(ui.dockTimeline->toggleViewAction());
	ui.menuFen_tres->addAction(ui.dockParts->toggleViewAction());
	ui.menuFen_tres->addAction(ui.dockEditKey->toggleViewAction());
	ui.menuFen_tres->addAction(ui.dockEditPart->toggleViewAction());
	ui.menuFen_tres->addAction(ui.dockParticles->toggleViewAction());

	m_languageActionGroup = new QActionGroup(ui.menuLangage);
	m_languageActionGroup->addAction(ui.actionFran_ais);
	m_languageActionGroup->addAction(ui.actionEnglish);
	m_languageActionGroup->addAction(ui.actionDeutsch);

	_connectWidgets();

	OpenFile("Model/Part_femaleHair06.o3d");
	OpenFile("Model/Part_femaleHead01.o3d");
	OpenFile("Model/Part_femaleHand.o3d");
	OpenFile("Model/Part_femaleLower.o3d");
	OpenFile("Model/Part_femaleUpper.o3d");
	OpenFile("Model/Part_femaleFoot.o3d");
	OpenFile("Model/mvr_female_GenStand.ani");

	_setShortcuts();
	CloseFile();
	_loadSettings();
	return true;
}

void CMainFrame::_connectWidgets()
{
	connect(ui.actionOuvrir, SIGNAL(triggered()), this, SLOT(OpenFile()));
	connect(ui.actionFermer, SIGNAL(triggered()), this, SLOT(CloseFile()));
	connect(ui.actionFusionner, SIGNAL(triggered()), this, SLOT(MergeFile()));
	connect(ui.actionPlein_cran, SIGNAL(triggered(bool)), this, SLOT(SwitchFullscreen(bool)));
	connect(ui.actionStop, SIGNAL(triggered()), this, SLOT(Stop()));
	connect(ui.actionJouer, SIGNAL(triggered(bool)), this, SLOT(Play(bool)));
	connect(ui.action_propos, SIGNAL(triggered()), this, SLOT(About()));
	connect(ui.actionQt, SIGNAL(triggered()), this, SLOT(AboutQt()));
	connect(ui.actionGrille, SIGNAL(triggered(bool)), this, SLOT(ShowGrid(bool)));
	connect(ui.actionModel_3D, SIGNAL(triggered(bool)), m_modelViewer, SLOT(ShowModel3D(bool)));
	connect(ui.actionEnregistrer, SIGNAL(triggered()), this, SLOT(SaveFile()));
	connect(ui.actionEnregistrer_sous, SIGNAL(triggered()), this, SLOT(SaveFileAs()));
	connect(ui.actionCouleur_du_fond, SIGNAL(triggered()), this, SLOT(SetBackgroundColor()));
	connect(m_languageActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetLanguage(QAction*)));

	connect(m_timeline, SIGNAL(CurrentFrameChanged(int)), m_modelViewer, SLOT(SetFrame(int)));
	connect(m_timeline, SIGNAL(SelectionChanged(int, int)), this, SLOT(EditKey(int, int)));
	connect(m_timeline, SIGNAL(KeyModified(int, int, bool)), this, SLOT(EditKeyModified(int, int, bool)));
	connect(ui.listParts, SIGNAL(currentRowChanged(int)), this, SLOT(ListPartChanged(int)));

	connect(ui.actionAjouter_une_partie, SIGNAL(triggered()), this, SLOT(AddPart()));
	connect(ui.actionSupprimer_la_partie, SIGNAL(triggered()), this, SLOT(RemovePart()));
	connect(ui.btnRemovePart, SIGNAL(clicked()), this, SLOT(RemovePart()));
	connect(ui.btnAddPart, SIGNAL(clicked()), this, SLOT(AddPart()));

	connect(ui.editKeyPosX, SIGNAL(valueChanged(double)), this, SLOT(EditKeyPosX(double)));
	connect(ui.editKeyPosY, SIGNAL(valueChanged(double)), this, SLOT(EditKeyPosY(double)));
	connect(ui.editKeyPosZ, SIGNAL(valueChanged(double)), this, SLOT(EditKeyPosZ(double)));
	connect(ui.editKeyPosRotX, SIGNAL(valueChanged(double)), this, SLOT(EditKeyPosRotX(double)));
	connect(ui.editKeyPosRotY, SIGNAL(valueChanged(double)), this, SLOT(EditKeyPosRotY(double)));
	connect(ui.editKeyPosRotZ, SIGNAL(valueChanged(double)), this, SLOT(EditKeyPosRotZ(double)));
	connect(ui.editKeyRotX, SIGNAL(valueChanged(double)), this, SLOT(EditKeyRotX(double)));
	connect(ui.editKeyRotY, SIGNAL(valueChanged(double)), this, SLOT(EditKeyRotY(double)));
	connect(ui.editKeyRotZ, SIGNAL(valueChanged(double)), this, SLOT(EditKeyRotZ(double)));
	connect(ui.editKeyScaleX, SIGNAL(valueChanged(double)), this, SLOT(EditKeyScaleX(double)));
	connect(ui.editKeyScaleY, SIGNAL(valueChanged(double)), this, SLOT(EditKeyScaleY(double)));
	connect(ui.editKeyScaleZ, SIGNAL(valueChanged(double)), this, SLOT(EditKeyScaleZ(double)));
	connect(ui.editKeyAlpha, SIGNAL(valueChanged(int)), this, SLOT(EditKeyAlpha(int)));

	connect(ui.editPartName, SIGNAL(textEdited(const QString&)), this, SLOT(EditPartName(const QString&)));
	connect(ui.editPartVisible, SIGNAL(toggled(bool)), this, SLOT(EditPartVisible(bool)));
	connect(ui.editPartBill, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(EditPartBill(const QString&)));
	connect(ui.editPartBlend, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(EditPartAlpha(const QString&)));
	connect(ui.editPartTexture, SIGNAL(editingFinished()), this, SLOT(EditPartTextureName()));
	connect(ui.btnSetPartTexture, SIGNAL(clicked()), this, SLOT(EditPartTexture()));
	connect(ui.editPartTexFrame, SIGNAL(valueChanged(int)), this, SLOT(EditPartTexFrame(int)));
	connect(ui.editPartTexLoop, SIGNAL(valueChanged(int)), this, SLOT(EditPartTexLoop(int)));
	connect(ui.editCustomMeshPointCount, SIGNAL(valueChanged(int)), this, SLOT(EditPartPointCount(int)));
	connect(ui.editParticleCreate, SIGNAL(valueChanged(int)), this, SLOT(EditPartParticleCreate(int)));
	connect(ui.editParticleCreateNum, SIGNAL(valueChanged(int)), this, SLOT(EditPartParticleCreateNum(int)));
	connect(ui.editParticleFrameAppear, SIGNAL(valueChanged(int)), this, SLOT(EditPartParticleFrameAppear(int)));
	connect(ui.editParticleFrameKeep, SIGNAL(valueChanged(int)), this, SLOT(EditPartParticleFrameKeep(int)));
	connect(ui.editParticleFrameDisappear, SIGNAL(valueChanged(int)), this, SLOT(EditPartParticleFrameDisappear(int)));
	connect(ui.editParticleStartPosVar, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleStartPosVar(double)));
	connect(ui.editParticleStartPosVarY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleStartPosVarY(double)));
	connect(ui.editParticleYLow, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleYLow(double)));
	connect(ui.editParticleYHigh, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleYHigh(double)));
	connect(ui.editParticleXZLow, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleXZLow(double)));
	connect(ui.editParticleXZHigh, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleXZHigh(double)));
	connect(ui.editParticleAccelX, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleAccelX(double)));
	connect(ui.editParticleAccelY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleAccelY(double)));
	connect(ui.editParticleAccelZ, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleAccelZ(double)));
	connect(ui.editParticleScaleX, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleX(double)));
	connect(ui.editParticleScaleY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleY(double)));
	connect(ui.editParticleScaleZ, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleZ(double)));
	connect(ui.editParticleRotationLowX, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleRotationLowX(double)));
	connect(ui.editParticleRotationLowY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleRotationLowY(double)));
	connect(ui.editParticleRotationLowZ, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleRotationLowZ(double)));
	connect(ui.editParticleRotationHighX, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleRotationHighX(double)));
	connect(ui.editParticleRotationHighY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleRotationHighY(double)));
	connect(ui.editParticleRotationHighZ, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleRotationHighZ(double)));
	connect(ui.editParticleScalSpeedXLow, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScalSpeedXLow(double)));
	connect(ui.editParticleScalSpeedYLow, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScalSpeedYLow(double)));
	connect(ui.editParticleScalSpeedZLow, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScalSpeedZLow(double)));
	connect(ui.editParticleScalSpeedXHigh, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScalSpeedXHigh(double)));
	connect(ui.editParticleScalSpeedYHigh, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScalSpeedYHigh(double)));
	connect(ui.editParticleScalSpeedZHigh, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScalSpeedZHigh(double)));
	connect(ui.editParticleRepeatScale, SIGNAL(toggled(bool)), this, SLOT(EditPartParticleRepeatScale(bool)));
	connect(ui.editParticleRepeat, SIGNAL(toggled(bool)), this, SLOT(EditPartParticleRepeat(bool)));
	connect(ui.editParticleScaleEndX, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleEndX(double)));
	connect(ui.editParticleScaleEndY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleEndY(double)));
	connect(ui.editParticleScaleEndZ, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleEndZ(double)));
	connect(ui.editParticleScaleSpeedX, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleSpeedX(double)));
	connect(ui.editParticleScaleSpeedY, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleSpeedY(double)));
	connect(ui.editParticleScaleSpeedZ, SIGNAL(valueChanged(double)), this, SLOT(EditPartParticleScaleSpeedZ(double)));
}

void CMainFrame::_setShortcuts()
{
	CShortcutsMng mng;
	mng.Add(ui.actionQuitter, "ExitApp");
	mng.Add(ui.actionEnregistrer, "SaveFile");
	mng.Add(ui.actionEnregistrer_sous, "SaveFileAs");
	mng.Add(ui.actionPlein_cran, "Fullscreen");
	mng.Add(ui.action_propos, "About");
	mng.Add(ui.actionGrille, "ShowGrid");
	mng.Add(ui.actionFermer, "CloseFile");
	mng.Add(ui.actionOuvrir, "OpenFile");
	mng.Add(ui.actionJouer, "PlayPause");
	mng.Add(ui.actionFusionner, "MergeFiles");
	mng.Add(ui.actionAjouter_une_partie, "AddPart");
	mng.Add(ui.actionSupprimer_la_partie, "DeletePart");
	mng.Load();
}

void CMainFrame::SetBackgroundColor()
{
	const QColor color = QColorDialog::getColor(g_global3D.backgroundColor, this, tr("Couleur du fond"));
	if (color.isValid())
	{
		g_global3D.backgroundColor = D3DCOLOR_ARGB(255, color.red(), color.green(), color.blue());
		if (!m_modelViewer->IsAutoRefresh())
			m_modelViewer->RenderEnvironment();
	}
}

void CMainFrame::_loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "SfxEditor");
	if (settings.status() == QSettings::NoError)
	{
		if (settings.contains("BackgroundColor"))
			g_global3D.backgroundColor = settings.value("BackgroundColor").toUInt();
		if (settings.contains("ShowGrid"))
		{
			g_global3D.grid = settings.value("ShowGrid").toBool();
			ui.actionGrille->setChecked(g_global3D.grid);
		}
		if (settings.contains("WindowGeometry"))
		{
			restoreGeometry(settings.value("WindowGeometry").toByteArray());
			if (isFullScreen())
				ui.actionPlein_cran->setChecked(true);
		}
		if (settings.contains("WindowState"))
			restoreState(settings.value("WindowState").toByteArray());
		if (settings.contains("Language"))
		{
			switch (settings.value("Language").toInt())
			{
			case LANG_ENG:
				ui.actionEnglish->setChecked(true);
				SetLanguage(ui.actionEnglish);
				break;
			case LANG_GER:
				ui.actionDeutsch->setChecked(true);
				SetLanguage(ui.actionDeutsch);
				break;
			}
		}

		if (m_modelViewer && !m_modelViewer->IsAutoRefresh())
			m_modelViewer->RenderEnvironment();
	}
}

void CMainFrame::SetLanguage(QAction* action)
{
	if (action == ui.actionFran_ais)
	{
		m_language = LANG_FRE;
		m_translator.load("", "");
	}
	else if (action == ui.actionEnglish)
	{
		m_language = LANG_ENG;
		m_translator.load("sfxeditor_en.qm", "Plugins/languages/English");
	}
	else if (action == ui.actionDeutsch)
	{
		m_language = LANG_GER;
		m_translator.load("sfxeditor_de.qm", "Plugins/languages/Deutsch");
	}

	qApp->installTranslator(&m_translator);
	ui.retranslateUi(this);
	m_status->setText(tr("Prêt"));
	m_modelRot->setText(tr("Rotation du model : "));

	if (m_filename.isEmpty())
		setWindowTitle(tr("SfxEditor"));
	else
		setWindowTitle(tr("SfxEditor - ") + QFileInfo(m_filename).fileName());
}

void CMainFrame::closeEvent(QCloseEvent* event)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "SfxEditor");
	if (settings.isWritable())
	{
		settings.setValue("BackgroundColor", (uint)g_global3D.backgroundColor);
		settings.setValue("ShowGrid", g_global3D.grid);
		settings.setValue("WindowGeometry", saveGeometry());
		settings.setValue("WindowState", saveState());
		settings.setValue("Language", m_language);
	}

	QMainWindow::closeEvent(event);
}

void CMainFrame::OpenFile()
{
	const string filename = QFileDialog::getOpenFileName(this, tr("Charger un sfx"), m_filename.isEmpty() ? "SFX/" : m_filename, tr("Fichier SFX") + " (*.sfx);; " + tr("Fichier 3D") + " (*.o3d);; " + tr("Fichier d'animation") + " (*.ani)");

	if (!filename.isEmpty())
		OpenFile(filename);
}

void CMainFrame::MergeFile()
{
	const string filename = QFileDialog::getOpenFileName(this, tr("Fusionner avec un sfx"), m_filename, tr("Fichier SFX") + " (*.sfx)");

	if (!filename.isEmpty())
	{
		const string filenameToLower = QFileInfo(filename).fileName().toLower();
		const string dir = QFileInfo(filename).path() + '/';

		TextureMng->SetSfxTexturePath(dir + "Texture/");
		ModelMng->SetSfxPath(dir);

		CSfx* sfx = ModelMng->GetSfx(filenameToLower);

		if (sfx)
		{
			int i, j;
			CSfxPart* part, *newPart;
			SfxKeyFrame* newKey;

			const int oldPartCount = m_sfx->m_sfx->m_parts.GetSize();

			for (i = 0; i < sfx->m_parts.GetSize(); i++)
			{
				part = sfx->m_parts[i];
				newPart = m_sfx->m_sfx->AddPart(part->GetType());

				newPart->m_visible = part->m_visible;
				newPart->m_name = part->m_name;
				newPart->m_textureName = part->m_textureName;
				newPart->m_texFrame = part->m_texFrame;
				newPart->m_texLoop = part->m_texLoop;
				newPart->m_billType = part->m_billType;
				newPart->m_alphaType = part->m_alphaType;

				newPart->_setTexture();

				for (j = 0; j < part->m_keys.GetSize(); j++)
				{
					newKey = newPart->AddKey(part->m_keys[j]->frame);
					if (newKey)
						*newKey = *part->m_keys[j];
				}

				if (newPart->GetType() == ESfxPartType::Particle)
					m_sfx->m_particles.Append(new CPtrArray<Particle>());
				else
					m_sfx->m_particles.Append(null);

				const string name = MakePartName(newPart);
				m_timeline->AddRow(name);
				ui.listParts->addItem(name);

				for (j = 0; j < newPart->m_keys.GetSize(); j++)
					m_timeline->AddKey(i + oldPartCount, newPart->m_keys[j]->frame);
			}

			const float f = m_sfx->GetCurrentFrame();
			m_sfx->SetFrame(0.0f);
			m_sfx->SetFrame(f);
		}

		Delete(sfx);
	}
}

void CMainFrame::SaveFile()
{
	if (m_filename.isEmpty())
	{
		m_filename = QFileDialog::getSaveFileName(this, tr("Enregistrer le SFX"), "SFX/", tr("Fichier SFX") + " (*.sfx)");

		if (!m_filename.isEmpty())
			setWindowTitle("SfxEditor - " + QFileInfo(m_filename).fileName());
	}

	if (!m_filename.isEmpty())
		_saveFile(m_filename);
}

void CMainFrame::SaveFileAs()
{
	const string filename = QFileDialog::getSaveFileName(this, tr("Enregistrer le SFX"), "SFX/", tr("Fichier SFX") + " (*.sfx)");

	if (!filename.isEmpty())
	{
		setWindowTitle("SfxEditor - " + QFileInfo(filename).fileName());
		m_filename = filename;
		_saveFile(m_filename);
	}
}

void CMainFrame::_saveFile(const string& filename)
{
	const bool result = m_sfx->m_sfx->Save(filename);

	if (result)
		m_status->setText(tr("Fichier enregistré avec succès"));
	else
	{
		qCritical("There was a problem while saving the file");
		m_status->setText(tr("Prêt"));
	}
}

void CMainFrame::OpenFile(const string& filename)
{
	const string filenameToLower = QFileInfo(filename).fileName().toLower();
	const string dir = QFileInfo(filename).path() + '/';

	if (GetExtension(filename) == "o3d")
	{
		TextureMng->SetModelTexturePath(dir);
		ModelMng->SetModelPath(dir);

		bool isPart = true;
		if (!((filenameToLower.startsWith("part_m") && m_meshSex == SEX_MALE)
			|| (filenameToLower.startsWith("part_f") && m_meshSex == SEX_FEMALE)))
		{
			Delete(m_mesh);
			m_meshSex = SEX_SEXLESS;
			m_modelViewer->PlayMeshMotion(false);

			m_mesh = new CAnimatedMesh(m_modelViewer->GetDevice());

			if (filenameToLower.startsWith("part_m"))
				m_meshSex = SEX_MALE;
			else if (filenameToLower.startsWith("part_f"))
				m_meshSex = SEX_FEMALE;
			else
			{
				isPart = false;
				m_meshSex = SEX_SEXLESS;
			}
		}

		int part = 0;
		if (isPart)
			part = ModelMng->GetModelPart(filenameToLower);

		if (!m_mesh->Load(filenameToLower, part))
		{
			if (!isPart)
				Delete(m_mesh);
		}

		m_modelViewer->SetMesh(m_mesh);
	}
	else if (GetExtension(filename) == "ani")
	{
		if (m_mesh)
		{
			ModelMng->SetModelPath(dir);
			m_mesh->SetMotion(filenameToLower);

			m_modelViewer->PlayMeshMotion(true);

			if (!m_modelViewer->IsAutoRefresh())
				m_modelViewer->RenderEnvironment();
		}
	}
	else if (GetExtension(filename) == "sfx")
	{
		TextureMng->SetSfxTexturePath(dir + "Texture/");
		ModelMng->SetSfxPath(dir);

		const bool wasPlaying = ui.actionJouer->isChecked();

		CloseFile();

		Delete(m_sfx->m_sfx);
		if (m_sfx->Load(filenameToLower))
		{
			CSfxPart* part;
			int j;
			for (int i = 0; i < m_sfx->m_sfx->m_parts.GetSize(); i++)
			{
				part = m_sfx->m_sfx->m_parts[i];

				const string name = MakePartName(part);
				m_timeline->AddRow(name);
				ui.listParts->addItem(name);

				for (j = 0; j < part->m_keys.GetSize(); j++)
					m_timeline->AddKey(i, part->m_keys[j]->frame);
			}

			const string filenameNotLower = QFileInfo(filename).fileName();
			setWindowTitle("SfxEditor - " + filenameNotLower);
			m_filename = filename;

			if (wasPlaying)
			{
				ui.actionJouer->setChecked(true);
				m_modelViewer->SetAutoRefresh(true);
			}
		}
		else
		{
			m_filename = "";
			m_sfx->m_sfx = new CSfx(m_modelViewer->GetDevice());
		}

		if (!m_modelViewer->IsAutoRefresh())
			m_modelViewer->RenderEnvironment();
	}

	m_status->setText(tr("Prêt"));
}

void CMainFrame::CloseFile()
{
	if (m_sfx)
		Delete(m_sfx->m_sfx);

	Delete(m_sfx);
	setWindowTitle("SfxEditor");
	m_modelViewer->SetSfx(null);
	m_modelViewer->SetAutoRefresh(false);
	m_timeline->SetCurrentFrame(0);
	m_timeline->RemoveAllRows();
	ui.actionJouer->setChecked(false);
	m_filename = "";
	ui.listParts->clear();
	m_currentEditPart = -1;
	ui.dockEditKey->setEnabled(false);
	_setPart();

	m_sfx = new CSfxModel(m_modelViewer->GetDevice());
	m_sfx->m_sfx = new CSfx(m_modelViewer->GetDevice());

	m_modelViewer->SetSfx(m_sfx);
}

void CMainFrame::Play(bool play)
{
	if (play)
	{
		if (!m_sfx)
			ui.actionJouer->setChecked(false);
		else
			m_modelViewer->SetAutoRefresh(true);
	}
	else
		m_modelViewer->SetAutoRefresh(false);

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CMainFrame::Stop()
{
	ui.actionJouer->setChecked(false);
	m_modelViewer->SetAutoRefresh(false);
	m_timeline->SetCurrentFrame(0);
	m_modelViewer->SetFrame(0);
}

void CMainFrame::About()
{
	CAboutDialog(this, m_modelViewer).exec();
}

void CMainFrame::AboutQt()
{
	QMessageBox::aboutQt(this, tr("À propos de Qt"));
}

void CMainFrame::ShowGrid(bool show)
{
	g_global3D.grid = show;

	if (!m_modelViewer->IsAutoRefresh())
		m_modelViewer->RenderEnvironment();
}

void CMainFrame::SwitchFullscreen(bool fullscreen)
{
	if (fullscreen)
		showFullScreen();
	else
		showNormal();
}

string CMainFrame::MakePartName(const CSfxPart* part) const
{
	string name;
	switch (part->GetType())
	{
	case ESfxPartType::Bill:
		name = 'B';
		break;
	case ESfxPartType::Particle:
		name = 'P';
		break;
	case ESfxPartType::CustomMesh:
		name = 'C';
		break;
	case ESfxPartType::Mesh:
		name = 'M';
		break;
	}
	name += ' ';
	name += part->m_name;
	return name;
}

void CMainFrame::dragEnterEvent(QDragEnterEvent* event)
{
	m_dragFilename.clear();

	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size() && i < 32; ++i)
			pathList.append(urlList.at(i).toLocalFile());

		if (pathList.size() > 0)
		{
			for (int i = 0; i < pathList.size(); i++)
			{
				if (GetExtension(pathList[i]) == "sfx"
					|| GetExtension(pathList[i]) == "o3d"
					|| GetExtension(pathList[i]) == "ani")
				{
					m_dragFilename = pathList[i];
					event->acceptProposedAction();
					return;
				}
			}

		}
	}

	m_dragFilename.clear();
}

void CMainFrame::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void CMainFrame::dragMoveEvent(QDragMoveEvent* event)
{
	if (m_dragFilename.size() > 0)
		event->acceptProposedAction();
}

void CMainFrame::dropEvent(QDropEvent* event)
{
	if (m_dragFilename.size() > 0)
		OpenFile(m_dragFilename);
}

void CMainFrame::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange && !isMinimized())
	{
		if (m_modelViewer && !m_modelViewer->IsAutoRefresh())
			m_modelViewer->RenderEnvironment();
	}

	QMainWindow::changeEvent(event);
}