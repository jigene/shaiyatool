///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Timeline.h"

CTimeline::CTimeline(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_frameCount(0),
	m_currentFrame(0),
	m_movingView(false),
	m_movingCurrentFrame(false),
	m_selection(-1, -1)
{
	setMinimumHeight(100);
	setAutoFillBackground(false);
	setMouseTracking(true);

	m_rowHeight = 15;
	m_frameWidth = 15;
	m_leftWidth = m_frameWidth * 10;
	m_font = font();
	m_font.setPointSizeF((float)m_frameWidth / 1.8f);
}

void CTimeline::wheelEvent(QWheelEvent* event)
{
	if (event->orientation() == Qt::Vertical)
	{
		int zoom = (int)((float)event->delta() / 120.0f);

		m_frameWidth += zoom;
		m_rowHeight += zoom;

		if (m_frameWidth < 10)
			m_frameWidth = 10;
		else if (m_frameWidth > 50)
			m_frameWidth = 50;
		if (m_rowHeight < 10)
			m_rowHeight = 10;
		else if (m_rowHeight > 50)
			m_rowHeight = 50;

		m_font.setPointSizeF((float)m_frameWidth / 2.0f);

		update();
		event->accept();
	}
}

void CTimeline::AddRow(const string& name, const QColor& color)
{
	Row row;
	row.name = name;
	row.color = color;
	m_rows.push_back(row);
	update();
}

void CTimeline::RemoveRow(int index)
{
	if (index < 0 || index >= m_rows.size())
		return;

	m_rows.removeAt(index);

	if (m_selection.y() == index)
	{
		m_selection = QPoint(-1, -1);
		_emitSelectionChanged();
	}

	update();
}

void CTimeline::RenameRow(int index, const string& newName)
{
	if (index < 0 || index >= m_rows.size())
		return;

	m_rows[index].name = newName;
	update();
}

void CTimeline::RemoveAllRows()
{
	if (m_rows.size() == 0)
		return;

	m_rows.clear();

	if (m_selection.y() != -1)
	{
		m_selection = QPoint(-1, -1);
		_emitSelectionChanged();
	}

	update();
}

void CTimeline::AddKey(int row, int frame)
{
	if (row < 0 || row >= m_rows.size() || frame < 0
		|| (m_frameCount != -1 && frame >= m_frameCount))
		return;

	if (!m_rows[row].keys.contains(frame))
	{
		m_rows[row].keys.push_back(frame);
		update();
	}
}

void CTimeline::RemoveKey(int row, int frame)
{
	if (row < 0 || row >= m_rows.size() || frame < 0)
		return;

	int i = 0;
	for (auto it = m_rows[row].keys.begin(); it != m_rows[row].keys.end(); it++)
	{
		if (*it == frame)
		{
			m_rows[row].keys.erase(it);
			if (m_selection.y() == row && m_selection.x() == i)
			{
				m_selection.setX(-1);
				_emitSelectionChanged();
			}
			update();
			break;
		}
		i++;
	}
}

void CTimeline::RemoveAllKeys()
{
	for (int i = 0; i < m_rows.size(); i++)
		m_rows[i].keys.clear();

	if (m_selection.x() != -1)
	{
		m_selection.setX(-1);
		_emitSelectionChanged();
	}

	update();
}

void CTimeline::SetCurrentFrame(int frame)
{
	if (frame < 0 || m_currentFrame == frame
		|| (m_frameCount != -1 && frame >= m_frameCount))
		return;

	m_currentFrame = frame;
	update();
}

void CTimeline::SetFrameCount(int frameCount)
{
	if (frameCount < -1)
		return;

	m_frameCount = frameCount;
	m_currentFrame = 0;

	for (int i = 0; i < m_rows.size(); i++)
		m_rows[i].keys.clear();

	if (m_selection.y() != -1)
	{
		m_selection.setX(-1);
		m_selection.setY(-1);
		_emitSelectionChanged();
	}

	update();
}

void CTimeline::SetSelectedRow(int row)
{
	if (row < -1 || row >= m_rows.size())
		return;

	if (m_selection.y() != row)
	{
		m_selection.setY(row);
		m_selection.setX(-1);
		_emitSelectionChanged();
		update();
	}
}

void CTimeline::paintEvent(QPaintEvent* event)
{
	const QRect rect = event->rect();

	int tablePosY = m_viewPos.y() / m_rowHeight - 1;
	if (tablePosY < 0)
		tablePosY = 0;

	int tablePosX = m_viewPos.x() / m_frameWidth - 1;
	if (tablePosX < 0)
		tablePosX = 0;

	m_painter.begin(this);

	m_painter.fillRect(rect, QColor(212, 208, 200));

	m_painter.setClipRect(rect);
	m_painter.setClipping(true);
	m_painter.translate(QPoint(0, -m_viewPos.y()));

	QVector<QLine> greyLines;
	QVector<QLine> blackLines;

	m_painter.setPen(Qt::black);
	m_painter.setFont(m_font);

	if (tablePosY == 0)
		m_painter.drawText(QRect(4, 0, m_leftWidth, m_rowHeight), "Frame");
	greyLines.push_back(QLine(0, m_rowHeight - 1, rect.right(), m_rowHeight - 1));
	greyLines.push_back(QLine(0, m_rowHeight, rect.right(), m_rowHeight));

	int i, j;
	for (i = tablePosY; i < m_rows.size(); i++)
	{
		const Row& row = m_rows[i];
		if (i == m_selection.y())
			m_painter.fillRect(QRect(0, (i + 1) * m_rowHeight, rect.right(), m_rowHeight), QColor(230, 230, 230));
		greyLines.push_back(QLine(rect.left(), (i + 2) * m_rowHeight, rect.right(), (i + 2) * m_rowHeight));
		m_painter.drawText(QRect(4, (i + 1) * m_rowHeight, m_leftWidth, m_rowHeight), row.name);
		if ((i + 2) * m_rowHeight >= rect.bottom() + m_viewPos.y())
			break;
	}

	if (m_frameCount != -1)
	{
		i = m_leftWidth + m_frameCount * m_frameWidth - m_viewPos.x() + 2;
		if (i < m_leftWidth)
			i = m_leftWidth;
		m_painter.fillRect(QRect(i, 0, i + rect.width() - m_leftWidth, rect.bottom() + m_viewPos.y() + 1), QColor(0, 0, 0, 30));
	}

	blackLines.push_back(QLine(m_leftWidth - 4, m_viewPos.y(), m_leftWidth - 4, rect.bottom() + m_viewPos.y()));
	blackLines.push_back(QLine(m_leftWidth - 1, m_viewPos.y(), m_leftWidth - 1, rect.bottom() + m_viewPos.y()));

	m_painter.setPen(QColor(170, 170, 170));
	m_painter.drawLines(greyLines);
	m_painter.setPen(Qt::black);
	m_painter.drawLines(blackLines);

	greyLines.clear();
	blackLines.clear();
	QVector<QLine> redLines;

	m_painter.setClipRect(QRect(m_leftWidth, m_viewPos.y(), rect.right() - m_leftWidth, m_viewPos.y() + rect.bottom()));
	m_painter.translate(QPoint(m_leftWidth - m_viewPos.x() + m_frameWidth / 2 + 1, 0));

	const int maxRight = m_viewPos.x() + rect.right() - m_leftWidth;
	int pos;
	for (i = tablePosX; i * m_frameWidth <= maxRight; i++)
	{
		pos = i * m_frameWidth;
		if (i % 5 == 0)
		{
			greyLines.push_back(QLine(pos, m_rowHeight + 1, pos, m_viewPos.y() + rect.bottom() + 1));
			blackLines.push_back(QLine(pos, 0, pos, m_rowHeight - 1));
			m_painter.drawText(QRect(pos + 2, -2, m_frameWidth * 2 - 1, m_rowHeight), QString::number(i));
		}
		else
			blackLines.push_back(QLine(pos, m_rowHeight - 5, pos, m_rowHeight - 1));
	}

	redLines.push_back(QLine(m_currentFrame * m_frameWidth, m_viewPos.y(), m_currentFrame * m_frameWidth, m_viewPos.y() + rect.bottom() + 1));
	redLines.push_back(QLine(m_currentFrame * m_frameWidth - 1, m_viewPos.y(), m_currentFrame * m_frameWidth - 1, m_viewPos.y() + rect.bottom() + 1));

	m_painter.setPen(QColor(170, 170, 170));
	m_painter.drawLines(greyLines);
	m_painter.setPen(Qt::black);
	m_painter.drawLines(blackLines);

	for (i = tablePosY; i < m_rows.size(); i++)
	{
		const Row& row = m_rows[i];
		if (row.keys.size() > 0)
		{
			m_painter.setBrush(row.color);
			for (j = 0; j < row.keys.size(); j++)
			{
				pos = row.keys[j] * m_frameWidth;
				if (pos >= m_viewPos.x() - m_frameWidth && pos <= maxRight)
				{
					if (m_selection.x() == j && m_selection.y() == i)
					{
						m_painter.setBrush(QColor(255, 255, 0));
						m_painter.drawEllipse(QPoint(pos, (i + 1) * m_rowHeight + m_rowHeight / 2 + 1), m_frameWidth / 2 - 2, m_rowHeight / 2 - 2);
						m_painter.setBrush(row.color);
					}
					else
						m_painter.drawEllipse(QPoint(pos, (i + 1) * m_rowHeight + m_rowHeight / 2 + 1), m_frameWidth / 2 - 2, m_rowHeight / 2 - 2);
				}
			}
		}
	}

	m_painter.setPen(Qt::red);
	m_painter.drawLines(redLines);

	m_painter.end();

	event->accept();
}

void CTimeline::mouseMoveEvent(QMouseEvent* event)
{
	const QPoint mousePos(event->x(), event->y());
	const QPoint mouseMove = m_lastMousePos - mousePos;

	if (m_movingView)
	{
		m_viewPos += mouseMove;

		if (m_viewPos.x() < 0)
			m_viewPos.setX(0);
		if (m_viewPos.y() < 0)
			m_viewPos.setY(0);
	}

	const QPoint oldSelection = m_selection;

	if (m_movingCurrentFrame)
	{
		if (mousePos.x() > m_leftWidth)
		{
			int frame = (mousePos.x() - m_leftWidth) + m_viewPos.x() - 2;
			if (frame <= 0)
				frame = 0;
			else
				frame /= m_frameWidth;
			if ((m_frameCount == -1 || frame < m_frameCount) && frame != m_currentFrame)
			{
				m_currentFrame = frame;
				emit CurrentFrameChanged(m_currentFrame);
			}
		}

		m_selection = QPoint(-1, -1);
		int row = event->y() + m_viewPos.y();
		row -= m_rowHeight;
		if (row > 0)
		{
			row /= m_rowHeight;
			if (row < m_rows.size())
			{
				m_selection.setY(row);

				if (mousePos.x() > m_leftWidth)
				{
					int frame = (mousePos.x() - m_leftWidth) + m_viewPos.x() - 2;
					if (frame <= 0)
						frame = 0;
					else
						frame /= m_frameWidth;
					if (m_frameCount == -1 || frame < m_frameCount)
					{
						for (int i = 0; i < m_rows[row].keys.size(); i++)
						{
							if (m_rows[row].keys[i] == frame)
							{
								m_selection.setX(i);
								break;
							}
						}
					}
				}
			}
		}
	}

	if (m_movingView || m_movingCurrentFrame)
		update();

	if (oldSelection != m_selection)
		_emitSelectionChanged();

	m_lastMousePos = mousePos;
	event->accept();
}

void CTimeline::mousePressEvent(QMouseEvent* event)
{
	m_lastMousePos = QPoint(event->x(), event->y());
	bool mustUpdate = false;

	const QPoint oldSelection = m_selection;

	if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton)
		m_movingView = true;
	else if (event->button() == Qt::LeftButton && (m_frameCount == -1 || m_frameCount > 0))
	{
		m_movingCurrentFrame = true;

		if (m_lastMousePos.x() > m_leftWidth)
		{
			int frame = (m_lastMousePos.x() - m_leftWidth) + m_viewPos.x() - 2;
			if (frame <= 0)
				frame = 0;
			else
				frame /= m_frameWidth;
			if ((m_frameCount == -1 || frame < m_frameCount) && frame != m_currentFrame)
			{
				m_currentFrame = frame;
				emit CurrentFrameChanged(m_currentFrame);
				mustUpdate = true;
			}
		}

		m_selection = QPoint(-1, -1);
		int row = event->y() + m_viewPos.y();
		row -= m_rowHeight;
		if (row > 0)
		{
			row /= m_rowHeight;
			if (row < m_rows.size())
			{
				m_selection.setY(row);
				mustUpdate = true;

				if (m_lastMousePos.x() > m_leftWidth)
				{
					int frame = (m_lastMousePos.x() - m_leftWidth) + m_viewPos.x() - 2;
					if (frame <= 0)
						frame = 0;
					else
						frame /= m_frameWidth;
					if (m_frameCount == -1 || frame < m_frameCount)
					{
						for (int i = 0; i < m_rows[row].keys.size(); i++)
						{
							if (m_rows[row].keys[i] == frame)
							{
								m_selection.setX(i);
								break;
							}
						}
					}
				}
			}
		}
	}

	if (mustUpdate)
		update();

	if (oldSelection != m_selection)
		_emitSelectionChanged();

	event->accept();
}

void CTimeline::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton)
		m_movingView = false;
	else if (event->button() == Qt::LeftButton)
		m_movingCurrentFrame = false;

	event->accept();
}

void CTimeline::mouseDoubleClickEvent(QMouseEvent* event)
{
	const QPoint mousePos(event->x(), event->y());

	if (m_frameCount == 0 || mousePos.x() <= m_leftWidth)
		return;

	int frame = (mousePos.x() - m_leftWidth) + m_viewPos.x() - 2;
	if (frame <= 0)
		frame = 0;
	else
		frame /= m_frameWidth;
	if (m_frameCount != -1 && frame >= m_frameCount)
		return;

	int row = event->y() + m_viewPos.y();
	row -= m_rowHeight;
	if (row <= 0)
		return;

	row /= m_rowHeight;
	if (row >= m_rows.size())
		return;

	if (m_rows[row].keys.contains(frame))
	{
		int i = 0;
		for (auto it = m_rows[row].keys.begin(); it != m_rows[row].keys.end();)
		{
			if ((*it) == frame)
			{
				it = m_rows[row].keys.erase(it);
				emit KeyModified(row, frame, true);
				if (m_selection.y() == row && m_selection.x() == i)
				{
					m_selection.setX(-1);
					_emitSelectionChanged();
				}
				break;
			}
			else
				it++;
			i++;
		}
	}
	else
	{
		m_rows[row].keys.push_back(frame);
		emit KeyModified(row, frame, false);

		m_selection = QPoint(m_rows[row].keys.size() - 1, row);
		_emitSelectionChanged();
	}

	update();

	event->accept();
}

void CTimeline::_emitSelectionChanged()
{
	int frame = -1;
	if (m_selection.y() >= 0 && m_selection.x() >= 0)
		frame = m_rows[m_selection.y()].keys[m_selection.x()];
	emit SelectionChanged(m_selection.y(), frame);
}

QPoint CTimeline::GetSelection() const
{
	QPoint selection = m_selection;
	if (selection.y() >= 0 && selection.x() >= 0)
		selection.setX(m_rows[selection.y()].keys[selection.x()]);
	return selection;
}