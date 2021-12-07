///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "Navigator.h"
#include <World.h>
#include "MainFrame.h"

CNavigator::CNavigator(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_world(null)
{
	setMinimumHeight(100);
	setAutoFillBackground(false);
	setMouseTracking(false);
}

void CNavigator::paintEvent(QPaintEvent* event)
{
	if (!m_world)
	{
		event->accept();
		return;
	}

	QPainter painter;
	painter.begin(this);

	const QRectF rect(QPointF(event->rect().left() + 1, event->rect().top() + 1),
		QPointF(event->rect().right(), event->rect().bottom()));

	const float worldWidth = m_world->m_width;
	const float worldHeight = m_world->m_height;

	const float landWidth = (rect.width() - worldWidth) / worldWidth;
	const float landHeight = (rect.height() - worldHeight) / worldHeight;

	QBrush light(QColor(208, 208, 208)), dark(QColor(128, 128, 128));

	int x, y;
	for (x = 0; x < m_world->m_width; x++)
	{
		for (y = 0; y < m_world->m_height; y++)
		{
			painter.fillRect(QRectF(
				rect.left() + (float)x * (landWidth + 1.0f),
				rect.bottom() - ((float)y * (landHeight + 1.0f)),
				landWidth, -landHeight).normalized(),
				m_world->GetLand(x, y) ? dark : light);
		}
	}

	const D3DXVECTOR3 camPos = m_world->m_cameraPos;
	if (m_world->VecInWorld(camPos))
	{
		painter.fillRect(QRectF(
			rect.left() + (rect.width() / (worldWidth * (float)MAP_SIZE * (float)MPU)) * camPos.x - 1.5f,
			rect.bottom() - ((rect.height() / (worldHeight * (float)MAP_SIZE * (float)MPU)) * camPos.z - 1.5f),
			3.0f, 3.0f),
			QBrush(QColor(0, 0, 255)));
	}

	painter.end();
	event->accept();

}

void CNavigator::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() != Qt::LeftButton)
		return;

	if (!m_world)
	{
		event->accept();
		return;
	}

	const QRectF rect(QPointF(1,  1), QPointF(width(), height()));
	const float worldWidth = m_world->m_width;
	const float worldHeight = m_world->m_height;

	m_world->m_cameraPos.x = (rect.left() - 1.5f - (float)event->x()) / (-(rect.width() / (worldWidth * (float)MAP_SIZE * (float)MPU)));
	m_world->m_cameraPos.z = (rect.bottom() + 1.5f - (float)event->y()) / (rect.height() / (worldHeight * (float)MAP_SIZE * (float)MPU));

	m_world->m_cameraAngle.x = 0.0f;
	m_world->m_cameraAngle.y = -89.89f;

	m_world->_loadLndFiles();
	m_world->m_cameraPos.y = m_world->GetHeight(m_world->m_cameraPos.x, m_world->m_cameraPos.z) + 200.0f;
	if (m_world->m_cameraPos.y > 999.0f)
		m_world->m_cameraPos.y = 999.0f;

	if (!MainFrame->IsEditingContinents())
		m_world->UpdateContinent();

	event->accept();

	MainFrame->UpdateWorldEditor();
	update();
}

void CNavigator::SetWorld(CWorld* world)
{
	m_world = world;
	update();
}