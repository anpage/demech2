#include "resourcefile.h"

#include "ai.h"
#include "config.h"
#include "error.h"
#include "loadres.h"
#include "prjfile.h"
#include "simmain.h"
#include "types.h"
#include "weapons.h"

#include <mbstring.h>
#include <windows.h>

// The resource type names and file extensions. In the original this object's data (these
// pointers, their strings, then FirstResource's literals) follows the rest of resource.c's.

// GLOBAL: MW2 0x100a8674
const char* g_unk0x100a8674 = "SNDS";

// GLOBAL: MW2 0x100a8678
const char* g_unk0x100a8678 = "CEL";

// GLOBAL: MW2 0x100a8680
const char* g_unk0x100a8680 = "SHP";

// GLOBAL: MW2 0x100a8684
const char* g_unk0x100a8684 = "FONT";

// GLOBAL: MW2 0x100a8690
const char* g_unk0x100a8690 = "XMID";

// GLOBAL: MW2 0x100a8694
const char* g_unk0x100a8694 = "PAL";

// GLOBAL: MW2 0x100a8698
const char* g_unk0x100a8698 = "TABL";

// GLOBAL: MW2 0x100a869c
const char* g_unk0x100a869c = "POLY";

// GLOBAL: MW2 0x100a86a0
const char* g_unk0x100a86a0 = "TEXT";

// GLOBAL: MW2 0x100a86a4
const char* g_unk0x100a86a4 = "ANIM";

// GLOBAL: MW2 0x100a86a8
const char* g_unk0x100a86a8 = "MGEO";

// GLOBAL: MW2 0x100a86ac
const char* g_unk0x100a86ac = "HUD";

// GLOBAL: MW2 0x100a86b0
const char* g_unk0x100a86b0 = "CPIT";

// GLOBAL: MW2 0x100a86bc
const char* g_unk0x100a86bc = g_unk0x100a87c0;

// GLOBAL: MW2 0x100a86c4
const char* g_unk0x100a86c4 = "AIT";

// GLOBAL: MW2 0x100a86c8
const char* g_unk0x100a86c8 = "MEK";

// GLOBAL: MW2 0x100a86cc
const char* g_unk0x100a86cc = "LUMA";

// GLOBAL: MW2 0x100a86d0
const char* g_unk0x100a86d0 = "MUS";

// GLOBAL: MW2 0x100a8704
const char* g_unk0x100a8704 = ".wtb";

// GLOBAL: MW2 0x100a870c
const char* g_unk0x100a870c = ".3di";

// GLOBAL: MW2 0x100a8710
const char* g_unk0x100a8710 = ".mgi";

// GLOBAL: MW2 0x100a8714
const char* g_unk0x100a8714 = ".hdi";

// GLOBAL: MW2 0x100a8718
const char* g_unk0x100a8718 = ".cpi";

// GLOBAL: MW2 0x100a872c
const char* g_unk0x100a872c = ".mek";

// GLOBAL: MW2 0x100a8740
undefined4 g_unk0x100a8740 = 0xffffffff;

// GLOBAL: MW2 0x100a8744
MechChar* g_unk0x100a8744 = NULL;

// GLOBAL: MW2 0x100a87c0
char g_unk0x100a87c0[] = "BWD";

// FUNCTION: MW2 0x10050780
MechS32 FirstResource(void)
{
	MechS32 result;

	result = TRUE;
	SetPrjAllocator(FUN_100508c0, FUN_100508dc);
	FUN_10019d73();
	if (!g_unk0x100a8744) {
		g_unk0x100a8744 = (MechChar*) _mbsdup((unsigned char*) BuildGamePath("mw2.prj"));
	}

	g_unk0x100a8740 = OpenPrjFile(g_unk0x100a8744, 0);
	if (g_unk0x100a8740 != -1) {
		LoadPrjIndexes(g_unk0x100a8740);
	}
	else {
		Error(3, "\nCan't find file \"%s\"", g_unk0x100a8744, 0);
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x1005082f
void CloseResourceFile(void)
{
	ClosePrjFile(g_unk0x100a8740);
}

// FUNCTION: MW2 0x10050848
void CachePreloads(void)
{
	FUN_10045a5b();
	FUN_1006f480();
	LoadAIScripts();
}

// FUNCTION: MW2 0x10050862
MechS32 FUN_10050862(MechS32 p_id, const char* p_type)
{
	MechS32 result;
	void* data;

	data = FUN_1001a19f(g_unk0x100a8740, p_id, p_type, 0);
	if (data) {
		result = TRUE;
		FUN_1001a163(p_id, p_type);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x100508c0
void* FUN_100508c0(MechU32 p_size)
{
	return MemAlloc(p_size);
}

// FUNCTION: MW2 0x100508dc
void FUN_100508dc(void* p_block)
{
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
}
