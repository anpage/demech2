#ifndef MW2PRJ_H
#define MW2PRJ_H

#include "types.h"

// The functions and globals of mw2prj.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern char* g_resTagSnds;
	extern char* g_resTagCel;
	extern char* g_resTagXyc;
	extern char* g_resTagShp;
	extern char* g_resTagFont;
	extern char* g_resTagMenu;
	extern char* g_resTagDisp;
	extern char* g_resTagXmid;
	extern char* g_resTagPal;
	extern char* g_resTagTable;
	extern char* g_resTagPoly;
	extern char* g_resTagText;
	extern char* g_resTagAnim;
	extern char* g_resTagMgeo;
	extern char* g_resTagHud;
	extern char* g_resTagCpit;
	extern char* g_resTagVpt;
	extern char* g_resTagMpit;
	extern char* g_resTagBwd;
	extern char* g_resTagVer;
	extern char* g_resTagAit;
	extern char* g_resTagMek;
	extern char* g_resTagLuma;
	extern char* g_resTagMus;
	extern char* g_resTagGif;
	extern char* g_resTagNtxt;
	extern char* g_resExtSfl;
	extern char* g_resExtXel;
	extern char* g_resExtXyc;
	extern char* g_resExtShp;
	extern char* g_resExtFnt;
	extern char* g_resExtMenuDll;
	extern char* g_resExtDispDll;
	extern char* g_resExtXmi;
	extern char* g_resExtCol;
	extern char* g_resExtTbl;
	extern char* g_resExtWtb;
	extern char* g_resExtXxt;
	extern char* g_resExt3di;
	extern char* g_resExtMgi;
	extern char* g_resExtHdi;
	extern char* g_resExtCpi;
	extern char* g_resExtVpi;
	extern char* g_resExtPit;
	extern char* g_resExtBwd;
	extern char* g_resExtAit;
	extern char* g_resExtMek;
	extern char* g_resExtLum;
	extern char* g_resExtMus;
	extern char* g_resExtGif;
	extern char* g_resExtTxt;
	extern MechS32 g_mw2PrjHandle;

	MechS32 InitializeMw2Prj(void);
	void ShutdownMw2Prj(void);

#ifdef __cplusplus
}
#endif

#endif // MW2PRJ_H
