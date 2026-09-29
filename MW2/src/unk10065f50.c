#include "unk10065f50.h"

#include "ai.h"
#include "decomp.h"
#include "menu.h"
#include "team.h"
#include "types.h"

// GLOBAL: MW2 0x100acaf0
MechS32 g_unk0x100acaf0[8] = {7, 7, 7, 7, 7, 0, 0, 0};

// FUNCTION: MW2 0x100661ef
MechS32 FUN_100661ef(MechS32 p_index)
{
	MechS32 state;

	state = 7;
	if (p_index < 8) {
		state = g_unk0x100acaf0[p_index];
	}

	return state;
}

// FUNCTION: MW2 0x10066223
MechS32 FUN_10066223(void)
{
	return GetTeamFormation(g_unk0x100a5918);
}

// FUNCTION: MW2 0x10066241
void FUN_10066241(MechS32 p_formation)
{
	g_unk0x100acaf0[0] = 0;
	SetTeamFormation(g_unk0x100a5918, p_formation);
	RequestMenuClose(1);
}

// FUNCTION: MW2 0x10066369
void FUN_10066369(MechS32 p_index)
{
	if (p_index < 8) {
		g_unk0x100acaf0[p_index] = 2;
		FUN_10054f50(p_index, 2);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x100663a4
void FUN_100663a4(MechS32 p_index)
{
	if (p_index < 8) {
		g_unk0x100acaf0[p_index] = 1;
		FUN_10054f50(p_index, 3);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x100663df
void FUN_100663df(MechS32 p_index)
{
	if (p_index < 8) {
		g_unk0x100acaf0[p_index] = 3;
		FUN_10054f50(p_index, 5);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x1006641a
void FUN_1006641a(MechS32 p_index)
{
	if (p_index < 8) {
		g_unk0x100acaf0[p_index] = 4;
		FUN_10054f50(p_index, 7);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x10066455
void FUN_10066455(MechS32 p_index)
{
	if (p_index < 8) {
		g_unk0x100acaf0[p_index] = 5;
		FUN_10054f50(p_index, 8);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x10066490
void FUN_10066490(MechS32 p_index)
{
	if (p_index < 8) {
		g_unk0x100acaf0[p_index] = 6;
		FUN_10054f50(p_index, 0xb);
	}

	RequestMenuClose(1);
}
