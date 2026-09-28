#ifndef MECHVARIANT_H
#define MECHVARIANT_H

#include "hazelstar0x80.h"
#include "tmpackdatabase.h"
#include "types.h"

// The functions and globals of mechvariant.cpp that other units use.
void FUN_10002d30();
void FUN_10002d8d();
MechS32 FUN_10002de7(MechS32 p_index, MechChar* p_variant, MechChar* p_name);
MechChar* FUN_10003013(MechS32 p_index);
MechS32 FUN_1000307c(MechS32 p_index);
MechS32 FUN_100030e5(MechS32 p_star);
HazelStar0x80* FUN_1000312e(MechS32 p_star);
void FUN_10003175(MechS32 p_star, MechS32 p_formation, MechS32 p_size, MechS32 p_count, MechS32 p_tonnage);
void FUN_10003221();
void FUN_10003d3a(TMPackDataBase* p_database, MechS32 p_campaign);

#endif // MECHVARIANT_H
