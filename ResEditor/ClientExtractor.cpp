///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "ClientExtractor.h"
#include "FileModel.h"

CClientExtractor::CClientExtractor(QWidget* parent, Qt::WindowFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(ui.selectDir, SIGNAL(clicked()), this, SLOT(SelectDir()));
	connect(ui.ok, SIGNAL(clicked()), this, SLOT(ExtractAll()));
}

void CClientExtractor::SelectDir()
{
	const string dir = QFileDialog::getExistingDirectory(this, tr("Dossier cible"));
	if (!dir.isEmpty())
		ui.dir->setText(dir);
}

void CClientExtractor::ExtractAll()
{
	const string root = ui.dir->text();

	if (root.isEmpty())
		return;

	_scan(root);

	if (m_files.size() > 0)
	{
		ui.progress->setMaximum(m_files.size());

		qApp->processEvents();

		CFileModel* model;
		for (int i = 0; i < m_files.size(); i++)
		{
			const string filename = m_files[i];

			model = new CFileModel(this);
			if (model->Load(filename))
				model->ExtractAllFiles(QFileInfo(filename).dir().path(), ui.replaceFiles->isChecked());
			Delete(model);

			if (ui.deleteRes->isChecked())
				QFile::remove(filename);

			ui.progress->setValue(i + 1);
			qApp->processEvents();
		}
	}

	QMessageBox::information(this, tr("Terminé"), tr("Tous les fichiers ont été extraits."));
	close();
}

void CClientExtractor::_scan(const string& dir)
{
	QDir directory(dir);
	if (!directory.exists())
		return;

	const QStringList files = directory.entryList(QStringList("*.res"), QDir::Files);

	for (int i = 0; i < files.size(); i++)
		m_files.append(dir + '/' + files[i]);

	QStringList subDirs = directory.entryList(QDir::Dirs);
	if (subDirs.size() > 0)
	{
		subDirs.removeAll(".");
		subDirs.removeAll("..");

		for (int i = 0; i < subDirs.size(); i++)
			_scan(dir + '/' + subDirs[i]);
	}
}