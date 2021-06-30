///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef SKINAUTO_H
#define SKINAUTO_H

#include "ui_SkinAuto.h"

class CAnimatedMesh;

class CDialogSkinAuto : public QDialog
{
	Q_OBJECT

public:
	CDialogSkinAuto(CAnimatedMesh* mesh, QWidget *parent = 0);
	~CDialogSkinAuto();

	public slots:
	void LoadSkel();
	void SkinSkel();

private:
	Ui::SkinAutoDialog ui;
	CAnimatedMesh* m_mesh;
	QMap<int, QTreeWidgetItem*> m_items;
};

#endif // SKINAUTO_H