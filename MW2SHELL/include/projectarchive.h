#ifndef PROJECTARCHIVE_H
#define PROJECTARCHIVE_H

#include "decomp.h"
#include "starmech.h"
#include "types.h"

// SIZE 0x10
class ProjectArchive {
public:
	ProjectArchive(const char* p_name);
	~ProjectArchive();

	MechS32 FUN_1002e346(char* p_name, MechS32 p_type);
	void* FUN_1002e384(MechS32 p_id, char* p_tag);
	void* FUN_1002e3cf(char* p_name, MechS32 p_type, char* p_tag);
	void FUN_1002e404(MechS32 p_id, char* p_tag);
	void FUN_1002e445(char* p_name, MechS32 p_type, char* p_tag);
	MechS32* FUN_1002e47a(MechS32* p_list, MechS32 p_index, MechS32* p_previous);
	MechU8 FUN_1002e512(char* p_name);
	MechS32* FUN_1002e5d8(MechS32 p_type);

private:
	MechS32 m_unk0x00;    // 0x00
	undefined4 m_unk0x04; // 0x04
	MechS32* m_unk0x08;   // 0x08
	MechS32 m_unk0x0c;    // 0x0c
};

// The functions and globals of projectarchive.cpp that other units use.
extern char g_unk0x100668c0[][4];
extern MechS32 g_unk0x10066a44;

void FUN_1002ea62(MechS32 p_count, StarMech* p_mechs, MechS32 p_enemyCount, StarMech* p_enemies);
void PrjBuildPlayerStarTemplates(MechS32 p_clan, MechS32 p_rival);

#endif // PROJECTARCHIVE_H
