#include "unk100758a0.h"

#include "approxlen.h"
#include "config.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "mech.h"
#include "simmain.h"
#include "types.h"

// Damages p_mech and p_other by their collision, when the difficulty has collision damage and
// they met faster than 200000.
// The only diff is a stack-slot permutation of speed and damage.
// FUNCTION: MW2 0x100765f8
void FUN_100765f8(Mech* p_mech, Mech* p_other)
{
	MechS32 speed;
	MechS32 damage;

	if (!g_difficulty->m_collisionDamage) {
		return;
	}

	speed = ApproximateVectorLength(
		p_mech->m_unk0x100 - p_other->m_unk0x100,
		p_mech->m_unk0x104 - p_other->m_unk0x104,
		p_mech->m_unk0x108 - p_other->m_unk0x108
	);
	if (speed > 200000) {
		damage = FixedDiv16(speed - 200000, 1300000) * 3;
		FUN_1007669e(p_mech, p_other, damage);
	}
}

// STUB: MW2 0x1007669e
void FUN_1007669e(Mech* p_mech, Mech* p_other, MechS32 p_damage)
{
	STUB(0x1007669e);
}
