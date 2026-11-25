/* The keys typed in a mission: the cheat codes (the last 15 keys typed, and a match against a code
   stored XORed with 0x1a), the chat message being typed and the game keys. */
#include "gamekeys.h"

#include "ai.h"
#include "clock.h"
#include "cockpit.h"
#include "config.h"
#include "damagepanel.h"
#include "decomp.h"
#include "displaybackend.h"
#include "dorcs.h"
#include "environment.h"
#include "eyepoint.h"
#include "hud.h"
#include "inputmap.h"
#include "mechclass.h"
#include "mechdamage.h"
#include "mechreload.h"
#include "mechviewpanel.h"
#include "menu.h"
#include "network.h"
#include "objective.h"
#include "pausebanner.h"
#include "players.h"
#include "polydraw.h"
#include "refreshmode.h"
#include "render.h"
#include "rendersettings.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "statuspanels.h"
#include "targeting.h"
#include "targetpanel.h"
#include "ticks.h"
#include "timedoverlays.h"
#include "types.h"
#include "weapons.h"

#ifdef MW2_TUNING_CHEATS
#include "accel/a3d.h"
#include "logwindow.h"
#include "powervr/a3dclient.h"

#include <sgl.h>
#endif

#ifdef MW2_AMS_KEY
#include "gbl/ams.h"
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// GLOBAL: MW2 0x100aa290
// GLOBAL: MW2GBL 0x1009dae0
// GLOBAL: MW2PREMERCS 0x100b5808
// GLOBAL: MW2MATROX 0x100a7cb8
// GLOBAL: MW23DFX 0x1025050c
// GLOBAL: MW2ATI 0x1018481c
// GLOBAL: MW2S3 0x1006262c
// GLOBAL: MW2POWERVR 0x101af8e4
MechS32 g_missionEndTime = 0;

// GLOBAL: MW2 0x100aa294
// GLOBAL: MW2GBL 0x1009dae4
// GLOBAL: MW2MATROX 0x100a7cbc
// GLOBAL: MW23DFX 0x10250510
// GLOBAL: MW2ATI 0x10184820
// GLOBAL: MW2S3 0x10062630
// GLOBAL: MW2POWERVR 0x101af8e8
MechS32 g_speechFlushTime = 0;

// A game-key toggle (GetSystemSetting's setting 0x40).
// GLOBAL: MW2 0x100aa298
// GLOBAL: MW2GBL 0x1009dae8
// GLOBAL: MW2MATROX 0x100a7cc0
// GLOBAL: MW23DFX 0x10250514
// GLOBAL: MW2ATI 0x10184824
// GLOBAL: MW2S3 0x10062634
// GLOBAL: MW2POWERVR 0x101af8ec
MechS32 g_overrideShutdown = 0;

// Set by game key 0x11.
// GLOBAL: MW2 0x100aa2a0
// GLOBAL: MW2GBL 0x1009daf0
// GLOBAL: MW2MATROX 0x100a7cc8
// GLOBAL: MW23DFX 0x1025051c
// GLOBAL: MW2ATI 0x1018482c
// GLOBAL: MW2S3 0x1006263c
// GLOBAL: MW2POWERVR 0x101af8f4
MechS32 g_feetToTorso = 0;

// GLOBAL: MW2 0x100aa2a4
// GLOBAL: MW2GBL 0x1009daf4
// GLOBAL: MW2MATROX 0x100a7ccc
// GLOBAL: MW23DFX 0x10250520
// GLOBAL: MW2ATI 0x10184830
// GLOBAL: MW2S3 0x10062640
// GLOBAL: MW2POWERVR 0x101af8f8
MechS32 g_mechViewMode = 1;

// GLOBAL: MW2 0x100aa2a8
// GLOBAL: MW2GBL 0x1009daf8
// GLOBAL: MW2PREMERCS 0x100b5820
// GLOBAL: MW2MATROX 0x100a7cd0
// GLOBAL: MW23DFX 0x10250524
// GLOBAL: MW2ATI 0x10184834
// GLOBAL: MW2S3 0x10062644
// GLOBAL: MW2POWERVR 0x101af8fc
MechS32 g_missionEnded = 0;

// GLOBAL: MW2 0x100aa2ac
// GLOBAL: MW2GBL 0x1009dafc
// GLOBAL: MW2PREMERCS 0x100b5824
// GLOBAL: MW2MATROX 0x100a7cd4
// GLOBAL: MW23DFX 0x10250528
// GLOBAL: MW2ATI 0x10184838
// GLOBAL: MW2S3 0x10062648
// GLOBAL: MW2POWERVR 0x101af900
MechS32 g_missionTimerStopped = 0;

// The "meepmeep" cheat: enables the time compression key.
// GLOBAL: MW2 0x100aa2b0
// GLOBAL: MW2GBL 0x1009db00
// GLOBAL: MW2PREMERCS 0x100b5828
// GLOBAL: MW2MATROX 0x100a7cd8
// GLOBAL: MW23DFX 0x1025052c
// GLOBAL: MW2ATI 0x1018483c
// GLOBAL: MW2S3 0x1006264c
// GLOBAL: MW2POWERVR 0x101af904
MechS32 g_timeCompressionCheat = 0;

// The length of the chat message being typed (HandleChatKey).
// GLOBAL: MW2 0x100aa2b8
// GLOBAL: MW2GBL 0x1009db08
// GLOBAL: MW2PREMERCS 0x100b5830
// GLOBAL: MW2MATROX 0x100a7ce0
// GLOBAL: MW23DFX 0x10250534
// GLOBAL: MW2S3 0x10062654
// GLOBAL: MW2POWERVR 0x101af90c
MechS32 g_chatLength = 0;

// GLOBAL: MW2 0x100aa2bc
// GLOBAL: MW2GBL 0x1009db0c
// GLOBAL: MW2PREMERCS 0x100b5834
// GLOBAL: MW2MATROX 0x100a7ce4
// GLOBAL: MW23DFX 0x10250538
// GLOBAL: MW2ATI 0x10184848
// GLOBAL: MW2S3 0x10062658
// GLOBAL: MW2POWERVR 0x101af910
MechS32 g_missionResolved = 0;

#ifdef MW2_CHEAT_TABLE
// Set once the cheat messages are decoded (DecodeCheatMessages).
// GLOBAL: MW2GBL 0x1009db10
// GLOBAL: MW2PREMERCS 0x100b5838
MechS32 g_cheatMessagesDecoded = 0;
#endif

// GLOBAL: MW2 0x100aa2c0
// GLOBAL: MW2GBL 0x1009db14
// GLOBAL: MW2MATROX 0x100a7ce8
// GLOBAL: MW23DFX 0x1025053c
// GLOBAL: MW2ATI 0x1018484c
// GLOBAL: MW2S3 0x1006265c
// GLOBAL: MW2POWERVR 0x101af914
MechS32 g_statusMessage = 0;

#ifdef MW2_CHEAT_TABLE
#define CHEAT_CODE_COUNT 21

// A cheat code and the messages its cheat shows when turned on and off, all stored XORed with 0xa.
typedef struct CheatCode {
	MechChar* m_code; // 0x00
	MechChar* m_on;   // 0x04
	MechChar* m_off;  // 0x08
} CheatCode;

// The cheat codes, in HandleCheatInput's order; the table ends with an empty entry. "dei", "fuck",
// "hangaround", "idkfa", "lairdo" and "shit" are listed but do nothing.
// GLOBAL: MW2GBL 0x1009db18
// GLOBAL: MW2PREMERCS 0x100b5840
CheatCode g_cheatCodes[CHEAT_CODE_COUNT + 1] = {
	// "kent" / "Invulnerbility ON." / "Invulnerbility OFF."
	{"aod~", "Cd|\177fdoxhcfc~s*ED$", "Cd|\177fdoxhcfc~s*ELL$"},
	// "thundros" / "Unlimited Ammo ON." / "Unlimited Ammo OFF."
	{"~b\177dnxey", "_dfcgc~on*Kgge*ED$", "_dfcgc~on*Kgge*ELL$"},
	// "burr" / "Heat tracking ON." / "Heat tracking OFF."
	{"h\177xx", "Bok~*~xkiacdm*ED$", "Bok~*~xkiacdm*ELL$"},
	// "dorcs" / "You asked for it."
	{"nexiy", "Se\177*kyaon*lex*c~$", NULL},
	// "dei" / "F E I F"
	{"noc", "L*O*C*L", NULL},
	// "kaboom"
	{"akheeg", NULL, NULL},
	// "jumbo" / "jumpjets"
	{"`\177ghe", "`\177gz`o~y", NULL},
	// "voyeur" / "Forward view instead of rear ON." / "Forward view instead of rear OFF."
	{"|eso\177x", "Lex}kxn*|co}*cdy~okn*el*xokx*ED$", "Lex}kxn*|co}*cdy~okn*el*xokx*ELL$"},
	// "fuck" / "Freebirth vulgarity will not be tolerated."
	{"l\177ia", "Lxoohcx~b*|\177fmkxc~s*}cff*de~*ho*~efoxk~on$", NULL},
	// "palex"
	{"zkfor", NULL, NULL},
	// "hangaround" / "That's better." / "Now how do i win."
	{"bkdmkxe\177dn", "^bk~-y*ho~~ox$", "De}*be}*ne*c*}cd$"},
	// "putz"
	{"z\177~p", NULL, NULL},
	// "idkfa" / "This ain't, DOOM bub."
	{"cnalk", "^bcy*kcd-~&*NEEG*h\177h$", NULL},
	// "lairdo" / "ATTENTION ENEMIES: Don't mess with the blimp."
	{"fkcxne", "K^^OD^CED*ODOGCOY0*Ned-~*goyy*}c~b*~bo*hfcgz$", NULL},
	// "gonzales" / "Time compression key enabled." / "Time compression key disabled."
	{"medpkfoy", "^cgo*iegzxoyyced*aos*odkhfon$", "^cgo*iegzxoyyced*aos*ncykhfon$"},
	// "tik" / "Bounding spheres."
	{"~ca", "He\177dncdm*yzboxoy$", NULL},
	// "crillion" / "Infinite jumpjets ON." / "Infinite jumpjets OFF."
	{"ixcffced", "Cdlcdc~o*`\177gz`o~y*ED$", "Cdlcdc~o*`\177gz`o~y*ELL$"},
	// "shit" / "Freebirth vulgarity will not be tolerated."
	{"ybc~", "Lxoohcx~b*|\177fmkxc~s*}cff*de~*ho*~efoxk~on$", NULL},
	// "drone" / "free-eye mode on."
	{"nxedo", "lxoo'oso*geno*ed$", NULL},
	// "clark" / "x-ray vision enabled."
	{"ifkxa", "r'xks*|cyced*odkhfon$", NULL},
	// "merlock" / "time expansion enabled." / "time expansion disabled."
	{"goxfeia", "~cgo*orzkdyced*odkhfon$", "~cgo*orzkdyced*ncykhfon$"},
};
#endif

// The last 15 characters typed, the newest last.
// GLOBAL: MW2 0x100e9620
// GLOBAL: MW2GBL 0x1015dd20
// GLOBAL: MW2PREMERCS 0x10157410
// GLOBAL: MW2MATROX 0x101d4a80
// GLOBAL: MW23DFX 0x10159f50
// GLOBAL: MW2ATI 0x1008df50
// GLOBAL: MW2S3 0x1014da00
// GLOBAL: MW2POWERVR 0x10084490
MechChar g_typedKeys[0xf];

// GLOBAL: MW2 0x100ea3e4
// GLOBAL: MW2GBL 0x1015dd30
// GLOBAL: MW2PREMERCS 0x101b7654
// GLOBAL: MW2MATROX 0x1012c1ec
// GLOBAL: MW23DFX 0x101e8e10
// GLOBAL: MW2ATI 0x1011cad0
// GLOBAL: MW2S3 0x100abc14
// GLOBAL: MW2POWERVR 0x10147790
MechS32 g_frontViewForRear;

// Appends a character key (key code type 7) to the typed keys. Returns whether it was one.
// FUNCTION: MW2 0x1005b7c0
// FUNCTION: MW2GBL 0x10013470
// FUNCTION: MW2PREMERCS 0x1006c640
// FUNCTION: MW2MATROX 0x10041ed0
// FUNCTION: MW23DFX 0x10036d10
// FUNCTION: MW2ATI 0x100358d0
// FUNCTION: MW2S3 0x10011b20
// FUNCTION: MW2POWERVR 0x10030350
MechS32 AppendTypedKey(MechS16 p_key)
{
	if ((p_key & 0xff00) != 0x700) {
		return FALSE;
	}

	memmove(g_typedKeys, &g_typedKeys[1], 0xe);
	g_typedKeys[0xe] = (MechChar) p_key;
	return TRUE;
}

// Returns whether the typed keys end with the code p_code, whose characters are stored XORed
// with 0x1a.
// FUNCTION: MW2 0x1005b807
// FUNCTION: MW2GBL 0x100134b7
// FUNCTION: MW2PREMERCS 0x1006c687
// FUNCTION: MW2MATROX 0x10041f17
// FUNCTION: MW23DFX 0x10036d40
// FUNCTION: MW2ATI 0x10035900
// FUNCTION: MW2S3 0x10011b50
// FUNCTION: MW2POWERVR 0x10030380
MechS32 TypedCodeMatches(MechChar* p_code)
{
	MechChar* typed;
	MechU32 n;
	MechChar* c;

	n = strlen(p_code);
	typed = &g_typedKeys[0xe];
	for (c = &p_code[n - 1]; n; n--, c--, typed--) {
#ifdef MW2_CHEAT_TABLE
		if ((*c ^ 0xa) != *typed) {
#else
		if ((*c ^ 0x1a) != *typed) {
#endif
			return FALSE;
		}
	}

	return TRUE;
}

#ifdef MW2_CHEAT_TABLE
// Decodes the cheat messages in place (g_cheatCodes; the codes stay encoded).
// FUNCTION: MW2GBL 0x1001352e
// FUNCTION: MW2PREMERCS 0x1006c6fe
void DecodeCheatMessages(void)
{
	MechS32 i;
	MechChar* message;
	MechS32 length;
	MechS32 j;

	for (i = 0; i < CHEAT_CODE_COUNT; i++) {
		message = g_cheatCodes[i].m_on;
		if (message) {
			length = strlen(message);
			for (j = 0; j < length; j++) {
				message[j] ^= 0xa;
			}
		}

		message = g_cheatCodes[i].m_off;
		if (message) {
			length = strlen(message);
			for (j = 0; j < length; j++) {
				message[j] ^= 0xa;
			}
		}
	}
}

// Takes a typed key for the cheat codes: decodes the messages at the first key, and when the last
// keys typed spell a code of g_cheatCodes (TypedCodeMatches), toggles or runs its cheat and says so.
// FUNCTION: MW2GBL 0x10013616
// FUNCTION: MW2PREMERCS 0x1006c7e6
void HandleCheatInput(MechS16 p_key)
{
	MechS32 target;
	MechS32 victim;

	if (!g_cheatMessagesDecoded) {
		g_cheatMessagesDecoded = 1;
		DecodeCheatMessages();
	}

	if (!AppendTypedKey(p_key)) {
		return;
	}

	if (TypedCodeMatches(g_cheatCodes[0].m_code)) { // "kent"
		if (!g_difficulty->m_invulnerable) {
			g_difficulty->m_invulnerable = 1;
			ShowInGameMessage(g_cheatCodes[0].m_on, 1, 0x16a, 0x32);
		}
		else {
			g_difficulty->m_invulnerable = 0;
			ShowInGameMessage(g_cheatCodes[0].m_off, 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[1].m_code)) { // "thundros"
		if (!g_difficulty->m_unlimitedAmmo) {
			g_difficulty->m_unlimitedAmmo = 1;
			ShowInGameMessage(g_cheatCodes[1].m_on, 1, 0x16a, 0x32);
		}
		else {
			g_difficulty->m_unlimitedAmmo = 0;
			ShowInGameMessage(g_cheatCodes[1].m_off, 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[2].m_code)) { // "burr"
		if (!g_difficulty->m_heatTracking) {
			g_difficulty->m_heatTracking = 1;
			ShowInGameMessage(g_cheatCodes[2].m_on, 1, 0x16a, 0x32);
		}
		else {
			g_difficulty->m_heatTracking = 0;
			ShowInGameMessage(g_cheatCodes[2].m_off, 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[3].m_code)) { // "dorcs"
		ShowDorcs();
		ShowInGameMessage(g_cheatCodes[3].m_on, 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches(g_cheatCodes[5].m_code)) { // "kaboom"
		target = GetLocalTargetGamePiece();
		if (target >= 0) {
			StartNuke(g_players[target]);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[6].m_code)) { // "jumbo"
		g_localSteering.m_grantJumpJets = 1;
		ShowInGameMessage(g_cheatCodes[6].m_on, 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches(g_cheatCodes[7].m_code)) { // "voyeur"
		if (!g_frontViewForRear) {
			g_frontViewForRear = 1;
			ShowInGameMessage(g_cheatCodes[7].m_on, 1, 0x16a, 0x32);
		}
		else {
			g_frontViewForRear = 0;
			ShowInGameMessage(g_cheatCodes[7].m_off, 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[9].m_code)) { // "palex"
		victim = GetLocalTargetGamePiece();
		if (victim >= 0) {
			KillMech(g_localPlayerId, g_players[victim]->m_mech);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[11].m_code)) { // "putz"
		g_forceMissionSuccess = 1;
	}
	else if (TypedCodeMatches(g_cheatCodes[14].m_code)) { // "gonzales"
		g_timeCompressionCheat = !g_timeCompressionCheat;

		if (g_timeCompressionCheat) {
			ShowInGameMessage(g_cheatCodes[14].m_on, 1, 0xb5, 0x32);
		}
		else {
			ShowInGameMessage(g_cheatCodes[14].m_off, 1, 0xb5, 0x32);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[15].m_code)) { // "tik"
		g_frameOutlineParts = !g_frameOutlineParts;

		g_showBoundingSpheres = !g_showBoundingSpheres;

		ShowInGameMessage(g_cheatCodes[15].m_on, 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches(g_cheatCodes[16].m_code)) { // "crillion"
		if (!g_infiniteJumpFuel) {
			g_infiniteJumpFuel = 1;
			ShowInGameMessage(g_cheatCodes[16].m_on, 1, 0x16a, 0x32);
		}
		else {
			g_infiniteJumpFuel = 0;
			ShowInGameMessage(g_cheatCodes[16].m_off, 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches(g_cheatCodes[18].m_code)) { // "drone"
		SetViewMode(c_viewFreeEye);
		g_mapFollowsFreeEye = 1;
		ShowInGameMessage(g_cheatCodes[18].m_on, 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches(g_cheatCodes[19].m_code)) { // "clark"
		g_renderSettings.m_wireframe = 2;
		g_renderSettings.m_wireframeColors = 0;
		ShowInGameMessage(g_cheatCodes[19].m_on, 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches(g_cheatCodes[20].m_code)) { // "merlock"
		g_timeExpansionEnabled = !g_timeExpansionEnabled;

		if (g_timeExpansionEnabled) {
			ShowInGameMessage(g_cheatCodes[20].m_on, 1, 0x16a, 0x32);
		}
		else {
			ShowInGameMessage(g_cheatCodes[20].m_off, 1, 0x16a, 0x32);
		}
	}
}
#else
// Takes a typed key for the cheat codes: when the last keys typed spell one (TypedCodeMatches; the codes
// are stored XORed with 0x1a), toggles or runs its cheat and says so.
// FUNCTION: MW2 0x1005b883
// FUNCTION: MW2MATROX 0x10041f93
// FUNCTION: MW23DFX 0x10036d80
// FUNCTION: MW2ATI 0x10035940
// FUNCTION: MW2S3 0x10011b90
// FUNCTION: MW2POWERVR 0x100303c0
void HandleCheatInput(MechS16 p_key)
{
	MechS32 target;
	MechS32 victim;

	if (!AppendTypedKey(p_key)) {
		return;
	}

	if (TypedCodeMatches("xvuhx")) { // "blorb"
		if (!g_difficulty->m_invulnerable) {
			g_difficulty->m_invulnerable = 1;
			ShowInGameMessage("Invulnerability ON", 1, 0x16a, 0x32);
		}
		else {
			g_difficulty->m_invulnerable = 0;
			ShowInGameMessage("Invulnerability OFF", 1, 0x16a, 0x32);
		}
	}
#ifdef MW2_CHRIS_FLAG_CHEATS
	else if (TypedCodeMatches("yrhsi")) { // "chris"
		ShowInGameMessage("Hey man!", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("|v{}")) { // "flag"
		g_difficulty->m_invulnerable = 1;
		g_difficulty->m_unlimitedAmmo = 3;
		g_difficulty->m_heatTracking = 0;
		g_localSteering.m_grantJumpJets = 1;
		g_infiniteJumpFuel = 1;
		ShowInGameMessage("Flag is invincible. Lemmy is a wuss.", 1, 0x16a, 0x32);
	}
#endif
#ifdef MW2_FLAG_CHEAT
	else if (TypedCodeMatches("|v{}")) { // "flag"
		g_difficulty->m_invulnerable = 1;
		g_difficulty->m_unlimitedAmmo = 3;
		g_difficulty->m_heatTracking = 0;
		g_localSteering.m_grantJumpJets = 1;
		g_infiniteJumpFuel = 1;
		ShowInGameMessage("Flag's Invincible!", 1, 0x16a, 0x32);
	}
#endif
#ifdef MW2_LEMMY_CHEAT
	else if (TypedCodeMatches("v\x7fwwc")) { // "lemmy"
		g_difficulty->m_invulnerable = 1;
		g_difficulty->m_unlimitedAmmo = 1;
		g_difficulty->m_heatTracking = 0;
		g_localSteering.m_grantJumpJets = 1;
		g_infiniteJumpFuel = 1;
		ShowInGameMessage("Invulnerable, Lot's o' Ammo, No Heat, & Jets", 1, 0x16a, 0x32);
	}
#endif
	else if (TypedCodeMatches("ys{")) { // "cia"
		if (!g_difficulty->m_unlimitedAmmo) {
			g_difficulty->m_unlimitedAmmo = 1;
			ShowInGameMessage("Unlimited Ammo ON", 1, 0x16a, 0x32);
		}
		else {
			g_difficulty->m_unlimitedAmmo = 0;
			ShowInGameMessage("Unlimited Ammo OFF", 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches("yuv~wsi\x7fh")) { // "coldmiser"
		if (!g_difficulty->m_heatTracking) {
			g_difficulty->m_heatTracking = 1;
			ShowInGameMessage("Heat tracking ON", 1, 0x16a, 0x32);
		}
		else {
			g_difficulty->m_heatTracking = 0;
			ShowInGameMessage("Heat tracking OFF", 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches("~\x7fs")) { // "dei"
		ShowInGameMessage("F E I F", 1, 0x16a, 0x32);
	}
#ifndef MW2_NO_DORCS_CHEAT
	else if (TypedCodeMatches("~uhyi")) { // "dorcs"
		ShowDorcs();
		ShowInGameMessage("You asked for it!", 1, 0x16a, 0x32);
	}
#endif
	else if (TypedCodeMatches("\x7ftuv{}{c")) { // "enolagay"
		target = GetLocalTargetGamePiece();
		if (target >= 0) {
			StartNuke(g_players[target]);
		}
	}
	else if (TypedCodeMatches("|vc}shv")) { // "flygirl"
		g_localSteering.m_grantJumpJets = 1;
		ShowInGameMessage("Jumpjets", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("|hutn")) { // "front"
		if (!g_frontViewForRear) {
			g_frontViewForRear = 1;
			ShowInGameMessage("forward view instead of rear ON", 1, 0x16a, 0x32);
		}
		else {
			g_frontViewForRear = 0;
			ShowInGameMessage("forward view instead of rear OFF", 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches("|oyq")) { // "fuck"
		ShowInGameMessage("Freebirth vulgarity will not be tolerated!", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("}{tq\x7fw")) { // "gankem"
		victim = GetLocalTargetGamePiece();
		if (victim >= 0) {
			KillMech(g_localPlayerId, g_players[victim]->m_mech);
		}
	}
	else if (TypedCodeMatches("r{t}{huot~")) { // "hangaround"
		g_missionTimerStopped = !g_missionTimerStopped;

		if (g_missionTimerStopped) {
			ShowInGameMessage("That's better........", 1, 0xb5, 0x32);
		}
		else {
			ShowInGameMessage("Now how do I win!", 1, 0xb5, 0x32);
		}
	}
	else if (TypedCodeMatches("sy{tnr{yqsn")) { // "icanthackit"
		g_forceMissionSuccess = 1;
	}
	else if (TypedCodeMatches("s~q|{")) { // "idkfa"
		ShowInGameMessage("This ain't DOOM, Bub.", 1, 0x16a, 0x50);
		RunGameKey(0x3b);
	}
	else if (TypedCodeMatches("v{sh~u")) { // "lairdo"
		g_lairdoCheat = !g_lairdoCheat;

		ShowInGameMessage("ATTENTION ENEMIES: Don't mess with the blimp.", 1, 0xb5, 0x32);
	}
	else if (TypedCodeMatches("w\x7f\x7fjw\x7f\x7fj")) { // "meepmeep"
		g_timeCompressionCheat = !g_timeCompressionCheat;

		if (g_timeCompressionCheat) {
			ShowInGameMessage("Time Compression key enabled", 1, 0xb5, 0x32);
		}
		else {
			ShowInGameMessage("Time Compression key disabled", 1, 0xb5, 0x32);
		}
	}
	else if (TypedCodeMatches("wsyr\x7fvst")) { // "michelin"
		g_frameOutlineParts = !g_frameOutlineParts;

		g_showBoundingSpheres = !g_showBoundingSpheres;

		ShowInGameMessage("bounding spheres", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("ws}rncwuoi\x7f")) { // "mightymouse"
		if (!g_infiniteJumpFuel) {
			g_infiniteJumpFuel = 1;
			ShowInGameMessage("Infinite Jumpjet juice ON", 1, 0x16a, 0x32);
		}
		else {
			g_infiniteJumpFuel = 0;
			ShowInGameMessage("Infinite Jumpjet juice OFF", 1, 0x16a, 0x32);
		}
	}
	else if (TypedCodeMatches("irsn")) { // "shit"
		ShowInGameMessage("Freebirth vulgarity will not be tolerated!", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("nstq\x7fhx\x7fvv")) { // "tinkerbell"
		SetViewMode(c_viewFreeEye);
		g_mapFollowsFreeEye = 1;
		ShowInGameMessage("Free-eye mode ON", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("bh{c")) { // "xray"
		g_renderSettings.m_wireframe = 2;
		g_renderSettings.m_wireframeColors = 0;
		ShowInGameMessage("X-Ray vision enabled", 1, 0x16a, 0x32);
	}
	else if (TypedCodeMatches("`w{q")) { // "zmak"
		g_timeExpansionEnabled = !g_timeExpansionEnabled;

		if (g_timeExpansionEnabled) {
			ShowInGameMessage("Time expansion enabled", 1, 0x16a, 0x32);
		}
		else {
			ShowInGameMessage("Time expansion disabled", 1, 0x16a, 0x32);
		}
	}
#ifdef MW2_TUNING_CHEATS
	// The original multiplies and adds apart (2.2 folds x += x * 0.1f into x * 1.1f without the
	// parentheses).
	else if (TypedCodeMatches("|j")) { // "fp"
		g_a3dFogDensity += (g_a3dFogDensity * 0.1f);
		sgl_set_fog(g_a3dMainView.m_camera, g_a3dFogColour, g_a3dFogDensity);
		DebugPrintInternal("fog = %.6f\n", g_a3dFogDensity);
	}
	else if (TypedCodeMatches("|w")) { // "fm"
		g_a3dFogDensity -= (g_a3dFogDensity * 0.1f);
		sgl_set_fog(g_a3dMainView.m_camera, g_a3dFogColour, g_a3dFogDensity);
		DebugPrintInternal("fog = %.6f\n", g_a3dFogDensity);
	}
	else if (TypedCodeMatches("}j")) { // "gp"
		g_groundTile += (g_groundTile * 0.1f);
		FUN_1005f0b0(-10.0f, 16000000.0f, g_groundTile);
		DebugPrintInternal("ground_tile = %.1f\n", g_groundTile);
	}
	else if (TypedCodeMatches("}w")) { // "gm"
		g_groundTile -= (g_groundTile * 0.1f);
		FUN_1005f0b0(-10.0f, 16000000.0f, g_groundTile);
		DebugPrintInternal("ground_tile = %.1f\n", g_groundTile);
	}
	else if (TypedCodeMatches("ij")) { // "sp"
		g_skyScroll += (g_skyScroll * 0.1f);
		g_skyTileU += (g_skyTileU * 0.1f);
		FUN_1005efe0(2000000.0f, 64000000.0f, g_skyTileU, g_skyScroll);
		DebugPrintInternal("sky_tile = %.1f\n", g_skyTileU);
	}
	else if (TypedCodeMatches("iw")) { // "sm"
		g_skyTileU -= (g_skyTileU * 0.1f);
		FUN_1005efe0(2000000.0f, 64000000.0f, g_skyTileU, g_skyScroll);
		DebugPrintInternal("sky_tile = %.1f\n", g_skyTileU);
		g_skyScroll -= (g_skyScroll * 0.1f);
	}
	else if (TypedCodeMatches("i{j")) { // "sap"
		g_skyScrollSpeed += (g_skyScrollSpeed * 0.2f);
		DebugPrintInternal("anim_rate = %.6f\n", g_skyScrollSpeed / g_skyTileU);
	}
	else if (TypedCodeMatches("i{w")) { // "sam"
		g_skyScrollSpeed -= (g_skyScrollSpeed * 0.2f);
		DebugPrintInternal("anim_rate = %.1f\n", g_skyScrollSpeed / g_skyTileU);
	}
#endif
}
#endif

// Turns a typed character into the one its key gives with shift held (the US layout).
// FUNCTION: MW2 0x1005bf7c
// FUNCTION: MW2GBL 0x10013baf
// FUNCTION: MW2PREMERCS 0x1006cd7f
// FUNCTION: MW2MATROX 0x100426b2
// FUNCTION: MW23DFX 0x10037280
// FUNCTION: MW2ATI 0x10035e40
// FUNCTION: MW2S3 0x100120d0
// FUNCTION: MW2POWERVR 0x10030c20
void ShiftCharacter(MechChar* p_char)
{
	MechChar shifted[16] = {'<', '_', '>', '?', ')', '!', '@', '#', '$', '%', '^', '&', '*', '(', ':', ':'};

	if (*p_char == '\'') {
		*p_char = '"';
	}
	else if (*p_char >= ',' && *p_char <= ';') {
		*p_char = shifted[*p_char - ','];
	}
	else if (*p_char >= '[' && *p_char <= ']') {
		*p_char += 0x20;
	}

	if (*p_char == '`') {
		*p_char = '~';
	}
}

// Takes a key while a chat message is being typed (g_chatRecipient is the recipient): Backspace
// edits, Enter sends it (to everyone from -1), Esc cancels, F-keys e and f pick the team (-3) or
// everyone (-2), and printable characters are added up to 40. Returns whether it took the key.
// FUNCTION: MW2 0x1005c057
// FUNCTION: MW2GBL 0x10013c8a
// FUNCTION: MW2PREMERCS 0x1006ce5a
// FUNCTION: MW2MATROX 0x1004278d
// FUNCTION: MW23DFX 0x10037310
// FUNCTION: MW2ATI 0x10035ed0
// FUNCTION: MW2S3 0x10012160
// FUNCTION: MW2POWERVR 0x10030cb0
MechS32 HandleChatKey(MechU32 p_keyCode)
{
	MechChar text[80];
	MechChar c;
	MechS32 to;

	if (g_localPlayerId == g_chatRecipient) {
		to = 0;
	}
	else {
		to = g_chatRecipient;
	}

	if (p_keyCode & 0x400) {
		return FALSE;
	}

	c = (MechChar) p_keyCode;
	if (c == '\b') {
		if (g_chatLength > 0) {
			g_chatLength--;
		}

		g_chatMessage[g_chatLength] = '\0';
		return TRUE;
	}

	switch (c) {
	case 0x1b:
		g_chatRecipient = 0;
		memset(g_chatMessage, 0, 40);
		g_chatLength = 0;
		return TRUE;
	case '\r':
		if (g_chatRecipient > 0) {
			g_chatRecipient = 0;
		}
		else if (g_chatRecipient == -1) {
			to = -1;
			g_chatRecipient = 0;
		}
	}

	if (g_chatLength > 39) {
		return TRUE;
	}

	if (p_keyCode & 0x100) {
		if (g_chatRecipient == -1) {
			switch (c) {
			case 'f':
				to = -2;
				g_chatRecipient = 0;
				break;
			case 'e':
				to = -3;
				g_chatRecipient = 0;
				break;
			default:
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}

	if (p_keyCode & 0x200) {
		c = toupper(c);
		ShiftCharacter(&c);
	}

	if (g_chatRecipient == 0) {
		SendChatMsg(to, g_chatMessage);
		sprintf(text, "%s: %s", g_players[g_localPlayerId]->m_name, g_chatMessage);
		ShowInGameMessage(text, 1, 0x2d4, 0x32);
		memset(g_chatMessage, 0, 40);
		g_chatLength = 0;
		return TRUE;
	}

	if (c < ' ' || c > '~') {
		return FALSE;
	}

	g_chatMessage[g_chatLength] = c;
	g_chatLength++;
	return TRUE;
}

// Runs the frame's game keys: after the mission ends, waits a while before offering to quit (or,
// with DifficultyCfg::m_regenerate, to regenerate); otherwise passes the key code (from the local
// steering, through the cheat codes, the chat message and LookupGameKey) to RunGameKey.
// FUNCTION: MW2 0x1005c2e1
// FUNCTION: MW2GBL 0x10013f1a
// FUNCTION: MW2MATROX 0x10042a17
// FUNCTION: MW23DFX 0x100374e0
// FUNCTION: MW2ATI 0x100360a0
// FUNCTION: MW2S3 0x10012330
// FUNCTION: MW2POWERVR 0x10030e80
void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechU16 p_key)
{
	if (g_simPaused) {
		return;
	}

	if (g_speechFlushTime > 0 && g_currentClock > g_speechFlushTime) {
		FlushSpeechQueue(1);
		g_speechFlushTime = 0;
	}

	if (!g_missionEndTime && !g_missionEnded) {
		if (g_missionResolved || g_fledToWindows) {
			g_statusMessage = 0;
			g_missionEnded = 1;
			g_speechFlushTime = g_currentClock + 0x108;
			g_missionEndTime = g_currentClock + 0x71c;
			ShowInGameMessage("Press CTRL-Q to exit...", 1, 0x1536, 100);
		}
		else if (g_localMechLost) {
			g_chatRecipient = 0;
			CycleTrackedPlayer(0, 1);
			g_renderSettings.m_wireframe = 0;
			SetInfrared(0, 0);
			g_speechFlushTime = g_currentClock + 0x108;
			if (!g_netRole) {
				g_missionEnded = 1;
				g_missionEndTime = g_currentClock + 0x71c;
				ShowInGameMessage("Press CTRL-Q to exit...", 1, 0x1536, 100);
			}
			else {
				g_missionEndTime = g_currentClock + 0x43e;
			}
		}
	}
	else if (g_missionEndTime > 0 && g_currentClock > g_missionEndTime) {
		g_missionEnded = 1;
		if (!g_netRole || g_missionResolved || g_fledToWindows) {
			g_shouldQuit = 1;
			g_quitStage = 0x29a;
		}
		else if (!g_difficulty->m_regenerate) {
			g_missionEndTime = -1;
			ShowInGameMessage("Press SPACEBAR to advance viewpoint, or CTRL-Q to exit.", 1, 0x58610, 100);
			g_statusMessage = 1;
			g_spectating = 1;
		}
		else {
			g_missionEndTime = 0;
			g_statusMessage = 2;
		}
	}

	p_key = g_localSteering.m_keyCode;
	if (!g_isNetworkGame) {
		HandleCheatInput(p_key);
	}

	if (g_chatRecipient && HandleChatKey(p_key)) {
		return;
	}

	p_key = LookupGameKey(p_key);
	if (!p_key) {
		return;
	}

	if (g_missionEnded) {
		if (!g_netRole || g_missionResolved || g_fledToWindows) {
			if (p_key == GAME_KEY_ACTION(0x59)) {
				g_shouldQuit = 1;
				g_quitStage = 0x29a;
				return;
			}
		}
		else if (g_difficulty->m_regenerate) {
			switch (p_key) {
			case GAME_KEY_ACTION(0x59):
				g_shouldQuit = 1;
				g_quitStage = 0x29a;
				return;
			case 0x33:
				RequestMenu(4);
				break;
			case GAME_KEY_ACTION(0x4f):
				g_renderSettings.m_wireframe = 0;
				SetInfrared(0, 0);
				ReloadPlayerMech(g_localPlayerId, 0, 0, 0);
				SetViewMode(c_viewCockpit);
				g_sinkPilotTiltReset = 1;
				g_sinkPilotPanReset = 1;
				g_sinkZoomFactorReset = 1;
				g_speechLocked = 0;
				g_speechFlushTime = 0;
				g_statusMessage = 0;
				g_missionEndTime = 0;
				g_missionEnded = 0;
				g_missionResolved = 0;
				g_spectating = 0;
				break;
			}
		}
		else if (g_spectating) {
			switch (p_key) {
			case GAME_KEY_ACTION(0x59):
				g_shouldQuit = 1;
				g_quitStage = 0x29a;
				return;
			case 0x33:
				RequestMenu(4);
				break;
			case GAME_KEY_ACTION(0x4f):
				CycleTrackedPlayer(1, 0);
				break;
			}
		}
	}
	else {
		RunGameKey(p_key);
	}
}

// Performs game key p_key's action: the cockpit's detail and views, the throttle steps (0x1a-0x23),
// the controls the local steering takes (a flag set for the frame), the menus, the chat
// recipients, ejecting (0x3b) and the screenshot (0x5b). Any other key goes to HandleDebugKey while
// the mission timer is stopped.
// FUNCTION: MW2 0x1005c78a
// FUNCTION: MW2GBL 0x100143c3
// FUNCTION: MW2MATROX 0x10042ec0
// FUNCTION: MW23DFX 0x10037880
// FUNCTION: MW2ATI 0x10036440
// FUNCTION: MW2S3 0x100126d0
// FUNCTION: MW2POWERVR 0x10031220
void RunGameKey(MechS32 p_key)
{
	Mech* mech;
	MechS32 step;
	MechChar text[40];

	step = -1;
	mech = g_players[g_localPlayerId]->m_mech;
	switch (p_key) {
	case 0x0:
		break;
	case GAME_KEY_ACTION(0x59):
		g_shouldQuit = 1;
		g_quitStage += 2;
		break;
	case 0x2:
		CycleMechViewMode();
		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 0, 0x32);
		}
		break;
	case 0x3:
		g_mechViewMode = g_mechViewMode == 2 ? 0 : 2;

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 1, 0x32);
		}
		break;
	case 0x4:
		g_mechViewMode = g_mechViewMode == 3 ? 0 : 3;

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 2, 0x32);
		}
		break;
	case 0x5:
		g_mechViewMode = g_mechViewMode == 4 ? 0 : 4;

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 3, 0x32);
		}
		break;
	case 0x6:
		g_mechViewMode = g_mechViewMode == 5 ? 0 : 5;

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 4, 0x32);
		}
		break;
	case 0x7:
		g_mechViewMode = g_mechViewMode == 1 ? 0 : 1;

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 6, 0x32);
		}
		break;
	case 0x8:
		g_targetPanelMode++;
		if (g_targetPanelMode > 2) {
			g_targetPanelMode = 0;
		}

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 7, 0x32);
		}
		break;
	case 0x9:
		if (!(g_players[g_localPlayerId]->m_flags & 2)) {
			if (GetViewMode() || IsSatelliteView()) {
				LeaveSatelliteView();
				SetViewMode(c_viewCockpit);
				g_sinkPilotTiltReset = 1;
				g_sinkPilotPanReset = 1;
				g_sinkZoomFactorReset = 1;
			}
			else {
				SetViewMode(c_viewTrack);
				CycleTrackedPlayer(0, 1);
			}
		}
		break;
	case 0xe:
		if (TrackLastShot()) {
			SetViewMode(c_viewOrdinance);
		}
		break;
	case 0x11:
		g_feetToTorso = 1;
		break;
	case 0x12:
		g_localSteering.m_inspectTarget = 1;
		break;
	case 0x13:
		g_showHud = !g_showHud;

		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			PlaySoundEffect(0xdc, 100, 0x40, 8, 0x32);
		}
		break;
	case 0x14:
		g_jettisonAmmoRequested = 1;
		break;
	case 0x17:
		g_sinkZoomFactorReset = 1;
		break;
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
		step = p_key - 0x1a;
		break;
	case 0x24:
		g_localSteering.m_advanceGamething = 1;
		break;
	case 0x25:
		g_localSteering.m_previousGamething = 1;
		break;
	case 0x26:
		g_localSteering.m_resetGamething = 1;
		break;
	case 0x27:
		ResetTargeting();
		break;
	case 0x28:
		g_localSteering.m_advanceNav = 1;
		break;
	case 0x29:
		g_localSteering.m_previousNav = 1;
		break;
	case 0x2a:
		g_localSteering.m_resetNav = 1;
		break;
	case 0x2b:
		g_localSteering.m_advanceGamepiece = 1;
		break;
	case 0x2c:
		g_localSteering.m_previousGamepiece = 1;
		break;
	case 0x2d:
		g_localSteering.m_resetGamepiece = 1;
		break;
	case 0x2e:
		CycleCockpitView();
		break;
	case 0x2f:
		ZoomMapView(1);
		break;
	case 0x30:
		ZoomMapView(2);
		break;
	case 0x31:
		ZoomMapView(0);
		break;
	case 0x32:
		ToggleSatelliteView();
		break;
	case 0x33:
		RequestMenu(4);
		break;
	case 0x34:
		ToggleMenu(5);
		break;
#ifdef MW2_PT_MENU_KEYS
	case 0x35:
#endif
	case GAME_KEY_ACTION(0x42):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = -1;
		}
		else {
			ToggleMenu(1);
		}
		break;
#ifdef MW2_PT_MENU_KEYS
	case 0x36:
#endif
	case GAME_KEY_ACTION(0x43):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 1;
		}
		else {
			ToggleMenu(7);
		}
		break;
#ifdef MW2_PT_MENU_KEYS
	case 0x37:
#endif
	case GAME_KEY_ACTION(0x44):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 2;
		}
		else {
			ToggleMenu(8);
		}
		break;
	case GAME_KEY_ACTION(0x45):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 3;
		}
		break;
	case GAME_KEY_ACTION(0x46):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 4;
		}
		break;
	case GAME_KEY_ACTION(0x47):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 5;
		}
		break;
	case GAME_KEY_ACTION(0x48):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 6;
		}
		break;
	case GAME_KEY_ACTION(0x49):
		g_showObjectives = 0;
		if (g_isNetworkGame) {
			g_chatRecipient = 7;
		}
		break;
	case 0x3b:
		EjectPlayer(g_players[g_localPlayerId]->m_mech, 1);
		break;
	case 0x3c:
		if (g_players[g_localPlayerId]->m_flags & 0x2000) {
			if (g_autoEject) {
				g_autoEject = 0;
				sprintf(text, "Automatic ejection OFF");
				ShowInGameMessage(text, 1, 0x16a, 0x32);
			}
			else {
				g_autoEject = 1;
				sprintf(text, "Automatic ejection ON");
				ShowInGameMessage(text, 1, 0x16a, 0x32);
			}
		}
		break;
	case 0x3d:
#ifndef MW2_POWER_KEYS_ONE_CASE
#ifdef MW2_POWER_STATE_7_KEYS
		if (mech->m_powerState == 7) {
			break;
		}

#endif
		if (mech->m_powerState == 3) {
			g_powerRequest = 1;
		}
		else {
			g_powerRequest = -1;
		}
		break;
#endif
	case 0x3e:
#ifdef MW2_POWER_STATE_7_KEYS
		if (mech->m_powerState == 7) {
			break;
		}

#endif
		if (mech->m_powerState == 3) {
			g_powerRequest = 1;
		}
		else {
#if !defined(MW2_POWER_STATE_7_KEYS) && !defined(MW2_NO_STATE_7_POWER_TEST)
			if (mech->m_powerState == 7) {
				break;
			}

#endif
			g_powerRequest = -1;
		}
		break;
	case 0x3f:
		g_localSteering.m_reverse = !g_localSteering.m_reverse;
		break;
	case 0x40:
		g_overrideShutdown = 1;
		break;
#ifndef MW2_NO_WIREFRAME_KEY
	case GAME_KEY_ACTION(0xa7):
		if (g_renderSettings.m_wireframe != 1 && (g_players[g_localPlayerId]->m_flags & 0x2000)) {
			g_renderSettings.m_wireframe = 1;
			g_renderSettings.m_wireframeColors = 0;
			PlayCockpitSound(0x1b, 1);
		}
		else {
			g_renderSettings.m_wireframe = 0;
		}
		break;
#endif
#ifdef MW2_AMS_KEY
	case 0xa9:
		ToggleAms();
		break;
#endif
	case GAME_KEY_ACTION(0xa6):
		SetInfrared(0, !g_infraredOn);
		break;
	case 0x41:
		g_chatRecipient = 0;
		g_showObjectives = !g_showObjectives;
		break;
#ifdef MW2_CHAT_KEY
	case 0x42:
		g_chatRecipient = !g_chatRecipient;
		break;
#endif
	case GAME_KEY_ACTION(0x4a):
		g_localSteering.m_autopilot = 1;
		break;
	case GAME_KEY_ACTION(0x4b):
		g_localSteering.m_toggleGroupFire = 1;
		break;
	case GAME_KEY_ACTION(0x4c):
		g_localSteering.m_advanceTarget = 1;
		break;
	case GAME_KEY_ACTION(0x4d):
		g_localSteering.m_previousTarget = 1;
		break;
	case GAME_KEY_ACTION(0x4e):
		g_localSteering.m_resetTarget = 1;
		break;
	case GAME_KEY_ACTION(0x50):
		g_localSteering.m_nearestEnemy = 1;
		break;
	case GAME_KEY_ACTION(0x53):
		g_localSteering.m_targetLastShot = 1;
		break;
	case GAME_KEY_ACTION(0x51):
		g_localSteering.m_targetFriendly = 1;
		break;
	case GAME_KEY_ACTION(0x54):
		g_localSteering.m_nextObjective = 1;
		break;
	case GAME_KEY_ACTION(0x52):
		g_localSteering.m_targetReticle = 1;
		break;
	case GAME_KEY_ACTION(0x55):
		g_punchInAutoHeadingRequested = 1;
		break;
	case GAME_KEY_ACTION(0x56):
		g_toggleMascRequested = 1;
		break;
	case GAME_KEY_ACTION(0x57):
		g_localSteering.m_selfDestruct = 1;
		break;
	case 0x1:
		g_localSteering.m_torsoTiltReset = 1;
		g_localSteering.m_torsoPanReset = 1;
		g_sinkPilotTiltReset = 1;
		g_sinkPilotPanReset = 1;
		g_sinkEyepointTiltReset = 1;
		g_sinkEyepointPanReset = 1;
		break;
	case GAME_KEY_ACTION(0x58):
		g_pauseRequested = 1;
		break;
	case GAME_KEY_ACTION(0x97):
		SetSelectedWeaponGroup(0);
		break;
	case GAME_KEY_ACTION(0x98):
		SetSelectedWeaponGroup(1);
		break;
	case GAME_KEY_ACTION(0x99):
		SetSelectedWeaponGroup(2);
		break;
	case GAME_KEY_ACTION(0x9a):
		g_localSteering.m_weaponCycleGroup = 1;
		break;
	case GAME_KEY_ACTION(0x9b):
		g_localSteering.m_weaponFireGroup = 1;
		break;
	case GAME_KEY_ACTION(0x9c):
		g_localSteering.m_weaponFireGroup1 = 1;
		break;
	case GAME_KEY_ACTION(0x9d):
		g_localSteering.m_weaponFireGroup2 = 1;
		break;
	case GAME_KEY_ACTION(0x9e):
		g_localSteering.m_weaponFireGroup3 = 1;
		break;
	case GAME_KEY_ACTION(0x92):
		if (!g_timeCompressionCheat) {
			break;
		}

		g_timeCompressionEnabled = !g_timeCompressionEnabled;

		if (g_timeCompressionEnabled) {
			sprintf(text, "Time compression enabled");
			ShowInGameMessage(text, 1, 0xb50, 0x32);
		}
		else {
			sprintf(text, "Time compression disabled");
			ShowInGameMessage(text, 1, 0x16a, 0x32);
		}
		break;
#ifndef MW2_NO_SCREENSHOT_KEY
	case GAME_KEY_ACTION(0x5b):
		PauseTimer(0x80, 1);
#ifdef MW2_RAW_SCREENSHOTS
		if (g_windowActive) {
			g_currentDisplayBackend->m_acquireFramebuffer();
		}
		else {
		}

		SaveScreenshot();
		PauseTimer(0x80, 0);
		sprintf(text, "Image saved - MW2000?.888");
#elif defined(MW2_SCREENSHOT_ACQUIRE_IF)
		if (g_windowActive) {
			g_currentDisplayBackend->m_acquireFramebuffer();
		}
		else {
		}

		SaveScreenshot();
		PauseTimer(0x80, 0);
		sprintf(text, "GIF saved - MW2000?.GIF");
#else
		g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1;
		SaveScreenshot();
		PauseTimer(0x80, 0);
		sprintf(text, "GIF saved - MW2000?.GIF");
#endif
		ShowInGameMessage(text, 1, 0x16a, 0x32);
		break;
#endif
	default:
		if (g_missionTimerStopped) {
			HandleDebugKey(p_key);
		}
		break;
	}

	if (step != -1) {
#ifdef MW2_FLOAT_SCALARS
		g_localSteering.m_throttle = step * (1.0f / 576.0f);
#else
		g_localSteering.m_throttle = step * 113;
#endif
		g_localSteering.m_throttleSet = 1;
	}
}
