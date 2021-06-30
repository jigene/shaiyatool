///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

class CD3DWidget;

class CAboutDialog : public QDialog
{
	Q_OBJECT

public:
	CAboutDialog(QMainWindow* mainWindow, CD3DWidget* renderWidget = null);
};

#endif // ABOUTDIALOG_H