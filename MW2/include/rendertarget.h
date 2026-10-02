#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "blit.h"
#include "decomp.h"
#include "navpoint.h"
#include "pane.h"
#include "pixelbuffer.h"
#include "types.h"

struct AmberWillow0x7c;
struct Player;
struct ScarletOrchid0x4c;

// The functions and globals of rendertarget.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_navCount;
	extern MechS32 g_unk0x100aaba8;
	extern MechS32 g_unk0x100aabac;
	extern struct SageLark0x1c* g_unk0x100aabd4;
	extern struct CockpitLayout* g_unk0x100ab0e8[6];
	extern NavPoint g_navTable[128];

	MechS32 FUN_1005ec80(MechU32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_1005ed4f(MechU32 p_owner, MechU32 p_nav);
	void FUN_1005ef5e(struct Player* p_player, MechS32 p_step, MechU32 p_flags);
	void FUN_1005f284(void);
	MechS32 FUN_1005f2ae(MechU32 p_player, MechS32 p_nav, MechU32 p_flags);
	MechS32 FUN_1005f4ac(MechS32 p_player, MechS32 p_index, MechU32 p_flags);
	MechS32 FUN_1005f798(MechS32 p_player, MechS32 p_index, MechU32 p_flags);
	MechS32 FUN_1005fa22(struct Player* p_player);
	MechS32 FUN_1005fe63(void);
	MechS32 FUN_1005febe(void);
	struct ScarletOrchid0x4c* FUN_1005ff19(void);
	struct AmberWillow0x7c* FUN_1005ff56(void);
	void FUN_10060010(void);
	void FUN_10060197(
		MechS32 p_dx,
		MechS32 p_dy,
		MechS32 p_dz,
		MechS32* p_unk0x0c,
		MechS32* p_unk0x10,
		MechU32* p_distance,
		MechS32* p_unk0x18
	);
	void FUN_100602b2(struct Player* p_player, MechS32 p_step, MechS32 p_unk0x08);
	void FUN_100602ec(MechS32 p_step);
	void FUN_1006031b(MechS32 p_step);
	void FUN_1006034a(MechS32 p_step);
	void FUN_1006037c(MechS32 p_step);
	void FUN_100603ae(void);

#ifdef __cplusplus
}
#endif

#endif // RENDERTARGET_H
