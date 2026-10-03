#include "mechviewpanel.h"

#include "cobaltharbor.h"
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
#include "rendertarget.h"
#include "screenscale.h"
#include "shots.h"
#include "simmain.h"
#include "types.h"

// The handlers of the cockpit panel FUN_1006fca5 sets up second (g_unk0x100c3280[2]): it
// cycles through five views of the local mech (FUN_100509a0), drawn into the panel's render
// target, optionally through the panel's rectangle transition.

// GLOBAL: MW2 0x100a88e8
MechS32 g_unk0x100a88e8 = 0;

// FUNCTION: MW2 0x100509a0
void FUN_100509a0(void)
{
	g_unk0x100aa2a4++;
	if (g_unk0x100aa2a4 == 6) {
		g_unk0x100aa2a4 = 1;
	}
}

// Stack-slot permutation: camera, mech and saved and view. The original's longer displacements
// make its code longer, so reccmp compares only the recompiled length of it.
// FUNCTION: MW2 0x100509c8
void FUN_100509c8(CobaltHarbor0x88* p_panel)
{
	MechS32* camera;
	MechS32 view[7];
	SlateHeron0x68 saved;
	Mech* mech;

	camera = NULL;
	if (!p_panel->m_enabled || !g_unk0x100aa2a4) {
		return;
	}

	p_panel->m_unk0x3c = g_unk0x100c3358;
	if (p_panel->m_unk0x06 == 1 && g_unk0x100aa2a4 != 1 && g_unk0x100aa2a4 != 2) {
		if (g_unk0x100a88e8) {
			if (RandomIntBelow(10) < 7) {
				g_unk0x100a88e8 = 0;
			}

			FUN_10050e20(p_panel);
			return;
		}

		if (RandomIntBelow(10) < 3) {
			g_unk0x100a88e8 = 1;
		}
	}
	else if (p_panel->m_unk0x06 > 2 && g_unk0x100aa2a4 != 1 && g_unk0x100aa2a4 != 2) {
		FUN_10050e20(p_panel);
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	switch (g_unk0x100aa2a4) {
	case 0:
		break;
	case 5:
		camera = FUN_1006beb5();
		if (!camera) {
			FUN_1006bf05();
			camera = FUN_1006beb5();
		}

		if (!camera) {
			VFX_pane_wipe(p_panel->m_target, 0);
		}
		else {
			FUN_10050dc3(&saved);
			camera[4] = 0;
			FUN_1004c8bd(5, 0x20000, camera, 0);
			g_unk0x100a6cc8 = saved;
		}

		FUN_10050e6c(p_panel, 6, 0xfd);
		break;
	case 4:
		FUN_100114ea(g_eyepoint, view);
		FUN_10050dc3(&saved);
		view[0] = mech->m_player->m_position.m_x;
		view[1] = mech->m_player->m_position.m_y;
		view[2] = mech->m_player->m_position.m_z;
		view[4] = 0x5a0000;
		view[5] = 0;
		FUN_100018ca(mech->m_player->m_obj);
		FUN_1004c8bd(5, 0x20000, view, 0);
		FUN_10001926(mech->m_player->m_obj);
		FUN_10050e6c(p_panel, 6, 0xf7);
		g_unk0x100a6cc8 = saved;
		break;
	case 3:
		FUN_100114ea(g_eyepoint, view);
		FUN_10050dc3(&saved);
		if (g_unk0x100ea3e4) {
			view[3] = mech->m_player->m_unk0x6c + mech->m_player->m_heading;
		}
		else {
			view[3] = mech->m_player->m_heading + 0xb40000;
		}

		view[4] = 0;
		FUN_100018ca(mech->m_player->m_obj);
		FUN_1004c8bd(5, 0x20000, view, 0);
		FUN_10001926(mech->m_player->m_obj);
		if (g_unk0x100ea3e4) {
			FUN_100570e9(p_panel->m_target, 6);
		}
		else {
			FUN_10050e6c(p_panel, 6, 0xfa);
		}

		g_unk0x100a6cc8 = saved;
		break;
	case 2:
		FUN_100407b6(mech, p_panel->m_target);
		break;
	default:
		FUN_10040511(mech, p_panel->m_target);
		break;
	}
}

// FUNCTION: MW2 0x10050dc3
void FUN_10050dc3(SlateHeron0x68* p_saved)
{
	*p_saved = g_unk0x100a6cc8;
	g_unk0x100a6cc8.m_unk0x18 = 0;
	g_unk0x100a6cc8.m_unk0x08 = 0;
	g_unk0x100a6cc8.m_unk0x0c = 1;
	g_unk0x100a6cc8.m_unk0x04 = 1;
	g_unk0x100a6cc8.m_unk0x50 = 0xb00;
	g_unk0x100a6cc8.m_unk0x4c = 1;
	g_unk0x100a6cc8.m_unk0x10 &= ~4;
}

// FUNCTION: MW2 0x10050e20
void FUN_10050e20(CobaltHarbor0x88* p_panel)
{
	if (!p_panel->m_enabled || !g_unk0x100aa2a4) {
		return;
	}

	p_panel->m_unk0x3c = g_unk0x100c3358;
	FUN_1007079d(p_panel->m_target, 0, 0, 0);
}

// Stack-slot permutation: target and y.
// FUNCTION: MW2 0x10050e6c
void FUN_10050e6c(CobaltHarbor0x88* p_panel, MechS32 p_color, MechS32 p_unk0x08)
{
	PANE* target;
	MechS32 x;
	MechS32 y;

	target = p_panel->m_target;
	FUN_100570e9(target, p_color);
	x = p_panel->m_width >> 1;
	y = 2;
	FUN_10041f06(x, y, p_unk0x08, target);
}

// Stack-slot permutation: frame, savedSlot and savedTarget and transition.
// FUNCTION: MW2 0x10050ebe
void FUN_10050ebe(CobaltHarbor0x88* p_panel)
{
	PANE savedTarget;
	PANE savedSlot;
	RectTransition* transition;
	PANE* frame;

	if (!p_panel->m_enabled || !g_unk0x100aa2a4 || g_unk0x100aa2a4 == 2 || g_unk0x100aa2a4 == 1) {
		return;
	}

	transition = p_panel->m_transition;
	if (transition) {
		if (p_panel->m_unk0x3c != 1) {
			StartRectTransition(transition);
		}

		frame = UpdateRectTransitionByAxis(0, transition);
		if (frame) {
			savedSlot = g_panes[5];
			savedTarget = *p_panel->m_target;
			g_panes[5] = *frame;
			*p_panel->m_target = *frame;
			FUN_100509c8(p_panel);
			g_panes[5] = savedSlot;
			*p_panel->m_target = savedTarget;
		}
		else {
			FUN_100509c8(p_panel);
		}
	}

	p_panel->m_unk0x3c = g_unk0x100c3358;
}

// Stack-slot permutation: frame, savedSlot and savedTarget and transition.
// FUNCTION: MW2 0x10050fd6
void FUN_10050fd6(CobaltHarbor0x88* p_panel)
{
	PANE savedTarget;
	PANE savedSlot;
	RectTransition* transition;
	PANE* frame;

	if (!p_panel->m_enabled || !g_unk0x100aa2a4 || g_unk0x100aa2a4 == 2 || g_unk0x100aa2a4 == 1) {
		return;
	}

	transition = p_panel->m_transition;
	if (transition) {
		if (p_panel->m_unk0x3c != 0 && p_panel->m_unk0x3c != 3 && p_panel->m_unk0x3c != 4) {
			StartRectTransition(transition);
		}

		frame = UpdateRectTransitionByAxis(1, transition);
		if (frame) {
			savedSlot = g_panes[5];
			savedTarget = *p_panel->m_target;
			g_panes[5] = *frame;
			*p_panel->m_target = *frame;
			FUN_100509c8(p_panel);
			g_panes[5] = savedSlot;
			*p_panel->m_target = savedTarget;
		}
	}

	p_panel->m_unk0x3c = g_unk0x100c3358;
}
