#ifndef RENDER_H
#define RENDER_H

#include "decomp.h"
#include "types.h"

// SIZE 0x18
typedef struct GameWindowGeometry {
	MechS32 m_width;      // 0x00
	MechS32 m_height;     // 0x04
	undefined4 m_unk0x08; // 0x08
	undefined4 m_unk0x0c; // 0x0c
	undefined4 m_unk0x10; // 0x10
	undefined4 m_unk0x14; // 0x14
} GameWindowGeometry;

// The functions and globals of render.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern GameWindowGeometry* g_gameWindowGeometry;
	extern MechS32 g_screenHeight;
	extern MechS32 g_screenHeightMinus1;
	extern MechS32 g_screenPixelCount;
	extern MechS32 g_screenWidth;
	extern MechS32 g_screenWidthMinus1;
	extern MechS32 g_screenHalfWidth;
	extern MechS32 g_screenHalfHeight;
	extern MechS32 g_unk0x10176ebc;

	MechS32 InitGameWindowGeometry(void);
	MechS32 InitDisplayGeometry(void);
	void FirstRender(void);
	void SecondRender(void);
	void Blit(void);
	void ShutdownRender(void);

#ifdef __cplusplus
}
#endif

#endif // RENDER_H
