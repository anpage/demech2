#include "overlay.h"

#include "clock.h"
#include "decomp.h"
#include "eyepoint.h"
#include "loadres.h"
#include "point.h"
#include "render.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "timedoverlays.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk1006d680.h"

#include <stdio.h>
#include <string.h>

// The debug overlays: frame rate, object and polygon counts, the eyepoint's position and
// the cache size. Each is switched on by a flag the debug keys toggle; its text is only
// formatted, most of the drawing is compiled out.

// GLOBAL: MW2 0x100a9478
MechChar g_unk0x100a9478[] = "                                 ";

// GLOBAL: MW2 0x100a949c
MechS32 g_unk0x100a949c = 0;

// GLOBAL: MW2 0x100a94a0
MechS32 g_unk0x100a94a0 = 0;

// GLOBAL: MW2 0x100a94a4
MechS32 g_unk0x100a94a4 = 3;

// GLOBAL: MW2 0x100a94a8
MechS32 g_unk0x100a94a8 = 3;

// GLOBAL: MW2 0x100a94ac
MechChar g_unk0x100a94ac[4] = "";

// GLOBAL: MW2 0x100a94b0
MechS32 g_unk0x100a94b0 = 0;

// GLOBAL: MW2 0x100a94b4
MechS32 g_unk0x100a94b4 = 0;

// GLOBAL: MW2 0x100a94b8
MechS32 g_unk0x100a94b8 = 0;

// GLOBAL: MW2 0x100a94bc
MechS32 g_unk0x100a94bc = 0;

// GLOBAL: MW2 0x100a94c0
Point g_unk0x100a94c0 = {0x51f, 0x147b};

// GLOBAL: MW2 0x100a94c8
Point g_unk0x100a94c8 = {0x51f, 0x2148};

// GLOBAL: MW2 0x100a94d0
Point g_unk0x100a94d0 = {0xccd, 0xb333};

// GLOBAL: MW2 0x100a94d8
MechS32 g_unk0x100a94d8 = 0;

// GLOBAL: MW2 0x100a94dc
MechS32 g_unk0x100a94dc = 0;

// GLOBAL: MW2 0x100a94e0
MechS32 g_unk0x100a94e0 = 0;

// GLOBAL: MW2 0x100a94e4
MechS32 g_unk0x100a94e4 = 0;

// GLOBAL: MW2 0x100a94e8
MechS32 g_unk0x100a94e8 = 0;

// GLOBAL: MW2 0x100a94ec
MechS32 g_unk0x100a94ec = 0;

// GLOBAL: MW2 0x100a94f0
MechS32 g_unk0x100a94f0 = 1;

// GLOBAL: MW2 0x100a94f4
MechS32 g_unk0x100a94f4 = 0;

// GLOBAL: MW2 0x100a94f8
MechS32 g_unk0x100a94f8 = 0;

// GLOBAL: MW2 0x100a94fc
MechS32 g_unk0x100a94fc = 0;

// GLOBAL: MW2 0x100a9500
MechS32 g_unk0x100a9500 = 0;

// GLOBAL: MW2 0x100a9504
MechS32 g_unk0x100a9504 = 1;

// The spinner FUN_10059085 steps through: the CP437 arrows up, left, down and right.
// GLOBAL: MW2 0x100a9508
MechChar g_unk0x100a9508[4] = {0x1e, 0x11, 0x1f, 0x10};

// GLOBAL: MW2 0x100a950c
MechS32 g_unk0x100a950c = 0;

// GLOBAL: MW2 0x100a9510
MechS32 g_unk0x100a9510 = 0;

// GLOBAL: MW2 0x100a9514
MechS32 g_unk0x100a9514 = 0;

// GLOBAL: MW2 0x100a9518
MechS32 g_unk0x100a9518 = 0;

// GLOBAL: MW2 0x100a951c
MechS32 g_unk0x100a951c = 0;

// GLOBAL: MW2 0x100a9520
MechS32 g_unk0x100a9520 = 0;

// GLOBAL: MW2 0x100a9524
MechS32 g_unk0x100a9524 = 0;

// GLOBAL: MW2 0x100a9528
MechS32 g_unk0x100a9528 = 0;

// GLOBAL: MW2 0x100a952c
MechS32 g_unk0x100a952c = 4;

// GLOBAL: MW2 0x100a9530
MechS32 g_unk0x100a9530 = 3;

// GLOBAL: MW2 0x100a9534
MechS32 g_unk0x100a9534 = 3;

// GLOBAL: MW2 0x100a9538
MechS32 g_unk0x100a9538 = 0;

// The debug build drew each text it formatted while g_unk0x100e9630 was set; the release
// build keeps only the test.
#define DRAW_DEBUG_TEXT() \
	do { \
		if (g_unk0x100e9630) { \
		} \
	} while (0)

// GLOBAL: MW2 0x100bea00
static MechS32 g_unk0x100bea00;

// GLOBAL: MW2 0x100bea04
static MechS32 g_unk0x100bea04;

// GLOBAL: MW2 0x100bea08
static MechS32 g_unk0x100bea08;

// GLOBAL: MW2 0x100bea0c
static MechS32 g_unk0x100bea0c;

// GLOBAL: MW2 0x100e9630
MechS32 g_unk0x100e9630;

// GLOBAL: MW2 0x100e9640
MechChar g_unk0x100e9640[0x50];

// GLOBAL: MW2 0x100e9690
MechChar g_unk0x100e9690;

// FUNCTION: MW2 0x10058750
void FUN_10058750(void)
{
	if (g_unk0x100a949c) {
		FUN_100588a7();
	}
	if (g_unk0x100a9504) {
		FUN_10059085();
	}

	if (g_unk0x100a94b4) {
		FUN_100589ae();
	}
	else if (g_unk0x100a94b0) {
		FUN_10058ae5();
	}

	if (g_unk0x100a94d8) {
		FUN_10058b34();
	}
	else if (g_unk0x100a94e0) {
		FUN_10058cda();
	}

	if (g_unk0x100a94e4) {
		FUN_10058d90(g_eyepoint);
	}
	else if (g_unk0x100a94ec) {
		FUN_10058ee0();
	}

	if (g_unk0x100a94f0) {
		FUN_10058fb2();
	}
	else if (g_unk0x100a94f8) {
		FUN_10059036();
	}

	if (g_unk0x100a94fc) {
		FUN_10058f3c();
	}
	else if (g_unk0x100a9500) {
		FUN_10058f78();
	}

	if (g_unk0x100a9518) {
		g_unk0x100a9514 = 1;
	}
	if (g_unk0x100a9510 && g_unk0x100a950c < g_currentClock) {
		g_unk0x100a950c = g_currentClock + 0xb5;
		FUN_1001a521(g_unk0x100a8740);
	}
}

// Draws the 256 palette colours as a grid of 16 swatches per row.
// Stack-slot permutation: x, y, color and i.
// FUNCTION: MW2 0x100588a7
void FUN_100588a7(void)
{
	MechS32 x;
	MechS32 y;
	MechS32 color;
	MechS32 i;

	x = 10;
	y = 10;
	for (color = 0; color < 0x100; color++) {
		for (i = 0; i < 2; i++) {
			FUN_100606ed(&g_currentRenderTarget, x, i + y, x + 3, i + y, 0, color);
		}

		if ((color + 1) % 16 == 0) {
			x = 10;
			y += 2;
		}
		else {
			x += 4;
		}
	}
}

// FUNCTION: MW2 0x10058958
void FUN_10058958(void)
{
	MechS32 i;

	if (FALSE) {
		g_unk0x100e9630 = TRUE;
		g_missionTimerStopped = TRUE;
	}

	for (i = 0; i < 0x50; i++) {
		g_unk0x100e9640[i] = ' ';
	}
	g_unk0x100e9690 = '\0';
}

// Stack-slot permutation: text and rate.
// FUNCTION: MW2 0x100589ae
void FUN_100589ae(void)
{
	MechChar text[64];
	MechS32 rate;

	if (!g_unk0x100e9630 && !g_unk0x100a94b8) {
		return;
	}

	g_unk0x100a9528++;
	g_unk0x100a9524 += g_deltaTime;
	if (g_unk0x100a9528 >= 10) {
		if (g_unk0x100a9524 > 0) {
			rate = (g_unk0x100a9528 * 0x712) / g_unk0x100a9524;
			g_unk0x100a951c = rate / 10;
			g_unk0x100a9520 = rate - g_unk0x100a951c * 10;
			sprintf(text, "Framerate %2.2ld.%1.1ld", g_unk0x100a951c, g_unk0x100a9520);
			DRAW_DEBUG_TEXT();
		}

		g_unk0x100a9528 = 0;
		g_unk0x100a9524 = 0;
	}

	if (g_unk0x100a94b8 && g_unk0x100a951c > 0) {
		sprintf(text, "%ld.%ld", g_unk0x100a951c, g_unk0x100a9520);
		FUN_1006f28f(0x4f, 1, text, g_unk0x100a94c0.m_x, g_unk0x100a94c0.m_y);
	}

	g_unk0x100a94b0 = 1;
}

// FUNCTION: MW2 0x10058ae5
void FUN_10058ae5(void)
{
	MechChar text[16];
	MechChar format[16];

	sprintf(format, "%%%d.%ds", 0xe, 0xe);
	sprintf(text, format, g_unk0x100a9478);
	DRAW_DEBUG_TEXT();
	g_unk0x100a94b0 = 0;
}

// FUNCTION: MW2 0x10058b34
void FUN_10058b34(void)
{
	MechChar text[80];

	if (!g_unk0x100e9630 && !g_unk0x100a94dc) {
		return;
	}

	g_unk0x100bea08 = g_unk0x100bea00 = g_unk0x100bea0c = g_unk0x100bea04 = 0;
	FUN_1003b48f(g_unk0x100ad5e8, FUN_10058d36);
	FUN_1003b48f(g_unk0x100ad5ec, FUN_10058d36);

	if (g_unk0x100e9630) {
		if (!g_unk0x100a94e0) {
			if (g_unk0x100e9630) {
				DRAW_DEBUG_TEXT();
			}

			if (g_unk0x100a94dc) {
				sprintf(
					text,
					"Objects: %d  Polygons: %d  Vertices: %d Curpolys: %d",
					g_unk0x100bea08,
					g_unk0x100bea0c,
					g_unk0x100bea00,
					g_unk0x100a2480
				);
				ShowInGameMessage(text, 1, 0xb5, 0x50);
			}

			g_unk0x100a94e0 = 1;
		}

		if (!g_unk0x100e9630) {
			return;
		}

		sprintf(text, "%4.4d", g_unk0x100bea08);
		DRAW_DEBUG_TEXT();
		sprintf(text, "%4.4d", g_unk0x100bea00);
		DRAW_DEBUG_TEXT();
		sprintf(text, "%4.4d", g_unk0x100bea0c);
		DRAW_DEBUG_TEXT();
		sprintf(text, "%4.4d", g_unk0x100bea04);
		DRAW_DEBUG_TEXT();
	}
}

// FUNCTION: MW2 0x10058cda
void FUN_10058cda(void)
{
	MechChar text[32];
	MechChar format[32];

	sprintf(format, "%%%d.%ds", 0x1e, 0x1e);
	sprintf(text, format, g_unk0x100a9478);
	DRAW_DEBUG_TEXT();
	DRAW_DEBUG_TEXT();
	g_unk0x100a94e0 = 0;
}

// Adds a shape's counts to the totals FUN_10058b34 shows.
// Stack-slot permutation: vertexCount and faceCount.
// FUNCTION: MW2 0x10058d36
void FUN_10058d36(ScarletOrchid0x4c* p_shape)
{
	MechS32 vertexCount;
	MechS32 faceCount;

	vertexCount = 0;
	faceCount = 0;
	FUN_1003b43d(p_shape, &vertexCount, &faceCount);

	g_unk0x100bea08++;
	g_unk0x100bea00 += vertexCount;
	g_unk0x100bea0c += faceCount;
	g_unk0x100bea04 += FUN_1003b7cc(p_shape);
}

// FUNCTION: MW2 0x10058d90
void FUN_10058d90(Eyepoint* p_eyepoint)
{
	MechChar text[132];

	if (!g_unk0x100a94e8 && !g_unk0x100e9630) {
		return;
	}

	sprintf(
		text,
		"txyz: %04.4ld %04.4ld %04.4ld",
		p_eyepoint->m_unk0x00,
		p_eyepoint->m_unk0x04,
		p_eyepoint->m_unk0x08
	);
	DRAW_DEBUG_TEXT();
	sprintf(
		text,
		"rxyz:  %04.4ld %04.4ld %04.4ld",
		(p_eyepoint->m_unk0x10 >> 16) % 360,
		(p_eyepoint->m_unk0x0c >> 16) % 360,
		(p_eyepoint->m_unk0x14 >> 16) % 360
	);
	DRAW_DEBUG_TEXT();

	if (g_unk0x100a94e8) {
		sprintf(
			text,
			"txyz: %5ld %5ld %5ld\nrxyz: %5d %5d %5d ",
			p_eyepoint->m_unk0x00,
			p_eyepoint->m_unk0x04,
			p_eyepoint->m_unk0x08,
			(p_eyepoint->m_unk0x10 >> 16) % 360,
			(p_eyepoint->m_unk0x0c >> 16) % 360,
			(p_eyepoint->m_unk0x14 >> 16) % 360
		);
		FUN_1006f28f(0x4f, 1, text, g_unk0x100a94d0.m_x, g_unk0x100a94d0.m_y);
	}

	g_unk0x100a94ec = 1;
}

// FUNCTION: MW2 0x10058ee0
void FUN_10058ee0(void)
{
	MechChar text[28];
	MechChar format[28];

	sprintf(format, "%%%d.%ds", 0x18, 0x18);
	sprintf(text, format, g_unk0x100a9478);
	DRAW_DEBUG_TEXT();
	DRAW_DEBUG_TEXT();
	g_unk0x100a94ec = 0;
}

// FUNCTION: MW2 0x10058f3c
void FUN_10058f3c(void)
{
	MechChar text[28];

	sprintf(text, "Items in cache: %li     ", g_cacheItemCount);
	DRAW_DEBUG_TEXT();
	g_unk0x100a9500 = 1;
}

// FUNCTION: MW2 0x10058f78
void FUN_10058f78(void)
{
	MechChar text[28];

	sprintf(text, "                           ", 0x19, 0x19);
	DRAW_DEBUG_TEXT();
	g_unk0x100a9500 = 0;
}

// Stack-slot permutation: text and value.
// FUNCTION: MW2 0x10058fb2
void FUN_10058fb2(void)
{
	MechChar text[40];
	MechS32 value;

	value = 0;
	if (!g_unk0x100a94f8) {
		DRAW_DEBUG_TEXT();
		g_unk0x100a94f8 = 1;
	}

	sprintf(text, "%8.8ld", value);
	DRAW_DEBUG_TEXT();
	if (g_unk0x100a94f4) {
		FUN_1006f28f(0x4f, 1, text, g_unk0x100a94c8.m_x, g_unk0x100a94c8.m_y);
	}
}

// FUNCTION: MW2 0x10059036
void FUN_10059036(void)
{
	MechChar text[16];
	MechChar format[16];

	sprintf(format, "%%%d.%ds", 0xc, 0xc);
	sprintf(text, format, g_unk0x100a9478);
	DRAW_DEBUG_TEXT();
	g_unk0x100a94f8 = 0;
}

// Steps the activity spinner.
// FUNCTION: MW2 0x10059085
void FUN_10059085(void)
{
	MechChar text[2];

	if (--g_unk0x100a952c == 0) {
		g_unk0x100a952c = 4;
		text[0] = g_unk0x100a9508[g_unk0x100a9530];
		text[1] = '\0';
		DRAW_DEBUG_TEXT();

		if (g_unk0x100a9530-- == 0) {
			g_unk0x100a9530 = 3;
		}
	}
}

// Formats a line for the debug console: a leading space, padded with spaces to 79 characters.
// Stack-slot permutation: i, text and dst.
// FUNCTION: MW2 0x100590ea
void FUN_100590ea(MechChar* p_text)
{
	MechS32 i;
	MechChar text[80];
	MechChar* dst;

	for (i = 0; i < 80; i++) {
		text[i] = '\0';
	}

	text[0] = ' ';
	dst = text + 1;
	strncpy(dst, p_text, 78);
	for (i = 0; i < 79; i++) {
		if (text[i] == '\0') {
			text[i] = ' ';
		}
	}

	DRAW_DEBUG_TEXT();
}

// Clears the debug console's rows 3 to 23.
// FUNCTION: MW2 0x1005917d
void FUN_1005917d(void)
{
	MechS32 i;

	for (i = 3; i <= 0x17; i++) {
		DRAW_DEBUG_TEXT();
	}

	g_unk0x100a94ac[0] = '\0';
	g_unk0x100a94a8 = 3;
	g_unk0x100a94a4 = 3;
}

// Writes text to the debug console, 80 columns and rows 3 to 23, clearing it when it fills.
// Stack-slot permutation: newline, ch and length.
// FUNCTION: MW2 0x100591d1
void FUN_100591d1(MechChar* p_text)
{
	MechS32 newline;
	MechChar ch[2];
	MechS32 i;
	MechS32 length;

	newline = FALSE;
	length = strlen(p_text);
	ch[1] = '\0';
	for (i = 0; i < length; i++) {
		if (p_text[i] == '\n') {
			newline = TRUE;
		}
		else {
			ch[0] = p_text[i];
			DRAW_DEBUG_TEXT();
		}

		g_unk0x100a9538++;
		if (g_unk0x100a9538 > 0x4f || newline) {
			g_unk0x100a9538 = 0;
			g_unk0x100a9534++;
			if (g_unk0x100a9534 > 0x17) {
				FUN_1005917d();
				g_unk0x100a9534 = 3;
			}

			newline = FALSE;
		}
	}

	g_unk0x100a94a4 = g_unk0x100a9534;
}

// FUNCTION: MW2 0x100592b0
void FUN_100592b0(void)
{
	ScalePointToScreen(&g_mainPixelBuffer, &g_unk0x100a94c0, &g_unk0x100a94c0);
	ScalePointToScreen(&g_mainPixelBuffer, &g_unk0x100a94d0, &g_unk0x100a94d0);
	ScalePointToScreen(&g_mainPixelBuffer, &g_unk0x100a94c8, &g_unk0x100a94c8);
}

// FUNCTION: MW2 0x10059300
void SimEntranceDbug(MechChar* p_mission, MechS32 p_memory)
{
	MechChar text[80];

	sprintf(
		text,
		"%s  Res:%dx%d  Mem:%ld",
		p_mission,
		g_gameWindowGeometry->m_width,
		g_gameWindowGeometry->m_height,
		p_memory
	);
	ShowInGameMessage(text, 0, 0x4f3, 0x50);

	if (!g_unk0x100e9630) {
		return;
	}

	sprintf(text, "WarThink / MechWarrior II %s %ld", p_mission, p_memory);
	FUN_100590ea(text);
}
