///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "File.h"

CFile::CFile()
	: m_cur(null),
	m_mode(QIODevice::NotOpen)
{
}

CFile::~CFile()
{
	Close();
}

bool CFile::Open(const string& filename, QIODevice::OpenModeFlag mode, bool error)
{
	Close();

	if (mode == QIODevice::ReadOnly)
		m_mode = QIODevice::ReadOnly;
	else if (mode == QIODevice::WriteOnly)
		m_mode = QIODevice::WriteOnly;
	else
	{
		qCritical("CFile open mode not supported");
		return false;
	}

	m_file.setFileName(filename);

	if (!m_file.open(m_mode))
	{
		if (error)
			qCritical(("Can't open file '" + filename + "'").toLocal8Bit());
		return false;
	}

	if (mode == QIODevice::ReadOnly)
	{
		m_buffer = m_file.readAll();
		m_cur = m_buffer.data();
	}

	return true;
}

void CFile::Close()
{
	if (m_mode == QIODevice::NotOpen)
		return;

	m_mode = QIODevice::NotOpen;
	m_buffer.clear();
	m_file.close();
	m_cur = null;
}

void CFile::Skip(int byteCount)
{
	if (m_mode == QIODevice::ReadOnly)
		m_cur += byteCount;
}

void CFile::SetPos(int bytePos)
{
	if (m_mode == QIODevice::ReadOnly)
		m_cur = m_buffer.data() + bytePos;
}

int CFile::GetSize()
{
	if (m_mode == QIODevice::ReadOnly)
		return (int)m_buffer.size();
	return 0;
}

const char* CFile::GetBuffer() const
{
	if (m_mode == QIODevice::ReadOnly)
		return m_buffer.constData();
	return null;
}