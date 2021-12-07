///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "EditCommand.h"
#include <MainFrame.h>

CEditCommand::CEditCommand(CWorld* world)
	: m_firstCall(true),
	m_world(world)
{
}

void CEditCommand::redo()
{
	if (m_firstCall)
		m_firstCall = false;
	else
	{
		Apply();
		MainFrame->UpdateWorldEditor();
	}
}

void CEditCommand::undo()
{
	Apply(true);
	MainFrame->UpdateWorldEditor();
}