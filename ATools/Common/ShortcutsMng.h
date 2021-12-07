///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SHORTCUTSMNG_H
#define SHORTCUTSMNG_H

class CShortcutsMng
{
public:
	CShortcutsMng();

	void Add(QAction* action, const string& name);
	void Load();

private:
	QMap<string, QAction*> m_actions;
};

#endif // SHORTCUTSMNG_H