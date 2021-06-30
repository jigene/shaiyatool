///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

class CWorld;

class CNavigator : public QWidget
{
	Q_OBJECT

public:
	CNavigator(QWidget* parent = null, Qt::WindowFlags flags = 0);

	void SetWorld(CWorld* world);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
	CWorld* m_world;
};

#endif // NAVIGATOR_H