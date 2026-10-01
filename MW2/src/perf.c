#include "perf.h"

#include "audio.h"
#include "decomp.h"
#include "geocache.h"
#include "mainmenu.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "soundconfig.h"
#include "types.h"
#include "unk10036230.h"
#include "unk1004b130.h"
#include "unk1004b980.h"
#include "unk1006d680.h"

// The display performance settings of MW2SND.CFG (SoundConfig): a getter and a setter each,
// with the unused leading argument of the in-mission menu's callbacks.

MechS32 ApplyPerfSettings(MenuDefinition* p_menu, MenuPage* p_page);
MechS32 GetObjectTextmaps(MechS32 p_arg);
MechS32 GetTerrainTextmaps(MechS32 p_arg);
MechS32 GetDisplayDetail(MechS32 p_arg);
MechS32 GetObjectDensity(MechS32 p_arg);
void SetObjectTextmaps(MechS32 p_arg, MechS32 p_objectTextmaps);
void SetTerrainTextmaps(MechS32 p_arg, MechS32 p_terrainTextmaps);
void SetDisplayDetail(MechS32 p_arg, MechS32 p_displayDetail);
void SetObjectDensity(MechS32 p_arg, MechS32 p_objectDensity);

// GLOBAL: MW2 0x100b1438
MechChar g_unk0x100b1438[] = "Combat Variables";

// GLOBAL: MW2 0x100b1450
MechChar g_unk0x100b1450[] = "COMBAT VARIABLES";

// GLOBAL: MW2 0x100b1468
MechChar g_unk0x100b1468[] = "Object textmaps";

// GLOBAL: MW2 0x100b1478
MechChar g_unk0x100b1478[] = "Terrain textmaps";

// GLOBAL: MW2 0x100b1490
MechChar g_unk0x100b1490[] = "Display detail";

// GLOBAL: MW2 0x100b14a0
MechChar g_unk0x100b14a0[] = "Object density";

// GLOBAL: MW2 0x100b14b0
MechChar g_unk0x100b14b0[] = "Explosion chunks";

// GLOBAL: MW2 0x100b14c8
MechChar g_unk0x100b14c8[] = "Affine";

// GLOBAL: MW2 0x100b14d0
MechChar g_unk0x100b14d0[] = "Perspective";

// GLOBAL: MW2 0x100b14e0
MenuChoices g_unk0x100b14e0 = {NULL, 2, {g_unk0x100b14c8, g_unk0x100b14d0}};

// GLOBAL: MW2 0x100b1528
MenuControl g_unk0x100b1528 = {2, 0, &g_unk0x100a1ba0, 0, NULL, GetObjectTextmaps, NULL, SetObjectTextmaps, NULL};

// GLOBAL: MW2 0x100b1550
MenuControl g_unk0x100b1550 = {2, 0, &g_unk0x100a1ba0, 0, NULL, GetTerrainTextmaps, NULL, SetTerrainTextmaps, NULL};

// GLOBAL: MW2 0x100b1578
MenuControl g_unk0x100b1578 = {2, 0, &g_unk0x100a1c30, 0, NULL, GetDisplayDetail, NULL, SetDisplayDetail, NULL};

// GLOBAL: MW2 0x100b15a0
MenuControl g_unk0x100b15a0 = {2, 0, &g_unk0x100a1c30, 0, NULL, GetObjectDensity, NULL, SetObjectDensity, NULL};

// GLOBAL: MW2 0x100b15c8
MenuControl g_unk0x100b15c8 = {2, 0, &g_unk0x100a1ba0, 0, NULL, FUN_10021423, NULL, SetExplosionChunks, NULL};

// GLOBAL: MW2 0x100b15f0
MenuPage g_combatVariablesPage = {
	0,
	g_unk0x100b1450,
	0,
	6,
	0,
	ApplyPerfSettings,
	{{1, g_unk0x100b1468, RunMenuChoice, &g_unk0x100b1528, NULL},
	 {1, g_unk0x100b1478, RunMenuChoice, &g_unk0x100b1550, NULL},
	 {1, g_unk0x100b1490, RunMenuChoice, &g_unk0x100b1578, NULL},
	 {1, g_unk0x100b14a0, RunMenuChoice, &g_unk0x100b15a0, NULL},
	 {1, g_unk0x100b14b0, RunMenuChoice, &g_unk0x100b15c8, NULL},
	 {2, g_unk0x100a1ad8, NULL, NULL, NULL}}
};

// FUNCTION: MW2 0x10076af0
void FirstPerfSetting(void)
{
	ApplyPerfSettings(NULL, NULL);
}

// FUNCTION: MW2 0x10076b07
MechS32 ApplyPerfSettings(MenuDefinition* p_menu, MenuPage* p_page)
{
	MechS32 result = 1;

	if (g_mw2SndCfgData != NULL) {
		SetObjectTextmaps(result, g_mw2SndCfgData->m_objectTextmaps);
		SetTerrainTextmaps(result, g_mw2SndCfgData->m_terrainTextmaps);
		SetDisplayDetail(result, g_mw2SndCfgData->m_displayDetail);
		SetObjectDensity(result, g_mw2SndCfgData->m_objectDensity);
		SetExplosionChunks(result, g_mw2SndCfgData->m_explosionChunks);
	}

	return result;
}

// FUNCTION: MW2 0x10076b9a
MechS32 GetObjectTextmaps(MechS32 p_arg)
{
	return FUN_10036867(0x100) || FUN_10036867(0x200);
}

// FUNCTION: MW2 0x10076be0
void SetObjectTextmaps(MechS32 p_arg, MechS32 p_objectTextmaps)
{
	FUN_10036891(0x100, p_objectTextmaps);
	FUN_10036891(0x200, p_objectTextmaps);
	g_mw2SndCfgData->m_objectTextmaps = p_objectTextmaps;
}

// FUNCTION: MW2 0x10076c19
MechS32 GetTerrainTextmaps(MechS32 p_arg)
{
	return FUN_10036867(0x800) || g_unk0x100a7120;
}

// FUNCTION: MW2 0x10076c57
void SetTerrainTextmaps(MechS32 p_arg, MechS32 p_terrainTextmaps)
{
	FUN_10036891(0x800, p_terrainTextmaps);
	FUN_1004b539(p_terrainTextmaps);
	g_mw2SndCfgData->m_terrainTextmaps = p_terrainTextmaps;
}

// FUNCTION: MW2 0x10076c8b
MechS32 GetDisplayDetail(MechS32 p_arg)
{
	return FUN_1004c7a6(p_arg) || FUN_100368bf(p_arg);
}

// FUNCTION: MW2 0x10076ccf
void SetDisplayDetail(MechS32 p_arg, MechS32 p_displayDetail)
{
	FUN_1004c7cf(p_arg, p_displayDetail);
	FUN_100368e8(p_arg, p_displayDetail);
	g_mw2SndCfgData->m_displayDetail = p_displayDetail;
}

// FUNCTION: MW2 0x10076d06
MechS32 GetObjectDensity(MechS32 p_arg)
{
	return g_mw2SndCfgData->m_objectDensity;
}

// FUNCTION: MW2 0x10076d1e
void SetObjectDensity(MechS32 p_arg, MechS32 p_objectDensity)
{
	FUN_1006dc7d(p_objectDensity);
	g_mw2SndCfgData->m_objectDensity = p_objectDensity;
}
