///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef DIALOGNEWPART_H
#define DIALOGNEWPART_H

#include "ui_CreateNewPart.h"
#include <Sfx.h>

class CDialogNewPart : public QDialog
{
	Q_OBJECT

public:
	CDialogNewPart(QWidget *parent = 0);

	ESfxPartType GetSfxType() const;
	void SetPart(CSfxPart* part);

	public slots:
	void SetTextureName();

private:
	Ui::Dialog ui;
};

#endif // DIALOGNEWPART_H