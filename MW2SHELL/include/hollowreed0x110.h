#ifndef HOLLOWREED0X110_H
#define HOLLOWREED0X110_H

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

private:
	MechU32 m_key;          // 0x00
	undefined4 m_length;    // 0x04
	undefined4 m_maxLength; // 0x08
	MechChar m_text[0x100]; // 0x0c
	undefined4 m_unk0x10c;  // 0x10c
};

#endif // HOLLOWREED0X110_H
