#include "brightnessmenu.h"

#include "audio.h"
#include "brightness.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "soundconfig.h"
#include "types.h"

// The brightness setting (0-15) as a 16.16 fraction: read, previewed, set (and saved to
// MW2SND.CFG's record), and the preview undone.

// FUNCTION: MW2 0x100745e0
// FUNCTION: MW2MATROX 0x100171a0
// FUNCTION: MW23DFX 0x100641a0
// FUNCTION: MW2ATI 0x10061850
// FUNCTION: MW2S3 0x100365b0
// FUNCTION: MW2POWERVR 0x1005c5c0
MechS32 GetBrightnessFraction(MechS32 p_arg)
{
#ifdef MW2_FLOAT_SCALARS
	return (MechS32) ((g_displayBrightness / 15.0f) * 65536.0f + 0.5f);
#else
	return FixedMul16(g_displayBrightness << 16, FixedDiv16(1, 15));
#endif
}

// FUNCTION: MW2 0x1007460e
// FUNCTION: MW2MATROX 0x100171eb
// FUNCTION: MW23DFX 0x100641d0
// FUNCTION: MW2ATI 0x10061880
// FUNCTION: MW2S3 0x100365e0
// FUNCTION: MW2POWERVR 0x1005c5f0
void PreviewBrightnessFraction(MechS32 p_arg, MechS32 p_value)
{
	MechS32 brightness;

	brightness = FixedMul16(p_value, 15);
	if (brightness != g_brightnessSetting) {
		g_brightnessSetting = brightness;
		PreviewBrightness(brightness);
	}
}

// FUNCTION: MW2 0x1007464f
// FUNCTION: MW2MATROX 0x1001722c
// FUNCTION: MW23DFX 0x10064210
// FUNCTION: MW2ATI 0x100618c0
// FUNCTION: MW2S3 0x10036620
// FUNCTION: MW2POWERVR 0x1005c630
void SetBrightnessFraction(MechS32 p_arg, MechS32 p_value)
{
	g_displayBrightness = g_brightnessSetting = FixedMul16(p_value, 15);
	g_mw2SndCfgData->m_displayBrightness = g_displayBrightness;
	PreviewBrightness(g_displayBrightness);
}

// FUNCTION: MW2 0x10074693
// FUNCTION: MW2MATROX 0x10017270
// FUNCTION: MW23DFX 0x10064260
// FUNCTION: MW2ATI 0x10061910
// FUNCTION: MW2S3 0x10036670
// FUNCTION: MW2POWERVR 0x1005c680
void RestoreBrightness(MechS32 p_arg)
{
#ifdef MW2_3DFX
	// The 3Dfx edition's loads g_displayBrightness once, for the store and the call.
	PreviewBrightness(g_brightnessSetting = g_displayBrightness);
#else
	g_brightnessSetting = g_displayBrightness;
	PreviewBrightness(g_displayBrightness);
#endif
}
