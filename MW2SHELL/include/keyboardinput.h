#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H

#include "decomp.h"
#include "font.h"
#include "types.h"

// Keyboard input for the shell: the last key code polled, and a line of edited text.
// SIZE 0x110
class KeyboardInput {
public:
	KeyboardInput();
	~KeyboardInput();

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

// The functions and globals of keyboardinput.cpp that other units use.
MechS32 FUN_10044451(
	Font* p_font,
	MechS32 p_left,
	MechS32 p_top,
	MechChar* p_text,
	undefined* p_colors,
	MechS32 p_maxLength,
	MechS32 p_width
);

#endif // KEYBOARDINPUT_H
