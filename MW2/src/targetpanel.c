#define FIXEDTRIG_FLOAT_SINE /* the accelerated editions' float sine, where they have it (fixedtrig.h) */
#include "targetpanel.h"

#include "classtable.h"
#include "clock.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixedfloat.h"
#include "fixedmul29.h"
#include "fixedtrig.h"
#include "hud.h"
#include "loadres.h"
#include "mech.h"
#include "mechviewpanel.h"
#include "menu.h"
#include "mw2prj.h"
#include "navpoint.h"
#include "object.h"
#include "palette.h"
#include "players.h"
#include "polydraw.h"
#include "random.h"
#include "recttransition.h"
#include "screenscale.h"
#include "setres.h"
#include "shape.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "targeting.h"
#include "team.h"
#include "types.h"
#include "vfxa.h"

#include <stdio.h>
#include <string.h>

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif
#ifdef MW2_POWERVR
#include "powervr/a3dpanel.h"
#endif
#ifdef MW2_LOD_PAR
#include "render.h"
#include "view.h"

MechS32 IsPlayerInView(Player* p_player);
#endif

// GLOBAL: MW2 0x100ba4bc
// GLOBAL: MW2MATROX 0x100a4a4c
// GLOBAL: MW23DFX 0x1024d04c
// GLOBAL: MW2ATI 0x101813cc
// GLOBAL: MW2S3 0x10062ae4
// GLOBAL: MW2POWERVR 0x101ac35c
MechS32 g_targetPanelMode = 1;

// Set to announce the target's side with the next name the target panel shows.
// GLOBAL: MW2 0x100ba4c0
// GLOBAL: MW2MATROX 0x100a4a50
// GLOBAL: MW23DFX 0x1024d050
// GLOBAL: MW2S3 0x10062ae8
// GLOBAL: MW2POWERVR 0x101ac360
MechS32 g_announceTargetSide = 0;

#ifdef MW2_LOD_PAR
// The level of detail the target panel loads a target mech's class at (LOD.PAR's
// TargetWindowREP, read once).
// GLOBAL: MW2MATROX 0x100a4a54
MechS32 g_targetWindowRep = 0;

// GLOBAL: MW2MATROX 0x100a4a58
MechS32 g_targetLodParRead = 0;
#endif

// When the target panel goes from the target's short name to its name.
// GLOBAL: MW2 0x100ba4c4
// GLOBAL: MW2MATROX 0x100a4a5c
// GLOBAL: MW23DFX 0x1024d054
// GLOBAL: MW2S3 0x10062aec
// GLOBAL: MW2POWERVR 0x101ac368
MechS32 g_targetFullNameTime = 0;

// The target the panel showed last.
// GLOBAL: MW2 0x100ba4c8
// GLOBAL: MW2MATROX 0x100a4a60
// GLOBAL: MW23DFX 0x1024d058
// GLOBAL: MW2S3 0x10062af0
// GLOBAL: MW2POWERVR 0x101ac36c
MechS32 g_lastPanelTarget = 0;

// The target panel flickers to static while set.
// GLOBAL: MW2 0x100ba4cc
// GLOBAL: MW2MATROX 0x100a4a64
// GLOBAL: MW23DFX 0x1024d05c
// GLOBAL: MW2S3 0x10062af4
// GLOBAL: MW2POWERVR 0x101ac364
MechS32 g_targetPanelStatic = 0;

#ifdef MW2_POWERVR
// The list of the target panel's view that holds the target (FUN_10020450).
// GLOBAL: MW2POWERVR 0x101ac370
MechS32 g_targetViewList = -1;

// The detail level the target's mech had before the panel loaded level 0 for it, to restore.
// GLOBAL: MW2POWERVR 0x101ac374
MechS32 g_targetSavedDetailLevel = -1;

// The root of the target's object tree, which the panel's view shows.
// GLOBAL: MW2POWERVR 0x101ac378
SceneObject* g_targetViewObject = NULL;
#endif

// The name the target panel shows for an unknown installation: 16 bytes in every edition, each
// with other globals after it.
// GLOBAL: MW2 0x100c26a0
// GLOBAL: MW2MATROX 0x10212d40
// GLOBAL: MW23DFX 0x101de7a0
// GLOBAL: MW2ATI 0x10112460
// GLOBAL: MW2S3 0x1014d9f0
// GLOBAL: MW2POWERVR 0x1013d120
MechChar g_anonymousInstallationName[16];

// Writes the target panel's text: the locked target's name (its short name when it changes, its
// name after a while), coloured by its side, and its distance (MW2_TARGET_RANGE_AT_BOTTOM: on the
// panel's bottom line).
// FUNCTION: MW2 0x1007b930
// FUNCTION: MW2MATROX 0x10017e50
// FUNCTION: MW23DFX 0x10020e40
// FUNCTION: MW2ATI 0x10021110
// FUNCTION: MW2S3 0x10013150
// FUNCTION: MW2POWERVR 0x1001ff60
void DrawTargetPanelText(CockpitPanel* p_panel)
{
	MechS32 index;
	MechFloat km;
	MechS32 color;
	MechS32 kind;
	MechS32 meters;
	Mech* mech;
	MechChar text[64];
	void* font;
#ifdef MW2_TARGET_RANGE_AT_BOTTOM
	MechS32 top;
#endif

	color = 0xe;
	if (!p_panel->m_enabled) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	kind = mech->m_player->m_targetInfo.m_target & 0xf00;
	index = mech->m_player->m_targetInfo.m_target & 0xff;
	if (!kind || (mech->m_player->m_targetInfo.m_target & 0x1000)) {
		return;
	}

	if (mech->m_player->m_targetInfo.m_target != g_lastPanelTarget) {
		g_targetFullNameTime = 0;
	}

	if (g_inspectResult == 2) {
		g_targetFullNameTime = g_currentClock + 362;
		p_panel->m_setName(p_panel, "Out of range");
		PlayCockpitSound(0x14, -1);
	}
	else if (g_inspectResult) {
		g_targetFullNameTime = g_currentClock + 362;
		switch (kind) {
		case 0x100:
			p_panel->m_setName(p_panel, g_navTable[index].m_shortName);
			PlaySoundEffect(0xdc, 100, 0x40, 5, 0x32);
			break;
		case 0x400:
			p_panel->m_setName(p_panel, g_gameThings[index].m_shortName);
			PlaySoundEffect(0xdc, 100, 0x40, 5, 0x32);
			break;
		case 0x200:
			p_panel->m_setName(p_panel, g_players[index]->m_shortName);
			PlaySoundEffect(0xdc, 100, 0x40, 5, 0x32);
			break;
		default:
			p_panel->m_setName(p_panel, "");
			break;
		}

		if (!p_panel->m_name[0]) {
			p_panel->m_setName(p_panel, "Contents unknown");
		}
	}

	if (!g_targetFullNameTime || g_currentClock > g_targetFullNameTime) {
		g_targetFullNameTime = 0;
		switch (kind) {
		case 0x100:
			if (!(g_navTable[index].m_flags & 0x20) && (g_navTable[index].m_flags & 0x100)) {
				p_panel->m_setName(p_panel, "Unknown");
			}
			else if (!strlen(g_navTable[index].m_name)) {
				p_panel->m_setName(p_panel, "Nav Point");
			}
			else {
				p_panel->m_setName(p_panel, g_navTable[index].m_name);
			}
			break;
		case 0x400:
			if (!(g_gameThings[index].m_flags & 0x20) && (g_gameThings[index].m_flags & 0x100)) {
				if (!g_anonymousInstallationName[0]) {
					strcpy(g_anonymousInstallationName, "Unknown");
				}

				p_panel->m_setName(p_panel, g_anonymousInstallationName);
			}
			else {
				if (!strlen(g_gameThings[index].m_name)) {
					p_panel->m_setName(p_panel, "Installation");
				}
				else {
					p_panel->m_setName(p_panel, g_gameThings[index].m_name);
				}

				if (g_announceTargetSide) {
					switch (GetThingSide(index)) {
					case 0:
						PlayCockpitSound(0xf, -1);
						break;
					case 2:
						PlayCockpitSound(0x10, -1);
						break;
					case 1:
						PlayCockpitSound(0xe, -1);
						break;
					}

					g_announceTargetSide = 0;
				}
			}
			break;
		case 0x200:
			if (!(g_players[index]->m_flags & 0x20) && (g_players[index]->m_flags & 0x100)) {
				p_panel->m_setName(p_panel, "Unknown");
			}
			else {
				if (!strlen(g_players[index]->m_name)) {
					p_panel->m_setName(p_panel, "Mech");
				}
				else {
					p_panel->m_setName(p_panel, g_players[index]->m_name);
				}

				if (g_announceTargetSide) {
					switch (GetPlayerSide(index)) {
					case 0:
						PlayCockpitSound(0xf, -1);
						break;
					case 2:
						PlayCockpitSound(0x10, -1);
						break;
					case 1:
						PlayCockpitSound(0xe, -1);
						break;
					}

					g_announceTargetSide = 0;
				}
			}
			break;
		default:
			p_panel->m_setName(p_panel, "");
			break;
		}
	}

	if (kind == 0x400) {
		switch (GetThingSide(index)) {
		case 0:
			color = 0xe;
			break;
		case 2:
			color = 6;
			break;
		case 1:
			color = 0xa;
			break;
		}
	}
	else if (kind == 0x200) {
		switch (GetPlayerSide(index)) {
		case 0:
			color = 0xe;
			break;
		case 2:
			color = 6;
			break;
		case 1:
			color = 0xa;
			break;
		}
	}
	else if (kind == 0x100) {
		if (!(g_navTable[index].m_flags & 0x20)) {
			color = 2;
		}
		else {
			color = 1;
		}
	}

	font = LoadCachedResource(g_mw2PrjHandle, HUD_ART_RESOLUTION + 1, g_resTagFont, 0);
	if (!font) {
		return;
	}

#ifdef MW2_VFX16
	g_textColors[0xe] = PIXEL_COLOR(color);
	VFX_string_draw(p_panel->m_target, 0, 0, font, p_panel->m_name, g_textColors);
	g_textColors[0xe] = PIXEL_COLOR(0xe);
#else
	g_textColors[0xe] = color;
	VFX_string_draw(p_panel->m_target, 0, 0, font, p_panel->m_name, g_textColors);
	g_textColors[0xe] = 0xe;
#endif
#ifdef MW2_FLOAT_SCALARS
	meters = FIXED_TO_INT(mech->m_player->m_targetInfo.m_range) / 100;
#ifdef MW2_TARGET_RANGE_AT_BOTTOM
	top = p_panel->m_target->m_y0;
	p_panel->m_target->m_y0 = p_panel->m_target->m_y1 - VFX_font_height16(font);
#endif
	if (meters > 1000) {
		km = meters / 1000.0f;
#else
	meters = mech->m_player->m_targetInfo.m_range / 100;
	if (meters > 1000) {
		km = meters / 1000.0;
#endif
#ifdef MW2_TARGET_RANGE_AT_BOTTOM
		sprintf(text, "%2.2fk", km);
#else
		sprintf(text, "\n%2.2fk", km);
#endif
		DrawWrappedText(p_panel->m_target, text, font);
	}
	else {
#ifdef MW2_TARGET_RANGE_AT_BOTTOM
		sprintf(text, "%3ldm", meters);
#else
		sprintf(text, "\n%3ldm", meters);
#endif
		DrawWrappedText(p_panel->m_target, text, font);
	}

#ifdef MW2_TARGET_RANGE_AT_BOTTOM
	p_panel->m_target->m_y0 = top;
#endif

	UnlockCachedResource(HUD_ART_RESOLUTION + 1, g_resTagFont);
	g_lastPanelTarget = mech->m_player->m_targetInfo.m_target;
#ifdef MW2_KEY_COLOR
	FUN_10088280(p_panel->m_target);
#endif
}

#ifdef MW2_POWERVR
// The target panel's m_unk0x90, before the scene is rendered: puts the locked target's object tree
// (its mech at detail level 0) in the panel's view, the camera behind it as DrawTargetPanel places
// it but closer (twice an installation's bounds, two and a half times a mech's radius), glowing in
// wireframe mode and textured in mode 2.
// FUNCTION: MW2POWERVR 0x10020450
void FUN_10020450(CockpitPanel* p_panel)
{
	MechS32 kind;
	Mech* mech;
	Player* player;
	Player* targetPlayer;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar heading;
	MechScalar distance;
	MechScalar dx;
	MechScalar dz;
	MechScalar view[7];
	SceneObject* object;

	g_targetSavedDetailLevel = -1;
	if (!p_panel->m_enabled || !g_targetPanelMode || !p_panel->m_view) {
		return;
	}

	if (p_panel->m_damage == 1) {
		if (g_targetPanelStatic) {
			return;
		}
	}
	else if (p_panel->m_damage > 2) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	player = mech->m_player;
	kind = player->m_targetInfo.m_target & 0xf00;
	if (!kind || (player->m_targetInfo.m_target & 0x1000) || kind == 0x100) {
		return;
	}

	if (g_cockpitPowerState != 2) {
		return;
	}

	g_targetViewObject = GetLocalTargetObject();
	if (!g_targetViewObject || !g_targetViewObject->m_shape) {
		return;
	}

	if (kind == 0x200) {
		targetPlayer = g_players[mech->m_player->m_targetInfo.m_target & 0xff];
		g_targetSavedDetailLevel = targetPlayer->m_detailLevel;
		LoadClassLevel(targetPlayer->m_index, 0);
		targetPlayer->m_detailLevel = 0;
	}

	x = player->m_targetInfo.m_position.m_x;
	y = player->m_targetInfo.m_position.m_y;
	z = player->m_targetInfo.m_position.m_z;
	heading = player->m_targetInfo.m_heading;
	if (kind == 0x400) {
		distance = GetShapeBounds(g_targetViewObject->m_shape, &x, &y, &z) * 2;
	}
	else {
		distance = g_players[player->m_targetInfo.m_target & 0xff]->m_mech->m_radius * 2.5f;
	}

	dx = -(FixedSin(heading) * distance);
	view[0] = x + dx;
	view[1] = y;
	dz = -(FixedCos(heading) * distance);
	view[2] = z + dz;
	FUN_1005fb60(((A3DView*) p_panel->m_view)->m_transform, 1, view[0], view[1], view[2]);
	FUN_1005fbc0(((A3DView*) p_panel->m_view)->m_transform, 0, 0, heading, 0, 0);
	if (g_targetViewList <= 0) {
		g_targetViewList = FUN_1005f200(((A3DView*) p_panel->m_view)->m_root);
	}
	else {
		FUN_1005f2a0(g_targetViewList);
	}

	object = g_targetViewObject;
	while (object->m_parent) {
		object = object->m_parent;
	}

	g_targetViewObject = object;
	FUN_10061c60(g_targetViewObject, g_targetPanelMode);
	FUN_1005f2c0(g_targetViewList, g_targetViewObject->m_list);
	p_panel->m_unk0x30 = 1;
	FUN_10060770((A3DView*) p_panel->m_view, 0, g_targetPanelMode == 2, 0, 0);
	if (g_targetPanelMode == 1) {
		FUN_10061160(g_a3dWhite, 1);
	}
}

// The target panel's m_unk0x94, after the scene is rendered: the materials back from wireframe
// mode, and the target's mech back to its detail level.
// FUNCTION: MW2POWERVR 0x100206e0
void FUN_100206e0(CockpitPanel* p_panel)
{
	MechU32 target;
	Player* targetPlayer;

	if (!g_targetViewObject) {
		return;
	}

	if (g_targetPanelMode == 1) {
		FUN_10061c60(g_targetViewObject, 2);
		FUN_10061160(g_a3dLightColour, 0);
	}

	target = g_players[g_localPlayerId]->m_mech->m_player->m_targetInfo.m_target;
	if ((target & 0xf00) == 0x200) {
		targetPlayer = g_players[target & 0xff];
		if (targetPlayer->m_detailLevel != g_targetSavedDetailLevel) {
			LoadClassLevel(targetPlayer->m_index, g_targetSavedDetailLevel);
			targetPlayer->m_detailLevel = g_targetSavedDetailLevel;
		}
	}
}

#endif
// Draws the target panel: the locked target through a camera behind it, a nav point's icon, or
// static while the panel is damaged (m_damage).
// FUNCTION: MW2 0x1007c126
// FUNCTION: MW2MATROX 0x10018683
// FUNCTION: MW23DFX 0x10021300
// FUNCTION: MW2ATI 0x100215d0
// FUNCTION: MW2S3 0x10013620
// FUNCTION: MW2POWERVR 0x10020760
void DrawTargetPanel(CockpitPanel* p_panel)
{
	MechS32 index;
	MechScalar dz;
	RenderSettings saved;
	Player* targetPlayer;
	MechScalar view[7];
	SceneObject* object;
	MechS32 kind;
	Mech* mech;
	MechS32 centerX;
	Player* player;
	MechS32 centerY;
	MechScalar distance;
	MechScalar heading;
	MechScalar x;
	MechScalar y;
	MechS32 targetIndex;
	void* noObject;
	void* noTarget;
	MechS32 icon;
	MechScalar dx;
	MechScalar z;
#ifdef MW2_LOD_PAR
	MechChar line[0x100];
	FILE* file;
#endif

#ifdef MW2_VFX16
	if (!p_panel->m_enabled || !g_targetPanelMode) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	if (p_panel->m_damage == 1) {
		if (g_targetPanelStatic) {
			if (RandomIntBelow(10) < 7) {
				g_targetPanelStatic = 0;
			}

			DrawTargetStatic(p_panel);
			return;
		}
		else {
			if (RandomIntBelow(10) < 3) {
				g_targetPanelStatic = 1;
			}
		}
	}
	else if (p_panel->m_damage > 2) {
		DrawTargetStatic(p_panel);
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	player = mech->m_player;
	kind = player->m_targetInfo.m_target & 0xf00;
	index = mech->m_player->m_targetInfo.m_target & 0xff;
	if (!kind || (player->m_targetInfo.m_target & 0x1000)) {
		VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
		OutlinePane(p_panel->m_target, 8);
#ifdef MW2_KEY_COLOR
		FUN_10088280(p_panel->m_target);
#endif
		return;
	}

	if (kind == 0x100) {
		centerX = (p_panel->m_target->m_x1 - p_panel->m_target->m_x0) / 2;
		centerY = (p_panel->m_target->m_y1 - p_panel->m_target->m_y0) / 2;
#ifdef MW2_HUD_SHAPE_IDS
		if (!(g_navTable[index].m_flags & 0x20)) {
			icon = 0x106;
		}
		else {
			icon = 0x109;
		}
#endif

		if (icon) {
			VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
#ifdef MW2_KEY_COLOR
			FUN_1001e01a(centerX, centerY, icon, p_panel->m_target);
#else
			DrawPaneShape(centerX, centerY, icon, p_panel->m_target);
#endif
			OutlinePane(p_panel->m_target, 8);
#ifdef MW2_KEY_COLOR
			FUN_10088280(p_panel->m_target);
#endif
		}

		return;
	}

	if (kind == 0x200) {
#ifdef MW2_LOD_PAR
		if (!g_targetLodParRead) {
			file = fopen("LOD.PAR", "rt");
			if (file) {
				while (fgets(line, 0x100, file)) {
					if (line[0] == '/') {
						continue;
					}

					if (sscanf(line, "TargetWindowREP = %i", &g_targetWindowRep)) {
						continue;
					}
				}

				fclose(file);
			}

			g_targetLodParRead = 1;
		}

		targetPlayer = g_players[mech->m_player->m_targetInfo.m_target & 0xff];
		if (!IsPlayerInView(targetPlayer)) {
			LoadClassLevel(targetPlayer->m_index, g_targetWindowRep);
		}
#else
		targetPlayer = g_players[mech->m_player->m_targetInfo.m_target & 0xff];
		LoadClassLevel(targetPlayer->m_index, 0);
#endif
	}

	SaveView(g_eyepoint, view);
	SetMechViewRenderSettings(&saved);
	x = player->m_targetInfo.m_position.m_x;
	y = player->m_targetInfo.m_position.m_y;
	z = player->m_targetInfo.m_position.m_z;
	heading = player->m_targetInfo.m_heading;
	object = GetLocalTargetObject();
	if (!object) {
#ifdef MW2_HUD_SHAPE_IDS
		noTarget = LoadCachedResource(g_mw2PrjHandle, 0x5e, g_resTagShp, 0);
#endif
		if (noTarget) {
			VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
			VFX_shape_draw(p_panel->m_target, noTarget, 0, 1, 1);
			OutlinePane(p_panel->m_target, 8);
#ifdef MW2_HUD_SHAPE_IDS
			UnlockCachedResource(0x5e, g_resTagShp);
#endif
#ifdef MW2_KEY_COLOR
			FUN_10088280(p_panel->m_target);
#endif
		}

		return;
	}
	else if (!object->m_shape) {
#ifdef MW2_HUD_SHAPE_IDS
		noObject = LoadCachedResource(g_mw2PrjHandle, 0x5b, g_resTagShp, 0);
#endif
		if (noObject) {
			VFX_pane_wipe(p_panel->m_target, PIXEL_COLOR(0));
			VFX_shape_draw(p_panel->m_target, noObject, 0, 1, 1);
			OutlinePane(p_panel->m_target, 8);
#ifdef MW2_HUD_SHAPE_IDS
			UnlockCachedResource(0x5b, g_resTagShp);
#endif
#ifdef MW2_KEY_COLOR
			FUN_10088280(p_panel->m_target);
#endif
		}

		return;
	}

	if (kind == 0x400) {
		distance = GetShapeBounds(object->m_shape, &x, &y, &z) * 3;
	}
	else {
		targetIndex = player->m_targetInfo.m_target & 0xff;
		distance = g_players[targetIndex]->m_mech->m_radius * 3;
	}

#ifdef MW2_FLOAT_SCALARS
	dx = -(FixedSin(heading) * distance);
#endif
	view[0] = x + dx;
	view[1] = y;
#ifdef MW2_FLOAT_SCALARS
	dz = -(FixedCos(heading) * distance);
#endif
	view[2] = z + dz;
	view[3] = heading;
	view[4] = 0;
	view[5] = 0;
	if (g_targetPanelMode == 1) {
		g_renderSettings.m_wireframe = 1;
		g_renderSettings.m_wireframeColors = 0;
	}
	else {
		g_renderSettings.m_wireframe = 0;
	}

	g_renderSettings.m_drawSky = g_renderSettings.m_drawGround = 0;
#ifdef MW2_A3D
	A3D_PANE_WIPE(p_panel->m_target, PIXEL_COLOR(0));
#endif
	if (g_cockpitPowerState == 2) {
		RenderViewToPane(7, FIXED_CONST(2), view, object);
	}

#ifdef MW2_KEY_COLOR
	VFX_pane_wipe(p_panel->m_target, 0xfffe);
#endif
	OutlinePane(p_panel->m_target, 8);
	g_renderSettings = saved;
#ifdef MW2_KEY_COLOR
	FUN_10088280(p_panel->m_target);
#endif
#else
	if (!p_panel->m_enabled || !g_targetPanelMode) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	if (p_panel->m_damage == 1) {
		if (g_targetPanelStatic) {
			if (RandomIntBelow(10) < 7) {
				g_targetPanelStatic = 0;
			}

			DrawTargetStatic(p_panel);
			return;
		}
		else {
			if (RandomIntBelow(10) < 3) {
				g_targetPanelStatic = 1;
			}
		}
	}
	else if (p_panel->m_damage > 2) {
		DrawTargetStatic(p_panel);
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	player = mech->m_player;
	kind = player->m_targetInfo.m_target & 0xf00;
	index = mech->m_player->m_targetInfo.m_target & 0xff;
	if (!kind || (player->m_targetInfo.m_target & 0x1000)) {
		VFX_pane_wipe(p_panel->m_target, 0);
		OutlinePane(p_panel->m_target, 8);
		return;
	}

	if (kind == 0x100) {
		centerX = (p_panel->m_target->m_x1 - p_panel->m_target->m_x0) / 2;
		centerY = (p_panel->m_target->m_y1 - p_panel->m_target->m_y0) / 2;
		if (!(g_navTable[index].m_flags & 0x20)) {
			icon = 0x100;
		}
		else {
			icon = 0x103;
		}

		if (icon) {
			VFX_pane_wipe(p_panel->m_target, 0);
			DrawPaneShape(centerX, centerY, icon, p_panel->m_target);
			OutlinePane(p_panel->m_target, 8);
		}

		return;
	}

	if (kind == 0x200) {
		targetPlayer = g_players[mech->m_player->m_targetInfo.m_target & 0xff];
		LoadClassLevel(targetPlayer->m_index, 0);
	}

	SaveView(g_eyepoint, view);
	SetMechViewRenderSettings(&saved);
	x = player->m_targetInfo.m_position.m_x;
	y = player->m_targetInfo.m_position.m_y;
	z = player->m_targetInfo.m_position.m_z;
	heading = player->m_targetInfo.m_heading;
	object = GetLocalTargetObject();
	if (!object) {
		noTarget = LoadCachedResource(g_mw2PrjHandle, 0x5b, g_resTagShp, 0);
		if (noTarget) {
			VFX_pane_wipe(p_panel->m_target, 0);
			VFX_shape_draw(p_panel->m_target, noTarget, 0, 1, 1);
			OutlinePane(p_panel->m_target, 8);
			UnlockCachedResource(0x5b, g_resTagShp);
		}

		return;
	}
	else if (!object->m_shape) {
		noObject = LoadCachedResource(g_mw2PrjHandle, 0x58, g_resTagShp, 0);
		if (noObject) {
			VFX_pane_wipe(p_panel->m_target, 0);
			VFX_shape_draw(p_panel->m_target, noObject, 0, 1, 1);
			OutlinePane(p_panel->m_target, 8);
			UnlockCachedResource(0x58, g_resTagShp);
		}

		return;
	}

	if (kind == 0x400) {
		distance = GetShapeBounds(object->m_shape, &x, &y, &z) * 3;
	}
	else {
		targetIndex = player->m_targetInfo.m_target & 0xff;
		distance = g_players[targetIndex]->m_mech->m_radius * 3;
	}

	dx = -FixedMul29(FixedSin(heading), distance);
	view[0] = x + dx;
	view[1] = y;
	dz = -FixedMul29(FixedCos(heading), distance);
	view[2] = z + dz;
	view[3] = heading;
	view[4] = 0;
	view[5] = 0;
	if (g_targetPanelMode == 1) {
		g_renderSettings.m_wireframe = 1;
		g_renderSettings.m_wireframeColors = 0;
	}
	else {
		g_renderSettings.m_wireframe = 0;
	}

	g_renderSettings.m_drawSky = g_renderSettings.m_drawGround = 0;
	VFX_pane_wipe(p_panel->m_target, 0);
	if (g_cockpitPowerState == 2) {
		RenderViewToPane(7, FIXED_CONST(2), view, object);
	}

	OutlinePane(p_panel->m_target, 8);
	g_renderSettings = saved;
#endif
}

#ifdef MW2_LOD_PAR
// Whether p_player's position projects into the main view (the target panel then leaves its
// level of detail alone).
// FUNCTION: MW2MATROX 0x10018d55
MechS32 IsPlayerInView(Player* p_player)
{
	MechScalar x;
	MechScalar y;
	MechScalar z;

	x = p_player->m_position.m_x;
	y = p_player->m_position.m_y;
	z = p_player->m_position.m_z;
	if (ProjectWorldPoint(&x, &y, &z)) {
		return 1;
	}
	else {
		return 0;
	}
}

#endif
// Draws a panel as static (animation 0).
// FUNCTION: MW2 0x1007c6df
// FUNCTION: MW2MATROX 0x10018db0
// FUNCTION: MW23DFX 0x10021710
// FUNCTION: MW2ATI 0x100219e0
// FUNCTION: MW2S3 0x10013a30
// FUNCTION: MW2POWERVR 0x10020a10
void DrawTargetStatic(CockpitPanel* p_panel)
{
	if (!p_panel->m_enabled) {
		return;
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
	DrawPanelAnim(p_panel->m_target, 0, 0, 0);
}

// Draws the target panel while its transition opens it, the view resized to the transition's
// rectangle (g_panes[7] and the panel's target) for the frame.
// FUNCTION: MW2 0x1007c71e
// FUNCTION: MW2MATROX 0x10018def
// FUNCTION: MW23DFX 0x10021730
// FUNCTION: MW2ATI 0x10021a00
// FUNCTION: MW2S3 0x10013a50
// FUNCTION: MW2POWERVR 0x10020a30
void DrawTargetPanelStartup(CockpitPanel* p_panel)
{
	PANE* rect;
	RectTransition* transition;
	PANE savedView;
	PANE savedTarget;

	if (!p_panel->m_enabled || !g_targetPanelMode) {
		return;
	}

	transition = p_panel->m_transition;
	if (transition) {
		if (p_panel->m_lastPowerState != 1) {
			StartRectTransition(transition);
		}

		rect = UpdateRectTransitionByAxis(0, transition);
		if (rect) {
			savedView = g_panes[7];
			savedTarget = *p_panel->m_target;
			g_panes[7] = *rect;
			*p_panel->m_target = *rect;
			DrawTargetPanel(p_panel);
			g_panes[7] = savedView;
			*p_panel->m_target = savedTarget;
		}
		else {
			DrawTargetPanel(p_panel);
		}
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
}

// Draws the target panel while its transition closes it, unless the view it last drew (0, 3 or
// 4) already had it closed.
// FUNCTION: MW2 0x1007c81c
// FUNCTION: MW2MATROX 0x10018eed
// FUNCTION: MW23DFX 0x100217f0
// FUNCTION: MW2ATI 0x10021ac0
// FUNCTION: MW2S3 0x10013b10
// FUNCTION: MW2POWERVR 0x10020af0
void DrawTargetPanelShutdown(CockpitPanel* p_panel)
{
	PANE* rect;
	RectTransition* transition;
	PANE savedView;
	PANE savedTarget;

	if (!p_panel->m_enabled || !g_targetPanelMode) {
		return;
	}

	transition = p_panel->m_transition;
	if (transition) {
		if (p_panel->m_lastPowerState != 0 && p_panel->m_lastPowerState != 3 && p_panel->m_lastPowerState != 4) {
			StartRectTransition(transition);
		}

		rect = UpdateRectTransitionByAxis(1, transition);
		if (rect) {
			savedView = g_panes[7];
			savedTarget = *p_panel->m_target;
			g_panes[7] = *rect;
			*p_panel->m_target = *rect;
			DrawTargetPanel(p_panel);
			g_panes[7] = savedView;
			*p_panel->m_target = savedTarget;
		}
	}

	p_panel->m_lastPowerState = g_cockpitPowerState;
}
