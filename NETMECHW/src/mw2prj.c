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

// Resource type tags in the mw2.prj archive and their file extensions, one global each: the
// original puts each string right before its pointer, which VC++ 2.2 gives separate globals (an
// array's strings come first). The shell's mw2prj.c has them as two arrays.
// GLOBAL: NETMECHW 0x10023960
char* g_resTagSnds = "SNDS";

// GLOBAL: NETMECHW 0x10023968
char* g_resTagCel = "CEL";

// GLOBAL: NETMECHW 0x10023970
char* g_resTagXyc = "XYC";

// GLOBAL: NETMECHW 0x10023978
char* g_resTagShp = "SHP";

// GLOBAL: NETMECHW 0x10023984
char* g_resTagFont = "FONT";

// GLOBAL: NETMECHW 0x10023990
char* g_resTagMenu = "MENU";

// GLOBAL: NETMECHW 0x1002399c
char* g_resTagDisp = "DISP";

// GLOBAL: NETMECHW 0x100239a8
char* g_resTagXmid = "XMID";

// GLOBAL: NETMECHW 0x100239b0
char* g_resTagPal = "PAL";

// GLOBAL: NETMECHW 0x100239bc
char* g_resTagTable = "TABL";

// GLOBAL: NETMECHW 0x100239c8
char* g_resTagPoly = "POLY";

// GLOBAL: NETMECHW 0x100239d4
char* g_resTagText = "TEXT";

// GLOBAL: NETMECHW 0x100239e0
char* g_resTagAnim = "ANIM";

// GLOBAL: NETMECHW 0x100239ec
char* g_resTagMgeo = "MGEO";

// GLOBAL: NETMECHW 0x100239f4
char* g_resTagHud = "HUD";

// GLOBAL: NETMECHW 0x10023a00
char* g_resTagCpit = "CPIT";

// GLOBAL: NETMECHW 0x10023a08
char* g_resTagVpt = "VPT";

// GLOBAL: NETMECHW 0x10023a14
char* g_resTagMpit = "MPIT";

// GLOBAL: NETMECHW 0x10023a1c
char* g_resTagBwd = "BWD";

// GLOBAL: NETMECHW 0x10023a24
char* g_resTagVer = "VER";

// GLOBAL: NETMECHW 0x10023a2c
char* g_resTagAit = "AIT";

// GLOBAL: NETMECHW 0x10023a34
char* g_resTagMek = "MEK";

// GLOBAL: NETMECHW 0x10023a40
char* g_resTagLuma = "LUMA";

// GLOBAL: NETMECHW 0x10023a48
char* g_resTagMus = "MUS";

// GLOBAL: NETMECHW 0x10023a50
char* g_resTagGif = "GIF";

// GLOBAL: NETMECHW 0x10023a5c
char* g_resTagNtxt = "NTXT";

// GLOBAL: NETMECHW 0x10023a68
char* g_resExtSfl = ".sfl";

// GLOBAL: NETMECHW 0x10023a74
char* g_resExtXel = ".xel";

// GLOBAL: NETMECHW 0x10023a80
char* g_resExtXyc = ".xyc";

// GLOBAL: NETMECHW 0x10023a8c
char* g_resExtShp = ".shp";

// GLOBAL: NETMECHW 0x10023a98
char* g_resExtFnt = ".fnt";

// GLOBAL: NETMECHW 0x10023aa4
char* g_resExtMenuDll = ".dll";

// GLOBAL: NETMECHW 0x10023ab0
char* g_resExtDispDll = ".dll";

// GLOBAL: NETMECHW 0x10023abc
char* g_resExtXmi = ".xmi";

// GLOBAL: NETMECHW 0x10023ac8
char* g_resExtCol = ".col";

// GLOBAL: NETMECHW 0x10023ad4
char* g_resExtTbl = ".tbl";

// GLOBAL: NETMECHW 0x10023ae0
char* g_resExtWtb = ".wtb";

// GLOBAL: NETMECHW 0x10023aec
char* g_resExtXxt = ".xxt";

// GLOBAL: NETMECHW 0x10023af8
char* g_resExt3di = ".3di";

// GLOBAL: NETMECHW 0x10023b04
char* g_resExtMgi = ".mgi";

// GLOBAL: NETMECHW 0x10023b10
char* g_resExtHdi = ".hdi";

// GLOBAL: NETMECHW 0x10023b1c
char* g_resExtCpi = ".cpi";

// GLOBAL: NETMECHW 0x10023b28
char* g_resExtVpi = ".vpi";

// GLOBAL: NETMECHW 0x10023b34
char* g_resExtPit = ".pit";

// GLOBAL: NETMECHW 0x10023b40
char* g_resExtBwd = ".bwd";

// GLOBAL: NETMECHW 0x10023b4c
char* g_resExtAit = ".ait";

// GLOBAL: NETMECHW 0x10023b58
char* g_resExtMek = ".mek";

// GLOBAL: NETMECHW 0x10023b64
char* g_resExtLum = ".lum";

// GLOBAL: NETMECHW 0x10023b70
char* g_resExtMus = ".mus";

// GLOBAL: NETMECHW 0x10023b7c
char* g_resExtGif = ".gif";

// GLOBAL: NETMECHW 0x10023b88
char* g_resExtTxt = ".txt";

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
