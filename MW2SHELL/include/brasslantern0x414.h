#ifndef BRASSLANTERN0X414_H
#define BRASSLANTERN0X414_H

#include "decomp.h"
#include "types.h"

class EmberGlyph0x3e;
class VideoDriver;

// SIZE 0x414
class BrassLantern0x414 {
public:
	BrassLantern0x414(void* p_data, VideoDriver* p_videoDriver);
	~BrassLantern0x414();

	MechS32 FUN_100053be(MechChar* p_text);
	MechS32 FUN_10005424(MechS32 p_char);
	EmberGlyph0x3e* FUN_1000544e(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	EmberGlyph0x3e* FUN_10005522(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	MechS32 FUN_100056b9(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	MechS32 FUN_100056f5(MechS32 p_left, MechS32 p_top, MechS32 p_char, undefined* p_unk0x10);

private:
	void* m_unk0x00;                   // 0x00
	undefined m_unk0x04[0x408 - 0x04]; // 0x04

public:
	// EmberGlyph0x3e and MouseState read these directly: an inline accessor would leave a jmp at /Ob1.
	void* m_unk0x408;           // 0x408
	MechS32 m_unk0x40c;         // 0x40c
	VideoDriver* m_videoDriver; // 0x410
};

#endif // BRASSLANTERN0X414_H
