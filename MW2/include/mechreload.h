#ifndef MECHRELOAD_H
#define MECHRELOAD_H

#include "decomp.h"
#include "mech.h"
#include "mechsegment.h"
#include "object.h"
#include "rememberedmech.h"
#include "types.h"

// The functions and globals of mechreload.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_reloadingPlayer;

	// Declared without a prototype: network.c calls it with two more (zero) arguments.
	MechS32 FUN_1007fbe0();
	void RememberLoadMech(Mech* p_mech, MechChar* p_name, MechS32 p_unk0x08, MechChar* p_unk0x0c);
	void RememberMechSegments(Mech* p_mech);
	AmberWillow0x7c* RestoreMechSegments(MechSegment* p_segment);
	MechSegment* SaveMechSegments(AmberWillow0x7c* p_obj);

#ifdef __cplusplus
}
#endif

#endif // MECHRELOAD_H
