#include "textglyph.h"

#include "decomp.h"
#include "font.h"
#include "menudata.h"
#include "stringutil.h"
#include "textglyph.h"
#include "types.h"
#include "unk1003bf90.h"
#include "videodriver.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(TextGlyph, 0x3e)

// Color remap tables for Font text, selected by the "\A" text escape.
// GLOBAL: MW2SHELL 0x10074658
undefined g_unk0x10074658[256] = {0xff, 1};

// GLOBAL: MW2SHELL 0x10074758
undefined g_unk0x10074758[256] = {0xff, 5};

// GLOBAL: MW2SHELL 0x10074858
undefined g_unk0x10074858[256] = {0xff, 8};

// FUNCTION: MW2SHELL 0x10047370
void FUN_10047370()
{
	MechS32 i;

	g_unk0x10074758[0] = g_unk0x10074658[0] = g_unk0x10074858[0] = 0xff;
	for (i = 1; i < 256; i++) {
		g_unk0x10074758[i] = g_unk0x10074658[i] = g_unk0x10074858[i] = i;
	}

	g_unk0x10074758[1] = 5;
	g_unk0x10074658[1] = 1;
	g_unk0x10074858[1] = 8;
}

// FUNCTION: MW2SHELL 0x10047404
void TextGlyph::FUN_10047404(undefined4 p_unk0x10)
{
	m_unk0x10 = p_unk0x10;
}

// FUNCTION: MW2SHELL 0x10047425
void TextGlyph::FUN_10047425()
{
	if (m_text != NULL && strlen(m_text) != 0) {
		if (m_unk0x10 == 1) {
			FUN_100474ab();
		}
		else {
			m_unk0x00->FUN_100056b9(m_left, m_top, m_text, m_unk0x04);
			m_unk0x35 = 1;
		}
	}
}

// FUNCTION: MW2SHELL 0x100474ab
void TextGlyph::FUN_100474ab()
{
	MechS32 textIndex;

	textIndex = m_textIndex;
	m_textIndex = -1;
	while (m_textIndex < textIndex) {
		FUN_100474f0();
	}
}

// Draws the next character of the text, or handles the escape sequence starting at it.
// FUNCTION: MW2SHELL 0x100474f0
MechU8 TextGlyph::FUN_100474f0()
{
	MechS32 i;
	MechChar number[0x80];

	if (!m_unk0x14) {
		m_videoDriver->FUN_100076e8(this, 1);
		m_unk0x14 = 1;
	}

	if (m_textIndex < 0) {
		m_unk0x08 = m_unk0x04;
		m_textIndex = 0;
		m_cursorX = m_left;
	}

	if (strlen(m_text) <= m_textIndex) {
		m_unk0x08 = m_unk0x04;
		m_unk0x35 = 1;
		return TRUE;
	}

	if (m_text[m_textIndex] == '\\') {
		m_unk0x08 = m_unk0x04;
		m_textIndex++;

		if (strlen(m_text) > m_textIndex) {
			switch (m_text[m_textIndex]) {
			case 'T':
			case 't':
				m_textIndex++;
				for (i = 0; i < 19; i++) {
					if (m_cursorX - m_left < g_unk0x1006e150[i]) {
						m_cursorX = g_unk0x1006e150[i] + m_left;
						break;
					}
				}
				break;
			case 'B':
			case 'b':
				m_textIndex++;
				number[0] = m_text[m_textIndex];
				m_textIndex++;
				number[1] = m_text[m_textIndex];
				m_textIndex++;
				number[2] = m_text[m_textIndex];
				m_textIndex++;
				number[3] = '\0';
				m_cursorX -= atoi(number);
				break;
			case 'G':
			case 'g':
				m_textIndex++;
				number[0] = m_text[m_textIndex];
				m_textIndex++;
				number[1] = m_text[m_textIndex];
				m_textIndex++;
				number[2] = m_text[m_textIndex];
				m_textIndex++;
				number[3] = '\0';
				m_cursorX = m_left + atoi(number);
				break;
			case 'A':
			case 'a':
				m_textIndex++;
				m_unk0x08 = g_unk0x10074658;
				break;
			default:
				break;
			}
		}
	}
	else {
		if (m_text[m_textIndex] == ' ') {
			m_unk0x08 = m_unk0x04;
		}

		m_cursorX += m_unk0x00->FUN_100056f5(m_cursorX, m_top, m_text[m_textIndex], m_unk0x08);
		m_textIndex++;
	}

	if (m_cursorX - m_left + 1 > m_width) {
		m_width = m_cursorX - m_left + 1;
	}

	return FALSE;
}

// FUNCTION: MW2SHELL 0x1004795e
MechU8 TextGlyph::FUN_1004795e()
{
	if (m_unk0x35 == 1) {
		return TRUE;
	}

	if (strlen(m_text) == 0) {
		return TRUE;
	}

	return FUN_100474f0();
}

// Operand order: the original computes m_right from m_left + m_width with m_left loaded first; it
// follows the unit's symbol table.
// FUNCTION: MW2SHELL 0x100479b7
TextGlyph::TextGlyph(MechChar* p_text, MechS32 p_left, MechS32 p_top, undefined* p_unk0x04, Font* p_unk0x00)
{
	m_unk0x00 = p_unk0x00;
	m_videoDriver = p_unk0x00->m_videoDriver;

	if (p_text == NULL) {
		p_text = "";
	}

	// A leading '~' centers the text on p_left.
	if (*p_text == '~') {
		m_text = FUN_10030900(p_text + 1);
	}
	else {
		m_text = FUN_10030900(p_text);
	}

	m_width = m_unk0x00->FUN_100053be(m_text);
	m_height = m_unk0x00->m_unk0x40c;

	if (*p_text == '~') {
		m_left = p_left - m_width / 2;
	}
	else {
		m_left = p_left;
	}

	m_top = p_top;
	m_right = m_left + m_width;
	m_bottom = m_top + m_height;
	m_unk0x35 = 0;
	m_cursorX = -1;
	m_textIndex = -1;
	m_unk0x10 = 0;
	m_unk0x14 = 0;
	m_unk0x15 = 0;
	m_unk0x04 = p_unk0x04;
	m_unk0x08 = p_unk0x04;
}

// Restores the background under the text and rewinds it.
// FUNCTION: MW2SHELL 0x10047b03
void TextGlyph::Shutdown()
{
	m_unk0x35 = 0;
	m_cursorX = -1;
	m_textIndex = -1;
	m_videoDriver->FUN_1000772d(this);
	m_unk0x14 = 0;
	m_videoDriver->FUN_100071ad(m_left, m_top, m_width, m_height);
}

// FUNCTION: MW2SHELL 0x10047b71
TextGlyph::~TextGlyph()
{
	Shutdown();

	if (m_text != NULL) {
		HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_text);
	}
}
