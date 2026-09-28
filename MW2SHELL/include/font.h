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

	MechS32 GetTextWidth(MechChar* p_text);
	MechS32 GetCharacterWidth(MechS32 p_char);
	TextGlyph* FUN_1000544e(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_colors);
	TextGlyph* FUN_10005522(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_colors);
	MechS32 DrawString(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_colors);
	TextGlyph* FUN_100055f6(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_colors);
	MechS32 DrawChar(MechS32 p_left, MechS32 p_top, MechS32 p_char, undefined* p_colors);
	void TypeKey(MechS32 p_left, MechS32 p_top, MechS32 p_key, undefined* p_colors);
	void ResetTyping();

private:
	void* m_data;                // 0x00
	MechS32 m_typedRight[0x100]; // 0x04
	MechS32 m_typedCount;        // 0x404

public:
	// TextGlyph and MouseState read these directly: an inline accessor would leave a jmp at /Ob1.
	void* m_unk0x408;           // 0x408
	MechS32 m_height;           // 0x40c
	VideoDriver* m_videoDriver; // 0x410
};

#endif // FONT_H
