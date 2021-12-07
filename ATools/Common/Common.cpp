///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Common.h"
#include <time.h>

#ifdef WORLD_EDITOR
#include <MainFrame.h>
#endif // WORLD_EDITOR

float RoundFloat(float value, float round)
{
	const float mod = fmod(value, round);
	if (mod > round / 2.0f)
		return value - mod + round;
	else
		return value - mod;
}

string GetExtension(const string& filename)
{
	const QStringList filenameSplit = filename.split('.');

	if (filenameSplit.empty())
		return "";

	return filenameSplit[filenameSplit.size() - 1].toLower();
}

void CustomMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

void InstallMsgHandler()
{
	srand(time(null));
	qInstallMessageHandler(CustomMsgHandler);
}

void CustomMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString stringDir("Log/");
	stringDir.append(APP_NAME);
	stringDir.append('/');

	QDir dir(stringDir);
	if (!dir.exists())
		dir.mkpath(".");

	QString filename;

	switch (type)
	{
	case QtDebugMsg:
		filename = "Debug";
		break;
	case QtWarningMsg:
		filename = "Warning";
		break;
	case QtCriticalMsg:
		filename = "Critical";
		break;
	case QtFatalMsg:
		filename = "Fatal";
		break;
	}

	QDateTime dateTime = QDateTime::currentDateTime();

	filename.append('-');
	filename.append(dateTime.toString("dd-MM-yyyy"));
	filename.append(".txt");

	QFile file(stringDir + filename);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
		return;

	QTextStream flux(&file);
	flux.setCodec("UTF-8");

	flux << "---------------------------------------------" << endl;
	flux << dateTime.toString("dd/MM/yyyy hh:mm:ss") << endl;
	flux << context.file << " line " << context.line << endl;
	flux << context.function << endl;
	flux << msg << endl;

#ifdef WORLD_EDITOR
	if (MainFrame->MsgBoxForErrors() && (type == QtCriticalMsg || type == QtFatalMsg))
	{
		MainFrame->HideDialogs();
#endif // WORLD_EDITOR
		if (type == QtCriticalMsg)
			QMessageBox::critical(null, "Error", msg + "\n" + file.fileName() + " for more details.");
		else if (type == QtFatalMsg)
			QMessageBox::critical(null, "Fatal error", msg + "\nThe application will be closed.\n" + file.fileName() + " for more details.");
#ifdef WORLD_EDITOR
		MainFrame->ShowDialogs();
	}
#else // WORLD_EDITOR
	else if (type == QtWarningMsg)
		QMessageBox::warning(null, "Warning", msg + "\n" + file.fileName() + " for more details.");
#endif // WORLD_EDITOR
}