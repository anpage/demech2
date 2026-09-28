#include "unk1002dc60.h"

#include "brasslantern0x414.h"
#include "cedarknot0x10.h"
#include "collection.h"
#include "decomp.h"
#include "page.h"
#include "tinwhistle0x3c.h"
#include "types.h"
#include "unk10030900.h"
#include "unk1003bf90.h"
#include "unk100711f8.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The index of the HTXT tag in g_unk0x100668c0 (cedarknot0x10.cpp).
enum {
	c_tagHtxt = 62
};

// The colors of the pages FUN_1002e1b1 lays out.
// GLOBAL: MW2SHELL 0x1008f658
MechU8 g_unk0x1008f658[0x100];

// The text with its escapes expanded: \\Q the quote, \\R0 to \\R2 the pilot's rank and the next
// two, \\H the pilot's honor. The original also writes the unexpanded text to tmp.out.
// GLOBAL: MW2SHELL 0x1008d658
MechChar g_unk0x1008d658[0x2000];

// Expands the escapes of p_text into g_unk0x1008d658 and returns a copy of the result.
// Not 100%: the stack slots of the locals are permuted (i and length move past the buffer, which
// lengthens their encodings).
// FUNCTION: MW2SHELL 0x1002dc60
MechChar* FUN_1002dc60(MechChar* p_text, MechChar* p_quote)
{
	MechS32 i;
	MechS32 length;
	MechChar number[0x80];
	MechS32 textLength;
	MechS32 rank;
	FILE* file;

	textLength = strlen(p_text);
	i = 0;
	length = 0;
	while (i < textLength) {
		if (p_text[i] == '\\') {
			i++;
			switch (p_text[i]) {
			case 'Q':
			case 'q':
				i++;
				g_unk0x1008d658[length] = '\0';
				strcat(g_unk0x1008d658, p_quote);
				length = strlen(g_unk0x1008d658);
				break;
			case 'R':
			case 'r':
				i++;
				g_unk0x1008d658[length] = '\0';
				switch (p_text[i]) {
				case '0':
					i++;
					rank = g_pCurrentPilot->m_rank;
					strcat(g_unk0x1008d658, g_rankNames[rank]);
					length = strlen(g_unk0x1008d658);
					break;
				case '1':
					i++;
					rank = g_pCurrentPilot->m_rank + 1;
					if (rank > 9) {
						rank = 9;
					}
					strcat(g_unk0x1008d658, g_rankNames[rank]);
					length = strlen(g_unk0x1008d658);
					break;
				case '2':
					i++;
					rank = g_pCurrentPilot->m_rank + 2;
					if (rank > 9) {
						rank = 9;
					}
					strcat(g_unk0x1008d658, g_rankNames[rank]);
					length = strlen(g_unk0x1008d658);
					break;
				default:
					break;
				}
				break;
			case 'H':
			case 'h':
				i++;
				g_unk0x1008d658[length] = '\0';
				sprintf(number, "%d", g_pCurrentPilot->m_honor);
				strcat(g_unk0x1008d658, number);
				length = strlen(g_unk0x1008d658);
				break;
			default:
				g_unk0x1008d658[length] = '\\';
				length++;
				g_unk0x1008d658[length] = p_text[i];
				i++;
				length++;
				break;
			}
		}
		else {
			g_unk0x1008d658[length] = p_text[i];
			i++;
			length++;
		}
	}

	g_unk0x1008d658[length] = '\0';
	file = fopen("tmp.out", "wb");
	fwrite(p_text, 1, strlen(p_text), file);
	fclose(file);
	return FUN_10030900(g_unk0x1008d658);
}

// Lays out p_text (p_size bytes, the last of which becomes its terminator) on as many pages as
// it takes, adding them to p_pages. With p_quote, the text's escapes are expanded first.
// Not 100%: the stack slots of page, text and the new temporaries are permuted.
// FUNCTION: MW2SHELL 0x1002e09b
void FUN_1002e09b(
	MechChar* p_text,
	MechS32 p_size,
	Collection* p_pages,
	BrassLantern0x414* p_font,
	undefined* p_colors,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height,
	MechChar* p_quote
)
{
	Page* page;
	MechChar* text;

	p_text[p_size - 1] = '\0';
	if (p_quote) {
		text = FUN_1002dc60(p_text, p_quote);
	}
	else {
		text = p_text;
	}

	do {
		page = new Page(p_font, g_pVideoDriver, p_colors, p_left, p_top, p_width, p_height);
		text = page->Layout(text);
		ExpandCollection(p_pages, page);
	} while (text);

	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, text);
}

// Lays out the text (HTXT node) of the project file p_name on pages, in colors that draw 0
// transparent and everything else in color 1.
// Not 100%: the stack slots of node and i are permuted.
// FUNCTION: MW2SHELL 0x1002e1b1
void FUN_1002e1b1(
	Collection* p_pages,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height,
	MechChar* p_name,
	BrassLantern0x414* p_font,
	MechChar* p_quote
)
{
	MechS32* node = NULL;
	MechS32 i;

	g_unk0x1008f658[0] = 0xff;
	g_unk0x1008f658[1] = 1;
	for (i = 2; i < 0x100; i++) {
		g_unk0x1008f658[i] = 0xff;
	}

	if (!g_unk0x10071230->FUN_1002e512(p_name)) {
		// The original does nothing about a missing file.
	}

	node = g_unk0x10071230->FUN_1002e5d8(*(MechS32*) g_unk0x100668c0[c_tagHtxt]);
	if (node) {
		FUN_1002e09b(
			(MechChar*) (node + 2),
			node[1] - 8,
			p_pages,
			p_font,
			g_unk0x1008f658,
			p_left,
			p_top,
			p_width,
			p_height,
			p_quote
		);
	}
}
