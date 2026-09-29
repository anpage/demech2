#ifndef UNK100563D0_H
#define UNK100563D0_H

#include "types.h"

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

	MechU32 GetStaticPoolSize(MechS32 p_index);
	MechS32* FUN_1005640e(char* p_mission);

#ifdef __cplusplus
}
#endif

#endif // UNK100563D0_H
