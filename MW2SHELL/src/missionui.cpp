#include "audiosample.h"
#include "brasslantern0x414.h"
#include "cedarknot0x10.h"
#include "decomp.h"
#include "emberglyph0x3e.h"
#include "granitemast0x18.h"
#include "hazelstar0x80.h"
#include "linenpacket0x218.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "ravenmark0x08.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <string.h>
#include <windows.h>

// The mission briefing screen: the mission's stars and their mechs, read from the mission's
// BWD file, and its briefing text and videos.

// SIZE 0x08
// A mech's name on the briefing screen: its glyph and the mech type it shows.
struct WillowTag0x08 {
	EmberGlyph0x3e* m_glyph; // 0x00
	MechS32 m_type;          // 0x04 — an index into g_unk0x10061560, negative for none
};

// SIZE 0x20
// A star in a mission's BWD file (node type 0x46). Its mechs' variant names follow, one per
// mech of the star's size.
struct FernStar0x20 {
	MechS32 m_unk0x00;           // 0x00
	MechS32 m_unk0x04;           // 0x04 — tonnage
	MechS32 m_unk0x08;           // 0x08 — mechs
	MechS32 m_unk0x0c;           // 0x0c — size
	MechChar m_unk0x10[1][0x10]; // 0x10 — variant names
};

DECOMP_SIZE_ASSERT(WillowTag0x08, 0x08)
DECOMP_SIZE_ASSERT(RavenMark0x08, 0x08)

extern AudioSubsystem* g_pAudioSubsystem;
extern VideoDriver* g_pVideoDriver;
extern BrassLantern0x414* g_unk0x1007120c;
extern BrassLantern0x414* g_unk0x10071210;
extern CedarKnot0x10* g_unk0x10071230;
extern MechS32 g_unk0x10066a44;
extern GraniteMast0x18 g_unk0x10061560[];
extern RavenMark0x08 g_unk0x1006e1a8[6];
extern MainMenuButton g_unk0x1006f618[0x19];
extern LinenPacket0x218 g_unk0x10090288;
extern MechChar g_unk0x1006a550[];

void FUN_10003175(MechS32 p_star, MechS32 p_formation, MechS32 p_size, MechS32 p_count, MechS32 p_tonnage);
MechS32 FUN_10002de7(MechS32 p_index, MechChar* p_variant, MechChar* p_name);
MechS32 FUN_1000307c(MechS32 p_index);
MechS32 FUN_100030e5(MechS32 p_star);
HazelStar0x80* FUN_1000312e(MechS32 p_star);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void FUN_1001661b();
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechU32 p_unk0x10,
	MechU32 p_unk0x14
);
void FUN_10038744(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32);

// The briefing videos of the missions, each with its looping continuation.
// GLOBAL: MW2SHELL 0x1006a1c0
MechChar* g_unk0x1006a1c0[12][2] = {
	{"aplan01", "aplan01c"},
	{"aplan02", "aplan02c"},
	{"aplan03", "aplan03c"},
	{"aplan04", "aplan04c"},
	{"aplan05", "aplan05c"},
	{"aplan06", "aplan06c"},
	{"aplan07", "aplan07c"},
	{"aplan08", "aplan08c"},
	{"aplan09", "aplan09c"},
	{"aplan10", "aplan10c"},
	{"aplan11", "aplan11c"},
	{"aplan12", "aplan12c"},
};

// GLOBAL: MW2SHELL 0x1006a220
MechChar* g_unk0x1006a220[12] = {
	"jackscn1",
	"chedscn1",
	"edamscn1",
	"provscn1",
	"goudscn1",
	"colbscn1",
	"goatscn1",
	"whizscn1",
	"ricoscn1",
	"swisscn1",
};

// GLOBAL: MW2SHELL 0x1006a250
MechChar* g_unk0x1006a250[6] = {"wiawolf", "wiajf", "wiaghost", "wiasmoke", "wianova", "wiasteel"};

// The names of the mechs of the player's star...
// GLOBAL: MW2SHELL 0x1006a268
WillowTag0x08 g_unk0x1006a268[3] = {0};

// ...and of the enemy's.
// GLOBAL: MW2SHELL 0x1006a280
WillowTag0x08 g_unk0x1006a280[3] = {0};

// GLOBAL: MW2SHELL 0x1006a298
MechS32 g_unk0x1006a298 = 0xf;

// The briefing video, an index into g_unk0x1006a1c0.
// GLOBAL: MW2SHELL 0x1006a29c
MechS32 g_unk0x1006a29c = 0;

// The formation names of the enemy's star...
// GLOBAL: MW2SHELL 0x1006a2a0
EmberGlyph0x3e* g_unk0x1006a2a0 = NULL;

// ...and of the player's.
// GLOBAL: MW2SHELL 0x1006a2a4
EmberGlyph0x3e* g_unk0x1006a2a4 = NULL;

// GLOBAL: MW2SHELL 0x1006a2a8
MenuList0x10d* g_unk0x1006a2a8 = NULL;

// GLOBAL: MW2SHELL 0x1006a2b0
AudioSample* g_unk0x1006a2b0 = NULL;

// Only for a trial (WM_USER + 0xe).
// GLOBAL: MW2SHELL 0x1006a2b4
AudioSample* g_unk0x1006a2b4 = NULL;

// The text colors: each color maps to itself, but 0 is transparent and 1 is drawn in 0x22.
// GLOBAL: MW2SHELL 0x10090058
MechU8 g_unk0x10090058[0x100];

// GLOBAL: MW2SHELL 0x10090158
MechS32 g_unk0x10090158;

// The three lines of briefing text.
// GLOBAL: MW2SHELL 0x10090160
EmberGlyph0x3e* g_unk0x10090160[3];

// GLOBAL: MW2SHELL 0x1009016c
MechS32 g_unk0x1009016c;

// GLOBAL: MW2SHELL 0x10090170
MechS32 g_unk0x10090170;

// GLOBAL: MW2SHELL 0x10090174
WPARAM g_unk0x10090174;

// GLOBAL: MW2SHELL 0x10090178
MechS32 g_unk0x10090178;

// GLOBAL: MW2SHELL 0x10090180
MechChar g_unk0x10090180[0x100];

// The mission, an index into g_unk0x1006a220.
// GLOBAL: MW2SHELL 0x10090280
MechS32 g_unk0x10090280;

// Sets up a star from a BWD star node, and registers its mechs' variants.
// FUNCTION: MW2SHELL 0x10037c60
void FUN_10037c60(MechS32 p_star, FernStar0x20* p_data)
{
	MechS32 i;

	FUN_10003175(p_star, 0, p_data->m_unk0x0c, p_data->m_unk0x08, p_data->m_unk0x04);
	for (i = 0; i < p_data->m_unk0x0c; i++) {
		FUN_10002de7(i, p_data->m_unk0x10[i], NULL);
	}
	FUN_10003175(p_star, 0, p_data->m_unk0x0c, p_data->m_unk0x08, p_data->m_unk0x04);
}

// Reads the mission's BWD file (the scenario's first four letters + "brf2"): its briefing
// name, optionally its two stars (p_stars) and its briefing video and text (p_video).
// Not 100%: the stack slots of name, file, title, star and the loop locals are permuted.
// FUNCTION: MW2SHELL 0x10037cf7
void ShellApplyMissionUiInfo(MechChar* p_scenario, MechS32 p_stars, MechS32 p_video)
{
	MechS32* briefing;
	MechChar name[16];
	MechS32* star;
	MechS32* file;
	MechS32 i;
	MechS32 pos;
	MechS32 j;
	MechS32* title;

	strcpy(name, p_scenario);
	name[4] = '\0';
	strcat(name, "brf2");

	file = (MechS32*) g_unk0x10071230->FUN_1002e3cf(name, 0xe, "BWD");
	if (!file) {
		return;
	}

	title = g_unk0x10071230->FUN_1002e47a(file, 0x47, NULL);
	if (title) {
		strcpy(g_unk0x1006a550, (MechChar*) (title + 2));
	}

	if (p_stars) {
		star = g_unk0x10071230->FUN_1002e47a(file, 0x46, NULL);
		if (star) {
			FUN_10037c60(0, (FernStar0x20*) (star + 2));
		}

		star = g_unk0x10071230->FUN_1002e47a(file, 0x46, star);
		if (star) {
			FUN_10037c60(1, (FernStar0x20*) (star + 2));
			g_unk0x10066a44 = star[2];
		}
	}

	FUN_10003175(0, -1, -1, -1, -1);
	FUN_10002de7(0, NULL, NULL);

	if (p_video) {
		briefing = g_unk0x10071230->FUN_1002e47a(file, 0x45, NULL);
		if (briefing) {
			g_unk0x1006a29c = briefing[2] - 1;
			if (g_unk0x1006a29c < 0 || g_unk0x1006a29c >= 12) {
				g_unk0x1006a29c = 0;
			}

			FUN_10017460(0, g_unk0x1006a1c0[g_unk0x1006a29c][0], 0x19e, 10, 0x42, 0);

			pos = 0;
			for (i = 0; i < 3; i++, pos++) {
				for (j = 0; pos < briefing[1] - 0xc && ((MechChar*) briefing)[pos + 0xc] >= ' '; pos++, j++) {
					g_unk0x10090180[j] = ((MechChar*) briefing)[pos + 0xc];
				}
				g_unk0x10090180[j] = '\0';

				if (g_unk0x10090160[i]) {
					delete g_unk0x10090160[i];
				}
				g_unk0x10090160[i] =
					g_unk0x10071210->FUN_1000544e(0xef, i * 12 + 0x45, g_unk0x10090180, g_unk0x10090058);
			}
		}
	}

	g_unk0x10071230->FUN_1002e445(name, 0xe, "BWD");
}

// Shows the name of mech type p_type at (p_left, p_top), replacing the tag's previous glyph.
// Not 100%: the stack slot of text is permuted with the delete temporaries.
// FUNCTION: MW2SHELL 0x10037feb
void FUN_10037feb(WillowTag0x08* p_tag, MechS32 p_type, MechS32 p_left, MechS32 p_top)
{
	MechChar* text;

	p_tag->m_type = p_type;
	if (p_tag->m_glyph) {
		delete p_tag->m_glyph;
	}

	if (p_type >= 0) {
		text = g_unk0x10061560[p_tag->m_type].m_unk0x0c;
	}
	else {
		text = "[none]";
	}

	p_tag->m_glyph = g_unk0x10071210->FUN_1000544e(p_left, p_top, text, g_unk0x10090058);
}

// Shows the formation names of both stars.
// FUNCTION: MW2SHELL 0x10038093
void FUN_10038093()
{
	POINT* pos;

	g_unk0x1009016c = FUN_100030e5(0);
	if (g_unk0x1006a2a4) {
		delete g_unk0x1006a2a4;
	}
	pos = &g_unk0x1006f618[6].m_textPos;
	g_unk0x1006a2a4 =
		g_unk0x10071210->FUN_1000544e(pos->x, pos->y, g_unk0x1006e1a8[g_unk0x1009016c].m_unk0x04, g_unk0x10090058);

	g_unk0x10090178 = FUN_100030e5(1);
	if (g_unk0x1006a2a0) {
		delete g_unk0x1006a2a0;
	}
	pos = &g_unk0x1006f618[17].m_textPos;
	g_unk0x1006a2a0 =
		g_unk0x10071210->FUN_1000544e(pos->x, pos->y, g_unk0x1006e1a8[g_unk0x10090178].m_unk0x04, g_unk0x10090058);
}

// Picks a value by the pilot of the player's first mech: 0x12 for Enzo, 0x11 for Hobbes, 0x10
// for Calvin and 0xf for anyone else.
// FUNCTION: MW2SHELL 0x100381c2
MechS32 FUN_100381c2()
{
	HazelStar0x80* star;

	star = FUN_1000312e(0);
	if (!strcmp(star->m_unk0x14[0].m_unk0x14, "Enzo")) {
		return 0x12;
	}
	if (!strcmp(star->m_unk0x14[0].m_unk0x14, "Hobbes")) {
		return 0x11;
	}
	if (!strcmp(star->m_unk0x14[0].m_unk0x14, "Calvin")) {
		return 0x10;
	}

	return 0xf;
}

// Sets up the mission briefing screen. p_wParam is WM_USER + 0xe for a trial.
// Not 100%: the stack slots of pos, audioData, i and audioSize are permuted.
// FUNCTION: MW2SHELL 0x100382e6
void FUN_100382e6(TMPackDataBase* p_database, MechChar** p_scenario, WPARAM p_wParam)
{
	POINT* pos;
	void* audioData = NULL;
	MechS32 i;
	MechS32 audioSize;

	g_unk0x10090174 = p_wParam;
	g_unk0x1006a298 = FUN_100381c2();

	for (i = 1; i < 0x100; i++) {
		g_unk0x10090058[i] = i;
	}
	g_unk0x10090058[0] = 0xff;
	g_unk0x10090058[1] = 0x22;

	*p_scenario = "pinkscn1";
	p_database->GetDBItem(78, &audioData, &audioSize);
	g_unk0x1006a2b0 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	if (p_wParam == 0x40e) {
		p_database->GetDBItem(82, &audioData, &audioSize);
		g_unk0x1006a2b4 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	}

	g_unk0x1006a2a8 = new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_unk0x1006f618, 0x19);
	for (i = 0; i < 3; i++) {
		g_unk0x10090160[i] = NULL;
	}

	if (p_wParam == 0x40e) {
		g_unk0x10090288.m_unk0x110 = 0;
	}
	g_unk0x10090280 = g_unk0x10090288.m_unk0x110;
	g_unk0x1006a29c = 0;
	*p_scenario = g_unk0x1006a220[g_unk0x10090280];
	ShellApplyMissionUiInfo(g_unk0x1006a220[g_unk0x10090280], p_wParam == 0x40e, 1);

	g_unk0x10090170 = 0;
	FUN_10017460(1, g_unk0x1006a250[g_unk0x10090170], 0xd, 0xcd, 6, 0);
	g_unk0x10090158 = 1;
	FUN_10017460(2, g_unk0x1006a250[g_unk0x10090158], 0x1e3, 0x149, 6, 0);

	for (i = 0; i < 3; i++) {
		g_unk0x1006a268[i].m_glyph = NULL;
		pos = &g_unk0x1006f618[i + 3].m_textPos;
		FUN_10003175(0, -1, -1, -1, -1);
		FUN_10037feb(&g_unk0x1006a268[i], FUN_1000307c(i), pos->x, pos->y);

		g_unk0x1006a280[i].m_glyph = NULL;
		pos = &g_unk0x1006f618[i + 0xe].m_textPos;
		FUN_10003175(1, -1, -1, -1, -1);
		FUN_10037feb(&g_unk0x1006a280[i], FUN_1000307c(i), pos->x, pos->y);
	}
	FUN_10003175(0, -1, -1, -1, -1);

	g_unk0x1006a2a0 = NULL;
	g_unk0x1006a2a4 = NULL;
	FUN_10038093();

	if (g_unk0x1006a2b4) {
		g_unk0x1006a2b4->SetVolume(0x1e);
		g_unk0x1006a2b4->Start();
	}

	FUN_10017460(0x10, "wialanch", 0xd1, 0x173, 0x24, 0);
	FUN_100108e5(FUN_10038744);
	g_pVideoDriver->FUN_10006c50(p_database, 9);
	FUN_1001661b();
	g_pVideoDriver->DrawShell();
}

// STUB: MW2SHELL 0x10038744
void FUN_10038744(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32)
{
	STUB(0x10038744);
}
