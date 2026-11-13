/* The accelerated editions' in-mission "Graphics" menu page: the textured sky and ground, kept in
   the registry (accel/registry.c) and in g_renderSettings, and the texture filter, which the
   renderer applies. Its controls for the monitor brightness (1.1's main menu page of its own) and
   the texture filter, and a fourth, unused setting, aren't on the page. One object of its own,
   between geocache.c's and players.c's in the Matrox edition. The 3Dfx edition's page lists all
   four items, its third a "Resolution 800x600" setting in the texture filter's place (the
   "Res800x600" registry value, off by default, which A3D_Init reads at the next start). */
#include "accel/graphicsmenu.h"

#include "accel/a3d.h"
#include "accel/registry.h"
#include "brightnessmenu.h"
#include "decomp.h"
#include "mainmenu.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "polydraw.h"
#include "rendersettings.h"
#include "types.h"

#include <windows.h>

MechS32 InitGraphicsPage(MenuDefinition* p_menu, MenuPage* p_page);
MechS32 GetTexturedSky(MechS32 p_arg);
void SetTexturedSky(MechS32 p_arg, MechS32 p_texturedSky);
MechS32 GetTexturedGround(MechS32 p_arg);
void SetTexturedGround(MechS32 p_arg, MechS32 p_texturedGround);
#ifdef MW2_3DFX
MechS32 GetRes800x600(MechS32 p_arg);
void SetRes800x600(MechS32 p_arg, MechS32 p_res800x600);
#else
MechS32 GetFilterTextures(MechS32 p_arg);
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures);
#endif
MechS32 GetUnusedSetting(MechS32 p_arg);
void SetUnusedSetting(MechS32 p_arg, MechS32 p_unusedSetting);

// GLOBAL: MW2MATROX 0x100aa470
// GLOBAL: MW23DFX 0x10265420
MechChar g_graphicsItem[] = "Graphics";

// GLOBAL: MW2MATROX 0x100aa480
// GLOBAL: MW23DFX 0x10265430
MechChar g_graphicsTitle[] = "GRAPHICS";

// GLOBAL: MW2MATROX 0x100aa490
// GLOBAL: MW23DFX 0x10265440
MechChar g_monitorBrightnessItem[] = "Monitor Brightness";

// GLOBAL: MW2MATROX 0x100aa4a8
// GLOBAL: MW23DFX 0x10265458
MechChar g_texturedSkyItem[] = "Textured Sky";

// GLOBAL: MW2MATROX 0x100aa4b8
// GLOBAL: MW23DFX 0x10265468
MechChar g_texturedGroundItem[] = "Textured Ground";

#ifdef MW2_3DFX
// GLOBAL: MW23DFX 0x10265478
MechChar g_res800x600Item[] = "Resolution 800x600";
#else
// GLOBAL: MW2MATROX 0x100aa4c8
MechChar g_filterTexturesItem[] = "Filter Textures";
#endif

// GLOBAL: MW2MATROX 0x100aa4d8
// GLOBAL: MW23DFX 0x10265490
MechChar g_unusedItem[] = "Unused";

// GLOBAL: MW2MATROX 0x100aa4e0
// GLOBAL: MW23DFX 0x10265498
MechS32 g_texturedSky = 1;

// GLOBAL: MW2MATROX 0x100aa4e4
// GLOBAL: MW23DFX 0x1026549c
MechS32 g_texturedGround = 1;

#ifdef MW2_3DFX
// GLOBAL: MW23DFX 0x102654a0
MechS32 g_res800x600 = 1;
#else
// GLOBAL: MW2MATROX 0x100aa4e8
MechS32 g_filterTextures = 1;
#endif

// GLOBAL: MW2MATROX 0x100aa4ec
// GLOBAL: MW23DFX 0x102654a4
MechS32 g_unusedSetting = 1;

// GLOBAL: MW2MATROX 0x100aa4f0
// GLOBAL: MW23DFX 0x102654a8
MenuControl g_brightnessControl = {
	2,
	0,
	g_sliderShapes,
	0,
	NULL,
	GetBrightnessFraction,
	PreviewBrightnessFraction,
	SetBrightnessFraction,
	RestoreBrightness
};

// GLOBAL: MW2MATROX 0x100aa518
// GLOBAL: MW23DFX 0x102654d0
MenuControl g_texturedSkyControl = {2, 0, &g_offOnChoices, 0, NULL, GetTexturedSky, NULL, SetTexturedSky, NULL};

// GLOBAL: MW2MATROX 0x100aa540
// GLOBAL: MW23DFX 0x102654f8
MenuControl g_texturedGroundControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetTexturedGround, NULL, SetTexturedGround, NULL};

#ifdef MW2_3DFX
// GLOBAL: MW23DFX 0x10265520
MenuControl g_res800x600Control = {2, 0, &g_noYesChoices, 0, NULL, GetRes800x600, NULL, SetRes800x600, NULL};
#else
// GLOBAL: MW2MATROX 0x100aa568
MenuControl g_filterTexturesControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetFilterTextures, NULL, SetFilterTextures, NULL};
#endif

// GLOBAL: MW2MATROX 0x100aa590
// GLOBAL: MW23DFX 0x10265548
MenuControl g_unusedControl = {2, 0, &g_offOnChoices, 0, NULL, GetUnusedSetting, NULL, SetUnusedSetting, NULL};

// The Matrox edition's counts four items but lists three.
// GLOBAL: MW2MATROX 0x100aa5b8
// GLOBAL: MW23DFX 0x10265570
MenuPage g_graphicsPage = {
	0,
	g_graphicsTitle,
	0,
	4,
	0,
	InitGraphicsPage,
	{{1, g_texturedSkyItem, RunMenuChoice, &g_texturedSkyControl, NULL},
	 {1, g_texturedGroundItem, RunMenuChoice, &g_texturedGroundControl, NULL},
#ifdef MW2_3DFX
	 {1, g_res800x600Item, RunMenuChoice, &g_res800x600Control, NULL},
#endif
	 {2, g_acceptText, NULL, NULL, NULL}}
};

// FUNCTION: MW2MATROX 0x10054c80
// FUNCTION: MW23DFX 0x10066400
MechS32 InitGraphicsPage(MenuDefinition* p_menu, MenuPage* p_page)
{
	return TRUE;
}

// FUNCTION: MW2MATROX 0x10054c95
// FUNCTION: MW23DFX 0x10066410
MechS32 GetTexturedSky(MechS32 p_arg)
{
	if (ReadRegistryDword("Sky", (DWORD*) &g_texturedSky)) {
		g_renderSettings.m_texturedSky = g_texturedSky;
	}
	else {
		g_texturedSky = g_renderSettings.m_texturedSky;
	}

	return g_texturedSky;
}

// FUNCTION: MW2MATROX 0x10054cdd
// FUNCTION: MW23DFX 0x10066450
void SetTexturedSky(MechS32 p_arg, MechS32 p_texturedSky)
{
	WriteRegistryDword("Sky", (DWORD*) &p_texturedSky);
	g_renderSettings.m_texturedSky = p_texturedSky;
	g_texturedSky = p_texturedSky;
}

// FUNCTION: MW2MATROX 0x10054d09
// FUNCTION: MW23DFX 0x10066480
MechS32 GetTexturedGround(MechS32 p_arg)
{
	if (ReadRegistryDword("Ground", (DWORD*) &g_texturedGround)) {
		g_renderSettings.m_texturedGround = g_texturedGround;
	}
	else {
		g_texturedGround = g_renderSettings.m_texturedGround;
	}

	return g_texturedGround;
}

// FUNCTION: MW2MATROX 0x10054d51
// FUNCTION: MW23DFX 0x100664c0
void SetTexturedGround(MechS32 p_arg, MechS32 p_texturedGround)
{
	WriteRegistryDword("Ground", (DWORD*) &p_texturedGround);
	g_renderSettings.m_texturedGround = p_texturedGround;
	g_texturedGround = p_texturedGround;
}

#ifdef MW2_3DFX
// FUNCTION: MW23DFX 0x100664f0
MechS32 GetRes800x600(MechS32 p_arg)
{
	if (!ReadRegistryDword("Res800x600", (DWORD*) &g_res800x600)) {
		g_res800x600 = 0;
	}

	return g_res800x600;
}

// FUNCTION: MW23DFX 0x10066520
void SetRes800x600(MechS32 p_arg, MechS32 p_res800x600)
{
	WriteRegistryDword("Res800x600", (DWORD*) &p_res800x600);
	g_res800x600 = p_res800x600;
	FUN_100667e0(p_res800x600);
}
#else
// FUNCTION: MW2MATROX 0x10054d7d
MechS32 GetFilterTextures(MechS32 p_arg)
{
	if (!ReadRegistryDword("Filter", (DWORD*) &g_filterTextures)) {
		g_filterTextures = 1;
	}

	return g_filterTextures;
}

// FUNCTION: MW2MATROX 0x10054db6
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures)
{
	WriteRegistryDword("Filter", (DWORD*) &p_filterTextures);
	g_filterTextures = p_filterTextures;
	FUN_1005f810(p_filterTextures);
}
#endif

// FUNCTION: MW2MATROX 0x10054de6
// FUNCTION: MW23DFX 0x10066550
MechS32 GetUnusedSetting(MechS32 p_arg)
{
	return g_unusedSetting;
}

// FUNCTION: MW2MATROX 0x10054dfb
// FUNCTION: MW23DFX 0x10066560
void SetUnusedSetting(MechS32 p_arg, MechS32 p_unusedSetting)
{
	g_unusedSetting = p_unusedSetting;
}
