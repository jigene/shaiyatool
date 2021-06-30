///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "DataManager.h"
#include "GUIEditor.h"
#include "WndWindow.h"
#include <WndManager.h>
#include <AboutDialog.h>
#include <ShortcutsMng.h>

CMainFrame::CMainFrame(QWidget *parent)
	: QMainWindow(parent),
	m_editor(null),
	m_dataMng(null),
	m_windowList(null),
	m_language(LANG_FRE),
	m_languageActionGroup(null),
	m_status(null),
	m_controlsGroup(null)
{
}

CMainFrame::~CMainFrame()
{
	for (int i = 0; i < m_clipboardControls.GetSize(); i++)
		Delete(m_clipboardControls[i]);
	m_clipboardControls.RemoveAll();

	Delete(m_editor);
	Delete(m_windowList);
	Delete(m_status);
	Delete(m_dataMng);
	Delete(m_controlsGroup);
	Delete(m_languageActionGroup);
}

bool CMainFrame::Initialize()
{
	ui.setupUi(this);

	m_dataMng = new CDataManager(ui.editControlID);
	if (!m_dataMng->Load())
		return false;

	m_windowList = new QStringListModel(ui.listWindows);
	m_dataMng->FillWindowList(m_windowList);
	ui.listWindows->setModel(m_windowList);
	m_dataMng->FillControlList();

	m_editor = new CGUIEditor(this);
	setCentralWidget(m_editor);

	if (!m_editor->CreateEnvironment())
		return false;

	m_status = new QLabel(tr("Prêt"));
	m_status->setStyleSheet("color: white;");
	ui.statusBar->addWidget(m_status);
	qApp->setStyleSheet("QStatusBar::item { border: none; }");

	ui.menuFen_tres->addAction(ui.dockWindows->toggleViewAction());
	ui.menuFen_tres->addAction(ui.dockEdit->toggleViewAction());

	m_controlsGroup = new QActionGroup(this);
	m_controlsGroup->addAction(ui.actionX);
	m_controlsGroup->addAction(ui.actionText);
	m_controlsGroup->addAction(ui.actionPicture);
	m_controlsGroup->addAction(ui.actionStatic);
	m_controlsGroup->addAction(ui.actionText_Edit);
	m_controlsGroup->addAction(ui.actionGroupBox);
	m_controlsGroup->addAction(ui.actionButton);
	m_controlsGroup->addAction(ui.actionCheckBox);
	m_controlsGroup->addAction(ui.actionRadioButton);
	m_controlsGroup->addAction(ui.actionComboBox);
	m_controlsGroup->addAction(ui.actionListBox);
	m_controlsGroup->addAction(ui.actionListCtrl);
	m_controlsGroup->addAction(ui.actionTreeCtrl);
	m_controlsGroup->addAction(ui.actionTabCtrl);
	m_controlsGroup->addAction(ui.actionCustom);
	m_createControlType = CREATECTRL_NONE;

	m_languageActionGroup = new QActionGroup(ui.menuLangage);
	m_languageActionGroup->addAction(ui.actionFran_ais);
	m_languageActionGroup->addAction(ui.actionEnglish);
	m_languageActionGroup->addAction(ui.actionDeutsch);

	_connectWidgets();
	_setShortcuts();
	SetEditControl();
	_loadSettings();
	return true;
}

void CMainFrame::_connectWidgets()
{
	connect(ui.actionPlein_cran, SIGNAL(triggered(bool)), this, SLOT(SwitchFullscreen(bool)));
	connect(ui.action_propos, SIGNAL(triggered()), this, SLOT(About()));
	connect(ui.actionQt, SIGNAL(triggered()), this, SLOT(AboutQt()));
	connect(ui.actionEnregistrer, SIGNAL(triggered()), this, SLOT(Save()));
	connect(ui.listWindows, SIGNAL(activated(const QModelIndex&)), this, SLOT(ShowWindow(const QModelIndex&)));
	connect(ui.actionGrille, SIGNAL(triggered(bool)), this, SLOT(ShowGrid(bool)));
	connect(ui.actionCouleur_du_fond, SIGNAL(triggered()), this, SLOT(SetBackgroundColor()));
	connect(ui.actionPr_cision_de_l_diteur, SIGNAL(triggered()), this, SLOT(SetGridSize()));
	connect(ui.actionCling, SIGNAL(triggered(bool)), this, SLOT(SetCling(bool)));
	connect(ui.actionNouvelle_fen_tre, SIGNAL(triggered()), this, SLOT(AddNewWindow()));
	connect(ui.btnAddWindow, SIGNAL(clicked()), this, SLOT(AddNewWindow()));
	connect(ui.btnRemoveWindow, SIGNAL(clicked()), this, SLOT(DeleteWindow()));
	connect(ui.actionSupprimer_le_control, SIGNAL(triggered()), this, SLOT(DeleteControl()));
	connect(ui.actionRamener_au_premier_plan, SIGNAL(triggered()), this, SLOT(SetControlOnTop()));
	connect(ui.editControlID, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(EditControlID(const QString&)));
	connect(ui.editControlWidth, SIGNAL(valueChanged(int)), this, SLOT(EditControlWidth(int)));
	connect(ui.editControlHeight, SIGNAL(valueChanged(int)), this, SLOT(EditControlHeight(int)));
	connect(ui.editControlText, SIGNAL(textEdited(const QString&)), this, SLOT(EditControlText(const QString&)));
	connect(ui.editControlTooltip, SIGNAL(textEdited(const QString&)), this, SLOT(EditControlTooltip(const QString&)));
	connect(ui.editControlTexture, SIGNAL(editingFinished()), this, SLOT(EditControlTexture()));
	connect(ui.btnSelectTexture, SIGNAL(clicked()), this, SLOT(SetControlTexture()));
	connect(ui.btnAdaptSizeToTexture, SIGNAL(clicked()), this, SLOT(ButtonFitTextureSize()));
	connect(ui.editControlTiles, SIGNAL(toggled(bool)), this, SLOT(EditControlTiles(bool)));
	connect(ui.editControlNoDrawFrame, SIGNAL(toggled(bool)), this, SLOT(EditControlNoDrawFrame(bool)));
	connect(ui.actionAligner_droite, SIGNAL(triggered()), this, SLOT(AlignControlsOnRight()));
	connect(ui.actionAligner_gauche, SIGNAL(triggered()), this, SLOT(AlignControlsOnLeft()));
	connect(ui.actionAligner_en_haut, SIGNAL(triggered()), this, SLOT(AlignControlsOnTop()));
	connect(ui.actionAligner_en_bas, SIGNAL(triggered()), this, SLOT(AlignControlsOnBottom()));
	connect(ui.editControlX, SIGNAL(valueChanged(int)), this, SLOT(EditControlPosX(int)));
	connect(ui.editControlY, SIGNAL(valueChanged(int)), this, SLOT(EditControlPosY(int)));
	connect(ui.editControlVisible, SIGNAL(toggled(bool)), this, SLOT(EditControlVisible(bool)));
	connect(ui.editControlEnabled, SIGNAL(toggled(bool)), this, SLOT(EditControlEnabled(bool)));
	connect(ui.editControlTabStop, SIGNAL(toggled(bool)), this, SLOT(EditControlTabStop(bool)));
	connect(ui.editControlGroup, SIGNAL(toggled(bool)), this, SLOT(EditControlGroup(bool)));
	connect(ui.editControlScrollBar, SIGNAL(toggled(bool)), this, SLOT(EditControlScrollBar(bool)));
	connect(ui.editHAlignLeft, SIGNAL(toggled(bool)), this, SLOT(EditControlHAlignLeft(bool)));
	connect(ui.editHAlignCenter, SIGNAL(toggled(bool)), this, SLOT(EditControlHAlignMiddle(bool)));
	connect(ui.editHAlignRight, SIGNAL(toggled(bool)), this, SLOT(EditControlHAlignRight(bool)));
	connect(ui.editVAlignTop, SIGNAL(toggled(bool)), this, SLOT(EditControlVAlignTop(bool)));
	connect(ui.editVAlignMiddle, SIGNAL(toggled(bool)), this, SLOT(EditControlVAlignCenter(bool)));
	connect(ui.editVAlignBottom, SIGNAL(toggled(bool)), this, SLOT(EditControlVAlignBottom(bool)));
	connect(ui.editWindowCaption, SIGNAL(toggled(bool)), this, SLOT(EditWindowCaption(bool)));
	connect(ui.editWindowNoFrame, SIGNAL(toggled(bool)), this, SLOT(EditWindowNoFrame(bool)));
	connect(ui.editWindowD3DFormat, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(EditWindowFormat(const QString&)));
	connect(m_controlsGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetCreateControl(QAction*)));
	connect(m_languageActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetLanguage(QAction*)));
	connect(ui.actionCopier, SIGNAL(triggered()), this, SLOT(CopyControl()));
	connect(ui.actionCouper, SIGNAL(triggered()), this, SLOT(CutControl()));
	connect(ui.actionColler, SIGNAL(triggered()), this, SLOT(PasteControl()));

#if __VER >= 19
	connect(ui.editWindowIcon, SIGNAL(textEdited(const QString&)), this, SLOT(EditWindowIcon(const QString&)));
	connect(ui.editControlColor, SIGNAL(clicked()), this, SLOT(EditControlColor()));
#endif
}

void CMainFrame::_setShortcuts()
{
	CShortcutsMng mng;
	mng.Add(ui.actionQuitter, "ExitApp");
	mng.Add(ui.actionEnregistrer, "SaveFile");
	mng.Add(ui.actionPlein_cran, "Fullscreen");
	mng.Add(ui.action_propos, "About");
	mng.Add(ui.actionCopier, "Copy");
	mng.Add(ui.actionCouper, "Cut");
	mng.Add(ui.actionColler, "Paste");
	mng.Add(ui.actionNouvelle_fen_tre, "NewWindow");
	mng.Add(ui.actionSupprimer_le_control , "DeleteCtrl");
	mng.Add(ui.actionAligner_en_haut, "AlignTop");
	mng.Add(ui.actionAligner_droite, "AlignRight");
	mng.Add(ui.actionAligner_gauche, "AlignLeft");
	mng.Add(ui.actionAligner_en_bas, "AlignBottom");
	mng.Add(ui.actionRamener_au_premier_plan, "BringToFront");
	mng.Add(ui.actionCling, "Cling");
	mng.Add(ui.actionGrille, "ShowGrid");
	mng.Add(ui.actionX, "X");
	mng.Load();
}

void CMainFrame::Save()
{
	const bool result = m_dataMng->Save();

	if (result)
		m_status->setText(tr("Enregistré avec succès"));
	else
	{
		qCritical("There was a problem while saving");
		m_status->setText(tr("Prêt"));
	}
}

void CMainFrame::_loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "GUIEditor");
	if (settings.status() == QSettings::NoError)
	{
		if (settings.contains("BackgroundColor") && m_editor)
			m_editor->SetBackgroundColor(settings.value("BackgroundColor").toUInt());
		if (settings.contains("ShowGrid"))
		{
			CWndWindow::s_showGrid = settings.value("ShowGrid").toBool();
			ui.actionGrille->setChecked(CWndWindow::s_showGrid);
		}
		if (settings.contains("Cling"))
		{
			CWndWindow::s_cling = settings.value("Cling").toBool();
			ui.actionCling->setChecked(CWndWindow::s_cling);
		}
		if (settings.contains("GridSize"))
			CWndWindow::s_gridSize = settings.value("GridSize").toInt();
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

		if (m_editor)
			m_editor->RenderEnvironment();
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
		m_translator.load("guieditor_en.qm", "Plugins/languages/English");
	}
	else if (action == ui.actionDeutsch)
	{
		m_language = LANG_GER;
		m_translator.load("guieditor_de.qm", "Plugins/languages/Deutsch");
	}

	qApp->installTranslator(&m_translator);
	ui.retranslateUi(this);
	m_status->setText(tr("Prêt"));
}

void CMainFrame::closeEvent(QCloseEvent* event)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "GUIEditor");
	if (settings.isWritable())
	{
		if (m_editor)
			settings.setValue("BackgroundColor", (uint)m_editor->GetBackgroundColor());
		settings.setValue("ShowGrid", CWndWindow::s_showGrid);
		settings.setValue("Cling", CWndWindow::s_cling);
		settings.setValue("GridSize", CWndWindow::s_gridSize);
		settings.setValue("WindowGeometry", saveGeometry());
		settings.setValue("WindowState", saveState());
		settings.setValue("Language", m_language);
	}

	QMainWindow::closeEvent(event);
}

void CMainFrame::SetGridSize()
{
	bool ok = false;
	const int value = QInputDialog::getInt(this, tr("Précision de l'éditeur"), tr("Précision en pixels: "), CWndWindow::s_gridSize, 1, 9999, 1, &ok);
	if (ok)
	{
		CWndWindow::s_gridSize = value;
		if (m_editor)
			m_editor->RenderEnvironment();
	}
}

void CMainFrame::SetCling(bool cling)
{
	CWndWindow::s_cling = cling;
	if (m_editor)
		m_editor->RenderEnvironment();
}

void CMainFrame::SetBackgroundColor()
{
	const QColor color = QColorDialog::getColor(m_editor->GetBackgroundColor(), this, tr("Couleur du fond"));
	if (color.isValid())
	{
		m_editor->SetBackgroundColor(D3DCOLOR_ARGB(255, color.red(), color.green(), color.blue()));
		if (m_editor)
			m_editor->RenderEnvironment();
	}
}

void CMainFrame::SwitchFullscreen(bool fullscreen)
{
	if (fullscreen)
		showFullScreen();
	else
		showNormal();
}

void CMainFrame::About()
{
	CAboutDialog(this, m_editor).exec();
}

void CMainFrame::AboutQt()
{
	QMessageBox::aboutQt(this, tr("À propos de Qt"));
}

void CMainFrame::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange && !isMinimized())
	{
		if (m_editor)
			m_editor->RenderEnvironment();
	}

	QMainWindow::changeEvent(event);
}

void CMainFrame::ShowGrid(bool show)
{
	CWndWindow::s_showGrid = show;
	m_editor->RenderEnvironment();
}