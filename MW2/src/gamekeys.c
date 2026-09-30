/* The keys typed in a mission: the cheat codes (the last 15 keys typed, and a match against a code
   stored XORed with 0x1a), the chat message being typed and the game keys. */
#include "gamekeys.h"

#include "decomp.h"
#include "network.h"
#include "players.h"
#include "simmain.h"
#include "timedoverlays.h"
#include "types.h"
#include "unk10004f40.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

// GLOBAL: MW2 0x100aa290
MechS32 g_unk0x100aa290 = 0;

// GLOBAL: MW2 0x100aa294
MechS32 g_unk0x100aa294 = 0;

// A game-key toggle (FUN_1005e9b0's setting 0x40).
// GLOBAL: MW2 0x100aa298
MechS32 g_unk0x100aa298 = 0;

// The last 15 characters typed, the newest last.
// GLOBAL: MW2 0x100e9620
MechChar g_unk0x100e9620[0xf];

// Appends a character key (key code type 7) to the typed keys. Returns whether it was one.
// FUNCTION: MW2 0x1005b7c0
MechS32 FUN_1005b7c0(MechS16 p_key)
{
	if ((p_key & 0xff00) != 0x700) {
		return FALSE;
	}

	memmove(g_unk0x100e9620, &g_unk0x100e9620[1], 0xe);
	g_unk0x100e9620[0xe] = (MechChar) p_key;
	return TRUE;
}

// Returns whether the typed keys end with the code p_code, whose characters are stored XORed
// with 0x1a.
// Stack-slot permutation of n and c.
// FUNCTION: MW2 0x1005b807
MechS32 FUN_1005b807(MechChar* p_code)
{
	MechChar* typed;
	MechU32 n;
	MechChar* c;

	n = strlen(p_code);
	typed = &g_unk0x100e9620[0xe];
	for (c = &p_code[n - 1]; n; n--, c--, typed--) {
		if ((*c ^ 0x1a) != *typed) {
			return FALSE;
		}
	}

	return TRUE;
}

// Turns a typed character into the one its key gives with shift held (the US layout).
// FUNCTION: MW2 0x1005bf7c
void FUN_1005bf7c(MechChar* p_char)
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

// Takes a key while a chat message is being typed (g_unk0x100a116c is the recipient): Backspace
// edits, Enter sends it (to everyone from -1), Esc cancels, F-keys e and f pick the team (-3) or
// everyone (-2), and printable characters are added up to 40. Returns whether it took the key.
// Stack-slot permutation; g_localPlayerId == g_unk0x100a116c compares in the other operand order.
// FUNCTION: MW2 0x1005c057
MechS32 HandleChatKey(MechU32 p_keyCode)
{
	MechChar text[80];
	MechChar c;
	MechS32 to;

	if (g_localPlayerId == g_unk0x100a116c) {
		to = 0;
	}
	else {
		to = g_unk0x100a116c;
	}

	if (p_keyCode & 0x400) {
		return FALSE;
	}

	c = (MechChar) p_keyCode;
	if (c == '\b') {
		if (g_unk0x100aa2b8 > 0) {
			g_unk0x100aa2b8--;
		}

		g_unk0x10179e90[g_unk0x100aa2b8] = '\0';
		return TRUE;
	}

	switch (c) {
	case 0x1b:
		g_unk0x100a116c = 0;
		memset(g_unk0x10179e90, 0, 40);
		g_unk0x100aa2b8 = 0;
		return TRUE;
	case '\r':
		if (g_unk0x100a116c > 0) {
			g_unk0x100a116c = 0;
		}
		else if (g_unk0x100a116c == -1) {
			to = -1;
			g_unk0x100a116c = 0;
		}
	}

	if (g_unk0x100aa2b8 > 39) {
		return TRUE;
	}

	if (p_keyCode & 0x100) {
		if (g_unk0x100a116c == -1) {
			switch (c) {
			case 'f':
				to = -2;
				g_unk0x100a116c = 0;
				break;
			case 'e':
				to = -3;
				g_unk0x100a116c = 0;
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
		FUN_1005bf7c(&c);
	}

	if (g_unk0x100a116c == 0) {
		FUN_1000efa4(to, g_unk0x10179e90);
		sprintf(text, "%s: %s", g_players[g_localPlayerId]->m_name, g_unk0x10179e90);
		ShowInGameMessage(text, 1, 0x2d4, 0x32);
		memset(g_unk0x10179e90, 0, 40);
		g_unk0x100aa2b8 = 0;
		return TRUE;
	}

	if (c < ' ' || c > '~') {
		return FALSE;
	}

	g_unk0x10179e90[g_unk0x100aa2b8] = c;
	g_unk0x100aa2b8++;
	return TRUE;
}

// STUB: MW2 0x1005c2e1
void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	STUB(0x1005c2e1);
}

// Performs game key p_key's action (0x3b ejects).
// STUB: MW2 0x1005c78a
void FUN_1005c78a(MechS32 p_key)
{
	STUB(0x1005c78a);
}
