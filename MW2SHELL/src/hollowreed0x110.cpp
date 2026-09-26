#include "hollowreed0x110.h"

#include "inputdriver.h"

#include <ctype.h>
#include <string.h>

DECOMP_SIZE_ASSERT(HollowReed0x110, 0x110)

extern "C" MechS16 KeyboardPollKeyCode();

enum {
	c_keyEscape = 0x1b,
	c_keyCodeControl = 0x100,
	c_keyCodeShift = 0x200
};

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
