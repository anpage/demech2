/* IsNavReached, which the Matrox edition places after DrawMapTarget in cockpit.c's object: cockpit.c
   includes this file at 1.1's position, and at the Matrox edition's. */
#include "cockpit.h"
#include "navpoint.h"
#include "types.h"

// Returns whether team p_team has reached the nav point. The team test is an | where an & was
// meant: any reached nav counts.
// FUNCTION: MW2 0x1003e645
// FUNCTION: MW2MATROX 0x1007461a
MechS32 IsNavReached(NavPoint* p_nav, MechS32 p_team)
{
	return (p_nav->m_flags & 0x20) && (p_nav->m_teamsReached | (1 << p_team));
}
