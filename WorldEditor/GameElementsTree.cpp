#include "stdafx.h"
#include "MainFrame.h"
#include "GameElementsTree.h"
#include <World.h>

CGameElementsTree::CGameElementsTree(QWidget* parent)
	: QTreeView(parent)
{
}

void CGameElementsTree::keyPressEvent(QKeyEvent* event)
{
	if (MainFrame->GetEditMode() != EEditMode::EDIT_ADD_OBJECTS && (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) && CWorld::s_selection.GetSize() > 0)
	{
		MainFrame->RotateObjects(event->key());
		event->accept();
		return;
	}

	QTreeView::keyPressEvent(event);

	if ((event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) && selectedIndexes().size() > 0)
		emit activated(selectedIndexes().at(0));
}