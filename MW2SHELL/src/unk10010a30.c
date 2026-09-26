#include "decomp.h"
#include "drawmode.h"
#include "drawmodeextension.h"
#include "pixelbuffer.h"
#include "types.h"

// GLOBAL: MW2SHELL 0x10062cc8
DrawModeExtension* g_currentDrawModeExtension = NULL;

// GLOBAL: MW2SHELL 0x10062ccc
DrawMode* g_currentDrawMode = NULL;

// GLOBAL: MW2SHELL 0x10063000
MechS32 g_unk0x10063000 = 1;

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

// FUNCTION: MW2SHELL 0x10010d49
void FUN_10010d49()
{
	if (g_currentDrawMode != NULL) {
		g_currentDrawMode->m_unk0x14();
	}
	if (g_currentDrawModeExtension != NULL) {
		g_currentDrawModeExtension->m_unk0x10();
	}

	g_unk0x10063000 = 1;
}
