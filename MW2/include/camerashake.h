#ifndef CAMERASHAKE_H
#define CAMERASHAKE_H

#include "fixedfloat.h"
#include "ramp.h"
#include "types.h"

// An offset of the eyepoint's first six fields (its position and orientation).
// SIZE 0x1c
typedef struct CameraShakeKey {
	MechScalar m_x;       // 0x00
	MechScalar m_y;       // 0x04
	MechScalar m_z;       // 0x08
	MechScalar m_heading; // 0x0c
	MechScalar m_pitch;   // 0x10
	MechScalar m_roll;    // 0x14
	MechS32 m_duration;   // 0x18 — in clock ticks
} CameraShakeKey;

// The functions and globals of camerashake.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_cameraShakeKeyCount;
	extern MechS32 g_cameraShakeActive;
	extern CameraShakeKey g_cameraShakeKeys[10];

	void ResetCameraShake(void);
	MechS32 UpdateCameraShake(void);
	void StartCameraShake(void);
	void StartCameraShakeKey(MechS32 p_key);
	void ClearCameraShakeKeys(void);
	void AddCameraShakeKey(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_pitch,
		MechScalar p_heading,
		MechScalar p_roll,
		MechDouble p_seconds
	);
	MechS32 IsCameraShaking(void);

#ifdef __cplusplus
}
#endif

#endif // CAMERASHAKE_H
