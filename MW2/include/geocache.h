#ifndef GEOCACHE_H
#define GEOCACHE_H

#include "callbacks.h"
#include "decomp.h"
#include "twilightgrove.h"
#include "types.h"
#include "unk1001df00.h"
#include "unk1003a530.h"

struct GameThing;

struct AmberWillow0x7c;
struct BwdBlockRecord;

// An entry of the class table: an ID and its class.
// SIZE 0x08
typedef struct GeoClass {
	MechS32 m_id;                      // 0x00
	struct ScarletOrchid0x4c* m_class; // 0x04
} GeoClass;

// The functions and globals of geocache.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32* g_unk0x100a3850;
	extern MechS32* g_unk0x100a3854;
	extern MechS32 g_currentBlock;
	extern MechS32 g_unk0x1010b6a0;
	extern MechS32 g_unk0x1012b7b0;

	MechS32 FUN_1001f3e0(void);
	void FUN_1001f5cb(void);
	ScarletOrchid0x4c** FUN_1001f873(MechS32 p_index);
	ScarletOrchid0x4c* FUN_1001f894(MechS32 p_index);
	MechS32 FUN_1001f8b5(
		MechS32 p_id,
		MechS32 p_resource,
		TwilightGrove0x24 p_xform,
		MechS32 p_block,
		MechS32 p_parent,
		MechS32 p_unk0x3c,
		MechU32 p_flags,
		MechU32 p_kind,
		undefined4 p_unk0x48
	);
	void BeginBlock(struct BwdBlockRecord* p_record);
	MechS32 FindStarIdxById(MechS32 p_id);
	struct ScarletOrchid0x4c* FindClassById(MechS32 p_id);
	MechS32 FindThingIdxById(MechS32 p_id);
	void ApplyBlockXform(TwilightGrove0x24 p_xform);
	void FUN_1001fea6(MechS32* p_point);
	void FUN_1001feef(MechS32 p_index);
	void FUN_1001ffda(void);
	void FirstStaticCache(void);
	void AttachTaskToObj(MechS32 p_index, TimedCallbackFn p_fn, MechS32 p_period, undefined4 p_data);
	void FUN_100200bd(void);
	void FUN_1002012a(MechS32 p_index, TimedCallback* p_callback);
	void FUN_1002015f(MechS32 p_index);
	void FUN_10020190(MechS32 p_index);
	void FUN_100201c1(MechS32 p_index, MechU32 p_unk0x0c);
	void FUN_100201fe(MechS32 p_index, MechS32 p_replacement, MechS32 p_thing);
	MechS32 FUN_10020292(MechS32 p_index);
	void FUN_10020429(struct GameThing* p_thing);
	void FUN_100204e8(void);
	MechS32 FUN_10020684(void);
	MechS32 FUN_10020704(MechS32 p_index, MechS32 p_block);
	void FUN_10020b95(MechS32 p_index);
	struct AmberWillow0x7c* FUN_10020bdd(MechS32 p_index);
	ScarletOrchid0x4c* FUN_10020c26(MechS32 p_index);
	void FUN_10020c6f(MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	MechS32 FUN_10020d51(void);
	void FUN_10021067(AzureThicket0x2c* p_root);
	void FUN_1002116a(AzureThicket0x2c* p_node, MechU8* p_data, MechS32 p_size);
	MechS32 FUN_10021314(MechU32 p_index);
	void FUN_100213cf(struct AmberWillow0x7c* p_obj);
	MechS32 FUN_10021423(void);
	void SetExplosionChunks(undefined4 p_unk0x00, MechS32 p_explosionChunks);

#ifdef __cplusplus
}
#endif

#endif // GEOCACHE_H
