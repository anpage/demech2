#include "perf.h"

#include "decomp.h"
#include "geocache.h"
#include "simmain.h"
#include "soundconfig.h"
#include "types.h"
#include "unk10036230.h"
#include "unk1004b130.h"
#include "unk1004b980.h"
#include "unk1006d680.h"

// The display performance settings of MW2SND.CFG (SoundConfig): a getter and a setter each,
// with the unused leading argument of the in-mission menu's callbacks.

MechS32 ApplyPerfSettings(undefined4 p_unk0x00, undefined4 p_unk0x04);
void SetObjectTextmaps(undefined4 p_unk0x00, MechS32 p_objectTextmaps);
void SetTerrainTextmaps(undefined4 p_unk0x00, MechS32 p_terrainTextmaps);
void SetDisplayDetail(undefined4 p_unk0x00, MechS32 p_displayDetail);
void SetObjectDensity(undefined4 p_unk0x00, MechS32 p_objectDensity);

// FUNCTION: MW2 0x10076af0
void FirstPerfSetting(void)
{
	ApplyPerfSettings(0, 0);
}

// FUNCTION: MW2 0x10076b07
MechS32 ApplyPerfSettings(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	MechS32 result = 1;

	if (g_mw2SndCfgData != NULL) {
		SetObjectTextmaps(result, g_mw2SndCfgData->m_objectTextmaps);
		SetTerrainTextmaps(result, g_mw2SndCfgData->m_terrainTextmaps);
		SetDisplayDetail(result, g_mw2SndCfgData->m_displayDetail);
		SetObjectDensity(result, g_mw2SndCfgData->m_objectDensity);
		SetExplosionChunks(result, g_mw2SndCfgData->m_explosionChunks);
	}

	return result;
}

// FUNCTION: MW2 0x10076b9a
MechS32 GetObjectTextmaps(undefined4 p_unk0x00)
{
	return FUN_10036867(0x100) || FUN_10036867(0x200);
}

// FUNCTION: MW2 0x10076be0
void SetObjectTextmaps(undefined4 p_unk0x00, MechS32 p_objectTextmaps)
{
	FUN_10036891(0x100, p_objectTextmaps);
	FUN_10036891(0x200, p_objectTextmaps);
	g_mw2SndCfgData->m_objectTextmaps = p_objectTextmaps;
}

// FUNCTION: MW2 0x10076c19
MechS32 GetTerrainTextmaps(undefined4 p_unk0x00)
{
	return FUN_10036867(0x800) || g_unk0x100a7120;
}

// FUNCTION: MW2 0x10076c57
void SetTerrainTextmaps(undefined4 p_unk0x00, MechS32 p_terrainTextmaps)
{
	FUN_10036891(0x800, p_terrainTextmaps);
	FUN_1004b539(p_terrainTextmaps);
	g_mw2SndCfgData->m_terrainTextmaps = p_terrainTextmaps;
}

// FUNCTION: MW2 0x10076c8b
MechS32 GetDisplayDetail(undefined4 p_unk0x00)
{
	return FUN_1004c7a6(p_unk0x00) || FUN_100368bf(p_unk0x00);
}

// FUNCTION: MW2 0x10076ccf
void SetDisplayDetail(undefined4 p_unk0x00, MechS32 p_displayDetail)
{
	FUN_1004c7cf(p_unk0x00, p_displayDetail);
	FUN_100368e8(p_unk0x00, p_displayDetail);
	g_mw2SndCfgData->m_displayDetail = p_displayDetail;
}

// FUNCTION: MW2 0x10076d06
MechS32 GetObjectDensity(undefined4 p_unk0x00)
{
	return g_mw2SndCfgData->m_objectDensity;
}

// FUNCTION: MW2 0x10076d1e
void SetObjectDensity(undefined4 p_unk0x00, MechS32 p_objectDensity)
{
	FUN_1006dc7d(p_objectDensity);
	g_mw2SndCfgData->m_objectDensity = p_objectDensity;
}
