#ifndef UNK100563D0_H
#define UNK100563D0_H

#include "types.h"

struct BwdStream;

// One entry of a mission's static memory table: a pool tag and its size.
// SIZE 0x08
typedef struct StaticPoolSize {
	MechU32 m_tag;  // 0x00
	MechU32 m_size; // 0x04
} StaticPoolSize;

// The functions and globals of unk100563d0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechU32 g_staticPoolTags[10];

	MechU32 GetStaticPoolSize(MechS32 p_index);
	MechS32* FUN_1005640e(char* p_mission);
	MechS32 FUN_10056503(struct BwdStream* p_stream);
	MechS32* FUN_100567ed(void);

#ifdef __cplusplus
}
#endif

#endif // UNK100563D0_H
