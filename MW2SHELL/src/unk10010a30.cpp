#include "decomp.h"
#include "drawmode.h"
#include "drawmodeextension.h"
#include "pixelbuffer.h"
#include "types.h"

// GLOBAL: MW2SHELL 0x10062cc8
DrawModeExtension* g_currentDrawModeExtension = NULL;

// GLOBAL: MW2SHELL 0x10062ccc
DrawMode* g_currentDrawMode = NULL;

// STUB: MW2SHELL 0x10010a30
MechS32 InitDrawMode(
	MechS32 p_unk0x00,
	MechS32 p_unk0x04,
	PixelBuffer* p_buffer,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_unk0x14
)
{
	STUB(0x10010a30);
	return 0;
}

// STUB: MW2SHELL 0x10010d49
void FUN_10010d49()
{
	STUB(0x10010d49);
}
