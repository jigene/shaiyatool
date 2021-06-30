///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef CLIENTEXTRACTOR_H
#define CLIENTEXTRACTOR_H

#include "ui_ClientExtractor.h"

class CClientExtractor : public QDialog
{
	Q_OBJECT

public:
	CClientExtractor(QWidget* parent = null, Qt::WindowFlags = 0);

	public slots:
	void SelectDir();
	void ExtractAll();

private:
	Ui::ClientExtractorDialog ui;
	QStringList m_files;

	void _scan(const string& dir);
};

#endif // CLIENTEXTRACTOR_H