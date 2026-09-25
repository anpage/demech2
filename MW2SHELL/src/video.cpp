#include "decomp.h"
#include "types.h"

#include <stdio.h>

extern MechChar g_szDataDrivePath[];

// GLOBAL: MW2SHELL 0x10064b28
undefined4 g_unk0x10064b28 = 0;

// GLOBAL: MW2SHELL 0x1007cdd0
MechChar g_unk0x1007cdd0[0x100];

// FUNCTION: MW2SHELL 0x10015d30
MechChar* GetPathToVideo(const MechChar* p_name)
{
	if (g_unk0x10064b28 && g_szDataDrivePath[0]) {
		sprintf(g_unk0x1007cdd0, "%ssmk\\%s.smk", g_szDataDrivePath, p_name);
	}
	else {
		sprintf(g_unk0x1007cdd0, "smk\\%s.smk", p_name);
	}

	g_unk0x10064b28 = 0;
	return g_unk0x1007cdd0;
}

// STUB: MW2SHELL 0x10015f58
void FUN_10015f58(const char* p_name, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	STUB(0x10015f58);
}

// STUB: MW2SHELL 0x10015fed
MechS32 PlayFullscreenVideo(const char* p_name, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	STUB(0x10015fed);
	return 0;
}

// STUB: MW2SHELL 0x10016be7
MechS32 FUN_10016be7()
{
	STUB(0x10016be7);
	return 0;
}

// STUB: MW2SHELL 0x10016c1d
void FUN_10016c1d()
{
	STUB(0x10016c1d);
}

// STUB: MW2SHELL 0x10016c3e
void FUN_10016c3e()
{
	STUB(0x10016c3e);
}

// STUB: MW2SHELL 0x10016d90
void FUN_10016d90(MechS32 p_unk0x00)
{
	STUB(0x10016d90);
}

// STUB: MW2SHELL 0x10016f45
void FUN_10016f45()
{
	STUB(0x10016f45);
}
