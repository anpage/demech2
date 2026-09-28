#include "hollowreed0x110.h"

#include "brasslantern0x414.h"
#include "emberglyph0x3e.h"
#include "inputdriver.h"
#include "mousestate.h"
#include "videodriver.h"

#include <ctype.h>
#include <string.h>

DECOMP_SIZE_ASSERT(HollowReed0x110, 0x110)

extern "C" MechS16 KeyboardPollKeyCode();
extern "C" MechChar* FUN_100309b6(MechChar* p_string);

extern VideoDriver* g_pVideoDriver;
extern MouseState* g_pMouseState;
extern HollowReed0x110* g_unk0x100711f8;
extern "C" MechS32 g_menuDialogOpen;

MechS32 FUN_1000fe0d();
void FUN_1001661b();

enum {
	c_keyBackspace = 0x08,
	c_keyReturn = 0x0d,
	c_keyEscape = 0x1b,
	c_keyTilde = 0x7e,
	c_keyCodeControl = 0x100,
	c_keyCodeShift = 0x200,
	c_keyCodeAlt = 0x400
};

// GLOBAL: MW2SHELL 0x10093978
MechChar g_unk0x10093978[0x100];

// FUNCTION: MW2SHELL 0x100440a0
HollowReed0x110::HollowReed0x110()
{
	m_key = 0;
	FUN_10044230(0x100);
	FUN_100440ed();
}

// FUNCTION: MW2SHELL 0x100440d7
HollowReed0x110::~HollowReed0x110()
{
}

// FUNCTION: MW2SHELL 0x100440ed
void HollowReed0x110::FUN_100440ed()
{
	g_keyboardDriver.m_unk0x1c();
	m_key = 0;
}

// FUNCTION: MW2SHELL 0x10044112
MechS32 HollowReed0x110::FUN_10044112()
{
	while (!FUN_10044189()) {
	}

	m_unk0x10c = 1;
	if (m_key == c_keyEscape) {
		return 3;
	}
	else {
		return 1;
	}
}

// FUNCTION: MW2SHELL 0x1004416a
undefined4 HollowReed0x110::FUN_1004416a()
{
	return m_unk0x10c;
}

// Returns 0 without a key, 3 for Escape and 1 for any other key.
// FUNCTION: MW2SHELL 0x10044189
MechS32 HollowReed0x110::FUN_10044189()
{
	m_key = KeyboardPollKeyCode();
	if (m_key != 0) {
		m_unk0x10c = 1;
		m_key &= ~c_keyCodeControl;
		if (m_key & c_keyCodeShift) {
			m_key &= ~c_keyCodeShift;
			m_key = toupper(m_key);
		}

		if (m_key == c_keyEscape) {
			return 3;
		}
		else {
			return 1;
		}
	}
	else {
		m_unk0x10c = 0;
		return 0;
	}
}

// FUNCTION: MW2SHELL 0x10044230
void HollowReed0x110::FUN_10044230(undefined4 p_maxLength)
{
	m_key = 0;
	m_length = 0;
	m_maxLength = p_maxLength;
	strcpy(m_text, "");
}

// FUNCTION: MW2SHELL 0x1004428e
void HollowReed0x110::FUN_1004428e(undefined4 p_maxLength, const MechChar* p_text)
{
	strcpy(m_text, p_text);
	m_key = 0;
	m_length = strlen(p_text);
	m_maxLength = p_maxLength;
}

// Returns 1 after an edit, 2 when Return copies the text out, 3 for Escape and 0 otherwise.
// FUNCTION: MW2SHELL 0x100442f7
MechS32 HollowReed0x110::FUN_100442f7(MechChar* p_text, MechU8 p_upperCase)
{
	if (FUN_10044189() == 1) {
		switch (m_key) {
		case c_keyBackspace:
			m_length--;
			if (m_length < 0) {
				m_length = 0;
			}

			m_text[m_length] = '\0';
			return 1;
		case c_keyReturn:
			m_text[m_length] = '\0';
			strcpy(p_text, m_text);
			return 2;
		case c_keyEscape:
			return 3;
		default:
			if (m_maxLength - 1 == m_length) {
				return 0;
			}

			if (m_key & c_keyCodeAlt) {
				return 0;
			}

			m_text[m_length] = m_key;
			m_length++;
			m_text[m_length] = '\0';
			if (p_upperCase == TRUE) {
				FUN_100309b6(m_text);
			}

			return 1;
		}
	}

	return 0;
}

// Edits p_text with a cursor ('_') drawn after it. Return, a click or the window closing store
// the text and return 1; Escape stores it and returns 0.
// Stack-slot permutation: key and glyph. The p_maxLength == length comparison also loads its
// operands in the opposite order, and swapping them in the source doesn't change the output.
// FUNCTION: MW2SHELL 0x10044451
MechS32 FUN_10044451(
	BrassLantern0x414* p_font,
	MechS32 p_left,
	MechS32 p_top,
	MechChar* p_text,
	undefined* p_colors,
	MechS32 p_maxLength,
	MechS32 p_width
)
{
	MechS32 key;
	MechS32 width;
	MechS32 length;
	EmberGlyph0x3e* glyph;

	glyph = NULL;
	length = strlen(p_text);
	strcpy(g_unk0x10093978, p_text);
	strcat(g_unk0x10093978, "_");
	width = p_font->FUN_100053be(g_unk0x10093978);
	glyph = p_font->FUN_10005522(p_left, p_top, g_unk0x10093978, p_colors);

	for (;;) {
		FUN_1001661b();
		g_pVideoDriver->DrawShell();
		g_pMouseState->ReadMouseState();

		if (!FUN_1000fe0d() || g_menuDialogOpen || g_pMouseState->GetLeftPressed() == 1) {
			g_unk0x10093978[length] = '\0';
			if (glyph) {
				delete glyph;
			}

			strcpy(p_text, g_unk0x10093978);
			return 1;
		}

		if (g_unk0x100711f8->FUN_10044189()) {
			switch (g_unk0x100711f8->m_key) {
			case c_keyBackspace:
				if (length == 0) {
					break;
				}

				length--;
				g_unk0x10093978[length] = '_';
				g_unk0x10093978[length + 1] = '\0';
				if (glyph) {
					delete glyph;
				}

				glyph = p_font->FUN_10005522(p_left, p_top, g_unk0x10093978, p_colors);
				break;
			case c_keyReturn:
				g_unk0x10093978[length] = '\0';
				if (glyph) {
					delete glyph;
				}

				strcpy(p_text, g_unk0x10093978);
				return 1;
			case c_keyEscape:
				g_unk0x10093978[length] = '\0';
				if (glyph) {
					delete glyph;
				}

				strcpy(p_text, g_unk0x10093978);
				return 0;
			default:
				key = g_unk0x100711f8->m_key;
				if (key < 0x20 || key > 0x7f || key == c_keyTilde) {
					break;
				}

				if (p_maxLength == length) {
					break;
				}

				if (!p_font->FUN_10005424(key)) {
					break;
				}

				g_unk0x10093978[length] = key;
				length++;
				g_unk0x10093978[length] = '_';
				g_unk0x10093978[length + 1] = '\0';

				if (p_font->FUN_100053be(g_unk0x10093978) < p_width) {
					if (glyph) {
						delete glyph;
					}

					glyph = p_font->FUN_10005522(p_left, p_top, g_unk0x10093978, p_colors);
				}
				else {
					length--;
					g_unk0x10093978[length] = '_';
					g_unk0x10093978[length + 1] = '\0';
				}
				break;
			}
		}
	}
}
