// What the units under test reference beyond the routines: clock.c's tables and the view and
// rendering globals, which the cases fill, and empty stand-ins for the functions and globals the
// units' other functions use. None of the functions is called.

#include "callbacks.h"
#include "clock.h"
#include "config.h"
#include "error.h"
#include "gamekeys.h"
#include "geocache.h"
#include "object.h"
#include "prjfile.h"
#include "ramp.h"
#include "resource.h"
#include "simmain.h"
#include "soundfx.h"
#include "staticmem.h"
#include "timedoverlays.h"
#include "types.h"
#include "unk1001ce90.h"
#include "unk100335d0.h"
#include "unk1003a530.h"
#include "unk10042e00.h"
#include "unk1004b980.h"
#include "unk100563d0.h"
#include "unk100737e0.h"

#include <stddef.h>

MechS32 g_sinTable[0x102];
MechS32 g_atanTable[0x102];
MechS16* g_sqrtTable;
MechS32 g_missionTimerStopped;
HANDLE g_primaryHeap;

// The view (unk1004b980.c), which the cases set: the near and far planes, the projection's shifts,
// centre and clip edges, the view rows, the eyepoint and the light.
MechS32 g_unk0x100ea820;
MechS32 g_unk0x100ea824;
MechS32 g_unk0x100ea828;
MechS32 g_unk0x100ea82c;
MechS32 g_unk0x100ea830;
MechS32 g_unk0x100ea834;
MechS32 g_unk0x100ea840;
MechS32 g_unk0x100ea84c;
MechS32 g_unk0x100ea850;
MechS32 g_unk0x100ea858;
MechS32 g_unk0x100ea864;
MechS32 g_unk0x100ea868;
MechS32 g_unk0x100ea86c;
MechS32 g_unk0x100ea870;
MechS32 g_unk0x100ea874;
MechS32 g_unk0x100ea878;
MechS32 g_unk0x100ea87c;
MechS32 g_unk0x100ea880;
MechS32 g_unk0x100ea884;
MechS32 g_unk0x100ea8b4;
MechS32 g_unk0x100ea8b8;
MechS32 g_unk0x100ea8bc;
MechS32 g_unk0x100ea8c0;
MechS32 g_unk0x100ea8c4;
MechS32 g_unk0x100ea8c8;

// The polygon list (unk100335d0.c) and the rendering hooks (unk10042e00.c), which the cases set.
MechS32 g_unk0x100a54b0;
MechS32 g_unk0x100a54b4;
AmberDune0x8* g_unk0x1010b5c4;
MechU32 g_unk0x1010b5c8;
SlateHeron0x68 g_unk0x100a6cc8;

struct Player* g_unk0x100a8638;
MechU32 g_staticPoolTags[10];

MechS32 FUN_1007c930(void)
{
	return 0;
}

MechS32 FUN_1007c9e3(void)
{
	return 0;
}

MechS32 FUN_1007ca8e(void)
{
	return 0;
}

MechS32 FUN_1007caf7(MechS32 p_x, MechS32 p_y)
{
	(void) p_x;
	(void) p_y;
	return 0;
}

TimedCallback* FUN_1007d2e0(void)
{
	return NULL;
}

void** FUN_1007d51f(TimedCallback* p_callback)
{
	(void) p_callback;
	return NULL;
}

struct ScarletOrchid0x4c* FindClassById(MechS32 p_id)
{
	(void) p_id;
	return NULL;
}

MechS32 FindStarIdxById(MechS32 p_id)
{
	(void) p_id;
	return 0;
}

MechS32 FindThingIdxById(MechS32 p_id)
{
	(void) p_id;
	return 0;
}

MechS32 FindResourceIdByName(MechS32 p_table, MechChar* p_name)
{
	(void) p_table;
	(void) p_name;
	return 0;
}

MechS32 MapResourceId(MechS32 p_id)
{
	(void) p_id;
	return 0;
}

MechS32 FUN_100708f4(struct ResourceRef* p_ref)
{
	(void) p_ref;
	return 0;
}

struct AmberWillow0x7c* FUN_1001d980(MechS32 p_index)
{
	(void) p_index;
	return NULL;
}

ScarletOrchid0x4c** FUN_1001f873(MechS32 p_index)
{
	(void) p_index;
	return NULL;
}

void FUN_1003b43d(ScarletOrchid0x4c* p_shape, MechS32* p_vertexCount, MechS32* p_faceCount)
{
	(void) p_shape;
	(void) p_vertexCount;
	(void) p_faceCount;
}

void FUN_1003b696(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32 p_unk0x00)
{
	(void) p_shape;
	(void) p_index;
	(void) p_unk0x00;
}

struct AmberWillow0x7c* FUN_1003b6e5(ScarletOrchid0x4c* p_shape)
{
	(void) p_shape;
	return NULL;
}

void SetObjPosition(AmberWillow0x7c* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	(void) p_obj;
	(void) p_x;
	(void) p_y;
	(void) p_z;
}

void FUN_10001667(AmberWillow0x7c* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	(void) p_obj;
	(void) p_x;
	(void) p_y;
	(void) p_z;
}

void SetObjRotation(AmberWillow0x7c* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10)
{
	(void) p_obj;
	(void) p_unk0x04;
	(void) p_unk0x08;
	(void) p_unk0x0c;
	(void) p_unk0x10;
}

void FUN_1000184b(AmberWillow0x7c* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10)
{
	(void) p_obj;
	(void) p_unk0x04;
	(void) p_unk0x08;
	(void) p_unk0x0c;
	(void) p_unk0x10;
}

void FUN_10001cf8(AmberWillow0x7c* p_obj)
{
	(void) p_obj;
}

MechS32 StartRamp(Ramp* p_ramp, MechS32 p_target, MechS32 p_value, MechDouble p_seconds)
{
	(void) p_ramp;
	(void) p_target;
	(void) p_value;
	(void) p_seconds;
	return 0;
}

MechS32 UpdateRamp(Ramp* p_ramp)
{
	(void) p_ramp;
	return 0;
}

MechS32 StartWrappedRamp(WrappedRamp* p_ramp, MechS32 p_target, MechS32 p_value, MechDouble p_seconds, MechS32 p_period)
{
	(void) p_ramp;
	(void) p_target;
	(void) p_value;
	(void) p_seconds;
	(void) p_period;
	return 0;
}

MechS32 UpdateWrappedRamp(WrappedRamp* p_ramp)
{
	(void) p_ramp;
	return 0;
}

void* StaticPoolAlloc(MechU32 p_size, MechU32 p_tag)
{
	(void) p_size;
	(void) p_tag;
	return NULL;
}

void UpdateAmbientSound(AmbientSound* p_sound)
{
	(void) p_sound;
}

void StopAmbientSound(AmbientSound* p_sound)
{
	(void) p_sound;
}

void Error(MechS32 p_code, const char* p_format, ...)
{
	(void) p_code;
	(void) p_format;
}

MechS32 GetPrjResourceSize(MechS32 p_file, const MechChar* p_type, MechU16 p_id)
{
	(void) p_file;
	(void) p_type;
	(void) p_id;
	return 0;
}

MechS32 ReadPrjResource(MechS32 p_file, const MechChar* p_type, MechU16 p_id, void* p_buffer)
{
	(void) p_file;
	(void) p_type;
	(void) p_id;
	(void) p_buffer;
	return 0;
}

MechS32 ShowInGameMessage(MechChar* p_text, MechS32 p_font, MechS32 p_duration, MechS32 p_priority)
{
	(void) p_text;
	(void) p_font;
	(void) p_duration;
	(void) p_priority;
	return 0;
}

#ifndef _WIN32
int _strcmpi(const char* p_a, const char* p_b)
{
	(void) p_a;
	(void) p_b;
	return 0;
}

LPVOID HeapAlloc(HANDLE p_heap, DWORD p_flags, SIZE_T p_size)
{
	(void) p_heap;
	(void) p_flags;
	(void) p_size;
	return NULL;
}

BOOL HeapFree(HANDLE p_heap, DWORD p_flags, LPVOID p_block)
{
	(void) p_heap;
	(void) p_flags;
	(void) p_block;
	return FALSE;
}

SIZE_T HeapSize(HANDLE p_heap, DWORD p_flags, LPCVOID p_block)
{
	(void) p_heap;
	(void) p_flags;
	(void) p_block;
	return 0;
}
#endif
