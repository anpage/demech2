#include "brasslantern0x414.h"

#include "emberglyph0x3e.h"
#include "shellmain.h"
#include "videodriver.h"

DECOMP_SIZE_ASSERT(BrassLantern0x414, 0x414)

// Font data readers: assembly routines in unk10032250.c.
extern "C"
{
	MechS32 FUN_10036aa9(void* p_data);
	MechS32 FUN_10036abc(void* p_data, MechS32 p_char);
}

// FUNCTION: MW2SHELL 0x10005340
BrassLantern0x414::BrassLantern0x414(void* p_data, VideoDriver* p_videoDriver)
{
	m_videoDriver = p_videoDriver;
	m_unk0x00 = p_data;
	m_unk0x408 = m_unk0x00;
	m_unk0x40c = FUN_10036aa9(m_unk0x00);
}

// Nothing in the shell deletes a BrassLantern0x414, so the destructor has no callers.
// FUNCTION: MW2SHELL 0x10005394
BrassLantern0x414::~BrassLantern0x414()
{
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_unk0x00);
}

// FUNCTION: MW2SHELL 0x100053be
MechS32 BrassLantern0x414::FUN_100053be(MechChar* p_text)
{
	MechS32 width;

	if (p_text == NULL) {
		return 0;
	}

	width = 0;
	for (; *p_text != '\0'; p_text++) {
		width += FUN_10036abc(m_unk0x00, *p_text);
	}

	return width;
}

// FUNCTION: MW2SHELL 0x10005424
MechS32 BrassLantern0x414::FUN_10005424(MechS32 p_char)
{
	return FUN_10036abc(m_unk0x00, p_char);
}

// FUNCTION: MW2SHELL 0x10005522
EmberGlyph0x3e* BrassLantern0x414::FUN_10005522(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10)
{
	EmberGlyph0x3e* glyph;

	if (p_text == NULL) {
		p_text = "";
	}

	glyph = new EmberGlyph0x3e(p_text, p_left, p_top, p_unk0x10, this);
	m_videoDriver->FUN_100076e8(glyph, 1);
	glyph->FUN_10047425();
	return glyph;
}

// FUNCTION: MW2SHELL 0x100056b9
MechS32 BrassLantern0x414::FUN_100056b9(MechS32 p_left, MechS32 p_top, MechChar* p_text, undefined* p_unk0x10)
{
	return m_videoDriver->FUN_100074d2(p_left, p_top, m_unk0x00, p_text, p_unk0x10);
}

// FUNCTION: MW2SHELL 0x100056f5
MechS32 BrassLantern0x414::FUN_100056f5(MechS32 p_left, MechS32 p_top, MechS32 p_char, undefined* p_unk0x10)
{
	return m_videoDriver->FUN_10007603(p_left, p_top, m_unk0x00, p_char, p_unk0x10);
}
