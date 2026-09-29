#ifndef COBALTHARBOR_H
#define COBALTHARBOR_H

#include "decomp.h"
#include "rendertarget.h"
#include "types.h"

typedef struct CobaltHarbor0x88 CobaltHarbor0x88;

/* One of the 26 cockpit panels FUN_1006fca5 allocates (0x100c3280): a named rectangle of a
   render target with a table of methods. FUN_100746c0 sets the defaults, and callers replace
   some of the methods and the handlers at 0x78-0x84. */
// SIZE 0x88
struct CobaltHarbor0x88 {
	CobaltHarbor0x88* m_self;                                                 // 0x00
	MechS16 m_enabled;                                                        // 0x04
	MechS16 m_unk0x06;                                                        // 0x06
	undefined4 m_unk0x08;                                                     // 0x08
	MechS32 m_unk0x0c;                                                        // 0x0c
	MechChar m_name[0x20];                                                    // 0x10
	RenderTarget* m_target;                                                   // 0x30
	undefined4 m_unk0x34;                                                     // 0x34
	undefined4 m_unk0x38;                                                     // 0x38
	undefined4 m_unk0x3c;                                                     // 0x3c
	MechS16 m_x;                                                              // 0x40
	MechS16 m_y;                                                              // 0x42
	MechS16 m_width;                                                          // 0x44
	MechS16 m_height;                                                         // 0x46
	void (*m_init)(CobaltHarbor0x88*);                                        // 0x48
	void (*m_unk0x4c)(CobaltHarbor0x88*);                                     // 0x4c
	void (*m_setUnk0x08)(CobaltHarbor0x88*, undefined4);                      // 0x50
	void (*m_setUnk0x0c)(CobaltHarbor0x88*, MechS32);                         // 0x54
	void (*m_setName)(CobaltHarbor0x88*, const MechChar*);                    // 0x58
	void (*m_setTarget)(CobaltHarbor0x88*, RenderTarget*);                    // 0x5c
	void (*m_setUnk0x34)(CobaltHarbor0x88*, undefined4);                      // 0x60
	void (*m_setUnk0x38)(CobaltHarbor0x88*, undefined4);                      // 0x64
	void (*m_setRect)(CobaltHarbor0x88*, MechS32, MechS32, MechS32, MechS32); // 0x68
	void (*m_setUnk0x06)(CobaltHarbor0x88*, MechS32);                         // 0x6c
	void (*m_enable)(CobaltHarbor0x88*);                                      // 0x70
	void (*m_disable)(CobaltHarbor0x88*);                                     // 0x74
	void (*m_unk0x78)();                                                      // 0x78
	void (*m_unk0x7c)();                                                      // 0x7c
	void (*m_unk0x80)();                                                      // 0x80
	void (*m_unk0x84)();                                                      // 0x84
};

#endif // COBALTHARBOR_H
