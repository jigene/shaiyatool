///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef MOVER_H
#define MOVER_H

#include "SpawnObject.h"

#define PARTSMESH_HAIR( sex )  ( sex == SEX_MALE ? ( "Part_maleHair%02d.o3d" ) : ( "Part_femaleHair%02d.o3d" ) )
#define PARTSMESH_HEAD( sex )  ( sex == SEX_MALE ? ( "Part_maleHead%02d.o3d" ) : ( "Part_femaleHead%02d.o3d" ) )
#define PARTSMESH_UPPER( sex ) ( sex == SEX_MALE ? ( "Part_maleUpper.o3d"    ) : ( "Part_femaleUpper.o3d"    ) )
#define PARTSMESH_LOWER( sex ) ( sex == SEX_MALE ? ( "Part_maleLower.o3d"    ) : ( "Part_femaleLower.o3d"    ) )
#define PARTSMESH_HAND( sex )  ( sex == SEX_MALE ? ( "Part_maleHand.o3d"     ) : ( "Part_femaleHand.o3d"     ) )
#define PARTSMESH_FOOT( sex )  ( sex == SEX_MALE ? ( "Part_maleFoot.o3d"     ) : ( "Part_femaleFoot.o3d"     ) )

struct MoverProp;
struct Character;

class CMover : public CSpawnObject
{
public:
	CMover();

	virtual void Read(CFile& file);
	virtual void Write(CFile& file, const D3DXVECTOR3& posOffset);
	virtual bool Init();
	virtual void RenderName();

	virtual DWORD GetRectColor() {
		return 0xffff0000;
	}
	bool IsPeaceful() const;

	void InitProperties();

protected:
	uint m_motion;
	uint m_AIInterface;
	string m_name;
	string m_characterKey;
	uint m_belligerence;
	MoverProp* m_moverProp;
	Character* m_character;

	virtual void _loadModel();

	friend class CObject;
	friend class CWorld;

#ifdef WORLD_EDITOR
	WORLD_EDITOR_FRIENDS
#endif // WORLD_EDITOR
};

#endif // MOVER_H