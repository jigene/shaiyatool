///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef DIALOGEDITEFFECTS_H
#define DIALOGEDITEFFECTS_H

#include "ui_DialogEditEffects.h"

struct MaterialBlock;
class CObject3D;
class CModelViewer;
struct GMObject;
struct LODGroup;

class CDialogEditEffects : public QDialog
{
	Q_OBJECT

public:
	CDialogEditEffects(CModelViewer* modelViewer, CObject3D* obj3D, QWidget *parent = 0);
	~CDialogEditEffects();

public slots:
	void CurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);
	void Set2Sides(bool b);
	void SetOpacity(bool b);
	void SetReflect(bool b);
	void SetSelfIlluminate(bool b);
	void SetHighlight(bool b);
	void SetAmount(int v);
	void ChangeTexture();
	void ChangeCW();

	void ResetBlocks();

private:
	Ui::DialogEditEffects ui;
	CObject3D* m_obj3D;
	CModelViewer* m_modelViewer;
	MaterialBlock* m_oldBlocks;
	MaterialBlock* m_editing;
	GMObject* m_editingObj;
	LODGroup* m_editingGroup;
	QMap<MaterialBlock*, QTreeWidgetItem*> m_items;

	void _setTree();
};

#endif // DIALOGEDITEFFECTS_H