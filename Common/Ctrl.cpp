///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Ctrl.h"

CCtrl::CCtrl()
{
	memset(&m_elem, 0, sizeof(m_elem));
}

void CCtrl::Read(CFile& file)
{
	CObject::Read(file);

	uint version;
	file.Read(version);
	if (version == 0x80000000)
		file.Read(m_elem);
	else if (version == 0x90000000)
	{
		file.Read(((char*)&m_elem), 88);
		file.Read(((char*)&m_elem) + 152, sizeof(m_elem) - 152);
	}
	else
	{
		m_elem.dwSet = version;
		file.Read(((char*)&m_elem.dwSetItem), sizeof(m_elem) - (sizeof(uint) * 10));
	}
}

void CCtrl::Write(CFile& file, const D3DXVECTOR3& posOffset)
{
	CObject::Write(file, posOffset);

	const uint version = 0x80000000;
	file.Write(version);
	file.Write(m_elem);
}