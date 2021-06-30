///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "FileModel.h"
#include "ClientExtractor.h"
#include <AboutDialog.h>
#include <ShortcutsMng.h>

CMainFrame::CMainFrame(QWidget *parent)
	: QMainWindow(parent),
	m_status(null),
	m_files(null),
	m_language(LANG_FRE),
	m_languageActionGroup(null),
	m_listMenu(null),
	m_sortMode(SORT_NAME),
	m_viewMode(1),
	m_sortActionGroup(null),
	m_viewActionGroup(null)
{
}

CMainFrame::~CMainFrame()
{
	Delete(m_listMenu);
	Delete(m_files);
	Delete(m_status);
	Delete(m_languageActionGroup);
	Delete(m_sortActionGroup);
	Delete(m_viewActionGroup);
}

bool CMainFrame::Initialize()
{
	ui.setupUi(this);

	m_status = new QLabel(tr("Prêt"));
	m_status->setStyleSheet("color: white;");
	ui.statusBar->addWidget(m_status);
	qApp->setStyleSheet("QStatusBar::item { border: none; }");

	m_languageActionGroup = new QActionGroup(ui.menuLangage);
	m_languageActionGroup->addAction(ui.actionFran_ais);
	m_languageActionGroup->addAction(ui.actionEnglish);
	m_languageActionGroup->addAction(ui.actionDeutsch);

	m_sortActionGroup = new QActionGroup(ui.menuTrier_par);
	m_sortActionGroup->addAction(ui.actionNom);
	m_sortActionGroup->addAction(ui.actionType);
	m_sortActionGroup->addAction(ui.actionDate);
	m_sortActionGroup->addAction(ui.actionTaille_2);

	m_viewActionGroup = new QActionGroup(ui.menuTaille);
	m_viewActionGroup->addAction(ui.actionGrandes_ic_nes);
	m_viewActionGroup->addAction(ui.actionMoyennes_ic_nes);
	m_viewActionGroup->addAction(ui.actionPetites_ic_nes);
	m_viewActionGroup->addAction(ui.actionListe);

	m_listMenu = new QMenu(this);
	m_listMenu->addAction(ui.actionExtraire_les_fichiers);
	m_listMenu->addAction(ui.actionSupprimer_les_fichiers);

	CFileModel::InitResources();

	_connectWidgets();
	_setShortcuts();
	CloseFile();
	_loadSettings();
	return true;
}

void CMainFrame::_connectWidgets()
{
	connect(ui.actionNouveau, SIGNAL(triggered()), this, SLOT(NewFile()));
	connect(ui.actionOuvrir, SIGNAL(triggered()), this, SLOT(OpenFile()));
	connect(ui.actionFermer, SIGNAL(triggered()), this, SLOT(CloseFile()));
	connect(m_languageActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetLanguage(QAction*)));
	connect(ui.actionPlein_cran, SIGNAL(triggered(bool)), this, SLOT(SwitchFullscreen(bool)));
	connect(ui.actionCrypter_les_fichiers, SIGNAL(triggered(bool)), this, SLOT(SetEncrypted(bool)));
	connect(ui.action_propos, SIGNAL(triggered()), this, SLOT(About()));
	connect(ui.actionQt, SIGNAL(triggered()), this, SLOT(AboutQt()));
	connect(ui.actionCl_de_cryptage, SIGNAL(triggered()), this, SLOT(SetKey()));
	connect(ui.centralWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowListMenu(const QPoint&)));
	connect(ui.actionSupprimer_les_fichiers, SIGNAL(triggered()), this, SLOT(DeleteFiles()));
	connect(ui.actionEnregistrer, SIGNAL(triggered()), this, SLOT(SaveFile()));
	connect(ui.actionEnregistrer_sous, SIGNAL(triggered()), this, SLOT(SaveFileAs()));
	connect(ui.actionAjouter_des_fichiers, SIGNAL(triggered()), this, SLOT(AddFiles()));
	connect(ui.actionExtraire_les_fichiers, SIGNAL(triggered()), this, SLOT(ExtractFiles()));
	connect(ui.actionExtracteur_de_client, SIGNAL(triggered()), this, SLOT(ClientExtractor()));
	connect(m_sortActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetSortMode(QAction*)));
	connect(m_viewActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(SetViewMode(QAction*)));
}

void CMainFrame::_setShortcuts()
{
	CShortcutsMng mng;
	mng.Add(ui.actionQuitter, "ExitApp");
	mng.Add(ui.actionEnregistrer, "SaveFile");
	mng.Add(ui.actionPlein_cran, "Fullscreen");
	mng.Add(ui.action_propos, "About");
	mng.Add(ui.actionFermer, "CloseFile");
	mng.Add(ui.actionOuvrir, "OpenFile");
	mng.Add(ui.actionNouveau, "NewFile");
	mng.Add(ui.actionEnregistrer_sous, "SaveFileAs");
	mng.Add(ui.actionAjouter_des_fichiers, "AddFiles");
	mng.Add(ui.actionExtraire_les_fichiers, "ExtractFiles");
	mng.Add(ui.actionSupprimer_les_fichiers, "DeleteFiles");
	mng.Add(ui.actionExtracteur_de_client, "ClientExtractor");
	mng.Load();
}

void CMainFrame::OpenFile(const string& filename)
{
	CloseFile();
	if (m_files->Load(filename))
	{
		ui.actionCrypter_les_fichiers->setChecked(m_files->IsEncrypted());
		setWindowTitle(tr("ResEditor - ") + QFileInfo(filename).fileName());
		m_filename = filename;
	}
	m_status->setText(tr("Prêt"));
}

void CMainFrame::CloseFile()
{
	setWindowTitle(tr("ResEditor - Nouveau fichier"));
	Delete(m_files);
	m_files = new CFileModel();
	m_files->SetSortMode(m_sortMode);
	ui.centralWidget->setModel(m_files);
	ui.actionCrypter_les_fichiers->setChecked(false);
	m_filename.clear();
	m_status->setText(tr("Prêt"));
}

void CMainFrame::OpenFile()
{
	const string filename = QFileDialog::getOpenFileName(this, tr("Charger un fichier ressource"), m_filename, tr("Fichier ressource") + " (*.res)");

	if (!filename.isEmpty())
		OpenFile(filename);
}

void CMainFrame::NewFile()
{
	if (m_files->rowCount() > 0)
	{
		CMainFrame* newWindow = new CMainFrame();
		newWindow->show();
	}
}

void CMainFrame::SetEncrypted(bool encrypted)
{
	m_files->SetEncrypted(encrypted);
}

void CMainFrame::SetKey()
{
	bool ok = false;
	const int key = QInputDialog::getInt(this, tr("Modifier la clé de cryptage"), tr("Clé de cryptage :"), (int)m_files->EncryptionKey(), 0, 255, 1, &ok);
	if (ok)
		m_files->SetEncryptionKey((byte)key);
}

void CMainFrame::ShowListMenu(const QPoint& pt)
{
	if (ui.centralWidget->selectionModel()->selectedIndexes().size() > 0)
		m_listMenu->exec(ui.centralWidget->mapToGlobal(pt));
}

void CMainFrame::AddFiles()
{
	const QStringList files = QFileDialog::getOpenFileNames(this, tr("Ajouter des fichiers"));
	if (files.size() > 0)
		m_files->AddFiles(files);
}

void CMainFrame::ExtractFiles()
{
	auto selection = ui.centralWidget->selectionModel()->selectedIndexes();
	if (selection.size() > 0)
	{
		const string dir = QFileDialog::getExistingDirectory(this, tr("Dossier cible"));
		if (!dir.isEmpty())
			m_files->ExtractFiles(dir, selection);
	}
}

void CMainFrame::DeleteFiles()
{
	auto selection = ui.centralWidget->selectionModel()->selectedIndexes();
	if (selection.size() > 0)
		m_files->DeleteFiles(selection);
}

void CMainFrame::ClientExtractor()
{
	CClientExtractor(this).exec();
}

void CMainFrame::SaveFile()
{
	if (m_filename.isEmpty())
	{
		m_filename = QFileDialog::getSaveFileName(this, tr("Enregistrer le fichier ressource"), "", tr("Fichier ressource") + " (*.res)");

		if (!m_filename.isEmpty())
			setWindowTitle("ResEditor - " + QFileInfo(m_filename).fileName());
	}

	if (!m_filename.isEmpty())
		_saveFile(m_filename);
}

void CMainFrame::SaveFileAs()
{
	const string filename = QFileDialog::getSaveFileName(this, tr("Enregistrer le fichier ressource"), m_filename, tr("Fichier ressource") + " (*.res)");

	if (!filename.isEmpty())
	{
		setWindowTitle("ResEditor - " + QFileInfo(filename).fileName());
		m_filename = filename;
		_saveFile(m_filename);
	}
}

void CMainFrame::_saveFile(const string& filename)
{
	const bool result = m_files->Save(filename);

	if (result)
		m_status->setText(tr("Fichier enregistré avec succès"));
	else
	{
		qCritical("There was a problem while saving the file");
		m_status->setText(tr("Prêt"));
	}
}

void CMainFrame::_loadSettings()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "ResEditor");
	if (settings.status() == QSettings::NoError)
	{
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
		if (settings.contains("ViewMode"))
		{
			m_viewMode = settings.value("ViewMode").toInt();
			switch (m_viewMode)
			{
			case 0:
				ui.actionGrandes_ic_nes->setChecked(true);
				SetViewMode(ui.actionGrandes_ic_nes);
				break;
			case 1:
				ui.actionMoyennes_ic_nes->setChecked(true);
				SetViewMode(ui.actionMoyennes_ic_nes);
				break;
			case 2:
				ui.actionPetites_ic_nes->setChecked(true);
				SetViewMode(ui.actionPetites_ic_nes);
				break;
			case 3:
				ui.actionListe->setChecked(true);
				SetViewMode(ui.actionListe);
				break;
			}
		}
		if (settings.contains("SortMode"))
		{
			m_sortMode = settings.value("SortMode").toInt();
			switch (m_sortMode)
			{
			case SORT_NAME:
				ui.actionNom->setChecked(true);
				SetSortMode(ui.actionNom);
				break;
			case SORT_TYPE:
				ui.actionType->setChecked(true);
				SetSortMode(ui.actionType);
				break;
			case SORT_DATE:
				ui.actionDate->setChecked(true);
				SetSortMode(ui.actionDate);
				break;
			case SORT_SIZE:
				ui.actionTaille_2->setChecked(true);
				SetSortMode(ui.actionTaille_2);
				break;
			}
		}
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
		m_translator.load("reseditor_en.qm", "Plugins/languages/English");
	}
	else if (action == ui.actionDeutsch)
	{
		m_language = LANG_GER;
		m_translator.load("reseditor_de.qm", "Plugins/languages/Deutsch");
	}

	qApp->installTranslator(&m_translator);
	ui.retranslateUi(this);
	m_status->setText(tr("Prêt"));

	if (!m_filename.isEmpty())
		setWindowTitle(tr("ResEditor - ") + QFileInfo(m_filename).fileName());
	else
		setWindowTitle(tr("ResEditor - Nouveau fichier"));
}

void CMainFrame::closeEvent(QCloseEvent* event)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ATools", "ResEditor");
	if (settings.isWritable())
	{
		settings.setValue("WindowGeometry", saveGeometry());
		settings.setValue("WindowState", saveState());
		settings.setValue("Language", m_language);
		settings.setValue("ViewMode", m_viewMode);
		settings.setValue("SortMode", m_sortMode);
	}

	QMainWindow::closeEvent(event);
}

void CMainFrame::SetViewMode(QAction* action)
{
	if (action == ui.actionGrandes_ic_nes)
	{
		m_viewMode = 0;
		ui.centralWidget->setViewMode(QListView::IconMode);
		ui.centralWidget->setIconSize(QSize(48, 48));
	}
	else if (action == ui.actionMoyennes_ic_nes)
	{
		m_viewMode = 1;
		ui.centralWidget->setViewMode(QListView::IconMode);
		ui.centralWidget->setIconSize(QSize(32, 32));
	}
	else if (action == ui.actionPetites_ic_nes)
	{
		m_viewMode = 2;
		ui.centralWidget->setViewMode(QListView::IconMode);
		ui.centralWidget->setIconSize(QSize(16, 16));
	}
	else if (action == ui.actionListe)
	{
		m_viewMode = 3;
		ui.centralWidget->setViewMode(QListView::ListMode);
		ui.centralWidget->setIconSize(QSize(16, 16));
		ui.centralWidget->setGridSize(QSize(300, 20));
	}

	if (m_viewMode != 3)
		ui.centralWidget->setGridSize(ui.centralWidget->iconSize() + QSize(62, 22));
}

void CMainFrame::SetSortMode(QAction* action)
{
	if (action == ui.actionNom)
		m_sortMode = SORT_NAME;
	else if (action == ui.actionType)
		m_sortMode = SORT_TYPE;
	else if (action == ui.actionDate)
		m_sortMode = SORT_DATE;
	else if (action == ui.actionTaille_2)
		m_sortMode = SORT_SIZE;

	m_files->SetSortMode(m_sortMode);
	m_files->Sort();
}

void CMainFrame::About()
{
	CAboutDialog(this).exec();
}

void CMainFrame::AboutQt()
{
	QMessageBox::aboutQt(this, tr("À propos de Qt"));
}

void CMainFrame::SwitchFullscreen(bool fullscreen)
{
	if (fullscreen)
		showFullScreen();
	else
		showNormal();
}

void CMainFrame::dragEnterEvent(QDragEnterEvent* event)
{
	event->accept();
}

void CMainFrame::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void CMainFrame::dragMoveEvent(QDragMoveEvent* event)
{
	event->acceptProposedAction();
}

void CMainFrame::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size(); ++i)
			pathList.append(urlList.at(i).toLocalFile());

		if (pathList.size() == 1 && GetExtension(pathList[0]) == "res")
		{
			OpenFile(pathList[0]);
		}
		else
		{
			QStringList files;
			for (int i = 0; i < urlList.size(); i++)
				files.push_back(pathList[i]);

			if (files.size() > 0)
				m_files->AddFiles(files);
		}
	}
}