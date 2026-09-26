#ifndef MOUSESTATE_H
#define MOUSESTATE_H

#include "decomp.h"
#include "types.h"

class BrassLantern0x414;
class VideoDriver;

#pragma pack(1)

// SIZE 0x43
class MouseState {
public:
	MouseState(VideoDriver* p_videoDriver, BrassLantern0x414* p_unk0x08, void* p_unk0x00);
	~MouseState();

	void MoveCursorTo(MechS32 p_x, MechS32 p_y);
	undefined GetDoubleClicked();
	void FUN_1003a91c(undefined4);
	undefined4 GetLeftPressed();
	undefined4 GetRightPressed();
	undefined4 GetMiddlePressed();
	void FUN_1003a988();
	void FUN_1003aa2a(MechS32 p_button);
	void ReadMouseState();

private:
	void* m_unk0x00;              // 0x00
	VideoDriver* m_videoDriver;   // 0x04
	BrassLantern0x414* m_unk0x08; // 0x08
	MechS32 m_unk0x0c;            // 0x0c
	undefined4 m_leftPressed;     // 0x10
	undefined4 m_rightPressed;    // 0x14
	undefined4 m_middlePressed;   // 0x18
	undefined m_doubleClicked;    // 0x1c
	undefined m_unk0x1d;          // 0x1d
	undefined m_unk0x1e;          // 0x1e
	undefined4 m_lastClickTime;   // 0x1f
	undefined4 m_unk0x23;         // 0x23
	undefined4 m_unk0x27;         // 0x27
	MechS32 m_x;                  // 0x2b
	MechS32 m_y;                  // 0x2f
	undefined4 m_leftDown;        // 0x33
	undefined4 m_rightDown;       // 0x37
	undefined4 m_middleDown;      // 0x3b
	undefined4 m_unk0x3f;         // 0x3f
};

#pragma pack()

// Free function sharing MouseState's unit.
MechS32 GetJoystickDeviceCount();

#endif // MOUSESTATE_H
