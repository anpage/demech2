#ifndef BRASSLANTERN0X414_H
#define BRASSLANTERN0X414_H

#include "decomp.h"
#include "types.h"

class VideoDriver;

// SIZE 0x414
class BrassLantern0x414 {
public:
	BrassLantern0x414(void* p_data, VideoDriver* p_videoDriver);

	MechS32 FUN_100053be(MechChar* p_text);
	MechS32 FUN_100056b9(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	MechS32 FUN_100056f5(MechS32 p_left, MechS32 p_top, MechS32 p_char, undefined* p_unk0x10);

private:
	undefined4 m_unk0x00;              // 0x00
	undefined m_unk0x04[0x40c - 0x04]; // 0x04

public:
	// EmberGlyph0x3e's constructor reads these directly: an inline accessor would leave a jmp at /Ob1.
	MechS32 m_unk0x40c;         // 0x40c
	VideoDriver* m_videoDriver; // 0x410
};

#endif // BRASSLANTERN0X414_H
