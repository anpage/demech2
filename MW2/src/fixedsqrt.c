#include "fixedsqrt.h"

#include "decomp.h"
#include "types.h"

#include <math.h>

// STUB: MW2 0x100169b4
void NormalizeVectorGuarded(MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x100169b4);
}

// FUNCTION: MW2 0x10016a2e
MechS32 FUN_10016a2e(MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechDouble x = p_x;
	MechDouble y = p_y;
	MechDouble z = p_z;

	/* Only this grouping of the sum matches. */
	return (MechS32) (sqrt(z * z + (x * x + y * y)) + 0.5);
}
