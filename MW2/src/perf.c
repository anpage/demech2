#include "perf.h"

#include "audio.h"
#include "decomp.h"
#include "faceshade.h"
#include "geocache.h"
#include "gridobject.h"
#include "mainmenu.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "shapelists.h"
#include "soundconfig.h"
#include "types.h"
#include "view.h"

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
// GLOBAL: MW2GBL 0x100a4210
// GLOBAL: MW2MATROX 0x100b1140
// GLOBAL: MW23DFX 0x10264eb0
// GLOBAL: MW2ATI 0x101991a8
// GLOBAL: MW2S3 0x10069f70
// GLOBAL: MW2POWERVR 0x101c4520
MechChar g_combatVariablesItem[] = "Combat Variables";

// GLOBAL: MW2 0x100b1450
// GLOBAL: MW2GBL 0x100a4228
// GLOBAL: MW2MATROX 0x100b1158
// GLOBAL: MW23DFX 0x10264ec8
// GLOBAL: MW2ATI 0x101991c0
// GLOBAL: MW2S3 0x10069f88
// GLOBAL: MW2POWERVR 0x101c4538
MechChar g_combatVariablesTitle[] = "COMBAT VARIABLES";

// GLOBAL: MW2 0x100b1468
// GLOBAL: MW2GBL 0x100a4240
// GLOBAL: MW2MATROX 0x100b1170
// GLOBAL: MW23DFX 0x10264ee0
// GLOBAL: MW2ATI 0x101991d8
// GLOBAL: MW2S3 0x10069fa0
// GLOBAL: MW2POWERVR 0x101c4550
MechChar g_objectTextmapsItem[] = "Object textmaps";

// GLOBAL: MW2 0x100b1478
// GLOBAL: MW2GBL 0x100a4250
// GLOBAL: MW2MATROX 0x100b1180
// GLOBAL: MW23DFX 0x10264ef0
// GLOBAL: MW2ATI 0x101991e8
// GLOBAL: MW2S3 0x10069fb0
// GLOBAL: MW2POWERVR 0x101c4560
MechChar g_terrainTextmapsItem[] = "Terrain textmaps";

// GLOBAL: MW2 0x100b1490
// GLOBAL: MW2GBL 0x100a4268
// GLOBAL: MW2MATROX 0x100b1198
// GLOBAL: MW23DFX 0x10264f08
// GLOBAL: MW2ATI 0x10199200
// GLOBAL: MW2S3 0x10069fc8
// GLOBAL: MW2POWERVR 0x101c4578
MechChar g_displayDetailItem[] = "Display detail";

// GLOBAL: MW2 0x100b14a0
// GLOBAL: MW2GBL 0x100a4278
// GLOBAL: MW2MATROX 0x100b11a8
// GLOBAL: MW23DFX 0x10264f18
// GLOBAL: MW2ATI 0x10199210
// GLOBAL: MW2S3 0x10069fd8
// GLOBAL: MW2POWERVR 0x101c4588
MechChar g_objectDensityItem[] = "Object density";

// GLOBAL: MW2 0x100b14b0
// GLOBAL: MW2GBL 0x100a4288
// GLOBAL: MW2MATROX 0x100b11b8
// GLOBAL: MW23DFX 0x10264f28
// GLOBAL: MW2ATI 0x10199220
// GLOBAL: MW2S3 0x10069fe8
// GLOBAL: MW2POWERVR 0x101c4598
MechChar g_explosionChunksItem[] = "Explosion chunks";

// GLOBAL: MW2 0x100b14c8
// GLOBAL: MW2MATROX 0x100b11d0
// GLOBAL: MW2POWERVR 0x101c45b0
MechChar g_affineText[] = "Affine";

// GLOBAL: MW2 0x100b14d0
// GLOBAL: MW2MATROX 0x100b11d8
// GLOBAL: MW2POWERVR 0x101c45b8
MechChar g_perspectiveText[] = "Perspective";

// GLOBAL: MW2 0x100b14e0
// GLOBAL: MW2MATROX 0x100b11e8
// GLOBAL: MW2POWERVR 0x101c45c8
MenuChoices g_affinePerspectiveChoices = {NULL, 2, {g_affineText, g_perspectiveText}};

// GLOBAL: MW2 0x100b1528
// GLOBAL: MW2GBL 0x100a4300
// GLOBAL: MW2MATROX 0x100b1230
// GLOBAL: MW23DFX 0x10264fa0
// GLOBAL: MW2ATI 0x10199298
// GLOBAL: MW2S3 0x1006a060
// GLOBAL: MW2POWERVR 0x101c4610
MenuControl g_objectTextmapsControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetObjectTextmaps, NULL, SetObjectTextmaps, NULL};

// GLOBAL: MW2 0x100b1550
// GLOBAL: MW2GBL 0x100a4328
// GLOBAL: MW2MATROX 0x100b1258
// GLOBAL: MW23DFX 0x10264fc8
// GLOBAL: MW2ATI 0x101992c0
// GLOBAL: MW2S3 0x1006a088
// GLOBAL: MW2POWERVR 0x101c4638
MenuControl g_terrainTextmapsControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetTerrainTextmaps, NULL, SetTerrainTextmaps, NULL};

// GLOBAL: MW2 0x100b1578
// GLOBAL: MW2GBL 0x100a4350
// GLOBAL: MW2MATROX 0x100b1280
// GLOBAL: MW23DFX 0x10264ff0
// GLOBAL: MW2ATI 0x101992e8
// GLOBAL: MW2S3 0x1006a0b0
// GLOBAL: MW2POWERVR 0x101c4660
MenuControl g_displayDetailControl = {2, 0, &g_lowHighChoices, 0, NULL, GetDisplayDetail, NULL, SetDisplayDetail, NULL};

// GLOBAL: MW2 0x100b15a0
// GLOBAL: MW2GBL 0x100a4378
// GLOBAL: MW2MATROX 0x100b12a8
// GLOBAL: MW23DFX 0x10265018
// GLOBAL: MW2ATI 0x10199310
// GLOBAL: MW2S3 0x1006a0d8
// GLOBAL: MW2POWERVR 0x101c4688
MenuControl g_objectDensityControl = {2, 0, &g_lowHighChoices, 0, NULL, GetObjectDensity, NULL, SetObjectDensity, NULL};

// GLOBAL: MW2 0x100b15c8
// GLOBAL: MW2GBL 0x100a43a0
// GLOBAL: MW2MATROX 0x100b12d0
// GLOBAL: MW23DFX 0x10265040
// GLOBAL: MW2ATI 0x10199338
// GLOBAL: MW2S3 0x1006a100
// GLOBAL: MW2POWERVR 0x101c46b0
MenuControl g_explosionChunksControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetExplosionChunks, NULL, SetExplosionChunks, NULL};

// The PowerVR edition's page lists neither the object textmaps nor the display detail (its count
// is still 6).
#ifdef MW2_POWERVR
// GLOBAL: MW2POWERVR 0x101c46d8
MenuPage g_combatVariablesPage = {
	0,
	g_combatVariablesTitle,
	0,
	6,
	0,
	ApplyPerfSettings,
	{{1, g_terrainTextmapsItem, RunMenuChoice, &g_terrainTextmapsControl, NULL},
	 {1, g_objectDensityItem, RunMenuChoice, &g_objectDensityControl, NULL},
	 {1, g_explosionChunksItem, RunMenuChoice, &g_explosionChunksControl, NULL},
	 {2, g_acceptText, NULL, NULL, NULL}}
};
#else
// GLOBAL: MW2 0x100b15f0
// GLOBAL: MW2GBL 0x100a43c8
// GLOBAL: MW2MATROX 0x100b12f8
// GLOBAL: MW23DFX 0x10265068
// GLOBAL: MW2ATI 0x10199360
// GLOBAL: MW2S3 0x1006a128
MenuPage g_combatVariablesPage = {
	0,
	g_combatVariablesTitle,
	0,
	6,
	0,
	ApplyPerfSettings,
	{{1, g_objectTextmapsItem, RunMenuChoice, &g_objectTextmapsControl, NULL},
	 {1, g_terrainTextmapsItem, RunMenuChoice, &g_terrainTextmapsControl, NULL},
	 {1, g_displayDetailItem, RunMenuChoice, &g_displayDetailControl, NULL},
	 {1, g_objectDensityItem, RunMenuChoice, &g_objectDensityControl, NULL},
	 {1, g_explosionChunksItem, RunMenuChoice, &g_explosionChunksControl, NULL},
	 {2, g_acceptText, NULL, NULL, NULL}}
};
#endif

// FUNCTION: MW2 0x10076af0
// FUNCTION: MW2GBL 0x1002ca60
// FUNCTION: MW2MATROX 0x10078cc0
// FUNCTION: MW23DFX 0x10065240
// FUNCTION: MW2ATI 0x100628b0
// FUNCTION: MW2S3 0x1002d850
// FUNCTION: MW2POWERVR 0x1005d620
void FirstPerfSetting(void)
{
	ApplyPerfSettings(NULL, NULL);
}

// FUNCTION: MW2 0x10076b07
// FUNCTION: MW2GBL 0x1002ca77
// FUNCTION: MW2MATROX 0x10078cd7
// FUNCTION: MW23DFX 0x10065250
// FUNCTION: MW2ATI 0x100628c0
// FUNCTION: MW2S3 0x1002d860
// FUNCTION: MW2POWERVR 0x1005d630
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
// FUNCTION: MW2GBL 0x1002cb0a
// FUNCTION: MW2MATROX 0x10078d6a
// FUNCTION: MW23DFX 0x100652c0
// FUNCTION: MW2ATI 0x10062930
// FUNCTION: MW2S3 0x1002d8d0
// FUNCTION: MW2POWERVR 0x1005d6a0
MechS32 GetObjectTextmaps(MechS32 p_arg)
{
	return AreTextureMapsOn(0x100) || AreTextureMapsOn(0x200);
}

// FUNCTION: MW2 0x10076be0
// FUNCTION: MW2GBL 0x1002cb50
// FUNCTION: MW2MATROX 0x10078db0
// FUNCTION: MW23DFX 0x100652f0
// FUNCTION: MW2ATI 0x10062960
// FUNCTION: MW2S3 0x1002d900
// FUNCTION: MW2POWERVR 0x1005d6d0
void SetObjectTextmaps(MechS32 p_arg, MechS32 p_objectTextmaps)
{
	EnableTextureMaps(0x100, p_objectTextmaps);
	EnableTextureMaps(0x200, p_objectTextmaps);
	g_mw2SndCfgData->m_objectTextmaps = p_objectTextmaps;
}

// FUNCTION: MW2 0x10076c19
// FUNCTION: MW2GBL 0x1002cb89
// FUNCTION: MW2MATROX 0x10078de9
// FUNCTION: MW23DFX 0x10065320
// FUNCTION: MW2ATI 0x10062990
// FUNCTION: MW2S3 0x1002d930
// FUNCTION: MW2POWERVR 0x1005d700
MechS32 GetTerrainTextmaps(MechS32 p_arg)
{
	return AreTextureMapsOn(0x800) || g_gridObjectShown;
}

// FUNCTION: MW2 0x10076c57
// FUNCTION: MW2GBL 0x1002cbc7
// FUNCTION: MW2MATROX 0x10078e27
// FUNCTION: MW23DFX 0x10065340
// FUNCTION: MW2ATI 0x100629b0
// FUNCTION: MW2S3 0x1002d950
// FUNCTION: MW2POWERVR 0x1005d720
void SetTerrainTextmaps(MechS32 p_arg, MechS32 p_terrainTextmaps)
{
	EnableTextureMaps(0x800, p_terrainTextmaps);
	ShowGridObject(p_terrainTextmaps);
	g_mw2SndCfgData->m_terrainTextmaps = p_terrainTextmaps;
}

// FUNCTION: MW2 0x10076c8b
// FUNCTION: MW2GBL 0x1002cbfb
// FUNCTION: MW2MATROX 0x10078e5b
// FUNCTION: MW23DFX 0x10065360
// FUNCTION: MW2ATI 0x100629d0
// FUNCTION: MW2S3 0x1002d970
// FUNCTION: MW2POWERVR 0x1005d740
MechS32 GetDisplayDetail(MechS32 p_arg)
{
	return IsLodQualityHigh(p_arg) || ArePerspectiveTexturesOn(p_arg);
}

// FUNCTION: MW2 0x10076ccf
// FUNCTION: MW2GBL 0x1002cc3f
// FUNCTION: MW2MATROX 0x10078e9f
// FUNCTION: MW23DFX 0x10065390
// FUNCTION: MW2ATI 0x10062a00
// FUNCTION: MW2S3 0x1002d9a0
// FUNCTION: MW2POWERVR 0x1005d770
void SetDisplayDetail(MechS32 p_arg, MechS32 p_displayDetail)
{
	SetLodQualityHigh(p_arg, p_displayDetail);
	EnablePerspectiveTextures(p_arg, p_displayDetail);
	g_mw2SndCfgData->m_displayDetail = p_displayDetail;
}

// FUNCTION: MW2 0x10076d06
// FUNCTION: MW2GBL 0x1002cc76
// FUNCTION: MW2MATROX 0x10078ed6
// FUNCTION: MW23DFX 0x100653b0
// FUNCTION: MW2ATI 0x10062a20
// FUNCTION: MW2S3 0x1002d9c0
// FUNCTION: MW2POWERVR 0x1005d790
MechS32 GetObjectDensity(MechS32 p_arg)
{
	return g_mw2SndCfgData->m_objectDensity;
}

// FUNCTION: MW2 0x10076d1e
// FUNCTION: MW2GBL 0x1002cc8e
// FUNCTION: MW2MATROX 0x10078eee
// FUNCTION: MW23DFX 0x100653c0
// FUNCTION: MW2ATI 0x10062a30
// FUNCTION: MW2S3 0x1002d9d0
// FUNCTION: MW2POWERVR 0x1005d7a0
void SetObjectDensity(MechS32 p_arg, MechS32 p_objectDensity)
{
	ShowDensityShapes(p_objectDensity);
	g_mw2SndCfgData->m_objectDensity = p_objectDensity;
}
