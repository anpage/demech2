#ifndef DRAWMODEEXTENSION_H
#define DRAWMODEEXTENSION_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)
// Function table of the active draw mode (GDI or DirectDraw).
// SIZE 0x28
struct DrawModeExtension {
	undefined m_unk0x00[0x14 - 0x00];                                                                   // 0x00
	void (*m_setPalette)(MechS32 p_first, MechS32 p_count, undefined* p_palette, undefined4 p_unk0x0c); // 0x14
	undefined m_unk0x18[0x20 - 0x18];                                                                   // 0x18
	// Returns 0 once the framebuffer can be drawn to (GDI: points the output buffer at the DIB bits).
	MechS32 (*m_acquireFramebuffer)(); // 0x20
	undefined4 m_unk0x24;              // 0x24
};
#pragma pack()

#endif // DRAWMODEEXTENSION_H
