#include "decomp.h"
#include "types.h"

/* FUN_100012d0 allocates these 0x7c-byte objects. */
// SIZE 0x7c
typedef struct {
	undefined m_unk0x00[0x30];        // 0x00
	undefined4 m_unk0x30;             // 0x30
	undefined4 m_unk0x34;             // 0x34
	undefined4 m_unk0x38;             // 0x38
	undefined m_unk0x3c[0x74 - 0x3c]; // 0x3c
	undefined4 m_unk0x74;             // 0x74
	undefined m_unk0x78[0x7c - 0x78]; // 0x78
} AmberWillow0x7c;

DECOMP_SIZE_ASSERT(AmberWillow0x7c, 0x7c)

void FUN_1000e2ea(void* p_unk0x00, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c);

// FUNCTION: MW2 0x10001596
void GetObjWorldPos(AmberWillow0x7c* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c)
{
	FUN_1000e2ea(p_obj->m_unk0x3c, p_unk0x04, p_unk0x08, p_unk0x0c);
}

// FUNCTION: MW2 0x1000163a
void SetObjPosition(AmberWillow0x7c* p_obj, undefined4 p_unk0x30, undefined4 p_unk0x34, undefined4 p_unk0x38)
{
	p_obj->m_unk0x30 = p_unk0x30;
	p_obj->m_unk0x34 = p_unk0x34;
	p_obj->m_unk0x38 = p_unk0x38;
	p_obj->m_unk0x74 |= 1;
}
