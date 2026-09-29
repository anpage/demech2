#include "fixedsqrt.h"

#include "approxlen.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "types.h"

#include <math.h>

// Scales the vector to length 1.0 (16.16); a zero vector stays as it is.
// FUNCTION: MW2 0x100169b4
void NormalizeVectorGuarded(MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	MechS32 length;

	length = ApproximateVectorLength(*p_x, *p_y, *p_z);
	if (length > 0) {
		*p_x = FixedDiv16(*p_x, length);
		*p_y = FixedDiv16(*p_y, length);
		*p_z = FixedDiv16(*p_z, length);
	}
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
