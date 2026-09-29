#ifndef BWDNAMES_H
#define BWDNAMES_H

#include "bwdname.h"
#include "types.h"

// The functions and globals of bwdnames.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_10058560(BwdName* p_name);
	MechS16* FUN_1005860e(BwdName* p_name);
	void FUN_100586ec(void);

#ifdef __cplusplus
}
#endif

#endif // BWDNAMES_H
