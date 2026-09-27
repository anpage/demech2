#include "frostpebble0x10.h"
#include "videodriver.h"

DECOMP_SIZE_ASSERT(FrostPebble0x10, 0x10)

// FUNCTION: MW2SHELL 0x10049145
FrostPebble0x10::FrostPebble0x10(undefined4 p_unk0x00, undefined4 p_unk0x04, undefined4 p_unk0x08, undefined4 p_unk0x0c)
{
	m_unk0x00 = p_unk0x00;
	m_unk0x04 = p_unk0x04;
	m_unk0x08 = p_unk0x08;
	m_unk0x0c = p_unk0x0c;
}

// FUNCTION: MW2SHELL 0x10049183
void FrostPebble0x10::FUN_10049183()
{
}

// FUNCTION: MW2SHELL 0x10049199
void FrostPebble0x10::FUN_10049199(VideoDriver* p_videoDriver)
{
	p_videoDriver->FUN_10006e51(m_unk0x00, m_unk0x0c, m_unk0x08, m_unk0x0c, 1);
	p_videoDriver->FUN_10006e51(m_unk0x00, m_unk0x04, m_unk0x08, m_unk0x04, 1);
	p_videoDriver->FUN_10006e51(m_unk0x08, m_unk0x04, m_unk0x08, m_unk0x0c, 1);
	p_videoDriver->FUN_10006e51(m_unk0x00, m_unk0x0c, m_unk0x00, m_unk0x04, 1);
}

// FUNCTION: MW2SHELL 0x10049245
MechU8 FrostPebble0x10::FUN_10049245(MechS32 p_x, MechS32 p_y)
{
	if (m_unk0x0c >= p_y && m_unk0x04 <= p_y && m_unk0x08 >= p_x && m_unk0x00 <= p_x) {
		return TRUE;
	}

	return FALSE;
}
