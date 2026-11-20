/* The accelerated editions' in-mission "Graphics" menu page: the textured sky and ground, kept in
   the registry (accel/registry.c) and in g_renderSettings, and the texture filter, which the
   renderer applies. Its controls for the monitor brightness (1.1's main menu page of its own) and
   the texture filter, and a fourth, unused setting, aren't on the page. One object of its own,
   between geocache.c's and players.c's in the Matrox edition. The 3Dfx edition's page lists all
   four items, its third a "Resolution 800x600" setting in the texture filter's place (the
   "Res800x600" registry value, off by default, which A3D_Init reads at the next start); the ATI
   edition's lists the texture filter there. */
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
#ifdef MW2_POWERVR
MechS32 GetFog(MechS32 p_arg);
void SetFog(MechS32 p_arg, MechS32 p_fog);
MechS32 GetSmoothShading(MechS32 p_arg);
void SetSmoothShading(MechS32 p_arg, MechS32 p_smoothShading);
MechS32 GetFilterTextures(MechS32 p_arg);
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures);
#else
MechS32 GetTexturedSky(MechS32 p_arg);
void SetTexturedSky(MechS32 p_arg, MechS32 p_texturedSky);
MechS32 GetTexturedGround(MechS32 p_arg);
void SetTexturedGround(MechS32 p_arg, MechS32 p_texturedGround);
#endif
#ifdef MW2_RES_800X600
MechS32 GetRes800x600(MechS32 p_arg);
void SetRes800x600(MechS32 p_arg, MechS32 p_res800x600);
#endif
#ifdef MW2_TEXTURE_FILTER
MechS32 GetFilterTextures(MechS32 p_arg);
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures);
#endif
MechS32 GetUnusedSetting(MechS32 p_arg);
void SetUnusedSetting(MechS32 p_arg, MechS32 p_unusedSetting);

// GLOBAL: MW2MATROX 0x100aa470
// GLOBAL: MW23DFX 0x10265420
// GLOBAL: MW2ATI 0x10199718
// GLOBAL: MW2S3 0x10065778
// GLOBAL: MW2POWERVR 0x101c4ac0
MechChar g_graphicsItem[] = "Graphics";

// GLOBAL: MW2MATROX 0x100aa480
// GLOBAL: MW23DFX 0x10265430
// GLOBAL: MW2ATI 0x10199728
// GLOBAL: MW2S3 0x10065788
// GLOBAL: MW2POWERVR 0x101c4ad0
MechChar g_graphicsTitle[] = "GRAPHICS";

// GLOBAL: MW2MATROX 0x100aa490
// GLOBAL: MW23DFX 0x10265440
// GLOBAL: MW2POWERVR 0x101c4ae0
MechChar g_monitorBrightnessItem[] = "Monitor Brightness";

#ifdef MW2_POWERVR
// GLOBAL: MW2POWERVR 0x101c4af4
MechChar g_fogItem[] = "Fog";

// GLOBAL: MW2POWERVR 0x101c4af8
MechChar g_smoothShadingItem[] = "Smooth Shading";
#else
// GLOBAL: MW2MATROX 0x100aa4a8
// GLOBAL: MW23DFX 0x10265458
// GLOBAL: MW2ATI 0x10199750
// GLOBAL: MW2S3 0x100657b0
MechChar g_texturedSkyItem[] = "Textured Sky";

// GLOBAL: MW2MATROX 0x100aa4b8
// GLOBAL: MW23DFX 0x10265468
// GLOBAL: MW2ATI 0x10199760
// GLOBAL: MW2S3 0x100657c0
MechChar g_texturedGroundItem[] = "Textured Ground";
#endif

#ifdef MW2_RES_800X600
// GLOBAL: MW23DFX 0x10265478
MechChar g_res800x600Item[] = "Resolution 800x600";
#endif
#ifdef MW2_TEXTURE_FILTER
// GLOBAL: MW2MATROX 0x100aa4c8
// GLOBAL: MW2ATI 0x10199770
// GLOBAL: MW2S3 0x100657d0
MechChar g_filterTexturesItem[] = "Filter Textures";
#endif

// GLOBAL: MW2MATROX 0x100aa4d8
// GLOBAL: MW23DFX 0x10265490
// GLOBAL: MW2POWERVR 0x101c4b10
MechChar g_unusedItem[] = "Unused";

#ifdef MW2_POWERVR
// The fog and smooth shading settings, as the graphics page last read or set them (the renderer
// keeps its own, in g_a3dMainView).
// GLOBAL: MW2POWERVR 0x101c4b18
MechS32 g_fog = 1;

// GLOBAL: MW2POWERVR 0x101c4b1c
MechS32 g_smoothShading = 1;

// The "Filter" registry setting: the PowerVR edition keeps it, though nothing reads it.
// GLOBAL: MW2POWERVR 0x101c4b20
MechS32 g_filterTextures = 1;
#else
// GLOBAL: MW2MATROX 0x100aa4e0
// GLOBAL: MW23DFX 0x10265498
// GLOBAL: MW2ATI 0x10199788
// GLOBAL: MW2S3 0x100657e8
MechS32 g_texturedSky = 1;

// GLOBAL: MW2MATROX 0x100aa4e4
// GLOBAL: MW23DFX 0x1026549c
// GLOBAL: MW2ATI 0x1019978c
// GLOBAL: MW2S3 0x100657ec
MechS32 g_texturedGround = 1;
#endif

#ifdef MW2_RES_800X600
// GLOBAL: MW23DFX 0x102654a0
MechS32 g_res800x600 = 1;
#endif
#ifdef MW2_TEXTURE_FILTER
// GLOBAL: MW2MATROX 0x100aa4e8
// GLOBAL: MW2ATI 0x10199790
// GLOBAL: MW2S3 0x100657f0
MechS32 g_filterTextures = 1;
#endif

// GLOBAL: MW2MATROX 0x100aa4ec
// GLOBAL: MW23DFX 0x102654a4
// GLOBAL: MW2ATI 0x10199794
// GLOBAL: MW2S3 0x100657f4
// GLOBAL: MW2POWERVR 0x101c4b24
MechS32 g_unusedSetting = 1;

// GLOBAL: MW2MATROX 0x100aa4f0
// GLOBAL: MW23DFX 0x102654a8
// GLOBAL: MW2POWERVR 0x101c4b28
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

#ifdef MW2_POWERVR
// GLOBAL: MW2POWERVR 0x101c4b50
MenuControl g_fogControl = {2, 0, &g_offOnChoices, 0, NULL, GetFog, NULL, SetFog, NULL};

// GLOBAL: MW2POWERVR 0x101c4b78
MenuControl g_smoothShadingControl = {2, 0, &g_offOnChoices, 0, NULL, GetSmoothShading, NULL, SetSmoothShading, NULL};

// GLOBAL: MW2POWERVR 0x101c4ba0
MenuControl g_filterTexturesControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetFilterTextures, NULL, SetFilterTextures, NULL};
#else
// GLOBAL: MW2MATROX 0x100aa518
// GLOBAL: MW23DFX 0x102654d0
// GLOBAL: MW2ATI 0x101997c0
// GLOBAL: MW2S3 0x10065820
MenuControl g_texturedSkyControl = {2, 0, &g_offOnChoices, 0, NULL, GetTexturedSky, NULL, SetTexturedSky, NULL};

// GLOBAL: MW2MATROX 0x100aa540
// GLOBAL: MW23DFX 0x102654f8
// GLOBAL: MW2ATI 0x101997e8
// GLOBAL: MW2S3 0x10065848
MenuControl g_texturedGroundControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetTexturedGround, NULL, SetTexturedGround, NULL};
#endif

#ifdef MW2_RES_800X600
// GLOBAL: MW23DFX 0x10265520
MenuControl g_res800x600Control = {2, 0, &g_noYesChoices, 0, NULL, GetRes800x600, NULL, SetRes800x600, NULL};
#endif
#ifdef MW2_TEXTURE_FILTER
// GLOBAL: MW2MATROX 0x100aa568
// GLOBAL: MW2ATI 0x10199810
// GLOBAL: MW2S3 0x10065870
MenuControl g_filterTexturesControl =
	{2, 0, &g_offOnChoices, 0, NULL, GetFilterTextures, NULL, SetFilterTextures, NULL};
#endif

// GLOBAL: MW2MATROX 0x100aa590
// GLOBAL: MW23DFX 0x10265548
// GLOBAL: MW2POWERVR 0x101c4bc8
MenuControl g_unusedControl = {2, 0, &g_offOnChoices, 0, NULL, GetUnusedSetting, NULL, SetUnusedSetting, NULL};

#ifdef MW2_POWERVR
// GLOBAL: MW2POWERVR 0x101c4bf0
MenuPage g_graphicsPage = {
	0,
	g_graphicsTitle,
	0,
	3,
	0,
	InitGraphicsPage,
	{{1, g_fogItem, RunMenuChoice, &g_fogControl, NULL},
	 {1, g_smoothShadingItem, RunMenuChoice, &g_smoothShadingControl, NULL},
	 {2, g_acceptText, NULL, NULL, NULL}}
};
#else
// The Matrox edition's counts four items but lists three.
// GLOBAL: MW2MATROX 0x100aa5b8
// GLOBAL: MW23DFX 0x10265570
// GLOBAL: MW2ATI 0x10199860
// GLOBAL: MW2S3 0x100658c0
MenuPage g_graphicsPage = {
	0,
	g_graphicsTitle,
	0,
	4,
	0,
	InitGraphicsPage,
	{{1, g_texturedSkyItem, RunMenuChoice, &g_texturedSkyControl, NULL},
	 {1, g_texturedGroundItem, RunMenuChoice, &g_texturedGroundControl, NULL},
#ifdef MW2_RES_800X600
	 {1, g_res800x600Item, RunMenuChoice, &g_res800x600Control, NULL},
#endif
#ifdef MW2_TEXTURE_FILTER_ITEM
	 {1, g_filterTexturesItem, RunMenuChoice, &g_filterTexturesControl, NULL},
#endif
	 {2, g_acceptText, NULL, NULL, NULL}}
};
#endif

// FUNCTION: MW2MATROX 0x10054c80
// FUNCTION: MW23DFX 0x10066400
// FUNCTION: MW2ATI 0x10063980
// FUNCTION: MW2S3 0x10021e10
// FUNCTION: MW2POWERVR 0x1005e6f0
MechS32 InitGraphicsPage(MenuDefinition* p_menu, MenuPage* p_page)
{
	return TRUE;
}

#ifdef MW2_POWERVR
// The PowerVR edition's page sets the renderer's fog and smooth shading (A3D_SetQuality) in place of
// the textured sky and ground, and keeps the "Filter" setting without passing it on.
// FUNCTION: MW2POWERVR 0x1005e700
MechS32 GetFog(MechS32 p_arg)
{
	g_fog = g_a3dMainView.m_fog;
	return g_fog;
}

// FUNCTION: MW2POWERVR 0x1005e710
void SetFog(MechS32 p_arg, MechS32 p_fog)
{
	WriteRegistryDword("Fog", (DWORD*) &p_fog);
	A3D_SetQuality(&g_a3dMainView, c_a3dQualityFog, p_fog);
	g_fog = p_fog;
}

// FUNCTION: MW2POWERVR 0x1005e750
MechS32 GetSmoothShading(MechS32 p_arg)
{
	g_smoothShading = g_a3dMainView.m_smoothShading;
	return g_smoothShading;
}

// FUNCTION: MW2POWERVR 0x1005e760
void SetSmoothShading(MechS32 p_arg, MechS32 p_smoothShading)
{
	WriteRegistryDword("Smooth Shading", (DWORD*) &p_smoothShading);
	A3D_SetQuality(&g_a3dMainView, c_a3dQualitySmoothShading, p_smoothShading);
	g_smoothShading = p_smoothShading;
}

// FUNCTION: MW2POWERVR 0x1005e7a0
MechS32 GetFilterTextures(MechS32 p_arg)
{
	if (!ReadRegistryDword("Filter", (DWORD*) &g_filterTextures)) {
		g_filterTextures = 1;
	}

	return g_filterTextures;
}

// FUNCTION: MW2POWERVR 0x1005e7d0
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures)
{
	WriteRegistryDword("Filter", (DWORD*) &p_filterTextures);
	g_filterTextures = p_filterTextures;
}
#else
// FUNCTION: MW2MATROX 0x10054c95
// FUNCTION: MW23DFX 0x10066410
// FUNCTION: MW2ATI 0x10063990
// FUNCTION: MW2S3 0x10021e20
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
// FUNCTION: MW2ATI 0x100639d0
// FUNCTION: MW2S3 0x10021e60
void SetTexturedSky(MechS32 p_arg, MechS32 p_texturedSky)
{
	WriteRegistryDword("Sky", (DWORD*) &p_texturedSky);
	g_renderSettings.m_texturedSky = p_texturedSky;
	g_texturedSky = p_texturedSky;
}

// FUNCTION: MW2MATROX 0x10054d09
// FUNCTION: MW23DFX 0x10066480
// FUNCTION: MW2ATI 0x10063a00
// FUNCTION: MW2S3 0x10021e90
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
// FUNCTION: MW2ATI 0x10063a40
// FUNCTION: MW2S3 0x10021ed0
void SetTexturedGround(MechS32 p_arg, MechS32 p_texturedGround)
{
	WriteRegistryDword("Ground", (DWORD*) &p_texturedGround);
	g_renderSettings.m_texturedGround = p_texturedGround;
	g_texturedGround = p_texturedGround;
}

#endif
#ifdef MW2_RES_800X600
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
#endif
#ifdef MW2_TEXTURE_FILTER
// FUNCTION: MW2MATROX 0x10054d7d
// FUNCTION: MW2ATI 0x10063a70
// FUNCTION: MW2S3 0x10021f00
MechS32 GetFilterTextures(MechS32 p_arg)
{
	if (!ReadRegistryDword("Filter", (DWORD*) &g_filterTextures)) {
		g_filterTextures = 1;
	}

	return g_filterTextures;
}

// FUNCTION: MW2MATROX 0x10054db6
// FUNCTION: MW2ATI 0x10063aa0
// FUNCTION: MW2S3 0x10021f30
void SetFilterTextures(MechS32 p_arg, MechS32 p_filterTextures)
{
	WriteRegistryDword("Filter", (DWORD*) &p_filterTextures);
	g_filterTextures = p_filterTextures;
	A3D_SET_TEXTURE_FILTER(p_filterTextures);
}
#endif

// FUNCTION: MW2MATROX 0x10054de6
// FUNCTION: MW23DFX 0x10066550
// FUNCTION: MW2ATI 0x10063ad0
// FUNCTION: MW2S3 0x10021f60
// FUNCTION: MW2POWERVR 0x1005e800
MechS32 GetUnusedSetting(MechS32 p_arg)
{
	return g_unusedSetting;
}

// FUNCTION: MW2MATROX 0x10054dfb
// FUNCTION: MW23DFX 0x10066560
// FUNCTION: MW2ATI 0x10063ae0
// FUNCTION: MW2S3 0x10021f70
// FUNCTION: MW2POWERVR 0x1005e810
void SetUnusedSetting(MechS32 p_arg, MechS32 p_unusedSetting)
{
	g_unusedSetting = p_unusedSetting;
}
