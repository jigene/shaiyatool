///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef OBJECTPROPERTIES_H
#define OBJECTPROPERTIES_H

#include "ui_ObjectProperites.h"

class CObject;

class CObjectPropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	CObjectPropertiesDialog(CObject* obj, QWidget* parent = 0);

	public slots:
	void SaveProperties(int result);
	void SetSpawn(bool spawn);
	void SetSpawnCount(int count);
	void SetMoverCharacter(const QString& key);
	void AddJob_();
	void RemoveJob();
	void SetMoverPatrolIndex(int index);

private:
	Ui::ObjectPropertiesDialog ui;
	CObject* m_obj;
};

#endif // OBJECTPROPERTIES_H