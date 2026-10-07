#include "ramp.h"

#include "clock.h"
#include "fixedfloat.h"
#include "types.h"

// Values that move toward a target over time, on the 181 Hz clock: linear ramps, which reach
// the target after a duration given in seconds, optionally wrapping around a period (angles),
// and eased values, which close a fixed fraction of the distance at each step.

DECOMP_SIZE_ASSERT(Ramp, 0x10)
#ifdef MW2_MATROX
DECOMP_SIZE_ASSERT(FloatRamp, 0x10)
#endif
DECOMP_SIZE_ASSERT(WrappedRamp, 0x14)
DECOMP_SIZE_ASSERT(EasedValue, 0x0c)

// FUNCTION: MW2 0x100495b0
// FUNCTION: MW2MATROX 0x1002dfe0
MechS32 StartRamp(Ramp* p_ramp, MechS32 p_target, MechS32 p_value, MechDouble p_seconds)
{
	MechS32 result = FALSE;

	p_ramp->m_target = p_target;
	p_ramp->m_value = p_value;
	p_ramp->m_duration = p_seconds * 181.0;
	p_ramp->m_time = g_currentClock;
	if (p_ramp->m_duration > 0) {
		result = TRUE;
	}

	return result;
}

// FUNCTION: MW2 0x10049611
// FUNCTION: MW2MATROX 0x1002e041
MechS32 UpdateRamp(Ramp* p_ramp)
{
	MechS32 delta;
	MechS32 step;

	delta = p_ramp->m_target - p_ramp->m_value;
	if (delta != 0) {
		step = (g_currentClock - p_ramp->m_time) * delta / p_ramp->m_duration;
		if ((delta < 0 && step <= delta) || (delta > 0 && step >= delta)) {
			p_ramp->m_value = p_ramp->m_target;
		}
		else {
			p_ramp->m_value += step;
		}
	}

	p_ramp->m_time = g_currentClock;
	return p_ramp->m_value;
}

#ifdef MW2_MATROX
// The edition's ramps of float values (Mech's, the camera's).
// FUNCTION: MW2MATROX 0x1002e0db
MechS32 StartFloatRamp(FloatRamp* p_ramp, MechFloat p_target, MechFloat p_value, MechDouble p_seconds)
{
	MechS32 result = FALSE;

	p_ramp->m_target = p_target;
	p_ramp->m_value = p_value;
	p_ramp->m_duration = p_seconds * 181.0;
	p_ramp->m_time = g_currentClock;
	if (p_ramp->m_duration > 1e-07f) {
		result = TRUE;
	}

	return result;
}

// Unlike UpdateRamp, a value within 1e-07 of the target lands on it.
// FUNCTION: MW2MATROX 0x1002e141
MechFloat UpdateFloatRamp(FloatRamp* p_ramp)
{
	MechFloat delta;
	MechFloat step;

	delta = p_ramp->m_target - p_ramp->m_value;
	if (FIXED_IS_NONZERO(delta)) {
		step = (g_currentClock - p_ramp->m_time) * delta / p_ramp->m_duration;
		if ((FIXED_IS_NEGATIVE(delta) && step <= delta) || (delta > 1e-07f && step >= delta)) {
			p_ramp->m_value = p_ramp->m_target;
		}
		else {
			p_ramp->m_value += step;
		}
	}
	else {
		p_ramp->m_value = p_ramp->m_target;
	}

	p_ramp->m_time = g_currentClock;
	return p_ramp->m_value;
}
#endif

// FUNCTION: MW2 0x100496ab
// FUNCTION: MW2MATROX 0x1002e226
MechS32 StartWrappedRamp(
	WrappedRamp* p_ramp,
	MechScalar p_target,
	MechScalar p_value,
	MechDouble p_seconds,
	MechScalar p_period
)
{
	MechS32 result = FALSE;

	p_ramp->m_target = p_target;
	p_ramp->m_value = p_value;
	p_ramp->m_duration = p_seconds * 181.0;
	p_ramp->m_time = g_currentClock;
	p_ramp->m_period = p_period;
	if (p_ramp->m_duration > 0) {
		result = TRUE;
	}

	return result;
}

// Moves the value the short way around the period.
// FUNCTION: MW2 0x10049715
// FUNCTION: MW2MATROX 0x1002e295
MechScalar UpdateWrappedRamp(WrappedRamp* p_ramp)
{
#ifdef MW2_MATROX
	// The declaration order puts the comparison with half after delta's store.
	MechScalar delta;
	MechScalar half;
#else
	MechScalar half;
	MechScalar delta;
#endif
	MechScalar step;

#ifdef MW2_MATROX
	half = p_ramp->m_period / 2.0f;
	delta = p_ramp->m_target - p_ramp->m_value;
	delta = fmod(delta, p_ramp->m_period);
#else
	half = p_ramp->m_period >> 1;
	delta = p_ramp->m_target - p_ramp->m_value;
	delta %= p_ramp->m_period;
#endif
	if (delta > half) {
		delta -= p_ramp->m_period;
	}
	else if (delta < -half) {
		delta += p_ramp->m_period;
	}

	step = (g_currentClock - p_ramp->m_time) * delta / p_ramp->m_duration;
#ifdef MW2_MATROX
	if ((FIXED_IS_NEGATIVE(delta) && step <= delta) || (delta > 1e-07f && step >= delta)) {
#else
	if ((delta < 0 && step <= delta) || (delta > 0 && step >= delta)) {
#endif
		p_ramp->m_value = p_ramp->m_target;
	}
	else {
		p_ramp->m_value += step;
	}

#ifdef MW2_MATROX
	p_ramp->m_value = fmod(p_ramp->m_value, p_ramp->m_period);
#else
	p_ramp->m_value = p_ramp->m_value % p_ramp->m_period;
#endif
	p_ramp->m_time = g_currentClock;
	return p_ramp->m_value;
}

// Sets the target, brought within half a period of zero.
// FUNCTION: MW2 0x10049805
// FUNCTION: MW2MATROX 0x1002e3de
MechScalar SetWrappedRampTarget(WrappedRamp* p_ramp, MechScalar p_target)
{
#ifdef MW2_MATROX
	MechScalar target;
	MechScalar half;
#else
	MechScalar half;
	MechScalar target;
#endif

#ifdef MW2_MATROX
	half = p_ramp->m_period / 2.0f;
	target = fmod(p_target, p_ramp->m_period);
#else
	half = p_ramp->m_period >> 1;
	target = p_target % p_ramp->m_period;
#endif
	if (target > half) {
		target -= p_ramp->m_period;
	}
	else if (target < -half) {
		target += p_ramp->m_period;
	}

	p_ramp->m_target = target;
	return target;
}

// FUNCTION: MW2 0x10049871
// FUNCTION: MW2MATROX 0x1002e471
MechS32 UpdateEasedValue(EasedValue* p_value)
{
	MechS32 delta;
	MechS32 step;

	delta = p_value->m_target - p_value->m_value;
	step = delta >> p_value->m_shift;
	p_value->m_value += step;
	return p_value->m_value;
}
