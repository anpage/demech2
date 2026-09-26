#ifndef EMBERGLYPH0X3E_H
#define EMBERGLYPH0X3E_H

#include "decomp.h"
#include "types.h"

class BrassLantern0x414;
class VideoDriver;

// A line of text drawn with a BrassLantern0x414, either at once or one character per step.
// SIZE 0x3e
#pragma pack(1)
class EmberGlyph0x3e {
public:
	EmberGlyph0x3e(MechChar* p_text, MechS32 p_left, MechS32 p_top, undefined* p_unk0x04, BrassLantern0x414* p_unk0x00);
	~EmberGlyph0x3e();

	void FUN_10047404(undefined4 p_unk0x10);
	void FUN_10047425();
	void FUN_100474ab();
	MechU8 FUN_100474f0();
	MechU8 FUN_1004795e();
	void Shutdown();

private:
	BrassLantern0x414* m_unk0x00; // 0x00
	undefined* m_unk0x04;         // 0x04
	undefined* m_unk0x08;         // 0x08
	VideoDriver* m_videoDriver;   // 0x0c
	undefined4 m_unk0x10;         // 0x10
	MechU8 m_unk0x14;             // 0x14
	undefined4 m_unk0x15;         // 0x15
	MechChar* m_text;             // 0x19
	MechS32 m_height;             // 0x1d
	MechS32 m_width;              // 0x21
	MechS32 m_left;               // 0x25
	MechS32 m_top;                // 0x29
	MechS32 m_right;              // 0x2d
	MechS32 m_bottom;             // 0x31
	MechU8 m_unk0x35;             // 0x35
	MechS32 m_cursorX;            // 0x36
	MechS32 m_textIndex;          // 0x3a
};
#pragma pack()

#endif // EMBERGLYPH0X3E_H
