#ifndef HOLLOWREED0X110_H
#define HOLLOWREED0X110_H

#include "brasslantern0x414.h"
#include "decomp.h"
#include "types.h"

// Keyboard input for the shell: the last key code polled, and a line of edited text.
// SIZE 0x110
class HollowReed0x110 {
public:
	HollowReed0x110();
	~HollowReed0x110();

	void FUN_100440ed();
	MechS32 FUN_10044112();
	undefined4 FUN_1004416a();
	MechS32 FUN_10044189();
	void FUN_10044230(undefined4 p_maxLength);
	void FUN_1004428e(undefined4 p_maxLength, const MechChar* p_text);
	MechS32 FUN_100442f7(MechChar* p_text, MechU8 p_upperCase);

	// The text entry loop reads the key directly: an inline accessor would leave a jmp at /Ob1.
	MechU32 m_key; // 0x00

private:
	MechS32 m_length;       // 0x04
	MechS32 m_maxLength;    // 0x08
	MechChar m_text[0x100]; // 0x0c
	undefined4 m_unk0x10c;  // 0x10c
};

// The functions and globals of hollowreed0x110.cpp that other units use.
MechS32 FUN_10044451(
	BrassLantern0x414* p_font,
	MechS32 p_left,
	MechS32 p_top,
	MechChar* p_text,
	undefined* p_colors,
	MechS32 p_maxLength,
	MechS32 p_width
);

#endif // HOLLOWREED0X110_H
