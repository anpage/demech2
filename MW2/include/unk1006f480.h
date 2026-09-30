#ifndef UNK1006F480_H
#define UNK1006F480_H

#include "rendertarget.h"
#include "types.h"

struct Mech;

// The functions and globals of unk1006f480.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1006fba3(void);
	void FUN_1006ff7b(void);
	void FUN_1007005a(struct Mech* p_mech);
	void FUN_100704c1(void);
	void FUN_1007079d(RenderTarget* p_target, MechS32 p_index, MechS32 p_x, MechS32 p_y);
	void FUN_1006f480(void);

#ifdef __cplusplus
}
#endif

#endif // UNK1006F480_H
