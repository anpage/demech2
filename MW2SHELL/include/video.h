#ifndef VIDEO_H
#define VIDEO_H

#include "decomp.h"
#include "types.h"

// The functions and globals of video.cpp that other units use.
MechS32 FUN_10015f58(const char* p_name, MechS32 p_msg, MechS32 p_wParam);
MechS32 PlayFullscreenVideo(const char* p_name, MechS32 p_msg, MechS32 p_wParam);
void FUN_1001661b();
MechS32 FUN_10016b11(MechS32 p_index);
MechS32 FUN_10016be7();
void FUN_10016c1d();
void FUN_10016c3e();
void FUN_10016cc0(MechS32 p_index, MechS32 p_mask, MechS32 p_value);
void FUN_10016d27(MechS32 p_index);
void FUN_10016d90(MechS32 p_index);
void FUN_10016f45();
void FUN_10016f82(MechS32 p_index, MechS32 p_left, MechS32 p_top);
MechS32 FUN_100175e2(MechChar* p_name, MechS32 p_left, MechS32 p_top, MechU32 p_flags, MechU32 p_unk0x14);
void FUN_10017698(MechS32 p_index, MechS32 p_frame);
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_left,
	undefined4 p_top,
	MechU32 p_flags,
	MechU32 p_fps
);

#endif // VIDEO_H
