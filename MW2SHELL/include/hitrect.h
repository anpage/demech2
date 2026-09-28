#ifndef HITRECT_H
#define HITRECT_H

#include "decomp.h"
#include "types.h"

class VideoDriver;

// SIZE 0x10
class HitRect {
public:
	HitRect(undefined4 p_unk0x00, undefined4 p_unk0x04, undefined4 p_unk0x08, undefined4 p_unk0x0c);
	void FUN_10049183();
	void FUN_10049199(VideoDriver* p_videoDriver);
	MechU8 FUN_10049245(MechS32 p_x, MechS32 p_y);

private:
	MechS32 m_unk0x00; // 0x00
	MechS32 m_unk0x04; // 0x04
	MechS32 m_unk0x08; // 0x08
	MechS32 m_unk0x0c; // 0x0c
};

#endif // HITRECT_H
