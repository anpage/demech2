#ifndef UNK1001DF00_H
#define UNK1001DF00_H

#include "decomp.h"
#include "types.h"

// A quadtree node: its bounds, its four children (m_unk0x18 == 0) and m_unk0x18 entries
// after the header (undefined4 each). FUN_1001e429 allocates it; ScarletOrchid0x4c::m_unk0x44 holds the root
// when the shape's m_unk0x24 is 5.
// SIZE 0x2c
typedef struct AzureThicket0x2c {
	undefined4 m_unk0x00;                   // 0x00
	undefined4 m_unk0x04;                   // 0x04
	undefined4 m_unk0x08;                   // 0x08
	undefined4 m_unk0x0c;                   // 0x0c
	undefined4 m_unk0x10;                   // 0x10
	undefined4 m_unk0x14;                   // 0x14
	MechS32 m_unk0x18;                      // 0x18
	struct AzureThicket0x2c* m_children[4]; // 0x1c
} AzureThicket0x2c;

// The functions and globals of unk1001df00.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	AzureThicket0x2c* FUN_1001e429(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		undefined4 p_unk0x08,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10,
		undefined4 p_unk0x14,
		MechS32 p_unk0x18
	);
	void FUN_1001e50d(AzureThicket0x2c* p_node);
	void FUN_1001edfa(void);
	MechS32 FUN_1001ee0f(AzureThicket0x2c* p_node);

#ifdef __cplusplus
}
#endif

#endif // UNK1001DF00_H
