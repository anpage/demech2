#ifndef MENULIST0X10D_H
#define MENULIST0X10D_H

#include "decomp.h"
#include "mainmenubutton.h"
#include "types.h"

#include <windows.h>

class BrassLantern0x414;
class VideoDriver;
struct Collection;

#pragma pack(1)

// SIZE 0x10d
// The buttons of a menu screen.
class MenuList0x10d {
public:
	MenuList0x10d(
		VideoDriver* p_videoDriver,
		BrassLantern0x414* p_font,
		MechU8 p_drawRect,
		MainMenuButton* p_buttons,
		MechS32 p_count
	);
	~MenuList0x10d();
	void FUN_100488ed(MechS32 p_id);
	MechS32 FUN_100489e9(MechS32 p_x, MechS32 p_y);
	void FUN_10048a7c();
	void FUN_10048aec(MechS32 p_id);
	void FUN_10048b95(MainMenuButton p_button, MechS32 p_id, MechU8 p_drawRect);
	void FUN_10048cc1(MechS32 p_id);
	void FUN_10048d65(MechS32 p_id);

private:
	Collection* m_items;        // 0x00
	VideoDriver* m_videoDriver; // 0x04
	BrassLantern0x414* m_font;  // 0x08
	undefined m_colors[0x100];  // 0x0c

public:
	// The archive reader passes it on to the buttons it adds.
	MechU8 m_drawRect; // 0x10c
};

#pragma pack()

#endif // MENULIST0X10D_H
