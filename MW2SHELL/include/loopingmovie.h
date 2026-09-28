#ifndef LOOPINGMOVIE_H
#define LOOPINGMOVIE_H

#include "decomp.h"
#include "smackw32.h"
#include "types.h"

// A looping Smacker movie drawn into a rectangle of the shell (the "amwlogo1" logo on the
// options, leaderboard and credits screens).
// SIZE 0x18
class LoopingMovie {
public:
	LoopingMovie(MechChar* p_name, MechS32 p_unk0x04, MechS32 p_unk0x08);
	~LoopingMovie();

	void FUN_100161dd(MechS32 p_left, MechS32 p_top);

	void FUN_1001630b();

private:
	Smack* m_smack;   // 0x00
	MechS32 m_left;   // 0x04
	MechS32 m_top;    // 0x08
	MechS32 m_width;  // 0x0c
	MechS32 m_height; // 0x10
	MechS32 m_frame;  // 0x14
};

#endif // LOOPINGMOVIE_H
