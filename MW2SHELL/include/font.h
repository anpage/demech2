#ifndef FONT_H
#define FONT_H

#include "decomp.h"
#include "types.h"

class TextGlyph;
class VideoDriver;

// SIZE 0x414
class Font {
public:
	Font(void* p_data, VideoDriver* p_videoDriver);
	~Font();

	MechS32 FUN_100053be(MechChar* p_text);
	MechS32 FUN_10005424(MechS32 p_char);
	TextGlyph* FUN_1000544e(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	TextGlyph* FUN_10005522(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	MechS32 FUN_100056b9(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	TextGlyph* FUN_100055f6(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10);
	MechS32 FUN_100056f5(MechS32 p_left, MechS32 p_top, MechS32 p_char, undefined* p_unk0x10);
	void FUN_10005731(MechS32 p_left, MechS32 p_top, MechS32 p_key, undefined* p_unk0x10);
	void FUN_10005913();

private:
	void* m_unk0x00;          // 0x00
	MechS32 m_unk0x04[0x100]; // 0x04
	MechS32 m_unk0x404;       // 0x404

public:
	// TextGlyph and MouseState read these directly: an inline accessor would leave a jmp at /Ob1.
	void* m_unk0x408;           // 0x408
	MechS32 m_unk0x40c;         // 0x40c
	VideoDriver* m_videoDriver; // 0x410
};

#endif // FONT_H
