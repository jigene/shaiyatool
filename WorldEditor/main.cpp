///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"

int main(int argc, char *argv[])
{
	const string workingDir = QFileInfo(string::fromLocal8Bit(argv[0])).absolutePath();
	QCoreApplication::addLibraryPath(workingDir + "/Plugins/");
	QDir::setCurrent(workingDir);

	InstallMsgHandler();

	QApplication app(argc, argv);

	QSplashScreen* splash = new QSplashScreen(QPixmap(":/MainFrame/Resources/Splashscreen_" + string::number(rand() % 6) + ".png"));
	splash->show();
	app.processEvents();

	CMainFrame* mainFrame = new CMainFrame();
	int result = -1;

	if (mainFrame->Initialize())
	{
		mainFrame->show();
		splash->finish(mainFrame);
		Delete(splash);

		if (argc >= 2)
			mainFrame->OpenFile(string::fromLocal8Bit(argv[1]));

		mainFrame->UpdateWorldEditor();
		result = app.exec();
	}
	else
		Delete(splash);
	
	Delete(mainFrame);
	return result;
}