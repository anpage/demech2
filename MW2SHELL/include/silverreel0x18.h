#ifndef SILVERREEL0X18_H
#define SILVERREEL0X18_H

#include "decomp.h"
#include "types.h"

// A looping Smacker animation drawn into a rectangle of the shell screen.
// SIZE 0x18
class SilverReel0x18 {
public:
	SilverReel0x18(MechChar* p_image, MechS32 p_width, MechS32 p_height);

	void FUN_1001630b();

private:
	undefined m_unk0x00[0x18]; // 0x00
};

#endif // SILVERREEL0X18_H
