#ifndef TEXTGLYPH_H
#define TEXTGLYPH_H

#include "decomp.h"
#include "types.h"

class Font;
class VideoDriver;

// A line of text drawn with a Font, either at once or one character per step.
// SIZE 0x3e
#pragma pack(1)
class TextGlyph {
public:
	TextGlyph(MechChar* p_text, MechS32 p_left, MechS32 p_top, undefined* p_unk0x04, Font* p_unk0x00);
	~TextGlyph();

	void FUN_10047404(undefined4 p_unk0x10);
	void FUN_10047425();
	void FUN_100474ab();
	MechU8 FUN_100474f0();
	MechU8 FUN_1004795e();
	void Shutdown();

private:
	Font* m_unk0x00;            // 0x00
	undefined* m_unk0x04;       // 0x04
	undefined* m_unk0x08;       // 0x08
	VideoDriver* m_videoDriver; // 0x0c
	undefined4 m_unk0x10;       // 0x10
	MechU8 m_unk0x14;           // 0x14
	undefined4 m_unk0x15;       // 0x15
	MechChar* m_text;           // 0x19

public:
	// The shell's field tables read the size directly: an inline accessor would leave a jmp at /Ob1.
	MechS32 m_height; // 0x1d
	MechS32 m_width;  // 0x21

private:
	MechS32 m_left; // 0x25
	MechS32 m_top;  // 0x29

public:
	// The cockpit controls screen places a field after the previous one's glyph.
	MechS32 m_right; // 0x2d

private:
	MechS32 m_bottom; // 0x31

public:
	// Page restarts the typing and checks for the end directly.
	MechU8 m_unk0x35;    // 0x35
	MechS32 m_cursorX;   // 0x36
	MechS32 m_textIndex; // 0x3a
};
#pragma pack()

// The functions and globals of textglyph.cpp that other units use.
void FUN_10047370();

#endif // TEXTGLYPH_H
