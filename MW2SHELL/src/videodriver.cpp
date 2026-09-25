#include "videodriver.h"

DECOMP_SIZE_ASSERT(VideoDriver, 0x3ae)

// STUB: MW2SHELL 0x10005f21
VideoDriver::VideoDriver()
{
	STUB(0x10005f21);
}

// STUB: MW2SHELL 0x10006202
VideoDriver::~VideoDriver()
{
	STUB(0x10006202);
}

// STUB: MW2SHELL 0x100062a0
void VideoDriver::ExpandRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x100062a0);
}

// STUB: MW2SHELL 0x10006330
void VideoDriver::ShrinkRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x10006330);
}

// STUB: MW2SHELL 0x100064ca
void VideoDriver::UpdatePalette()
{
	STUB(0x100064ca);
}

// STUB: MW2SHELL 0x10006502
void VideoDriver::DrawShell()
{
	STUB(0x10006502);
}

// STUB: MW2SHELL 0x10006842
void VideoDriver::DrawFmv()
{
	STUB(0x10006842);
}

// STUB: MW2SHELL 0x100077ea
void VideoDriver::ActivateFramebuffer()
{
	STUB(0x100077ea);
}
