#ifndef RAMP_H
#define RAMP_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

// SIZE 0x10
typedef struct Ramp {
	MechS32 m_time;     // 0x00 — the clock at the last update
	MechS32 m_target;   // 0x04
	MechS32 m_value;    // 0x08
	MechS32 m_duration; // 0x0c — in clock ticks
} Ramp;

#ifdef MW2_MATROX
// The Matrox edition's ramp of a float value, with its duration in (fractional) clock ticks.
// SIZE 0x10
typedef struct FloatRamp {
	MechS32 m_time;       // 0x00 — the clock at the last update
	MechFloat m_target;   // 0x04
	MechFloat m_value;    // 0x08
	MechFloat m_duration; // 0x0c — in clock ticks
} FloatRamp;
#endif

// ScalarRamp: a ramp of a MechScalar value (a Ramp in 1.1, a FloatRamp in the Matrox edition,
// which keeps 1.1's integer ramps for the player's aim distance only).
#ifdef MW2_MATROX
#define ScalarRamp FloatRamp
#define StartScalarRamp StartFloatRamp
#define UpdateScalarRamp UpdateFloatRamp
#else
#define ScalarRamp Ramp
#define StartScalarRamp StartRamp
#define UpdateScalarRamp UpdateRamp
#endif

// A ramp whose value wraps around a period (an angle). The Matrox edition's is all floats.
// SIZE 0x14
typedef struct WrappedRamp {
	MechS32 m_time;        // 0x00
	MechScalar m_target;   // 0x04
	MechScalar m_value;    // 0x08
	MechScalar m_duration; // 0x0c
	MechScalar m_period;   // 0x10
} WrappedRamp;

// SIZE 0x0c
typedef struct EasedValue {
	MechS32 m_value;  // 0x00
	MechS32 m_target; // 0x04
	MechS32 m_shift;  // 0x08 — each step closes 1 / 2^m_shift of the distance
} EasedValue;

// The functions of ramp.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 StartRamp(Ramp* p_ramp, MechS32 p_target, MechS32 p_value, MechDouble p_seconds);
	MechS32 UpdateRamp(Ramp* p_ramp);
#ifdef MW2_MATROX
	MechS32 StartFloatRamp(FloatRamp* p_ramp, MechFloat p_target, MechFloat p_value, MechDouble p_seconds);
	MechFloat UpdateFloatRamp(FloatRamp* p_ramp);
#endif
	MechS32 StartWrappedRamp(
		WrappedRamp* p_ramp,
		MechScalar p_target,
		MechScalar p_value,
		MechDouble p_seconds,
		MechScalar p_period
	);
	MechScalar UpdateWrappedRamp(WrappedRamp* p_ramp);
	MechScalar SetWrappedRampTarget(WrappedRamp* p_ramp, MechScalar p_target);
	MechS32 UpdateEasedValue(EasedValue* p_value);

#ifdef __cplusplus
}
#endif

#endif // RAMP_H
