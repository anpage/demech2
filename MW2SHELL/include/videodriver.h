#ifndef VIDEODRIVER_H
#define VIDEODRIVER_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)
// SIZE 0x3ae
class VideoDriver {
public:
	VideoDriver();
	~VideoDriver();

	void ExpandRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	void ShrinkRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	void UpdatePalette();
	void DrawShell();
	void DrawFmv();
	void ActivateFramebuffer();

private:
	undefined m_unk0x00[0x42 - 0x00]; // 0x00

public:
	// ShellWindowProc clears it directly: an inline accessor would leave a jmp at /Ob1.
	undefined* m_backBuffer; // 0x42

private:
	undefined m_unk0x46[0x3ae - 0x46]; // 0x46
};
#pragma pack()

#endif // VIDEODRIVER_H
