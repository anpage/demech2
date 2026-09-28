#ifndef MECHVARIANT_H
#define MECHVARIANT_H

#include "customstar.h"
#include "tmpackdatabase.h"
#include "types.h"

// The functions and globals of mechvariant.cpp that other units use.
void SaveStars();
void RestoreStars();
MechS32 SetStarMech(MechS32 p_index, MechChar* p_variant, MechChar* p_name);
MechChar* FUN_10003013(MechS32 p_index);
MechS32 FUN_1000307c(MechS32 p_index);
MechS32 GetStarFormation(MechS32 p_star);
CustomStar* FUN_1000312e(MechS32 p_star);
void SelectStar(MechS32 p_star, MechS32 p_formation, MechS32 p_size, MechS32 p_count, MechS32 p_tonnage);
void WriteStarFiles();
void DrawStarConfig(TMPackDataBase* p_database, MechS32 p_campaign);

#endif // MECHVARIANT_H
