///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "AboutDialog.h"
#include "D3DWidget.h"

CAboutDialog::CAboutDialog(QMainWindow* mainWindow, CD3DWidget* renderWidget)
	: QDialog(mainWindow)
{
	 QSize size(360, 238);

	 if (!renderWidget)
		 size.setHeight(135);

	 QIcon icon;
	 icon.addFile(":/MainFrame/Resources/about.png");

	setFixedSize(size);
	setSizeGripEnabled(false);
	setModal(true);
	setWindowIcon(icon);
	setWindowTitle(tr("À propos"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QPixmap logo(":/MainFrame/Resources/" + string(APP_NAME) + ".png");

	QLabel* picture = new QLabel(this);
	picture->setPixmap(logo);
	picture->setGeometry(QRect(QPoint(32 + 10, 23 + 10), logo.size()));

	QLabel* label = new QLabel(this);
	label->setOpenExternalLinks(true);
	label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	label->setTextFormat(Qt::RichText);
	label->setGeometry(QRect(120 + 20, 10, size.width() - (120 + 20), 111 - 10));
	label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

	label->setText("ATools v" + string::number(VERSION) + '.' + string::number(SUB_VERSION) + " - " + tr("Par") + " Aishiro"
		+ "<br/>Open Source"
		+ "<br/>Made in France"
		+ "<br/>" + tr("Splashscreen et logo par") + " Adler"
		+ "<br/>" + tr("Icônes de") + " <a href=\"http://icons8.com/\">Icons8</a>"
		+ "<br/><br/><a href=\"http://github.com/Aishiro/ATools\">" + tr("Contributez sur") + " GitHub</a>");

	if (renderWidget)
	{
		QTextEdit* infos = new QTextEdit(this);
		infos->setReadOnly(true);
		infos->setGeometry(10, 111 + 25, size.width() - 20, 90);
		label->setTextFormat(Qt::RichText);

#ifndef RES_EDITOR
		infos->setText(renderWidget->GetDeviceStats());
#endif // RES_EDITOR
	}
}