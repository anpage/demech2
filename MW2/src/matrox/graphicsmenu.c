/* The Matrox edition's in-mission "Graphics" menu page: the textured sky and ground, kept in
   the registry (matrox/registry.c) and in g_renderSettings, and the texture filter, which the
   renderer applies. Its controls for the monitor brightness (1.1's main menu page of its own) and
   the texture filter, and a fourth, unused setting, aren't on the page. One object of its own,
   between geocache.c's and players.c's. */
#include "matrox/graphicsmenu.h"

#include "brightnessmenu.h"
#include "decomp.h"
#include "mainmenu.h"
#include "matrox/a3d.h"
#include "matrox/registry.h"
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
MechS32 GetFilterTextures(MechS32 p_arg);
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures);
MechS32 GetUnusedSetting(MechS32 p_arg);
void SetUnusedSetting(MechS32 p_arg, MechS32 p_unusedSetting);

// GLOBAL: MW2MATROX 0x100aa470
MechChar g_graphicsItem[] = "Graphics";

// GLOBAL: MW2MATROX 0x100aa480
MechChar g_graphicsTitle[] = "GRAPHICS";

// GLOBAL: MW2MATROX 0x100aa490
MechChar g_monitorBrightnessItem[] = "Monitor Brightness";

// GLOBAL: MW2MATROX 0x100aa4a8
MechChar g_texturedSkyItem[] = "Textured Sky";

// GLOBAL: MW2MATROX 0x100aa4b8
MechChar g_texturedGroundItem[] = "Textured Ground";

// GLOBAL: MW2MATROX 0x100aa4c8
MechChar g_filterTexturesItem[] = "Filter Textures";

// GLOBAL: MW2MATROX 0x100aa4d8
MechChar g_unusedItem[] = "Unused";

// GLOBAL: MW2MATROX 0x100aa4e0
MechS32 g_texturedSky = 1;

// GLOBAL: MW2MATROX 0x100aa4e4
MechS32 g_texturedGround = 1;

// GLOBAL: MW2MATROX 0x100aa4e8
MechS32 g_filterTextures = 1;

// GLOBAL: MW2MATROX 0x100aa4ec
MechS32 g_unusedSetting = 1;

// GLOBAL: MW2MATROX 0x100aa4f0
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
MenuControl g_texturedSkyControl = {2, 0, &g_offOnChoices, 0, NULL, GetTexturedSky, NULL, SetTexturedSky, NULL};

// GLOBAL: MW2MATROX 0x100aa540
MenuControl g_texturedGroundControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetTexturedGround, NULL, SetTexturedGround, NULL};

// GLOBAL: MW2MATROX 0x100aa568
MenuControl g_filterTexturesControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetFilterTextures, NULL, SetFilterTextures, NULL};

// GLOBAL: MW2MATROX 0x100aa590
MenuControl g_unusedControl = {2, 0, &g_offOnChoices, 0, NULL, GetUnusedSetting, NULL, SetUnusedSetting, NULL};

// The page counts four items but lists three.
// GLOBAL: MW2MATROX 0x100aa5b8
MenuPage g_graphicsPage = {
	0,
	g_graphicsTitle,
	0,
	4,
	0,
	InitGraphicsPage,
	{{1, g_texturedSkyItem, RunMenuChoice, &g_texturedSkyControl, NULL},
	 {1, g_texturedGroundItem, RunMenuChoice, &g_texturedGroundControl, NULL},
	 {2, g_acceptText, NULL, NULL, NULL}}
};

// FUNCTION: MW2MATROX 0x10054c80
MechS32 InitGraphicsPage(MenuDefinition* p_menu, MenuPage* p_page)
{
	return TRUE;
}

// FUNCTION: MW2MATROX 0x10054c95
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
void SetTexturedSky(MechS32 p_arg, MechS32 p_texturedSky)
{
	WriteRegistryDword("Sky", (DWORD*) &p_texturedSky);
	g_renderSettings.m_texturedSky = p_texturedSky;
	g_texturedSky = p_texturedSky;
}

// FUNCTION: MW2MATROX 0x10054d09
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
void SetTexturedGround(MechS32 p_arg, MechS32 p_texturedGround)
{
	WriteRegistryDword("Ground", (DWORD*) &p_texturedGround);
	g_renderSettings.m_texturedGround = p_texturedGround;
	g_texturedGround = p_texturedGround;
}

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

// FUNCTION: MW2MATROX 0x10054de6
MechS32 GetUnusedSetting(MechS32 p_arg)
{
	return g_unusedSetting;
}

// FUNCTION: MW2MATROX 0x10054dfb
void SetUnusedSetting(MechS32 p_arg, MechS32 p_unusedSetting)
{
	g_unusedSetting = p_unusedSetting;
}
