#ifndef EYEPOINT_H
#define EYEPOINT_H

#include "decomp.h"
#include "types.h"

// The view the scene is drawn from (g_eyepoint), at 0x00-0x08. SelectRenderTarget sizes its view rectangle
// (0x2c-0x38) to the render target; FUN_100024f0 reads its centre, offset by 0x4c/0x50.
// SIZE 0xe0
typedef struct Eyepoint {
	MechS32 m_unk0x00;                       // 0x00
	MechS32 m_unk0x04;                       // 0x04
	MechS32 m_unk0x08;                       // 0x08
	undefined4 m_unk0x0c[(0x2c - 0x0c) / 4]; // 0x0c
	MechS32 m_unk0x2c;                       // 0x2c
	MechS32 m_unk0x30;                       // 0x30
	MechS32 m_unk0x34;                       // 0x34
	MechS32 m_unk0x38;                       // 0x38
	undefined4 m_unk0x3c[(0x4c - 0x3c) / 4]; // 0x3c
	MechS32 m_unk0x4c;                       // 0x4c
	MechS32 m_unk0x50;                       // 0x50
	undefined4 m_unk0x54[(0xe0 - 0x54) / 4]; // 0x54
} Eyepoint;

#endif // EYEPOINT_H
