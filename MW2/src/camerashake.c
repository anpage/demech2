#include "camerashake.h"

#include "clock.h"
#include "decomp.h"
#include "eyepoint.h"
#include "polydraw.h"
#include "ramp.h"
#include "types.h"

#include <string.h>

// The camera shake: a list of up to ten keys, each an offset of the eyepoint's position and
// orientation reached over a duration. The keys play in order, ramping from one offset to the
// next, on top of the eyepoint's base values.

DECOMP_SIZE_ASSERT(CameraShakeKey, 0x1c)

// GLOBAL: MW2 0x100acb10
// GLOBAL: MW2MATROX 0x100ac624
MechS32 g_cameraShakeKeyCount = 10;

// GLOBAL: MW2 0x100acb14
// GLOBAL: MW2MATROX 0x100ac628
MechS32 g_cameraShakeActive = 0;

// GLOBAL: MW2 0x100becc8
// GLOBAL: MW2MATROX 0x100c21e8
ScalarRamp g_cameraShakeHeading;

// GLOBAL: MW2 0x100becd8
// GLOBAL: MW2MATROX 0x100c21c8
MechS32 g_cameraShakeKey;

// GLOBAL: MW2 0x100bece0
// GLOBAL: MW2MATROX 0x100c2090
ScalarRamp g_cameraShakeZ;

// GLOBAL: MW2 0x100becf0
// GLOBAL: MW2MATROX 0x100c21e0
MechS32 g_cameraShakeKeyTime;

// GLOBAL: MW2 0x100becf8
// GLOBAL: MW2MATROX 0x100c2080
ScalarRamp g_cameraShakeRoll;

// GLOBAL: MW2 0x100bed08
// GLOBAL: MW2MATROX 0x100c21d0
ScalarRamp g_cameraShakeX;

// GLOBAL: MW2 0x100bed18
// GLOBAL: MW2MATROX 0x100c21b8
ScalarRamp g_cameraShakePitch;

// GLOBAL: MW2 0x100bed28
// GLOBAL: MW2MATROX 0x100c20a0
CameraShakeKey g_cameraShakeKeys[10];

// GLOBAL: MW2 0x100bee40
// GLOBAL: MW2MATROX 0x100c2070
ScalarRamp g_cameraShakeY;

// FUNCTION: MW2 0x100665e0
// FUNCTION: MW2MATROX 0x100591a0
void ResetCameraShake(void)
{
	ClearCameraShakeKeys();
}

// Moves the eyepoint by the shake's current offset. Returns FALSE when no shake is playing.
// Stack-slot permutation: base10, base0c, base14, x, y and z.
// FUNCTION: MW2 0x100665f0
// FUNCTION: MW2MATROX 0x100591b0
MechS32 UpdateCameraShake(void)
{
	MechScalar base10;
	MechScalar base0c;
	MechScalar base14;
	MechScalar x;
	MechScalar y;
	MechScalar z;

	if (g_cameraShakeKey < 0 || g_cameraShakeKey >= g_cameraShakeKeyCount) {
		g_cameraShakeActive = FALSE;
	}

	if (!g_cameraShakeActive) {
		return FALSE;
	}

	g_cameraShakeKeyTime += g_deltaTime;
	if (g_cameraShakeKeyTime >= g_cameraShakeKeys[g_cameraShakeKey].m_duration) {
		g_cameraShakeKeyTime = 0;
		g_cameraShakeKey++;
		if (g_cameraShakeKey < g_cameraShakeKeyCount) {
			StartCameraShakeKey(g_cameraShakeKey);
		}
	}

	GetCockpitEyeView(&base10, &base0c, &base14, &x, &y, &z);
	g_eyepoint->m_x = x + UpdateScalarRamp(&g_cameraShakeX);
	g_eyepoint->m_y = y + UpdateScalarRamp(&g_cameraShakeY);
	g_eyepoint->m_z = z + UpdateScalarRamp(&g_cameraShakeZ);
	g_eyepoint->m_pitch = base10 + UpdateScalarRamp(&g_cameraShakePitch);
	g_eyepoint->m_heading = base0c + UpdateScalarRamp(&g_cameraShakeHeading);
	g_eyepoint->m_roll = base14 + UpdateScalarRamp(&g_cameraShakeRoll);
	return TRUE;
}

// FUNCTION: MW2 0x10066758
// FUNCTION: MW2MATROX 0x1005930c
void StartCameraShake(void)
{
	g_cameraShakeKey = 0;
	g_cameraShakeKeyTime = 0;
	if (g_cameraShakeKeyCount > 0) {
		g_cameraShakeActive = TRUE;
		StartCameraShakeKey(0);
	}
}

// Starts the ramps toward a key, from the eyepoint's current offset (the first key) or from the
// previous key.
// Stack-slot permutation: x, seconds, off14, off0c, off10, z, key and y. Operand order: the
// original compares p_key >= g_cameraShakeKeyCount with p_key in eax.
// FUNCTION: MW2 0x10066798
// FUNCTION: MW2MATROX 0x1005934c
void StartCameraShakeKey(MechS32 p_key)
{
	MechScalar x;
	MechDouble seconds;
	MechScalar off14;
	MechScalar off0c;
	MechScalar off10;
	MechScalar z;
	CameraShakeKey* key;
	MechScalar y;

	if (p_key >= g_cameraShakeKeyCount || p_key >= 10 || p_key < 0) {
		return;
	}

	if (p_key == 0) {
		GetCockpitEyeView(&off10, &off0c, &off14, &x, &y, &z);
		x = g_eyepoint->m_x - x;
		y = g_eyepoint->m_y - y;
		z = g_eyepoint->m_z - z;
		off10 = g_eyepoint->m_pitch - off10;
		off0c = g_eyepoint->m_heading - off0c;
		off14 = g_eyepoint->m_roll - off14;
	}
	else {
		x = g_cameraShakeX.m_value;
		y = g_cameraShakeY.m_value;
		z = g_cameraShakeZ.m_value;
		off10 = g_cameraShakePitch.m_value;
		off0c = g_cameraShakeHeading.m_value;
		off14 = g_cameraShakeRoll.m_value;
	}

	key = &g_cameraShakeKeys[p_key];
	seconds = key->m_duration / 181.0;
	StartScalarRamp(&g_cameraShakeX, key->m_x, x, seconds);
	StartScalarRamp(&g_cameraShakeY, key->m_y, y, seconds);
	StartScalarRamp(&g_cameraShakeZ, key->m_z, z, seconds);
	StartScalarRamp(&g_cameraShakePitch, key->m_pitch, off10, seconds);
	StartScalarRamp(&g_cameraShakeHeading, key->m_heading, off0c, seconds);
	StartScalarRamp(&g_cameraShakeRoll, key->m_roll, off14, seconds);
}

// FUNCTION: MW2 0x10066968
// FUNCTION: MW2MATROX 0x10059537
void ClearCameraShakeKeys(void)
{
	memset(g_cameraShakeKeys, 0, sizeof(g_cameraShakeKeys));
	g_cameraShakeKeyCount = 0;
	g_cameraShakeKey = 0;
	g_cameraShakeKeyTime = 0;
	g_cameraShakeActive = FALSE;
}

// FUNCTION: MW2 0x100669a9
// FUNCTION: MW2MATROX 0x10059578
void AddCameraShakeKey(
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_pitch,
	MechScalar p_heading,
	MechScalar p_roll,
	MechDouble p_seconds
)
{
	CameraShakeKey* key;

	if (g_cameraShakeKeyCount >= 10) {
		return;
	}

	key = &g_cameraShakeKeys[g_cameraShakeKeyCount];
	key->m_x = p_x;
	key->m_y = p_y;
	key->m_z = p_z;
	key->m_pitch = p_pitch;
	key->m_heading = p_heading;
	key->m_roll = p_roll;
	key->m_duration = p_seconds * 181.0;
	g_cameraShakeKeyCount++;
}

// FUNCTION: MW2 0x10066a2e
// FUNCTION: MW2MATROX 0x100595fd
MechS32 IsCameraShaking(void)
{
	return g_cameraShakeActive;
}
