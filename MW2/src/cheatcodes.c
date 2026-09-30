/* The cheat codes: the last 15 keys typed, and a match against a code stored XORed with 0x1a. */
#include "cheatcodes.h"

#include "decomp.h"
#include "types.h"

#include <string.h>

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
