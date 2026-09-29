/* The "paused" banner and the pause and resume sounds. */
#include "pausebanner.h"

#include "audio.h"
#include "decomp.h"
#include "loadres.h"
#include "network.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"

// The banner's rectangle, in 16.16 fractions of the screen until the first draw scales it.
// GLOBAL: MW2 0x100a15e0
RenderTarget g_pausedBannerRect = {NULL, 0, 0x3333, 0x10000, 0x6666};

// GLOBAL: MW2 0x100a15f4
MechS32 g_pausedBannerUnscaled = 1;

// FUNCTION: MW2 0x10009e50
void DrawPausedBanner(void)
{
	void* shape;

	shape = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 0x5e, g_unk0x100a8680, 0);
	if (shape) {
		if (g_pausedBannerUnscaled) {
			g_pausedBannerRect.m_buffer = &g_mainPixelBuffer;
			ScaleRectToScreen(&g_mainPixelBuffer, &g_pausedBannerRect, &g_pausedBannerRect);
			FUN_10056fcf(&g_pausedBannerRect, &g_pausedBannerRect, shape, 0);
			g_pausedBannerUnscaled = 0;
		}

		DrawShapeFrame(&g_pausedBannerRect, shape, 0, 0, 0);
	}
}

// FUNCTION: MW2 0x10009ef1
void PlayPauseSound(void)
{
	FUN_1007ea11(0xc6, 100, 0x40, RandomSampleRate());
}

// FUNCTION: MW2 0x10009f13
void PlayResumeSound(void)
{
	FUN_1007ea11(0xf1, 0x32, 0x40, RandomSampleRate());
}

// Pauses the clock and the audio, outside a network game.
// FUNCTION: MW2 0x10009f35
void FUN_10009f35(void)
{
	if (!g_netRole) {
		PauseTimer(0x80, 1);
		PauseAudio();
	}
}

// Resumes the clock and the audio, outside a network game.
// FUNCTION: MW2 0x10009f61
void FUN_10009f61(void)
{
	if (!g_netRole) {
		PauseTimer(0x80, 0);
		ResumeAudio();
	}
}
