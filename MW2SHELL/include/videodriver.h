#ifndef VIDEODRIVER_H
#define VIDEODRIVER_H

#include "decomp.h"
#include "pixelbuffer.h"
#include "pixelview.h"
#include "types.h"

class CopperLedger0x04;
class EmberGlyph0x3e;
class TMPackDataBase;

#pragma pack(1)
// SIZE 0x3ae
class VideoDriver {
public:
	VideoDriver();
	~VideoDriver();

	void ExpandRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	void ExpandRectBySize(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	MechS32 IntersectsRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	MechS32 IntersectsRectBySize(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	void UpdatePalette();
	void DrawShell();
	void DrawFmv();
	void GetPalette(undefined* p_palette);
	void SetPalette(undefined* p_palette, undefined4 p_unk0x22);
	void FUN_10006a6d(undefined* p_data, MechS32 p_size, undefined* p_palette);
	void FUN_10006a99(MechS32* p_maxX, MechS32* p_maxY, undefined* p_data, MechS32 p_size, MechS32 p_type);
	void FUN_10006b21(undefined* p_unk0x0a, MechS32 p_unk0x0e, MechS32 p_unk0x12, MechU8 p_unk0x08, MechU8 p_unk0x09);
	void FUN_10006c50(TMPackDataBase* p_database, MechS32 p_id);
	void FUN_10006da9(undefined* p_data, MechS32 p_type, PixelView* p_view);
	void FUN_10006e51(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechS32 p_color);
	void FUN_10006ed4(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	void FUN_10006f87(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	void FUN_1000705f(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	void FUN_10007112(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	void FUN_100071ad(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height);
	void FUN_1000725d();
	void FUN_10007293();
	void LoadPalette(MechS32 p_id);
	void FUN_100073b3(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_width,
		MechS32 p_height
	);
	void FUN_10007430(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_width,
		MechS32 p_height
	);
	MechS32 FUN_100074d2(MechS32 p_left, MechS32 p_top, undefined4 p_unk0x08, MechChar* p_text, undefined* p_unk0x10);
	MechS32 FUN_10007603(MechS32 p_left, MechS32 p_top, undefined4 p_unk0x08, MechS32 p_char, undefined* p_unk0x10);
	void FUN_100076e8(EmberGlyph0x3e* p_item, MechS32 p_unk0x16);
	void FUN_1000772d(EmberGlyph0x3e* p_item);
	void FUN_10007763(MechS32 p_unk0x16);
	void FUN_100077b4(MechU8 p_delete);
	void ActivateFramebuffer();

private:
	MechS32 m_unk0x00;                // 0x00
	undefined4 m_unk0x04;             // 0x04
	MechU8 m_unk0x08;                 // 0x08
	MechU8 m_unk0x09;                 // 0x09
	undefined* m_unk0x0a;             // 0x0a
	MechS32 m_unk0x0e;                // 0x0e
	MechS32 m_unk0x12;                // 0x12
	CopperLedger0x04* m_unk0x16;      // 0x16
	CopperLedger0x04* m_unk0x1a;      // 0x1a
	undefined4 m_unk0x1e;             // 0x1e
	undefined4 m_unk0x22;             // 0x22
	undefined m_unk0x26[0x2e - 0x26]; // 0x26
	PixelBuffer m_screenBuffer;       // 0x2e

public:
	// ShellWindowProc clears m_backBuffer.m_pixels directly: an inline accessor would leave a jmp at /Ob1.
	PixelBuffer m_backBuffer; // 0x42

private:
	PixelView m_screenView;     // 0x56
	PixelView m_backView;       // 0x6a
	PixelView m_dirtyView;      // 0x7e
	undefined m_palette[0x300]; // 0x92
	MechS32 m_width;            // 0x392
	MechS32 m_height;           // 0x396
	MechS32 m_unk0x39a;         // 0x39a
	MechS32 m_unk0x39e;         // 0x39e
	undefined4 m_unk0x3a2;      // 0x3a2
	MechS32 m_unk0x3a6;         // 0x3a6
	undefined4 m_unk0x3aa;      // 0x3aa
};
#pragma pack()

#endif // VIDEODRIVER_H
