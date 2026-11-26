#include "mechviewpanel.h"

#include "cockpitpanel.h"
#include "config.h"
#include "damagepanel.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "gamekeys.h"
#include "hud.h"
#include "object.h"
#include "palette.h"
#include "players.h"
#include "polydraw.h"
#include "random.h"
#include "recttransition.h"
#include "screenscale.h"
#include "shots.h"
#include "simmain.h"
#include "targeting.h"
#include "types.h"

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif
#ifdef MW2_POWERVR
#include "accel/a3d.h"
#include "powervr/a3dviews.h"
#endif
#if defined(MW2_STRUCT_COPY_MEMCPY) || defined(MW2_MFD_PANELS)
#include <string.h>
#endif
#ifdef MW2_MFD_PANELS
#include "cockpit.h"
#include "statuspanels.h"
#include "targetpanel.h"

#include <stdio.h>
#endif

// The handlers of the cockpit panel InitCockpitPanels sets up second (g_cockpitPanels[c_panelMechView]): it
// cycles through five views of the local mech (CycleMechViewMode), drawn into the panel's render
// target, optionally through the panel's rectangle transition.

#ifdef MW2_MFD_PANELS
// Whether g_savedMfdModes holds MERCS.HUD's modes.
// GLOBAL: MW2PREMERCS 0x100b0d94
MechS32 g_mfdModesLoaded = 0;

#endif
// GLOBAL: MW2 0x100a88e8
// GLOBAL: MW2GBL 0x1009d96c
// GLOBAL: MW2PREMERCS 0x100b0d98
// GLOBAL: MW2MATROX 0x100bbe94
// GLOBAL: MW23DFX 0x1024b444
// GLOBAL: MW2ATI 0x1017f7c4
// GLOBAL: MW2S3 0x1006a4b0
// GLOBAL: MW2POWERVR 0x101aa594
MechS32 g_mechViewStatic = 0;

#ifdef MW2_KEY_COLOR
// The Matrox edition clears the rendered view's pane with its transparent pixel (0xfffe) for the
// 3D card's picture to show through (the 3Dfx edition doesn't).
#define MECH_VIEW_WIPE 0xfffe
#endif
#ifdef MW2_HUD_ART
// The accelerated editions' frames' shapes come 6 entries further on.
#define MECH_VIEW_SHAPE(id) ((id) + 6)
#else
#define MECH_VIEW_SHAPE(id) (id)
#endif

#ifdef MW2_MFD_PANELS
// The early Mercenaries build's mech view panels are seven MFDs (panels 2, 17, 22, 26, 31, 32
// and 33), each showing one of DrawMechViewPanel's modes (CockpitPanel::m_mfdMode) in its own
// pane (m_weapon). SAVE_HUD saves their modes, the weapon display's, the compass's and the cockpit
// layout to MERCS.HUD, which ApplyMfdModes restores; MFDS_RESET resets them.
#define MECH_VIEW_MODE(p_panel) ((p_panel)->m_mfdMode)
#define MECH_VIEW_SLOT(p_panel) ((p_panel)->m_weapon)

// The modes RememberMfdModes keeps.
// GLOBAL: MW2PREMERCS 0x101b7660
MechS32 g_mfdModes[7];

// MERCS.HUD's modes and settings.
// GLOBAL: MW2PREMERCS 0x101b7690
MechS32 g_savedMfdModes[7];

// GLOBAL: MW2PREMERCS 0x101b7658
undefined4 g_savedUnk0x100b20f8;

// GLOBAL: MW2PREMERCS 0x101b767c
MechS32 g_savedShowCompass;

// GLOBAL: MW2PREMERCS 0x101b7680
MechS32 g_savedCockpitLayout;

// Saves the MFDs' modes, the weapon display's and the compass's settings and the cockpit layout
// to MERCS.HUD (SAVE_HUD).
// FUNCTION: MW2PREMERCS 0x10035bd0
void SaveMfdModes(void)
{
	FILE* file;
	MechS32 i;

	g_mfdModesLoaded = 1;
	RememberMfdModes();
	for (i = 0; i < 7; i++) {
		g_savedMfdModes[i] = g_mfdModes[i];
	}

	g_savedUnk0x100b20f8 = g_unk0x100b20f8;
	g_savedShowCompass = g_showCompass;
	g_savedCockpitLayout = g_cockpitLayoutIndex;
	file = fopen("mercs.hud", "wb");
	if (file) {
		fwrite(g_savedMfdModes, 4, 7, file);
		fwrite(&g_savedUnk0x100b20f8, 4, 1, file);
		fwrite(&g_savedShowCompass, 4, 1, file);
		fwrite(&g_savedCockpitLayout, 4, 1, file);
		fclose(file);
	}
}

// Reads MERCS.HUD, if there is one.
// FUNCTION: MW2PREMERCS 0x10035cbc
void LoadMfdModes(void)
{
	FILE* file;

	file = fopen("mercs.hud", "rb");
	if (file) {
		g_mfdModesLoaded = 1;
		fread(g_savedMfdModes, 4, 7, file);
		fread(&g_savedUnk0x100b20f8, 4, 1, file);
		fread(&g_savedShowCompass, 4, 1, file);
		fread(&g_savedCockpitLayout, 4, 1, file);
		fclose(file);
	}
	else {
		g_mfdModesLoaded = 0;
	}
}

// Gives the MFDs and the HUD MERCS.HUD's settings, if it was read. Returns whether it was.
// FUNCTION: MW2PREMERCS 0x10035d62
MechS32 ApplyMfdModes(void)
{
	if (g_mfdModesLoaded) {
		g_cockpitPanels[22]->m_setMfdMode(g_cockpitPanels[22], g_savedMfdModes[2]);
		g_cockpitPanels[26]->m_setMfdMode(g_cockpitPanels[26], g_savedMfdModes[3]);
		g_cockpitPanels[17]->m_setMfdMode(g_cockpitPanels[17], g_savedMfdModes[1]);
		g_cockpitPanels[2]->m_setMfdMode(g_cockpitPanels[2], g_savedMfdModes[0]);
		g_cockpitPanels[31]->m_setMfdMode(g_cockpitPanels[31], g_savedMfdModes[4]);
		g_cockpitPanels[32]->m_setMfdMode(g_cockpitPanels[32], g_savedMfdModes[5]);
		g_cockpitPanels[33]->m_setMfdMode(g_cockpitPanels[33], g_savedMfdModes[6]);
		g_unk0x100b20f8 = g_savedUnk0x100b20f8;
		g_showCompass = g_savedShowCompass;
		g_requestedCockpitView = g_savedCockpitLayout;
		RequestCockpitLayout(g_savedCockpitLayout);
	}

	return g_mfdModesLoaded;
}

// Steps an MFD to its next mode (MFD_CYCLE_A to _F), after 12 back to off.
// FUNCTION: MW2PREMERCS 0x10035e66
void CycleMfdMode(CockpitPanel* p_panel)
{
	p_panel->m_mfdMode++;
	if (p_panel->m_mfdMode == 13) {
		p_panel->m_mfdMode = 0;
	}
}

// Gives the MFDs and the HUD their first settings (MFDS_RESET).
// FUNCTION: MW2PREMERCS 0x10035e8e
void ResetMfdModes(void)
{
	g_cockpitPanels[22]->m_setMfdMode(g_cockpitPanels[22], 9);
	g_cockpitPanels[26]->m_setMfdMode(g_cockpitPanels[26], 10);
	g_cockpitPanels[17]->m_setMfdMode(g_cockpitPanels[17], 8);
	g_cockpitPanels[2]->m_setMfdMode(g_cockpitPanels[2], 1);
	g_cockpitPanels[31]->m_setMfdMode(g_cockpitPanels[31], 0);
	g_cockpitPanels[32]->m_setMfdMode(g_cockpitPanels[32], 11);
	g_cockpitPanels[33]->m_setMfdMode(g_cockpitPanels[33], 12);
	g_unk0x100b20f8 = 1;
	g_showCompass = 1;
	g_requestedCockpitView = 1;
	RequestCockpitLayout(1);
}

// Turns every MFD off.
// FUNCTION: MW2PREMERCS 0x10035f5b
void ClearMfdModes(void)
{
	g_cockpitPanels[22]->m_setMfdMode(g_cockpitPanels[22], 0);
	g_cockpitPanels[26]->m_setMfdMode(g_cockpitPanels[26], 0);
	g_cockpitPanels[17]->m_setMfdMode(g_cockpitPanels[17], 0);
	g_cockpitPanels[2]->m_setMfdMode(g_cockpitPanels[2], 0);
	g_cockpitPanels[31]->m_setMfdMode(g_cockpitPanels[31], 0);
	g_cockpitPanels[32]->m_setMfdMode(g_cockpitPanels[32], 0);
	g_cockpitPanels[33]->m_setMfdMode(g_cockpitPanels[33], 0);
}

// Keeps the MFDs' modes in g_mfdModes.
// FUNCTION: MW2PREMERCS 0x10036000
void RememberMfdModes(void)
{
	g_mfdModes[1] = g_cockpitPanels[17]->m_mfdMode;
	g_mfdModes[0] = g_cockpitPanels[2]->m_mfdMode;
	g_mfdModes[2] = g_cockpitPanels[22]->m_mfdMode;
	g_mfdModes[3] = g_cockpitPanels[26]->m_mfdMode;
	g_mfdModes[4] = g_cockpitPanels[31]->m_mfdMode;
	g_mfdModes[5] = g_cockpitPanels[32]->m_mfdMode;
	g_mfdModes[6] = g_cockpitPanels[33]->m_mfdMode;
}

// Gives the MFDs back the modes RememberMfdModes kept.
// FUNCTION: MW2PREMERCS 0x10036066
void RestoreMfdModes(void)
{
	g_cockpitPanels[22]->m_setMfdMode(g_cockpitPanels[22], g_mfdModes[2]);
	g_cockpitPanels[26]->m_setMfdMode(g_cockpitPanels[26], g_mfdModes[3]);
	g_cockpitPanels[17]->m_setMfdMode(g_cockpitPanels[17], g_mfdModes[1]);
	g_cockpitPanels[2]->m_setMfdMode(g_cockpitPanels[2], g_mfdModes[0]);
	g_cockpitPanels[31]->m_setMfdMode(g_cockpitPanels[31], g_mfdModes[4]);
	g_cockpitPanels[32]->m_setMfdMode(g_cockpitPanels[32], g_mfdModes[5]);
	g_cockpitPanels[33]->m_setMfdMode(g_cockpitPanels[33], g_mfdModes[6]);
}

// The MFD's modes: 1 (and the others not listed) the damage outline, 2 the armor bars, 3 the
// rear view, 4 the view from the mech's eye, 5 the view from above, 6 the satellite view, 7 the
// tracked shot, 8 the target, 9 the speed, 10 the jump jets, 11 the heat and 12 the heat rate.
// FUNCTION: MW2PREMERCS 0x10036127
void DrawMechViewPanel(CockpitPanel* p_panel)
{
	MechScalar* camera;
	MechScalar view[7];
	RenderSettings saved;
	Mech* mech;

	camera = NULL;
	if (!p_panel->m_enabled || !p_panel->m_mfdMode) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	if (p_panel->m_damage == 1 && p_panel->m_mfdMode != 1 && p_panel->m_mfdMode != 2) {
		if (g_mechViewStatic) {
			if (RandomIntBelow(10) < 7) {
				g_mechViewStatic = 0;
			}

			DrawMechViewStatic(p_panel);
			return;
		}
		else if (RandomIntBelow(10) < 3) {
			g_mechViewStatic = 1;
		}
	}
	else if (p_panel->m_damage > 2 && p_panel->m_mfdMode != 1 && p_panel->m_mfdMode != 2) {
		DrawMechViewStatic(p_panel);
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	switch (p_panel->m_mfdMode) {
	case 0:
		break;
	case 11:
		DrawHeatPanel(p_panel);
		break;
	case 12:
		DrawHeatRatePanel(p_panel);
		break;
	case 10:
		DrawJetsPanel(p_panel);
		break;
	case 9:
		DrawSpeedPanel(p_panel);
		break;
	case 8:
		DrawTargetPanel(p_panel);
		break;
	case 6:
		DrawSatelliteToPane(p_panel->m_target);
		OutlinePane(p_panel->m_target, 6);
		break;
	case 7:
		camera = GetTrackedShotView();
		if (!camera) {
			TrackLastShot();
			camera = GetTrackedShotView();
		}

		if (!camera) {
			VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
		}
		else {
			SetMechViewRenderSettings(&saved);
			camera[4] = 0;
			RenderViewToPane(p_panel->m_weapon, FIXED_CONST(2), camera, 0);
			memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
		}

		DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xfd));
		break;
	case 5:
		SaveView(g_eyepoint, view);
		SetMechViewRenderSettings(&saved);
		view[0] = mech->m_player->m_position.m_x;
		view[1] = mech->m_player->m_position.m_y;
		view[2] = mech->m_player->m_position.m_z;
		view[4] = FIXED_CONST(90);
		view[5] = 0;
		HideObjTree(mech->m_player->m_obj);
		RenderViewToPane(p_panel->m_weapon, FIXED_CONST(2), view, 0);
		ShowObjTree(mech->m_player->m_obj);
		DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xf7));
		memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
		break;
	case 3:
		SaveView(g_eyepoint, view);
		SetMechViewRenderSettings(&saved);
		if (g_frontViewForRear) {
			view[3] = mech->m_player->m_torsoTwist + mech->m_player->m_heading;
		}
		else {
			view[3] = mech->m_player->m_heading + FIXED_CONST(180);
		}

		view[4] = 0;
		HideObjTree(mech->m_player->m_obj);
		RenderViewToPane(p_panel->m_weapon, FIXED_CONST(2), view, 0);
		ShowObjTree(mech->m_player->m_obj);
		if (g_frontViewForRear) {
			OutlinePane(p_panel->m_target, 6);
		}
		else {
			DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xfa));
		}

		memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
		break;
	case 4:
		SaveView(g_eyepoint, view);
		SetEyeViewRenderSettings(&saved);
		GetPlayerEyeView(&view[4], &view[3], &view[5], &view[0], &view[1], &view[2]);
		HideObjTree(mech->m_player->m_obj);
		RenderViewToPaneFor(p_panel->m_weapon, FIXED_CONST(2), view, 0, mech);
		ShowObjTree(mech->m_player->m_obj);
		OutlinePane(p_panel->m_target, 6);
		memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
		break;
	case 2:
		DrawArmorBars(mech, p_panel->m_target);
		break;
	default:
		DrawDamageOutline(mech, p_panel->m_target, p_panel->m_weapon);
		break;
	}
}
#else
#define MECH_VIEW_MODE(p_panel) g_mechViewMode
#define MECH_VIEW_SLOT(p_panel) 5

// FUNCTION: MW2 0x100509a0
// FUNCTION: MW2GBL 0x10012190
// FUNCTION: MW2MATROX 0x1008ad50
// FUNCTION: MW23DFX 0x10011700
// FUNCTION: MW2ATI 0x10011c60
// FUNCTION: MW2S3 0x1002e780
// FUNCTION: MW2POWERVR 0x10010af0
void CycleMechViewMode(void)
{
	g_mechViewMode++;
	if (g_mechViewMode == 6) {
		g_mechViewMode = 1;
	}
}
#endif

#ifdef MW2_POWERVR
// The PowerVR edition renders the panel's views as an inset of the 3D view (the panel's m_view),
// so its panel only places the inset's camera for the mode (m_unk0x90, before the frame is
// rendered) and draws the frame over it (DrawMechViewPanel). The camera follows the rear view (3),
// the overhead view (4) or the tracked shot (5), and only while the mech runs.
// FUNCTION: MW2POWERVR 0x10010b10
void FUN_10010b10(CockpitPanel* p_panel)
{
	MechScalar view[7];
	MechScalar* camera;
	Mech* mech;

	if (!p_panel->m_enabled || !g_mechViewMode || !p_panel->m_view) {
		return;
	}

	if (p_panel->m_damage == 1 && g_mechViewMode != 1 && g_mechViewMode != 2) {
		if (g_mechViewStatic) {
			return;
		}
	}
	else if (p_panel->m_damage > 2 && g_mechViewMode != 1 && g_mechViewMode != 2) {
		return;
	}

	if (g_cockpitPowerState != 2) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	switch (g_mechViewMode) {
	case 3:
		SaveView(g_eyepoint, view);
		if (g_frontViewForRear) {
			view[3] = mech->m_player->m_torsoTwist + mech->m_player->m_heading;
		}
		else {
			view[3] = mech->m_player->m_heading + 180.0f;
		}

		view[4] = 0.0f;
		FUN_1005fb60(((A3DView*) p_panel->m_view)->m_transform, TRUE, view[0], view[1], view[2]);
		FUN_1005fbc0(((A3DView*) p_panel->m_view)->m_transform, 0, view[4], view[3], view[5], FALSE);
		p_panel->m_unk0x30 = 1;
		FUN_10060770((A3DView*) p_panel->m_view, 0, 1, 0, 0);
		break;
	case 4:
		SaveView(g_eyepoint, view);
		view[0] = mech->m_player->m_position.m_x;
		view[1] = mech->m_player->m_position.m_y;
		view[2] = mech->m_player->m_position.m_z;
		view[4] = 90.0f;
		view[5] = 0.0f;
		FUN_1005fb60(((A3DView*) p_panel->m_view)->m_transform, TRUE, view[0], view[1], view[2]);
		FUN_1005fbc0(((A3DView*) p_panel->m_view)->m_transform, 0, view[4], view[3], view[5], FALSE);
		p_panel->m_unk0x30 = 1;
		FUN_10060770((A3DView*) p_panel->m_view, 0, 1, 0, 0);
		break;
	case 5:
		camera = GetTrackedShotView();
		if (!camera) {
			TrackLastShot();
			camera = GetTrackedShotView();
			if (!camera) {
				return;
			}
		}

		camera[4] = 0.0f;
		FUN_1005fb60(((A3DView*) p_panel->m_view)->m_transform, TRUE, camera[0], camera[1], camera[2]);
		FUN_1005fbc0(((A3DView*) p_panel->m_view)->m_transform, 0, camera[4], camera[3], camera[5], FALSE);
		p_panel->m_unk0x30 = 1;
		FUN_10060770((A3DView*) p_panel->m_view, 0, 1, 0, 0);
		break;
	}
}

// The panel's other handler: the inset view's qualities, fog, texturing and smooth shading.
// FUNCTION: MW2POWERVR 0x10010d50
void FUN_10010d50(CockpitPanel* p_panel)
{
	FUN_10060770(&g_a3dPanel2View, 1, 1, 1, 0);
}

// The PowerVR edition's: as the others' below, but the renderer has drawn the views 3 to 5 into the
// inset, so the panel only draws their frames (and the black of a shot view it can't track).
// FUNCTION: MW2POWERVR 0x10010d70
void DrawMechViewPanel(CockpitPanel* p_panel)
{
	Mech* mech;

	if (!p_panel->m_enabled || !g_mechViewMode) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	if (p_panel->m_damage == 1 && g_mechViewMode != 1 && g_mechViewMode != 2) {
		if (g_mechViewStatic) {
			if (RandomIntBelow(10) < 7) {
				g_mechViewStatic = 0;
			}

			DrawMechViewStatic(p_panel);
			return;
		}
		else if (RandomIntBelow(10) < 3) {
			g_mechViewStatic = 1;
		}
	}
	else if (p_panel->m_damage > 2 && g_mechViewMode != 1 && g_mechViewMode != 2) {
		DrawMechViewStatic(p_panel);
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	switch (g_mechViewMode) {
	case 0:
		break;
	case 5:
		if (!GetTrackedShotView()) {
			TrackLastShot();
			if (!GetTrackedShotView()) {
				VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
			}
		}

		DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xfd));
		break;
	case 4:
		DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xf7));
		break;
	case 3:
		if (g_frontViewForRear) {
			OutlinePane(p_panel->m_target, 6);
		}
		else {
			DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xfa));
		}
		break;
	case 2:
		DrawArmorBars(mech, p_panel->m_target);
		break;
	default:
		DrawDamageOutline(mech, p_panel->m_target);
		break;
	}
}
#elif !defined(MW2_MFD_PANELS)
// MW2MATROX: clears each rendered view with the transparent pixel and presents the pane.
// FUNCTION: MW2 0x100509c8
// FUNCTION: MW2GBL 0x100121b8
// FUNCTION: MW2MATROX 0x1008ad78
// FUNCTION: MW23DFX 0x10011720
// FUNCTION: MW2ATI 0x10011c80
// FUNCTION: MW2S3 0x1002e7a0
void DrawMechViewPanel(CockpitPanel* p_panel)
{
	MechScalar* camera;
	MechScalar view[7];
	RenderSettings saved;
	Mech* mech;

	camera = NULL;
	if (!p_panel->m_enabled || !g_mechViewMode) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	if (p_panel->m_damage == 1 && g_mechViewMode != 1 && g_mechViewMode != 2) {
		if (g_mechViewStatic) {
			if (RandomIntBelow(10) < 7) {
				g_mechViewStatic = 0;
			}

			DrawMechViewStatic(p_panel);
			return;
		}
		else if (RandomIntBelow(10) < 3) {
			g_mechViewStatic = 1;
		}
	}
	else if (p_panel->m_damage > 2 && g_mechViewMode != 1 && g_mechViewMode != 2) {
		DrawMechViewStatic(p_panel);
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	switch (g_mechViewMode) {
	case 0:
		break;
	case 5:
		camera = GetTrackedShotView();
		if (!camera) {
			TrackLastShot();
			camera = GetTrackedShotView();
		}

		if (!camera) {
			VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
		}
		else {
			SetMechViewRenderSettings(&saved);
			camera[4] = 0;
			RenderViewToPane(5, FIXED_CONST(2), camera, 0);
#ifdef MW2_STRUCT_COPY_MEMCPY
			memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
#else
			g_renderSettings = saved;
#endif
#ifdef MW2_KEY_COLOR
			VFX_pane_wipe(p_panel->m_target, MECH_VIEW_WIPE);
#endif
		}

		DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xfd));
#ifdef MW2_KEY_COLOR
		FUN_10088280(p_panel->m_target);
#endif
		break;
	case 4:
		SaveView(g_eyepoint, view);
		SetMechViewRenderSettings(&saved);
		view[0] = mech->m_player->m_position.m_x;
		view[1] = mech->m_player->m_position.m_y;
		view[2] = mech->m_player->m_position.m_z;
		view[4] = FIXED_CONST(90);
		view[5] = 0;
		HideObjTree(mech->m_player->m_obj);
		RenderViewToPane(5, FIXED_CONST(2), view, 0);
		ShowObjTree(mech->m_player->m_obj);
#ifdef MW2_KEY_COLOR
		VFX_pane_wipe(p_panel->m_target, MECH_VIEW_WIPE);
#endif
		DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xf7));
#ifdef MW2_STRUCT_COPY_MEMCPY
		memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
#else
		g_renderSettings = saved;
#endif
#ifdef MW2_KEY_COLOR
		FUN_10088280(p_panel->m_target);
#endif
		break;
	case 3:
		SaveView(g_eyepoint, view);
		SetMechViewRenderSettings(&saved);
		if (g_frontViewForRear) {
			view[3] = mech->m_player->m_torsoTwist + mech->m_player->m_heading;
		}
		else {
			view[3] = mech->m_player->m_heading + FIXED_CONST(180);
		}

		view[4] = 0;
		HideObjTree(mech->m_player->m_obj);
		RenderViewToPane(5, FIXED_CONST(2), view, 0);
		ShowObjTree(mech->m_player->m_obj);
#ifdef MW2_KEY_COLOR
		VFX_pane_wipe(p_panel->m_target, MECH_VIEW_WIPE);
#endif
		if (g_frontViewForRear) {
			OutlinePane(p_panel->m_target, 6);
		}
		else {
			DrawMechViewFrame(p_panel, 6, MECH_VIEW_SHAPE(0xfa));
		}

#ifdef MW2_STRUCT_COPY_MEMCPY
		memcpy(&g_renderSettings, &saved, sizeof(RenderSettings));
#else
		g_renderSettings = saved;
#endif
#ifdef MW2_KEY_COLOR
		FUN_10088280(p_panel->m_target);
#endif
		break;
	case 2:
		DrawArmorBars(mech, p_panel->m_target);
#ifdef MW2_KEY_COLOR
		FUN_10088280(p_panel->m_target);
#endif
		break;
	default:
		DrawDamageOutline(mech, p_panel->m_target);
		break;
	}
}
#endif

// MW2MATROX (0x1008b1da) writes m_affineTextures and m_untexturedKinds 8 bytes further on
// (0x54, 0x58): the Matrox edition's RenderSettings has its two extra dwords at 0x28 or 0x30 (its
// g_renderSettings holds 1, 1, 0xe0, 0xef from 0x28, and RenderViewToPane saves and clears
// 0x28 and 0x2c), not at the end.
// FUNCTION: MW2 0x10050dc3
// FUNCTION: MW2GBL 0x100125bf
// FUNCTION: MW2PREMERCS 0x100366ac
// FUNCTION: MW2MATROX 0x1008b1da
// FUNCTION: MW23DFX 0x100119f0
// FUNCTION: MW2ATI 0x10011f50
// FUNCTION: MW2S3 0x1002ea70
void SetMechViewRenderSettings(RenderSettings* p_saved)
{
#ifdef MW2_STRUCT_COPY_MEMCPY
	memcpy(p_saved, &g_renderSettings, sizeof(RenderSettings));
#else
	*p_saved = g_renderSettings;
#endif
	g_renderSettings.m_drawPixels = 0;
	g_renderSettings.m_unk0x08 = 0;
	g_renderSettings.m_textures = 1;
	g_renderSettings.m_gouraud = 1;
	g_renderSettings.m_untexturedKinds = 0xb00;
	g_renderSettings.m_affineTextures = 1;
	g_renderSettings.m_flags &= ~4;
}

#ifdef MW2_MFD_PANELS
// SetMechViewRenderSettings again, for the view from the mech's eye (mode 4).
// FUNCTION: MW2PREMERCS 0x1003670d
void SetEyeViewRenderSettings(RenderSettings* p_saved)
{
	memcpy(p_saved, &g_renderSettings, sizeof(RenderSettings));
	g_renderSettings.m_drawPixels = 0;
	g_renderSettings.m_unk0x08 = 0;
	g_renderSettings.m_textures = 1;
	g_renderSettings.m_gouraud = 1;
	g_renderSettings.m_untexturedKinds = 0xb00;
	g_renderSettings.m_affineTextures = 1;
	g_renderSettings.m_flags &= ~4;
}

#endif
// FUNCTION: MW2 0x10050e20
// FUNCTION: MW2GBL 0x10012620
// FUNCTION: MW2PREMERCS 0x1003676e
// FUNCTION: MW2MATROX 0x1008b237
// FUNCTION: MW23DFX 0x10011a40
// FUNCTION: MW2ATI 0x10011fa0
// FUNCTION: MW2S3 0x1002eac0
// FUNCTION: MW2POWERVR 0x10010ee0
void DrawMechViewStatic(CockpitPanel* p_panel)
{
	if (!p_panel->m_enabled || !MECH_VIEW_MODE(p_panel)) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	DrawPanelAnim(p_panel->m_target, 0, 0, 0);
}

// FUNCTION: MW2 0x10050e6c
// FUNCTION: MW2GBL 0x1001266c
// FUNCTION: MW2PREMERCS 0x100367c0
// FUNCTION: MW2MATROX 0x1008b283
// FUNCTION: MW23DFX 0x10011a70
// FUNCTION: MW2ATI 0x10011fd0
// FUNCTION: MW2S3 0x1002eaf0
// FUNCTION: MW2POWERVR 0x10010f10
void DrawMechViewFrame(CockpitPanel* p_panel, MechS32 p_color, MechS32 p_shapeId)
{
	PANE* target;
	MechS32 x;
	MechS32 y;

	target = p_panel->m_target;
	OutlinePane(target, p_color);
	x = p_panel->m_width >> 1;
	y = 2;
	DrawPaneShape(x, y, p_shapeId, target);
}

// FUNCTION: MW2 0x10050ebe
// FUNCTION: MW2GBL 0x100126be
// FUNCTION: MW2PREMERCS 0x10036818
// FUNCTION: MW2MATROX 0x1008b2d5
// FUNCTION: MW23DFX 0x10011aa0
// FUNCTION: MW2ATI 0x10012000
// FUNCTION: MW2S3 0x1002eb20
// FUNCTION: MW2POWERVR 0x10010f40
void DrawMechViewStartup(CockpitPanel* p_panel)
{
	PANE savedTarget;
	PANE savedSlot;
	RectTransition* transition;
	PANE* frame;

	if (!p_panel->m_enabled || !MECH_VIEW_MODE(p_panel) || MECH_VIEW_MODE(p_panel) == 2 ||
		MECH_VIEW_MODE(p_panel) == 1) {
		return;
	}

	transition = p_panel->m_transition;
	if (transition) {
		if (p_panel->m_lastPowerState != 1) {
			StartRectTransition(transition);
		}

		frame = UpdateRectTransitionByAxis(0, transition);
		if (frame) {
			savedSlot = g_panes[MECH_VIEW_SLOT(p_panel)];
			savedTarget = *p_panel->m_target;
			g_panes[MECH_VIEW_SLOT(p_panel)] = *frame;
			*p_panel->m_target = *frame;
			DrawMechViewPanel(p_panel);
			g_panes[MECH_VIEW_SLOT(p_panel)] = savedSlot;
			*p_panel->m_target = savedTarget;
		}
		else {
			DrawMechViewPanel(p_panel);
		}
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
}

// FUNCTION: MW2 0x10050fd6
// FUNCTION: MW2GBL 0x100127d6
// FUNCTION: MW2PREMERCS 0x10036960
// FUNCTION: MW2MATROX 0x1008b3ed
// FUNCTION: MW23DFX 0x10011b70
// FUNCTION: MW2ATI 0x100120d0
// FUNCTION: MW2S3 0x1002ebf0
// FUNCTION: MW2POWERVR 0x10011010
void DrawMechViewShutdown(CockpitPanel* p_panel)
{
	PANE savedTarget;
	PANE savedSlot;
	RectTransition* transition;
	PANE* frame;

	if (!p_panel->m_enabled || !MECH_VIEW_MODE(p_panel) || MECH_VIEW_MODE(p_panel) == 2 ||
		MECH_VIEW_MODE(p_panel) == 1) {
		return;
	}

	transition = p_panel->m_transition;
	if (transition) {
		if (p_panel->m_lastPowerState != 0 && p_panel->m_lastPowerState != 3 && p_panel->m_lastPowerState != 4) {
			StartRectTransition(transition);
		}

		frame = UpdateRectTransitionByAxis(1, transition);
		if (frame) {
			savedSlot = g_panes[MECH_VIEW_SLOT(p_panel)];
			savedTarget = *p_panel->m_target;
			g_panes[MECH_VIEW_SLOT(p_panel)] = *frame;
			*p_panel->m_target = *frame;
			DrawMechViewPanel(p_panel);
			g_panes[MECH_VIEW_SLOT(p_panel)] = savedSlot;
			*p_panel->m_target = savedTarget;
		}
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
}
