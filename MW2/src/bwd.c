#include "bwd.h"

#include "decomp.h"
#include "loadres.h"
#include "overlay.h"
#include "simmain.h"
#include "types.h"

#include <stdio.h>
#include <windows.h>

// Only LogKeywordName's dead first store takes its address.
// GLOBAL: MW2 0x10109c40
MechChar g_unk0x10109c40[1];

// STUB: MW2 0x1003fb00
BwdStream* OpenBwdStream(MechS16* p_id, undefined* p_unk0x04)
{
	STUB(0x1003fb00);
	return NULL;
}

// Frees a stream's data, or releases its resource.
// FUNCTION: MW2 0x1003feb8
void UnloadResource(BwdStream* p_stream)
{
	if (p_stream) {
		if (p_stream->m_fromResource == 0) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_stream->m_data);
		}
		else {
			FUN_1001a4e5(p_stream->m_id, g_unk0x100a86bc);
		}
	}
}

// STUB: MW2 0x1003ff09
MechChar* GetKeywordName(MechU32 p_code)
{
	STUB(0x1003ff09);
	return NULL;
}

// Appends a line to mw2debug.txt and shows it on screen.
// FUNCTION: MW2 0x1003ff75
void FUN_1003ff75(MechChar* p_text)
{
	FILE* file;

	file = fopen("mw2debug.txt", "a");
	if (file) {
		fprintf(file, "%s", p_text);
	}
	fclose(file);
	FUN_100591d1(p_text);
}

// Logs a keyword's name with FUN_1003ff75.
// Stack-slot permutation: line and name.
// FUNCTION: MW2 0x1003ffcf
void LogKeywordName(MechU32 p_code)
{
	MechChar line[256];
	MechChar* name;

	name = g_unk0x10109c40;
	name = GetKeywordName(p_code);
	sprintf(line, "%s, ", name);
	FUN_1003ff75(line);
}
