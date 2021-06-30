///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef EDITCOMMAND_H
#define EDITCOMMAND_H

class CWorld;

class CEditCommand : public QUndoCommand
{
public:
	CEditCommand(CWorld* world);

	// dont override this
	virtual void redo();
	virtual void undo();

	// override this
	virtual void Apply(bool undo = false) = 0;
	virtual bool IsEmpty() const = 0;

protected:
	CWorld* m_world;

private:
	bool m_firstCall;
};

#endif // EDITCOMMAND_H