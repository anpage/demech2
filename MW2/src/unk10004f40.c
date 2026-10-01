#include "unk10004f40.h"

#include "approxlen.h"
#include "clock.h"
#include "cobaltharbor.h"
#include "config.h"
#include "decomp.h"
#include "eyepoint.h"
#include "gamekeys.h"
#include "loadres.h"
#include "mech.h"
#include "menu.h"
#include "network.h"
#include "objective.h"
#include "players.h"
#include "point.h"
#include "rendertarget.h"
#include "resourcefile.h"
#include "screenscale.h"
#include "setres.h"
#include "simmain.h"
#include "starmission.h"
#include "team.h"
#include "types.h"
#include "unk100079d0.h"
#include "unk10016ad0.h"
#include "unk1004d020.h"

#include <stdio.h>
#include <string.h>

// Who the chat message goes to: 0 nobody (no message is being typed), -1 everybody, or a
// player.
// GLOBAL: MW2 0x100a116c
MechS32 g_unk0x100a116c = 0;

// Whether the objectives panel shows.
// GLOBAL: MW2 0x100a1170
MechS32 g_unk0x100a1170 = 0;

// GLOBAL: MW2 0x100bcd88
static MechChar g_unk0x100bcd88[16];

// GLOBAL: MW2 0x100bcd98
static MechChar g_unk0x100bcd98[16];

// The chat message being typed. HandleChatKey edits it.
// GLOBAL: MW2 0x10179e90
MechChar g_unk0x10179e90[0x30];

// Formats a tick count (181 per second) as hours:minutes:seconds.
// FUNCTION: MW2 0x10004f40
MechChar* FUN_10004f40(MechS32 p_ticks)
{
	MechDouble seconds;
	MechS32 hours;
	MechS32 minutes;

	hours = p_ticks / (181 * 3600);
	minutes = (p_ticks - hours * (181 * 3600)) / (181 * 60);
	seconds = (p_ticks - hours * (181 * 3600) - minutes * (181 * 60)) / 181.0;
	sprintf(g_unk0x100bcd88, "%2.2d:%2.2d:%05.2f", hours, minutes, seconds);
	return g_unk0x100bcd88;
}

// Formats a count of seconds as hours:minutes:seconds.
// FUNCTION: MW2 0x10004ff5
MechChar* FUN_10004ff5(MechS32 p_seconds)
{
	MechS32 seconds;
	MechS32 hours;
	MechS32 minutes;

	hours = p_seconds / 3600;
	minutes = (p_seconds - hours * 3600) / 60;
	seconds = p_seconds - hours * 3600 - minutes * 60;
	sprintf(g_unk0x100bcd98, "%2.2d:%2.2d:%2.2d", hours, minutes, seconds);
	return g_unk0x100bcd98;
}

// Matches except for the stack slots of width and c (a consistent permutation).
// Returns the width of p_text in p_font.
// FUNCTION: MW2 0x1000507d
MechS32 FUN_1000507d(const MechChar* p_text, void* p_font)
{
	MechS32 width;
	const MechChar* c;

	width = 0;
	for (c = p_text; *c; c++) {
		width += FontGetCharWidth(p_font, *c);
	}

	return width;
}

// Lists the objectives of priority p_priority the local player's star can see, from p_pos, each
// with its state at the right edge of the panel. A name longer than 31 characters takes two
// lines. In a network game whose only listed objective is a secondary one, it counts as primary.
// Stack-slot permutation of the locals (label, objective, twoLines, height, i, target, mission,
// primary, secondary, count, x and y).
// FUNCTION: MW2 0x100050d1
void FUN_100050d1(CobaltHarbor0x88* p_panel, Point* p_pos, void* p_font, MechU8 p_priority)
{
	RenderTarget* target;
	StarMission* mission;
	MechS32 height;
	MechS32 primary;
	MechS32 count;
	MechS32 secondary;
	MechS32 i;
	MissionObjective* objective;
	MechChar* label;
	MechS32 twoLines;
	MechS32 x;
	MechS32 y;
	MechChar text[256];

	target = p_panel->m_target;
	mission = &g_objectiveTable[g_unk0x100a5918];
	height = FontGetHeight(p_font);
	primary = FALSE;
	if (g_isNetworkGame && !g_difficulty->m_unk0x0a) {
		count = 0;
		secondary = FALSE;
		for (i = 0; i < mission->m_objectiveCount; i++) {
			objective = &g_objectiveTable[g_unk0x100a5918].m_objectives[i];
			count += objective->m_unk0x74;
			if (objective->m_unk0x74 && objective->m_priority == 2) {
				secondary = TRUE;
			}
		}

		if (secondary && count == 1) {
			primary = TRUE;
		}
	}

	for (i = 0; i < mission->m_objectiveCount; i++) {
		objective = &g_objectiveTable[g_unk0x100a5918].m_objectives[i];
		if (!objective->m_unk0x74 || objective->m_priority != p_priority) {
			continue;
		}

		{
			p_pos->m_x = p_panel->m_unk0x34->m_x;
			twoLines = FALSE;
			switch (p_priority) {
			case 1:
				label = "Primary: ";
				break;
			case 2:
				if (primary) {
					label = "Primary: ";
				}
				else {
					label = "Secondary: ";
				}
				break;
			case 4:
				label = "Tertiary: ";
				break;
			case 8:
				label = "Return: ";
				break;
			default:
				label = "Tertiary: ";
				break;
			}

			g_unk0x100e9350[0xe] = 6;
			BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, label, g_unk0x100e9350);
			g_unk0x100e9350[0xe] = 0xe;
			p_pos->m_x += FUN_1000507d("Secondary: ", p_font);
			if (strlen(objective->m_name) < 0x20) {
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, objective->m_name, g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
			}
			else {
				x = p_pos->m_x;
				y = p_pos->m_y;
				strncpy(text, objective->m_name, 0x20);
				text[0x20] = '\0';
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, text, g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
				p_pos->m_x = x;
				p_pos->m_y += height;
				strcpy(text, objective->m_name + 0x20);
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, text, g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
				p_pos->m_x = x;
				p_pos->m_y = y;
				twoLines = TRUE;
			}

			switch (objective->m_state) {
			case 5:
				sprintf(text, "Successful");
				p_pos->m_x = target->m_right - target->m_left - FUN_1000507d(text, p_font);
				g_unk0x100e9350[0xe] = 7;
				BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, text, g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
				break;
			case 6:
				sprintf(text, "Failed");
				p_pos->m_x = target->m_right - target->m_left - FUN_1000507d(text, p_font);
				g_unk0x100e9350[0xe] = 0xb;
				BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, text, g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
				break;
			default:
				sprintf(text, "In progress");
				p_pos->m_x = target->m_right - target->m_left - FUN_1000507d(text, p_font);
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(p_panel->m_target, p_pos->m_x, p_pos->m_y, p_font, text, g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
				break;
			}

			p_pos->m_y += height;
			if (twoLines) {
				p_pos->m_y += height;
			}
		}
	}
}

// Draws the objectives panel: the objectives by priority, then the mission clock or how the
// mission ended.
// Stack-slot permutation of the locals (cursor, mission, font, height, gap, ticks and text).
// FUNCTION: MW2 0x100056f0
void FUN_100056f0(CobaltHarbor0x88* p_panel)
{
	StarMission* mission;
	void* font;
	Point pos;
	Point* cursor = &pos;
	MechS32 ticks;
	MechS32 height;
	MechChar text[256];
	MechS32 gap;

	if (!p_panel->m_enabled || !g_unk0x100a1170) {
		return;
	}

	mission = &g_objectiveTable[g_unk0x100a5918];
	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	height = FontGetHeight(font);
	gap = height / 2;
	pos = *p_panel->m_unk0x34;
	g_unk0x100e9350[0xe] = 6;
	BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, "MISSION OBJECTIVES", g_unk0x100e9350);
	g_unk0x100e9350[0xe] = 0xe;
	FUN_100571ea(p_panel->m_target, "MISSION OBJECTIVES", pos, font, 6);
	cursor->m_y += gap + height;
	FUN_100050d1(p_panel, cursor, font, 1);
	cursor->m_y += gap;
	FUN_100050d1(p_panel, cursor, font, 2);
	cursor->m_y += gap;
	FUN_100050d1(p_panel, cursor, font, 4);
	FUN_100050d1(p_panel, cursor, font, 0);
	cursor->m_y += gap;
	FUN_100050d1(p_panel, cursor, font, 8);
	cursor->m_x = p_panel->m_unk0x34->m_x;
	cursor->m_y += gap + height;
	switch (mission->m_status) {
	case 0:
		if (mission->m_timeLimit > 0) {
			ticks = (mission->m_startTime + mission->m_timeLimit) * 181 - g_currentClock;
			sprintf(text, "Time Remaining: %s", FUN_10004f40(ticks));
		}
		else {
			ticks = g_currentClock - mission->m_startTime * 181;
			sprintf(text, "Elapsed Time: %s", FUN_10004f40(ticks));
		}
		break;
	case 2:
		sprintf(text, "Successful at %s", FUN_10004ff5(mission->m_endTime));
		break;
	case 4:
		sprintf(text, "Out of time at %s", FUN_10004ff5(mission->m_endTime));
		break;
	case 3:
		sprintf(text, "Failed at %s", FUN_10004ff5(mission->m_endTime));
		break;
	}

	g_unk0x100e9350[0xe] = 6;
	BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, text, g_unk0x100e9350);
	g_unk0x100e9350[0xe] = 0xe;
	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
}

// Draws the network status panel: whom the camera tracks, or how to regenerate, or that the
// game waits for the other players; then the chat message being typed, with its recipients and
// a cursor.
// Stack-slot permutation of the locals (state, cursor, pos, color, height, player, gap, text and font).
// FUNCTION: MW2 0x10005add
void FUN_10005add(CobaltHarbor0x88* p_panel)
{
	MechChar* state;
	Point pos;
	Point* cursor = &pos;
	MechS32 color;
	MechS32 height;
	MechS32 player;
	MechS32 gap;
	MechChar text[40];
	void* font;

	color = 0xe;
	state = "";
	if (!p_panel->m_enabled) {
		return;
	}

	if (!g_unk0x100aa2c0 && !g_unk0x100a116c) {
		return;
	}

	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	height = FontGetHeight(font);
	gap = height / 2;
	pos = *p_panel->m_unk0x34;
	if (g_unk0x100aa2c0 == 1) {
		if (GetPlayerSide(g_unk0x100a2430)) {
			color = 0xb;
		}

		if (g_players[g_unk0x100a2430]->m_flags & 6) {
			state = "(dead)";
		}

		sprintf(text, "Tracking: %s %s", g_players[g_unk0x100a2430]->m_name, state);
		g_unk0x100e9350[0xe] = color;
		BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, text, g_unk0x100e9350);
		g_unk0x100e9350[0xe] = 0xe;
	}
	else if (g_unk0x100aa2c0 == 2) {
		g_unk0x100e9350[0xe] = 6;
		BlitString(
			p_panel->m_target,
			cursor->m_x,
			cursor->m_y,
			font,
			"Press SPACEBAR to regenerate, or CTRL-Q to exit.",
			g_unk0x100e9350
		);
		g_unk0x100e9350[0xe] = 0xe;
	}
	else if (g_unk0x100aa2c0 == 3) {
		g_unk0x100e9350[0xe] = 0xe;
		BlitString(
			p_panel->m_target,
			cursor->m_x,
			cursor->m_y,
			font,
			"         Waiting for remote players...",
			g_unk0x100e9350
		);
		g_unk0x100e9350[0xe] = 0xe;
	}

	cursor->m_y += gap + height;
	if (g_unk0x100a116c) {
		switch (g_unk0x100a116c) {
		case -1:
			g_unk0x100e9350[0xe] = 0xe;
			BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, "Communication", g_unk0x100e9350);
			g_unk0x100e9350[0xe] = 0xe;
			FUN_100571ea(p_panel->m_target, "Communication", pos, font, 2);
			cursor->m_y += gap + height;
			if (g_difficulty->m_unk0x0a) {
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, " [Enter] Send to all", g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
				cursor->m_y += height;
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(
					p_panel->m_target,
					cursor->m_x,
					cursor->m_y,
					font,
					" CTRL-F  Send to friendly mechs",
					g_unk0x100e9350
				);
				g_unk0x100e9350[0xe] = 0xe;
				cursor->m_y += height;
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(
					p_panel->m_target,
					cursor->m_x,
					cursor->m_y,
					font,
					" CTRL-E  Send to enemy mechs",
					g_unk0x100e9350
				);
				g_unk0x100e9350[0xe] = 0xe;
				cursor->m_y += height;
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, " [Esc] to abort", g_unk0x100e9350);
				g_unk0x100e9350[0xe] = 0xe;
			}
			else {
				g_unk0x100e9350[0xe] = 0xe;
				BlitString(
					p_panel->m_target,
					cursor->m_x,
					cursor->m_y,
					font,
					" [Enter] Send to all,    [Esc] to abort",
					g_unk0x100e9350
				);
				g_unk0x100e9350[0xe] = 0xe;
			}

			cursor->m_y += gap + height;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			if (g_unk0x100a116c == g_localPlayerId) {
				player = 0;
			}
			else {
				player = g_unk0x100a116c;
			}

			if (!g_players[player] || g_players[player]->m_unk0x00 != 1) {
				return;
			}

			if (g_players[player]->m_flags & 0x4800) {
				return;
			}

			sprintf(text, "Communication to %s", g_players[player]->m_name);
			g_unk0x100e9350[0xe] = 0xe;
			BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, text, g_unk0x100e9350);
			g_unk0x100e9350[0xe] = 0xe;
			FUN_100571ea(p_panel->m_target, text, pos, font, 2);
			cursor->m_y += gap + height;
			g_unk0x100e9350[0xe] = 0xe;
			BlitString(
				p_panel->m_target,
				cursor->m_x,
				cursor->m_y,
				font,
				" [Enter] to send,    [ESC] to abort",
				g_unk0x100e9350
			);
			g_unk0x100e9350[0xe] = 0xe;
			cursor->m_y += gap + height;
			break;
		}

		g_unk0x100e9350[0xe] = 0xe;
		BlitString(p_panel->m_target, cursor->m_x, cursor->m_y, font, g_unk0x10179e90, g_unk0x100e9350);
		g_unk0x100e9350[0xe] = 0xe;
		FUN_10057282(p_panel->m_target, "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM", pos, font, 6);
		cursor->m_x = FUN_1000507d(g_unk0x10179e90, font);
		FUN_100571ea(p_panel->m_target, " ", pos, font, 0xe);
	}

	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
}

// Shows the local player's kill count in a network game.
// FUNCTION: MW2 0x100060b6
void FUN_100060b6(CobaltHarbor0x88* p_panel)
{
	void* font;
	MechChar text[40];

	if (!p_panel->m_enabled) {
		return;
	}

	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	if (g_isNetworkGame && !g_unk0x100aa2c0 && !g_unk0x100a2c04) {
		sprintf(text, "Kills: %i", g_unk0x100a15a0);
		BlitString(p_panel->m_target, p_panel->m_unk0x34->m_x, p_panel->m_unk0x34->m_y, font, text, g_unk0x100e9350);
	}

	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
}

// Shows the autopilot's state.
// Stack-slot permutation of text and font.
// FUNCTION: MW2 0x10006189
void FUN_10006189(CobaltHarbor0x88* p_panel)
{
	MechChar* text;
	Mech* mech;
	void* font;

	if (!p_panel->m_enabled) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	switch (mech->m_unk0xbc) {
	case 1:
		text = "AUTOPILOT";
		break;
	case 2:
		text = "AUTOPILOT";
		break;
	default:
		text = "";
		return;
	}

	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	if (*text) {
		BlitString(p_panel->m_target, p_panel->m_unk0x34->m_x, p_panel->m_unk0x34->m_y, font, text, g_unk0x100e9350);
	}

	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
}

// Stack-slot permutation of mech and font.
// Shows the local mech's speed in kph, in a different color while it backs up, and its throttle bar.
// FUNCTION: MW2 0x10006291
void FUN_10006291(CobaltHarbor0x88* p_panel)
{
	MechS32 speed;
	Mech* mech;
	MechChar text[64];
	void* font;

	if (!p_panel->m_enabled) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	speed = ApproximateVectorLength(mech->m_unk0xf4, mech->m_unk0xf8, mech->m_unk0xfc) / 10002 * 1.5;
	if (mech->m_unk0x24.m_value < 0) {
		speed = -speed;
		g_unk0x100e9350[0xe] = 6;
	}
	else {
		g_unk0x100e9350[0xe] = 0xe;
	}

	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	sprintf(text, "%d kph", speed);
	BlitString(p_panel->m_target, p_panel->m_unk0x34->m_x, p_panel->m_unk0x34->m_y, font, text, g_unk0x100e9350);
	g_unk0x100e9350[0xe] = 0xe;
	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
	FUN_1004d48a(p_panel->m_target);
}

// Labels the MASC panel while MASC is available.
// FUNCTION: MW2 0x100063cd
void FUN_100063cd(CobaltHarbor0x88* p_panel)
{
	void* font;

	if (!p_panel->m_enabled || !g_unk0x100a2bf0) {
		return;
	}

	p_panel->m_setName(p_panel, "MASC");
	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	BlitString(
		p_panel->m_target,
		p_panel->m_unk0x34->m_x,
		p_panel->m_unk0x34->m_y,
		font,
		p_panel->m_name,
		g_unk0x100e9350
	);
	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
}

// Stack-slot permutation of mech and font.
// Labels the heat panel with the shutdown state and draws the heat bar.
// FUNCTION: MW2 0x10006484
void FUN_10006484(CobaltHarbor0x88* p_panel)
{
	Mech* mech;
	void* font;

	mech = g_players[g_localPlayerId]->m_mech;
	if (!p_panel->m_enabled) {
		return;
	}

	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	if (mech->m_unk0x10c & 4) {
		if (!(mech->m_unk0x10c & 8)) {
			p_panel->m_setName(p_panel, "Shutdown...");
			g_unk0x100e9350[0xe] = 0xb;
		}
		else {
			p_panel->m_setName(p_panel, "Overridden");
			g_unk0x100e9350[0xe] = 0xb;
		}
	}
	else {
		p_panel->m_setName(p_panel, "Heat");
		g_unk0x100e9350[0xe] = 0xe;
	}

	BlitString(
		p_panel->m_target,
		p_panel->m_unk0x34->m_x,
		p_panel->m_unk0x34->m_y,
		font,
		p_panel->m_name,
		g_unk0x100e9350
	);
	g_unk0x100e9350[0xe] = 0xe;
	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
	FUN_1004d175(p_panel->m_target);
}

// Labels the heat rate panel and draws its bar.
// FUNCTION: MW2 0x100065c3
void FUN_100065c3(CobaltHarbor0x88* p_panel)
{
	void* font;

	if (!p_panel->m_enabled) {
		return;
	}

	p_panel->m_setName(p_panel, "dH/dT");
	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	BlitString(
		p_panel->m_target,
		p_panel->m_unk0x34->m_x,
		p_panel->m_unk0x34->m_y,
		font,
		p_panel->m_name,
		g_unk0x100e9350
	);
	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
	FUN_1004d310(p_panel->m_target);
}

// Labels the jump jet panel of a mech with jump jets and draws the fuel bar.
// FUNCTION: MW2 0x1000667c
void FUN_1000667c(CobaltHarbor0x88* p_panel)
{
	void* font;

	if (!p_panel->m_enabled) {
		return;
	}

	if (g_players[g_localPlayerId]->m_mech->m_unk0xc0 < 0) {
		return;
	}

	p_panel->m_setName(p_panel, "Jets");
	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	BlitString(
		p_panel->m_target,
		p_panel->m_unk0x34->m_x,
		p_panel->m_unk0x34->m_y,
		font,
		p_panel->m_name,
		g_unk0x100e9350
	);
	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
	FUN_1004d660(p_panel->m_target);
}
