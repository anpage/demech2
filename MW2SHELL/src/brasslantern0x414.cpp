#include "brasslantern0x414.h"

#include "videodriver.h"

DECOMP_SIZE_ASSERT(BrassLantern0x414, 0x414)

// STUB: MW2SHELL 0x10005340
BrassLantern0x414::BrassLantern0x414(void* p_data, VideoDriver* p_videoDriver)
{
	STUB(0x10005340);
}

// STUB: MW2SHELL 0x100053be
MechS32 BrassLantern0x414::FUN_100053be(MechChar* p_text)
{
	STUB(0x100053be);
	return 0;
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
