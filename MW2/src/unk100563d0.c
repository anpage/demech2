#include "unk100563d0.h"

#include "bwd.h"
#include "bwdnames.h"
#include "bwdstreamkey.h"
#include "decomp.h"
#include "resource.h"
#include "types.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

DECOMP_SIZE_ASSERT(StaticPoolSize, 0x08)

// The pool's block tags, four characters each: players (AGP), mechs (MGP), SEG, TLIS, ADAT, ANTK,
// ANFL, OBJI, CID and CINS.
// GLOBAL: MW2 0x100a9428
MechU32 g_staticPoolTags[10] =
	{0x504741, 0x50474d, 0x474553, 0x53494c54, 0x54414441, 0x4b544e41, 0x4c464e41, 0x494a424f, 0x444943, 0x534e4943};

// The mission's static memory table, which FUN_1005640e fills.
// GLOBAL: MW2 0x100e9dc0
StaticPoolSize g_staticPoolSizes[10];

// Returns the size of the table's entry at an index, or 0.
// FUNCTION: MW2 0x100563d0
MechU32 GetStaticPoolSize(MechS32 p_index)
{
	MechU32 size;

	size = 0;
	if (p_index >= 0 && p_index < 10) {
		size = g_staticPoolSizes[p_index].m_size;
	}

	return size;
}

// Reads a mission's static memory table (seven tag and size pairs), or returns NULL. A mission
// named by a number is looked up by that resource id.
// Stack-slot permutation: result, key, keyData and buffer.
// FUNCTION: MW2 0x1005640e
MechS32* FUN_1005640e(char* p_mission)
{
	BwdStream* stream;
	BwdStreamKey* key;
	MechS32* result;
	BwdStreamKey keyData;
	undefined buffer[0x20];

	result = NULL;
	key = &keyData;
	strncpy(key->m_name, p_mission, 0xc);
	key->m_name[0xc] = '\0';
	if (isdigit(*p_mission)) {
		key->m_id = atoi(p_mission);
	}
	else {
		key->m_id = -1;
	}

	stream = OpenBwdStream(key, (BwdStream*) buffer);
	if (stream) {
		if (FUN_10056503(stream)) {
			result = FUN_100567ed();
		}

		UnloadResource(stream);
		FUN_100586ec();
		FUN_1004fd55();
	}

	return result;
}

// STUB: MW2 0x10056503
MechS32 FUN_10056503(BwdStream* p_stream)
{
	STUB(0x10056503);
	return 0;
}

// STUB: MW2 0x100567ed
MechS32* FUN_100567ed(void)
{
	STUB(0x100567ed);
	return NULL;
}
