#include "fadepal.h"

#include "camerashake.h"
#include "decomp.h"
#include "displaybackend.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "fixedsqrt.h"
#include "loadres.h"
#include "palette.h"
#include "refreshmode.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"

// FUNCTION: MW2 0x1004c890
void FUN_1004c890(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4 p_unk0x08)
{
	if (p_unk0x04 > 0) {
		FUN_1007eb64(0, 0, p_unk0x04, 0x32, p_unk0x08, 0x32);
	}
}

// STUB: MW2 0x1004c8bd
void FUN_1004c8bd(MechU32 p_target, undefined4 p_unk0x04, undefined4* p_unk0x08, undefined4 p_unk0x0c)
{
	STUB(0x1004c8bd);
}

// Fades to palette 0x11 over two seconds (0x16a clock ticks).
// FUNCTION: MW2 0x1004ca0d
void FUN_1004ca0d(void)
{
	StartPaletteFade(0x11, 0x16a, 1);
}

// Fades to palette 0x11 over p_level (0-15) fifteenths of two seconds.
// FUNCTION: MW2 0x1004ca29
void FUN_1004ca29(MechU32 p_level)
{
	MechS32 duration;

	if (p_level > 15) {
		p_level = 0x10000;
	}
	else {
		p_level = FixedDiv16(p_level, 15);
	}

	duration = FixedMul16(0x16a, p_level);
	StartPaletteFade(0x11, duration, 1);
}

// Fades (blocking, over 60 steps) to palette slot 0x10, or 0x11 if p_alternate is set, then applies
// that slot.
// Stack-slot permutation: slot and palette.
// FUNCTION: MW2 0x1004ca82
void FadeToEndPalette(MechS32 p_alternate)
{
	MechS32 slot;
	PaletteColor* palette;

	if (p_alternate) {
		slot = 0x11;
	}
	else {
		slot = 0x10;
	}

	palette = FUN_1001a19f(g_unk0x100a8740, g_paletteResourceIds[slot], g_unk0x100a8694, 0);
	if (palette) {
		g_currentDisplayBackend->m_blendPalettes(palette, 0x3c);
		FUN_1001a163(g_paletteResourceIds[slot], g_unk0x100a8694);
		ApplyPaletteResource(slot);
	}
}

// Shakes the camera away from a hit's direction, unless a shake is already playing.
// Stack-slot permutation: off10 and off0c.
// FUNCTION: MW2 0x1004cee2
void PlayPlayerHitFeedback(MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 off10;
	MechS32 off0c;
	MechS32 off14;

	if (IsCameraShaking()) {
		return;
	}

	NormalizeVectorGuarded(&p_x, &p_y, &p_z);
	if (p_x == 0 && p_z == 0) {
		off10 = p_y * 30;
	}
	else {
		off10 = (p_y - p_z) * 5;
	}

	off0c = 0;
	off14 = 0;
	p_x = FixedMul16(p_x, -25);
	p_y = FixedMul16(p_y, -25);
	p_z = FixedMul16(p_z, -25);
	ClearCameraShakeKeys();
	AddCameraShakeKey(p_x, p_y, p_z, off10, off0c, off14, 0.2);
	AddCameraShakeKey(-(p_x >> 1), -(p_y >> 1), -(p_z >> 1), -(off10 >> 1), -(off0c >> 1), -(off14 >> 1), 0.5);
	AddCameraShakeKey(0, 0, 0, 0, 0, 0, 0.2);
	StartCameraShake();
}
