#include "unk1006f480.h"

#include "anim2d.h"
#include "decomp.h"
#include "resource.h"
#include "simmain.h"
#include "types.h"

// Loads eight sounds ahead of their use.
// FUNCTION: MW2 0x1006f480
void FUN_1006f480(void)
{
	MechS32 ids[8];
	MechU32 i;

	ids[0] = 0xbd;
	ids[1] = 0xf7;
	ids[2] = 0xdb;
	ids[3] = 0xf0;
	ids[4] = 0xf6;
	ids[5] = 0xcf;
	ids[6] = 0xce;
	ids[7] = 0xfe;
	for (i = 0; i < 8; i++) {
		FUN_10050862(ids[i], g_unk0x100a8674);
	}
}

// STUB: MW2 0x1006fba3
void FUN_1006fba3(void)
{
	STUB(0x1006fba3);
}

// STUB: MW2 0x1006ff7b
void FUN_1006ff7b(void)
{
	STUB(0x1006ff7b);
}

// STUB: MW2 0x1007005a
void FUN_1007005a(struct Mech* p_mech)
{
	STUB(0x1007005a);
}

// STUB: MW2 0x100704c1
void FUN_100704c1(void)
{
	STUB(0x100704c1);
}

// FUNCTION: MW2 0x1007079d
void FUN_1007079d(RenderTarget* p_target, MechS32 p_index, MechS32 p_x, MechS32 p_y)
{
	DrawAnim2d(p_target, p_index, p_x, p_y);
}
