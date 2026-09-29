#include "unk10036230.h"

#include "decomp.h"
#include "simmain.h"
#include "slateheron.h"
#include "transform.h"
#include "types.h"

// FUNCTION: MW2 0x10036853
void FUN_10036853(MechU32 p_flags)
{
	g_unk0x100a6cc8.m_unk0x50 ^= p_flags;
}

// FUNCTION: MW2 0x10036867
MechS32 FUN_10036867(MechU32 p_flags)
{
	return !(p_flags & g_unk0x100a6cc8.m_unk0x50);
}

// FUNCTION: MW2 0x10036891
void FUN_10036891(MechU32 p_flags, MechS32 p_enable)
{
	if (p_enable) {
		g_unk0x100a6cc8.m_unk0x50 &= ~p_flags;
	}
	else {
		g_unk0x100a6cc8.m_unk0x50 |= p_flags;
	}
}

// FUNCTION: MW2 0x100368bf
MechS32 FUN_100368bf(undefined4 p_unk0x00)
{
	return !g_unk0x100a6cc8.m_unk0x4c;
}

// FUNCTION: MW2 0x100368e8
void FUN_100368e8(undefined4 p_unk0x00, MechS32 p_enable)
{
	if (!p_enable) {
		g_unk0x100a6cc8.m_unk0x4c = TRUE;
	}
	else {
		g_unk0x100a6cc8.m_unk0x4c = FALSE;
	}
}

// STUB: MW2 0x10039a30
void FUN_10039a30(GraniteLattice0x18* p_model, Matrix* p_matrix)
{
	STUB(0x10039a30);
}

// STUB: MW2 0x10039b94
void FUN_10039b94(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix)
{
	STUB(0x10039b94);
}
