#ifndef GAMEELEMENTSTREE_H
#define GAMEELEMENTSTREE_H

class CGameElementsTree : public QTreeView
{
	Q_OBJECT

public:
	CGameElementsTree(QWidget* parent = null);

protected:
	virtual void keyPressEvent(QKeyEvent* event);
};

#endif // GAMEELEMENTSTREE_H