#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include "types.h"

// The functions and globals of resourcefile.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechChar* MakeResourcePath(MechChar* p_name);
	MechS32 LoadFile(MechChar* p_name, MechS32* p_size, void** p_data, MechS32 p_preallocated);

#ifdef __cplusplus
}
#endif

#endif // RESOURCEFILE_H
