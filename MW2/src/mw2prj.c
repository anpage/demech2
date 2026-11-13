#include "mw2prj.h"

#include "ai.h"
#include "animation.h"
#include "config.h"
#include "error.h"
#include "loadres.h"
#include "prjfile.h"
#include "simmain.h"
#include "types.h"
#include "weapons.h"

#include <mbstring.h>
#include <windows.h>

// The resource type tags in the mw2.prj archive and their file extensions, one global each: the
// 3Dfx edition's 2.2 puts each string right before its pointer, which only separate globals give
// (an array's strings come first; 4.1 lays both out the same way). MW2SHELL's mw2prj.c has them as
// two arrays. In the original this object's data (the pointers, their strings, then
// FirstResource's literals) follows the rest of resource.c's.
// GLOBAL: MW2 0x100a8674
// GLOBAL: MW2MATROX 0x100a5a44
// GLOBAL: MW23DFX 0x102596a8
char* g_resTagSnds = "SNDS";

// GLOBAL: MW2 0x100a8678
// GLOBAL: MW2MATROX 0x100a5a48
// GLOBAL: MW23DFX 0x102596b0
char* g_resTagCel = "CEL";

// GLOBAL: MW2 0x100a867c
// GLOBAL: MW2MATROX 0x100a5a4c
// GLOBAL: MW23DFX 0x102596b8
char* g_resTagXyc = "XYC";

// GLOBAL: MW2 0x100a8680
// GLOBAL: MW2MATROX 0x100a5a50
// GLOBAL: MW23DFX 0x102596c0
char* g_resTagShp = "SHP";

// GLOBAL: MW2 0x100a8684
// GLOBAL: MW2MATROX 0x100a5a54
// GLOBAL: MW23DFX 0x102596cc
char* g_resTagFont = "FONT";

// GLOBAL: MW2 0x100a8688
// GLOBAL: MW2MATROX 0x100a5a58
// GLOBAL: MW23DFX 0x102596d8
char* g_resTagMenu = "MENU";

// GLOBAL: MW2 0x100a868c
// GLOBAL: MW2MATROX 0x100a5a5c
// GLOBAL: MW23DFX 0x102596e4
char* g_resTagDisp = "DISP";

// GLOBAL: MW2 0x100a8690
// GLOBAL: MW2MATROX 0x100a5a60
// GLOBAL: MW23DFX 0x102596f0
char* g_resTagXmid = "XMID";

// GLOBAL: MW2 0x100a8694
// GLOBAL: MW2MATROX 0x100a5a64
// GLOBAL: MW23DFX 0x102596f8
char* g_resTagPal = "PAL";

// GLOBAL: MW2 0x100a8698
// GLOBAL: MW2MATROX 0x100a5a68
// GLOBAL: MW23DFX 0x10259704
char* g_resTagTable = "TABL";

// GLOBAL: MW2 0x100a869c
// GLOBAL: MW2MATROX 0x100a5a6c
// GLOBAL: MW23DFX 0x10259710
char* g_resTagPoly = "POLY";

// GLOBAL: MW2 0x100a86a0
// GLOBAL: MW2MATROX 0x100a5a70
// GLOBAL: MW23DFX 0x1025971c
char* g_resTagText = "TEXT";

// GLOBAL: MW2 0x100a86a4
// GLOBAL: MW2MATROX 0x100a5a74
// GLOBAL: MW23DFX 0x10259728
char* g_resTagAnim = "ANIM";

// GLOBAL: MW2 0x100a86a8
// GLOBAL: MW2MATROX 0x100a5a78
// GLOBAL: MW23DFX 0x10259734
char* g_resTagMgeo = "MGEO";

// GLOBAL: MW2 0x100a86ac
// GLOBAL: MW2MATROX 0x100a5a7c
// GLOBAL: MW23DFX 0x1025973c
char* g_resTagHud = "HUD";

// GLOBAL: MW2 0x100a86b0
// GLOBAL: MW2MATROX 0x100a5a80
// GLOBAL: MW23DFX 0x10259748
char* g_resTagCpit = "CPIT";

// GLOBAL: MW2 0x100a86b4
// GLOBAL: MW2MATROX 0x100a5a84
// GLOBAL: MW23DFX 0x10259750
char* g_resTagVpt = "VPT";

// GLOBAL: MW2 0x100a86b8
// GLOBAL: MW2MATROX 0x100a5a88
// GLOBAL: MW23DFX 0x1025975c
char* g_resTagMpit = "MPIT";

// GLOBAL: MW2 0x100a86bc
// GLOBAL: MW2MATROX 0x100a5a8c
// GLOBAL: MW23DFX 0x10259764
char* g_resTagBwd = "BWD";

// GLOBAL: MW2 0x100a86c0
// GLOBAL: MW2MATROX 0x100a5a90
// GLOBAL: MW23DFX 0x1025976c
char* g_resTagVer = "VER";

// GLOBAL: MW2 0x100a86c4
// GLOBAL: MW2MATROX 0x100a5a94
// GLOBAL: MW23DFX 0x10259774
char* g_resTagAit = "AIT";

// GLOBAL: MW2 0x100a86c8
// GLOBAL: MW2MATROX 0x100a5a98
// GLOBAL: MW23DFX 0x1025977c
char* g_resTagMek = "MEK";

// GLOBAL: MW2 0x100a86cc
// GLOBAL: MW2MATROX 0x100a5a9c
// GLOBAL: MW23DFX 0x10259788
char* g_resTagLuma = "LUMA";

// GLOBAL: MW2 0x100a86d0
// GLOBAL: MW2MATROX 0x100a5aa0
// GLOBAL: MW23DFX 0x10259790
char* g_resTagMus = "MUS";

// GLOBAL: MW2 0x100a86d4
// GLOBAL: MW2MATROX 0x100a5aa4
// GLOBAL: MW23DFX 0x10259798
char* g_resTagGif = "GIF";

// GLOBAL: MW2 0x100a86d8
// GLOBAL: MW2MATROX 0x100a5aa8
// GLOBAL: MW23DFX 0x102597a4
char* g_resTagNtxt = "NTXT";

// GLOBAL: MW2 0x100a86dc
// GLOBAL: MW2MATROX 0x100a5aac
// GLOBAL: MW23DFX 0x102597b0
char* g_resExtSfl = ".sfl";

// GLOBAL: MW2 0x100a86e0
// GLOBAL: MW2MATROX 0x100a5ab0
// GLOBAL: MW23DFX 0x102597bc
char* g_resExtXel = ".xel";

// GLOBAL: MW2 0x100a86e4
// GLOBAL: MW2MATROX 0x100a5ab4
// GLOBAL: MW23DFX 0x102597c8
char* g_resExtXyc = ".xyc";

// GLOBAL: MW2 0x100a86e8
// GLOBAL: MW2MATROX 0x100a5ab8
// GLOBAL: MW23DFX 0x102597d4
char* g_resExtShp = ".shp";

// GLOBAL: MW2 0x100a86ec
// GLOBAL: MW2MATROX 0x100a5abc
// GLOBAL: MW23DFX 0x102597e0
char* g_resExtFnt = ".fnt";

// GLOBAL: MW2 0x100a86f0
// GLOBAL: MW2MATROX 0x100a5ac0
// GLOBAL: MW23DFX 0x102597ec
char* g_resExtMenuDll = ".dll";

// GLOBAL: MW2 0x100a86f4
// GLOBAL: MW2MATROX 0x100a5ac4
// GLOBAL: MW23DFX 0x102597f8
char* g_resExtDispDll = ".dll";

// GLOBAL: MW2 0x100a86f8
// GLOBAL: MW2MATROX 0x100a5ac8
// GLOBAL: MW23DFX 0x10259804
char* g_resExtXmi = ".xmi";

// GLOBAL: MW2 0x100a86fc
// GLOBAL: MW2MATROX 0x100a5acc
// GLOBAL: MW23DFX 0x10259810
char* g_resExtCol = ".col";

// GLOBAL: MW2 0x100a8700
// GLOBAL: MW2MATROX 0x100a5ad0
// GLOBAL: MW23DFX 0x1025981c
char* g_resExtTbl = ".tbl";

// GLOBAL: MW2 0x100a8704
// GLOBAL: MW2MATROX 0x100a5ad4
// GLOBAL: MW23DFX 0x10259828
char* g_resExtWtb = ".wtb";

// GLOBAL: MW2 0x100a8708
// GLOBAL: MW2MATROX 0x100a5ad8
// GLOBAL: MW23DFX 0x10259834
char* g_resExtXxt = ".xxt";

// GLOBAL: MW2 0x100a870c
// GLOBAL: MW2MATROX 0x100a5adc
// GLOBAL: MW23DFX 0x10259840
char* g_resExt3di = ".3di";

// GLOBAL: MW2 0x100a8710
// GLOBAL: MW2MATROX 0x100a5ae0
// GLOBAL: MW23DFX 0x1025984c
char* g_resExtMgi = ".mgi";

// GLOBAL: MW2 0x100a8714
// GLOBAL: MW2MATROX 0x100a5ae4
// GLOBAL: MW23DFX 0x10259858
char* g_resExtHdi = ".hdi";

// GLOBAL: MW2 0x100a8718
// GLOBAL: MW2MATROX 0x100a5ae8
// GLOBAL: MW23DFX 0x10259864
char* g_resExtCpi = ".cpi";

// GLOBAL: MW2 0x100a871c
// GLOBAL: MW2MATROX 0x100a5aec
// GLOBAL: MW23DFX 0x10259870
char* g_resExtVpi = ".vpi";

// GLOBAL: MW2 0x100a8720
// GLOBAL: MW2MATROX 0x100a5af0
// GLOBAL: MW23DFX 0x1025987c
char* g_resExtPit = ".pit";

// GLOBAL: MW2 0x100a8724
// GLOBAL: MW2MATROX 0x100a5af4
// GLOBAL: MW23DFX 0x10259888
char* g_resExtBwd = ".bwd";

// GLOBAL: MW2 0x100a8728
// GLOBAL: MW2MATROX 0x100a5af8
// GLOBAL: MW23DFX 0x10259894
char* g_resExtAit = ".ait";

// GLOBAL: MW2 0x100a872c
// GLOBAL: MW2MATROX 0x100a5afc
// GLOBAL: MW23DFX 0x102598a0
char* g_resExtMek = ".mek";

// GLOBAL: MW2 0x100a8730
// GLOBAL: MW2MATROX 0x100a5b00
// GLOBAL: MW23DFX 0x102598ac
char* g_resExtLum = ".lum";

// GLOBAL: MW2 0x100a8734
// GLOBAL: MW2MATROX 0x100a5b04
// GLOBAL: MW23DFX 0x102598b8
char* g_resExtMus = ".mus";

// GLOBAL: MW2 0x100a8738
// GLOBAL: MW2MATROX 0x100a5b08
// GLOBAL: MW23DFX 0x102598c4
char* g_resExtGif = ".gif";

// GLOBAL: MW2 0x100a873c
// GLOBAL: MW2MATROX 0x100a5b0c
// GLOBAL: MW23DFX 0x102598d0
char* g_resExtTxt = ".txt";

// GLOBAL: MW2 0x100a8740
// GLOBAL: MW2MATROX 0x100a5b10
// GLOBAL: MW23DFX 0x102598d4
MechS32 g_mw2PrjHandle = -1;

// GLOBAL: MW2 0x100a8744
// GLOBAL: MW2MATROX 0x100a5b14
// GLOBAL: MW23DFX 0x102598d8
char* g_mw2PrjPath = NULL;

// FUNCTION: MW2 0x10050780
// FUNCTION: MW2MATROX 0x1002b360
// FUNCTION: MW23DFX 0x1003cf70
MechS32 FirstResource(void)
{
	MechS32 result;

	result = TRUE;
	SetPrjAllocator(Mw2PrjAlloc, Mw2PrjFree);
	InitializeResourceCache();
	if (!g_mw2PrjPath) {
		g_mw2PrjPath = (MechChar*) _mbsdup((unsigned char*) BuildGamePath("mw2.prj"));
	}

	g_mw2PrjHandle = OpenPrjFile(g_mw2PrjPath, 0);
	if (g_mw2PrjHandle != -1) {
		LoadPrjIndexes(g_mw2PrjHandle);
	}
	else {
		Error(3, "\nCan't find file \"%s\"", g_mw2PrjPath, 0);
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x1005082f
// FUNCTION: MW2MATROX 0x1002b40f
// FUNCTION: MW23DFX 0x1003cff0
void ShutdownMw2Prj(void)
{
	ClosePrjFile(g_mw2PrjHandle);
}

// The 3Dfx edition also writes MW2.MIP, the converted textures, on its first run.
// FUNCTION: MW2 0x10050848
// FUNCTION: MW2MATROX 0x1002b428
// FUNCTION: MW23DFX 0x1003d000
void CachePreloads(void)
{
	LoadWeaponSounds();
	PreloadCockpitSounds();
	LoadAIScripts();
#ifdef MW2_3DFX
	FUN_1002cfd0();
#endif
}

// FUNCTION: MW2 0x10050862
// FUNCTION: MW2MATROX 0x1002b442
// FUNCTION: MW23DFX 0x1003d020
MechS32 PreloadResource(MechS32 p_id, const char* p_type)
{
	MechS32 result;
	void* data;

	data = LoadCachedResource(g_mw2PrjHandle, p_id, p_type, 0);
	if (data) {
		result = TRUE;
		UnlockCachedResource(p_id, p_type);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x100508c0
// FUNCTION: MW2MATROX 0x1002b4a0
// FUNCTION: MW23DFX 0x1003d060
void* Mw2PrjAlloc(MechU32 p_size)
{
	return MemAlloc(p_size);
}

// FUNCTION: MW2 0x100508dc
// FUNCTION: MW2MATROX 0x1002b4bc
// FUNCTION: MW23DFX 0x1003d070
void Mw2PrjFree(void* p_block)
{
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
}
