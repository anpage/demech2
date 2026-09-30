#ifndef EYEPOINT_H
#define EYEPOINT_H

#include "decomp.h"
#include "transform.h"
#include "types.h"

// The view the scene is drawn from (g_eyepoint), at 0x00-0x08. SelectRenderTarget sizes its view rectangle
// (0x2c-0x38) to the render target; FUN_100024f0 reads its centre, offset by 0x4c/0x50.
// SIZE 0xe0
typedef struct Eyepoint {
	MechS32 m_unk0x00;                       // 0x00
	MechS32 m_unk0x04;                       // 0x04
	MechS32 m_unk0x08;                       // 0x08
	MechS32 m_unk0x0c;                       // 0x0c
	MechS32 m_unk0x10;                       // 0x10
	MechS32 m_unk0x14;                       // 0x14
	MechS32 m_fovX;                          // 0x18 — 16.16
	MechS32 m_unk0x1c;                       // 0x1c
	MechS32 m_unk0x20;                       // 0x20
	MechS32 m_unk0x24;                       // 0x24
	MechS16 m_unk0x28;                       // 0x28
	MechS16 m_unk0x2a;                       // 0x2a
	MechS32 m_unk0x2c;                       // 0x2c
	MechS32 m_unk0x30;                       // 0x30
	MechS32 m_unk0x34;                       // 0x34
	MechS32 m_unk0x38;                       // 0x38
	MechS32 m_unk0x3c;                       // 0x3c
	MechS32 m_unk0x40;                       // 0x40
	MechS32 m_pixelAspect;                   // 0x44 — pixel width / height, 16.16
	MechS32 m_unk0x48;                       // 0x48
	MechS32 m_unk0x4c;                       // 0x4c
	MechS32 m_unk0x50;                       // 0x50
	Matrix m_unk0x54;                        // 0x54
	MechS32 m_halfWidth;                     // 0x84
	MechS32 m_halfHeight;                    // 0x88
	MechS32 m_centerX;                       // 0x8c
	MechS32 m_centerY;                       // 0x90
	MechS32 m_unk0x94;                       // 0x94
	MechS32 m_unk0x98;                       // 0x98
	MechS32 m_unk0x9c;                       // 0x9c
	MechS32 m_unk0xa0;                       // 0xa0
	MechS16 m_unk0xa4;                       // 0xa4
	MechS16 m_unk0xa6;                       // 0xa6
	MechS32 m_unk0xa8;                       // 0xa8
	MechS32 m_unk0xac;                       // 0xac
	MechS32 m_fovY;                          // 0xb0
	MechS32 m_unk0xb4;                       // 0xb4
	MechS32 m_unk0xb8;                       // 0xb8
	undefined4 m_unk0xbc[(0xe0 - 0xbc) / 4]; // 0xbc
} Eyepoint;

// The functions of eyepoint.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10011cb0(void);
	void FUN_10011e45(
		MechS32* p_unk0x10,
		MechS32* p_unk0x0c,
		MechS32* p_unk0x14,
		MechS32* p_x,
		MechS32* p_y,
		MechS32* p_z
	);
	MechS32 FUN_10011440(void);
	MechS32 FUN_1001156a(Eyepoint* p_eyepoint, MechS32* p_view);
	void FUN_100115f4(MechS32 p_next, MechS32 p_home);
	void FUN_100116c3(
		MechS32* p_unk0x10,
		MechS32* p_unk0x0c,
		MechS32* p_unk0x14,
		MechS32* p_x,
		MechS32* p_y,
		MechS32* p_z
	);
	void FUN_10011819(void);
	void FUN_10011401(MechS32 p_zoom);
	MechS32 FUN_100114ea(Eyepoint* p_eyepoint, MechS32* p_view);

#ifdef __cplusplus
}
#endif

#endif // EYEPOINT_H
