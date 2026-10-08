// A fragment of joystick.c (see CLAUDE.md, "Code the Matrox edition placed elsewhere goes in a
// fragment"): the Matrox edition defines ScaleJoystickAxis before JoystickPoll, 1.1 after
// JoystickFlushKeyCodes. joystick.c includes it at each target's position.

#include "decomp.h"
#include "types.h"

// Scales a joystick axis reading about p_center to -0x10000..0x10000 by p_scale, with a dead
// zone of p_deadZone on either side.
// FUNCTION: MW2 0x1004ad6c
// FUNCTION: MW2MATROX 0x10026ac1
MechS32 ScaleJoystickAxis(MechS32 p_value, MechS32 p_deadZone, MechS32 p_center, MechDouble p_scale)
{
	if ((p_value -= p_center) < 0) {
		if (p_value < -p_deadZone) {
			p_value += p_deadZone;
			p_value = p_value * p_scale;
			if (p_value < -0x10000) {
				p_value = -0x10000;
			}
		}
		else {
			p_value = 0;
		}
	}
	else if (p_value > p_deadZone) {
		p_value -= p_deadZone;
		p_value = p_value * p_scale;
		if (p_value > 0x10000) {
			p_value = 0x10000;
		}
	}
	else {
		p_value = 0;
	}

	return p_value;
}
