#include "mw2prj.h"

#include "decomp.h"
#include "prjfile.h"
#include "resourcecache.h"
#include "resourcefile.h"
#include "types.h"

#include <mbstring.h>
#include <stdlib.h>

// NETMECHW.DLL's copy of the shell's mw2prj.c, the same source but where noted.

void* Mw2PrjAlloc(undefined4 p_size);
void Mw2PrjFree(void* p_mem);

// Resource type tags in the mw2.prj archive (0x10023960) and their file extensions
// (0x10023a60), unused in NETMECHW.DLL. Not annotated: the original puts each string right
// before its pointer, where VC++ 2.2 emits an array's strings ahead of the whole array.
char* g_resourceTypeTags[26] = {"SNDS", "CEL",  "XYC",  "SHP",  "FONT", "MENU", "DISP", "XMID", "PAL",
								"TABL", "POLY", "TEXT", "ANIM", "MGEO", "HUD",  "CPIT", "VPT",  "MPIT",
								"BWD",  "VER",  "AIT",  "MEK",  "LUMA", "MUS",  "GIF",  "NTXT"};

char* g_resourceTypeExtensions[25] = {".sfl", ".xel", ".xyc", ".shp", ".fnt", ".dll", ".dll", ".xmi", ".col",
									  ".tbl", ".wtb", ".xxt", ".3di", ".mgi", ".hdi", ".cpi", ".vpi", ".pit",
									  ".bwd", ".ait", ".mek", ".lum", ".mus", ".gif", ".txt"};

// GLOBAL: NETMECHW 0x10023b8c
MechS32 g_mw2PrjHandle = -1;

// GLOBAL: NETMECHW 0x10023b90
char* g_mw2PrjPath = NULL;

// FUNCTION: NETMECHW 0x10011c20
MechS32 InitializeMw2Prj(void)
{
	MechS32 result;

	result = TRUE;
	SetArchiveAllocator(Mw2PrjAlloc, Mw2PrjFree);
	InitializeResourceCache();

	if (g_mw2PrjPath == NULL) {
		g_mw2PrjPath = (char*) _mbsdup((unsigned char*) MakeResourcePath("mw2.prj"));
	}

	g_mw2PrjHandle = OpenArchive(g_mw2PrjPath, 0);
	if (g_mw2PrjHandle != -1) {
		LoadArchiveEntries(g_mw2PrjHandle);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: NETMECHW 0x10011cb8
void ShutdownMw2Prj(void)
{
	CloseArchive(g_mw2PrjHandle);
}

// FUNCTION: NETMECHW 0x10011cd1
void* Mw2PrjAlloc(undefined4 p_size)
{
	return AllocateMemory(p_size);
}

// FUNCTION: NETMECHW 0x10011ced
void Mw2PrjFree(void* p_mem)
{
	free(p_mem);
}
