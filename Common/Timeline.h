///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef TIMELINE_H
#define TIMELINE_H

class CTimeline : public QWidget
{
	Q_OBJECT

public:
	CTimeline(QWidget* parent = null, Qt::WindowFlags flags = 0);

	void AddRow(const string& name, const QColor& color = QColor(255, 100, 100));
	void RemoveRow(int index);
	void RenameRow(int index, const string& newName);
	void RemoveAllRows();

	void AddKey(int row, int frame);
	void RemoveKey(int row, int frame);
	void RemoveAllKeys();

	void SetCurrentFrame(int frame);
	void SetFrameCount(int frameCount);
	void SetSelectedRow(int row);

	QPoint GetSelection() const;

signals:
	void KeyModified(int row, int frame, bool removed);
	void CurrentFrameChanged(int frame);
	void SelectionChanged(int row, int frame);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent *event);

private:
	struct Row
	{
		string name;
		QColor color;
		QVector<int> keys;
	};

	QPainter m_painter;
	QVector<Row> m_rows;
	QPoint m_viewPos;
	QPoint m_lastMousePos;
	bool m_movingView;
	bool m_movingCurrentFrame;
	int m_frameCount;
	int m_currentFrame;
	QPoint m_selection;
	QFont m_font;
	int m_leftWidth, m_rowHeight, m_frameWidth;

	void _emitSelectionChanged();
};

#endif // TIMELINE_H