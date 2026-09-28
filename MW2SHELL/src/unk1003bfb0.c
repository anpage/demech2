#include "unk1003bfb0.h"

#include "decomp.h"
#include "types.h"
#include "unk10013690.h"
#include "unk1002fb90.h"
#include "unk10031970.h"

#include <mbstring.h>
#include <stdlib.h>

void* FUN_1003c061(undefined4 p_size);
void FUN_1003c07d(void* p_mem);

// Resource type tags in the mw2.prj archive (0x1006a9f8) and their file extensions (0x1006aa60).
// Not annotated yet: reccmp pairs strings by text, and the original's earlier "MEK", ".mek"
// and "BWD" literals in units not decompiled yet take the matches for these tables' entries,
// so datacmp would report those entries as diffs.
char* g_unk0x1006a9f8[26] = {"SNDS", "CEL",  "XYC",  "SHP",  "FONT", "MENU", "DISP", "XMID", "PAL",
							 "TABL", "POLY", "TEXT", "ANIM", "MGEO", "HUD",  "CPIT", "VPT",  "MPIT",
							 "BWD",  "VER",  "AIT",  "MEK",  "LUMA", "MUS",  "GIF",  "NTXT"};

char* g_unk0x1006aa60[25] = {".sfl", ".xel", ".xyc", ".shp", ".fnt", ".dll", ".dll", ".xmi", ".col",
							 ".tbl", ".wtb", ".xxt", ".3di", ".mgi", ".hdi", ".cpi", ".vpi", ".pit",
							 ".bwd", ".ait", ".mek", ".lum", ".mus", ".gif", ".txt"};

// GLOBAL: MW2SHELL 0x1006aac4
MechS32 g_unk0x1006aac4 = -1;

// GLOBAL: MW2SHELL 0x1006aac8
char* g_unk0x1006aac8 = NULL;

// FUNCTION: MW2SHELL 0x1003bfb0
MechS32 FUN_1003bfb0(void)
{
	MechS32 result;

	result = TRUE;
	FUN_1002fb90(FUN_1003c061, FUN_1003c07d);
	FUN_10013907();

	if (g_unk0x1006aac8 == NULL) {
		g_unk0x1006aac8 = (char*) _mbsdup((unsigned char*) FUN_10031a8f("mw2.prj"));
	}

	g_unk0x1006aac4 = FUN_1002fcdc(g_unk0x1006aac8, 0);
	if (g_unk0x1006aac4 != -1) {
		FUN_1003024f(g_unk0x1006aac4);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2SHELL 0x1003c048
void FUN_1003c048(void)
{
	FUN_1002ffc9(g_unk0x1006aac4);
}

// FUNCTION: MW2SHELL 0x1003c061
void* FUN_1003c061(undefined4 p_size)
{
	return FUN_10013fa9(p_size);
}

// FUNCTION: MW2SHELL 0x1003c07d
void FUN_1003c07d(void* p_mem)
{
	free(p_mem);
}
