///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include <stdafx.h>
#include "Path.h"
#include "Project.h"

CPath::CPath()
	: m_index(0),
	m_motion(MTI_WALK),
	m_delay(0)
{
}

bool CPath::Init()
{
	m_modelProp = Project->GetModelProp(OT_REGION, RI_BEGIN);
	ResetScale();
	return m_modelProp != null;
}

void CPath::ResetScale()
{
	m_scale.x = m_scale.y = m_scale.z = 0.2f;
	m_updateMatrix = true;
}