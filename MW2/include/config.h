#ifndef CONFIG_H
#define CONFIG_H

#include "decomp.h"
#include "soundconfig.h"
#include "types.h"

struct Mech;
struct Point;
struct RenderTarget;
struct ResourceRef;

#pragma pack(1)
// The difficulty settings, read from a .cfg file as one block.
// SIZE 0x17
typedef struct DifficultyCfg {
	undefined m_unk0x00;         // 0x00 — cleared in network games with more than one player
	undefined m_unk0x01;         // 0x01 — cleared in network games
	undefined m_splashDamage;    // 0x02 — splash damage hurts mechs; set in network games with more than one player
	undefined m_collisionDamage; // 0x03 — collisions hurt mechs; set in network games with more than one player
	undefined m_heatTracking;    // 0x04 — fires heat mechs nearby; set in network games with more than one player
	undefined m_unk0x05;         // 0x05 — 2 in network games
	undefined m_unk0x06[3];      // 0x06
	undefined m_unk0x09;         // 0x09 — set outside network games
	undefined m_unk0x0a;         // 0x0a
	undefined4 m_unk0x0b;        // 0x0b — cleared outside network games
	undefined4 m_unk0x0f;        // 0x0f — cleared outside network games
	undefined4 m_unk0x13;        // 0x13 — cleared outside network games
} DifficultyCfg;
#pragma pack()

// The functions and globals of config.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern struct RenderTarget g_unk0x100adf58[26];
	extern MechChar g_gameDir[256];
	extern MechS32 g_unk0x100ae380;

	MechS32 LoadFile(MechChar* p_path, MechS32* p_size, void** p_data, MechU32* p_poolTag);
	MechS32 LoadDifficultyCfg(MechChar* p_name, DifficultyCfg** p_cfg);
	MechS32 FUN_100712b0(MechChar* p_name, void* p_data);
	MechS32 LoadSndCfg(MechChar* p_name, SoundConfig** p_cfg);
	MechS32 SaveSndCfg(MechChar* p_name, SoundConfig* p_cfg);
	MechChar* BuildGamePath(MechChar* p_name);
	void FUN_1006f4fa(struct Mech* p_mech);
	void FUN_1006fba3(void);
	void FUN_1006fca5(void);
	void FUN_1006ff7b(void);
	void FUN_1007005a(struct Mech* p_mech);
	void FUN_100704c1(void);
	void FUN_1007053d(struct Mech* p_mech, MechS32 p_heavy);
	void FUN_100705dd(struct Mech* p_mech);
	void FUN_1007079d(struct RenderTarget* p_target, MechS32 p_index, MechS32 p_x, MechS32 p_y);
	MechS32 FUN_100707c0(
		struct ResourceRef* p_ref,
		MechS32* p_unk0x04,
		MechS32* p_unk0x08,
		MechS32* p_unk0x0c,
		MechS32* p_unk0x10,
		MechS32* p_unk0x14,
		MechS32* p_unk0x18,
		MechS32* p_unk0x1c
	);
	MechS32 FUN_100708f4(struct ResourceRef* p_ref);
	MechS32 FUN_10070bda(struct ResourceRef* p_ref);
	MechS32 FUN_10070e22(
		struct ResourceRef* p_ref,
		struct RenderTarget* p_gauges,
		struct RenderTarget* p_panels,
		struct Point* p_point
	);
	void FUN_1006f480(void);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
