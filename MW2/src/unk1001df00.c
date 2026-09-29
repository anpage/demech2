#include "unk1001df00.h"

#include "decomp.h"
#include "overlay.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(AzureThicket0x2c, 0x2c)

// GLOBAL: MW2 0x100a37dc
MechS32 g_unk0x100a37dc = 0;

// Allocates a quadtree node with room for p_unk0x18 entries, cleared.
// The only diff is a stack-slot permutation of entries and node.
// FUNCTION: MW2 0x1001e429
AzureThicket0x2c* FUN_1001e429(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14,
	MechS32 p_unk0x18
)
{
	undefined4* entries;
	AzureThicket0x2c* node;
	MechS32 i;

	node = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, p_unk0x18 * sizeof(undefined4) + sizeof(AzureThicket0x2c));
	if (node) {
		node->m_unk0x00 = p_unk0x00;
		node->m_unk0x04 = p_unk0x04;
		node->m_unk0x08 = p_unk0x08;
		node->m_unk0x0c = p_unk0x0c;
		node->m_unk0x10 = p_unk0x10;
		node->m_unk0x14 = p_unk0x14;
		node->m_unk0x18 = p_unk0x18;
		i = 4;
		while (i--) {
			node->m_children[i] = NULL;
		}

		if (p_unk0x18 > 0) {
			entries = (undefined4*) (node + 1);
			memset(entries, 0, p_unk0x18 * sizeof(undefined4));
		}
	}
	else {
		FUN_100591d1("Not enough memory for quadtrees!!!!");
	}

	return node;
}

// Frees a quadtree.
// FUNCTION: MW2 0x1001e50d
void FUN_1001e50d(AzureThicket0x2c* p_node)
{
	MechS32 i;

	if (!p_node) {
		return;
	}

	if (p_node->m_unk0x18 == 0) {
		for (i = 0; i < 4; i++) {
			FUN_1001e50d(p_node->m_children[i]);
		}
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_node);
}

// FUNCTION: MW2 0x1001edfa
void FUN_1001edfa(void)
{
	g_unk0x100a37dc = 1;
}

// Returns the bytes a quadtree takes.
// FUNCTION: MW2 0x1001ee0f
MechS32 FUN_1001ee0f(AzureThicket0x2c* p_node)
{
	MechS32 i;
	MechS32 size;

	if (!p_node) {
		return 0;
	}

	size = p_node->m_unk0x18 * sizeof(undefined4) + sizeof(AzureThicket0x2c);
	for (i = 0; i < 4; i++) {
		size += FUN_1001ee0f(p_node->m_children[i]);
	}

	return size;
}
