#include "unk10004f40.h"

#include "decomp.h"
#include "rendertarget.h"
#include "types.h"

#include <stdio.h>

// A string FUN_100063cd uses, defined here until that function is decompiled: as a literal
// nobody references yet, reccmp would pair it with the first "MASC" of speech.c's tables.
// GLOBAL: MW2 0x100a1458
MechChar g_unk0x100a1458[] = "MASC";

// GLOBAL: MW2 0x100bcd88
static MechChar g_unk0x100bcd88[16];

// GLOBAL: MW2 0x100bcd98
static MechChar g_unk0x100bcd98[16];

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
		width += FUN_10064d60(p_font, *c);
	}

	return width;
}
