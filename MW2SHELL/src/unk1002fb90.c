#include "decomp.h"
#include "types.h"

#include <io.h>

// The heap callbacks the archive unit allocates through, registered by FUN_1002fb90
// (CedarKnot0x10 passes FUN_1002e302 and FUN_1002e324).
// GLOBAL: MW2SHELL 0x10066da0
void* (*g_unk0x10066da0)(undefined4) = NULL;

// GLOBAL: MW2SHELL 0x10066da4
void (*g_unk0x10066da4)(void*) = NULL;

// FUNCTION: MW2SHELL 0x1002fb90
void FUN_1002fb90(void* (*p_alloc)(undefined4), void (*p_free)(void*))
{
	g_unk0x10066da0 = p_alloc;
	g_unk0x10066da4 = p_free;
}

// FUNCTION: MW2SHELL 0x1002fbab
void* FUN_1002fbab(undefined4 p_size)
{
	return g_unk0x10066da0(p_size);
}

// FUNCTION: MW2SHELL 0x1002fbc8
void FUN_1002fbc8(void* p_block)
{
	g_unk0x10066da4(p_block);
}

// Writes p_size bytes in chunks of at most 0x4000. Returns p_size, or -1 on a short write.
// Stack-slot permutation: remaining and chunk swap homes.
// FUNCTION: MW2SHELL 0x1002fbe0
MechS32 FUN_1002fbe0(MechS32 p_fd, MechU8* p_buffer, MechU32 p_size)
{
	MechU8* buffer;
	MechU32 remaining;
	MechU32 chunk;

	remaining = p_size;
	buffer = p_buffer;
	while (remaining > 0) {
		chunk = remaining < 0x4000 ? remaining : 0x4000;
		if (_write(p_fd, buffer, chunk) != chunk) {
			return -1;
		}

		remaining -= chunk;
		buffer += chunk;
	}

	return p_size;
}

// Reads p_size bytes in chunks of at most 0x4000. Returns p_size, or -1 on a short read.
// Stack-slot permutation: remaining and chunk swap homes.
// FUNCTION: MW2SHELL 0x1002fc5e
MechS32 FUN_1002fc5e(MechS32 p_fd, MechU8* p_buffer, MechU32 p_size)
{
	MechU8* buffer;
	MechU32 remaining;
	MechU32 chunk;

	remaining = p_size;
	buffer = p_buffer;
	while (remaining > 0) {
		chunk = remaining < 0x4000 ? remaining : 0x4000;
		if (_read(p_fd, buffer, chunk) != chunk) {
			return -1;
		}

		remaining -= chunk;
		buffer += chunk;
	}

	return p_size;
}

// STUB: MW2SHELL 0x1002fcdc
MechS32 FUN_1002fcdc(char* p_name, undefined p_unk0x04)
{
	STUB(0x1002fcdc);
	return 0;
}

// Returns a status: ~CedarKnot0x10 compares the result against 0.
// STUB: MW2SHELL 0x1002ffc9
MechS32 FUN_1002ffc9(MechS32 p_handle)
{
	STUB(0x1002ffc9);
	return 0;
}

// STUB: MW2SHELL 0x1003024f
MechS32 FUN_1003024f(MechS32 p_handle)
{
	STUB(0x1003024f);
	return 0;
}
