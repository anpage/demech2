#ifndef MOUSESTATE_H
#define MOUSESTATE_H

#include "decomp.h"
#include "types.h"

class BrassLantern0x414;
class VideoDriver;

// SIZE 0x43
class MouseState {
public:
	MouseState(VideoDriver* p_videoDriver, BrassLantern0x414* p_unk0x08, void* p_unk0x0c);
	~MouseState();

	void ReadMouseState();

private:
	undefined m_unk0x00[0x43]; // 0x00
};

#endif // MOUSESTATE_H
