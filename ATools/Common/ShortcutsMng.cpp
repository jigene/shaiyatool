///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "ShortcutsMng.h"
#include "TextFile.h"

CShortcutsMng::CShortcutsMng()
{
}

void CShortcutsMng::Add(QAction* action, const string& name)
{
	if (!action)
		return;

	m_actions[name.toLower()] = action;
}

void CShortcutsMng::Load()
{
	CTextFile file;
	if (!file.Load("Plugins/Shortcuts.inc", false))
		return;

	const string common = "common";
	const string appName = string(APP_NAME).toLower();

	QVector<string> shortcuts;

	QMap<string, QAction*>::iterator it;
	string shortcut;
	string category = file.GetString(false).toLower();
	while (file.TokenType() != ETokenType::End)
	{
		file.NextToken(); // {
		if (category == common || category == appName)
		{
			while (file.TokenType() != ETokenType::EndBlock)
			{
				it = m_actions.find(file.GetString(false).toLower());
				if (it != m_actions.end())
				{
					file.NextToken(); // =
					shortcut = file.GetLine();
					if (!shortcut.isEmpty())
					{
						if (shortcuts.contains(shortcut))
							qWarning("Shortcut %s already used.", shortcut.toLocal8Bit().constData());
						else
						{
							it.value()->setShortcut(shortcut);
							shortcuts.push_back(shortcut);
						}
					}
				}
			}
		}
		else
		{
			while (file.TokenType() != ETokenType::EndBlock)
				file.NextToken();
		}
		category = file.GetString(false).toLower();
	}

	file.Close();
}