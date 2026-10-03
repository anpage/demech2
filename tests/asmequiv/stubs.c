// What the units under test reference beyond the routines: clock.c's tables and the view and
// rendering globals, which the cases fill, and empty stand-ins for the functions and globals the
// units' other functions use. None of the functions is called.

#include "callbacks.h"
#include "classtable.h"
#include "clock.h"
#include "config.h"
#include "depthsort.h"
#include "error.h"
#include "gamekeys.h"
#include "geocache.h"
#include "object.h"
#include "polydraw.h"
#include "poolsizes.h"
#include "prjfile.h"
#include "ramp.h"
#include "resource.h"
#include "resourcename.h"
#include "shape.h"
#include "simmain.h"
#include "soundfx.h"
#include "staticmem.h"
#include "timedoverlays.h"
#include "types.h"
#include "view.h"

#include <stddef.h>

MechS32 g_sinTable[0x102];
MechS32 g_atanTable[0x102];
MechS16* g_sqrtTable;
MechS32 g_missionTimerStopped;
HANDLE g_primaryHeap;

// The view (view.c), which the cases set: the near and far planes, the projection's shifts,
// centre and clip edges, the view rows, the eyepoint and the light.
MechS32 g_viewNear;
MechS32 g_viewShiftX;
MechS32 g_viewShiftY;
MechS32 g_viewFar;
MechS32 g_viewLeft;
MechS32 g_viewCenterX;
MechS32 g_viewBottom;
MechS32 g_viewRight;
MechS32 g_viewTop;
MechS32 g_viewCenterY;
MechS32 g_viewProjX0;
MechS32 g_viewProjX1;
MechS32 g_viewProjX2;
MechS32 g_viewProjY0;
MechS32 g_viewProjY1;
MechS32 g_viewProjY2;
MechS32 g_viewProjZ0;
MechS32 g_viewProjZ1;
MechS32 g_viewProjZ2;
MechS32 g_viewEyeY;
MechS32 g_viewEyeX;
MechS32 g_viewEyeZ;
MechS32 g_viewLightZ;
MechS32 g_viewLightX;
MechS32 g_viewLightY;

// The polygon list (depthsort.c) and the rendering hooks (polydraw.c), which the cases set.
MechS32 g_depthEntryCount;
MechS32 g_polygonCount;
DepthEntry* g_depthList;
MechU32 g_queuedShapeFlags;
RenderSettings g_renderSettings;

struct Player* g_lastPlayer;
MechU32 g_staticPoolTags[10];

MechS32 InitSinAtanTables(void)
{
	return 0;
}

MechS32 InitSlopeTables(void)
{
	return 0;
}

MechS32 InitSqrtTable(void)
{
	return 0;
}

MechS32 Hypot2D(MechS32 p_x, MechS32 p_y)
{
	(void) p_x;
	(void) p_y;
	return 0;
}

TimedCallback* GetCurrentCallback(void)
{
	return NULL;
}

void** GetCallbackData(TimedCallback* p_callback)
{
	(void) p_callback;
	return NULL;
}

struct Shape* FindClassById(MechS32 p_id)
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

MechS32 LoadReels(struct ResourceRef* p_ref)
{
	(void) p_ref;
	return 0;
}

struct SceneObject* GetClassObject(MechS32 p_index)
{
	(void) p_index;
	return NULL;
}

Shape** GetStaticShapeSlot(MechS32 p_index)
{
	(void) p_index;
	return NULL;
}

void GetModelCounts(Shape* p_shape, MechS32* p_vertexCount, MechS32* p_faceCount)
{
	(void) p_shape;
	(void) p_vertexCount;
	(void) p_faceCount;
}

void SetFaceColor(Shape* p_shape, MechS32 p_index, MechS32 p_unk0x00)
{
	(void) p_shape;
	(void) p_index;
	(void) p_unk0x00;
}

struct SceneObject* GetShapeObject(Shape* p_shape)
{
	(void) p_shape;
	return NULL;
}

void SetObjPosition(SceneObject* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	(void) p_obj;
	(void) p_x;
	(void) p_y;
	(void) p_z;
}

void MoveObj(SceneObject* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	(void) p_obj;
	(void) p_x;
	(void) p_y;
	(void) p_z;
}

void SetObjRotation(SceneObject* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10)
{
	(void) p_obj;
	(void) p_unk0x04;
	(void) p_unk0x08;
	(void) p_unk0x0c;
	(void) p_unk0x10;
}

void RotateObj(SceneObject* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10)
{
	(void) p_obj;
	(void) p_unk0x04;
	(void) p_unk0x08;
	(void) p_unk0x0c;
	(void) p_unk0x10;
}

void UpdateObj(SceneObject* p_obj)
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
