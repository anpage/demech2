#include "brasslantern0x414.h"
#include "collection.h"
#include "emberglyph0x3e.h"
#include "frostpebble0x10.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "unk1003bf90.h"
#include "unk100711f8.h"
#include "videodriver.h"

#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(FrostPebble0x10, 0x10)

// The menu's collection owns 0x2a-byte entries with a hit rectangle and two text glyphs.
#pragma pack(1)
struct MenuEntry0x2a {
	MechChar* m_text;          // 0x00
	POINT m_textPos;           // 0x04
	MechS32 m_unk0x0c;         // 0x0c
	BrassLantern0x414* m_font; // 0x10
	undefined* m_colors;       // 0x14
	FrostPebble0x10* m_rect;   // 0x18
	EmberGlyph0x3e* m_hover;   // 0x1c
	EmberGlyph0x3e* m_label;   // 0x20
	MechS32 m_id;              // 0x24
	MechU8 m_drawRect;         // 0x28
	MechU8 m_enabled;          // 0x29

	MenuEntry0x2a(
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_right,
		MechS32 p_bottom,
		MechS32 p_id,
		MechU8 p_drawRect,
		VideoDriver* p_videoDriver,
		BrassLantern0x414* p_font,
		MechChar* p_text,
		POINT p_textPos,
		undefined* p_colors,
		MechU8 p_enabled
	);
	~MenuEntry0x2a();
	void FUN_10049003(VideoDriver* p_videoDriver);
	MechU8 FUN_1004902a(MechS32 p_x, MechS32 p_y);
};
#pragma pack()

DECOMP_SIZE_ASSERT(MenuEntry0x2a, 0x2a)

DECOMP_SIZE_ASSERT(MenuList0x10d, 0x10d)

// The font argument is ignored: the buttons always use g_unk0x10071218. m_colors maps colour 0
// to 0xff and 1 to 6, and leaves the others as they are.
// FUNCTION: MW2SHELL 0x100485f0
MenuList0x10d::MenuList0x10d(
	VideoDriver* p_videoDriver,
	BrassLantern0x414* p_font,
	MechU8 p_drawRect,
	MainMenuButton* p_buttons,
	MechS32 p_count
)
{
	EmberGlyph0x3e* label = NULL;
	MechS32 i;
	MechChar* text;
	MenuEntry0x2a* item;

	p_font = g_unk0x10071218;
	m_drawRect = p_drawRect;
	m_videoDriver = p_videoDriver;
	m_font = p_font;

	m_colors[0] = 0xff;
	for (i = 1; i < 0x100; i++) {
		m_colors[i] = i;
	}
	m_colors[1] = 6;

	CreateCollection(&m_items, 10, NULL, 4, NULL);
	for (i = 0; i < p_count; i++) {
		text = p_buttons[i].m_text;
		if (text != NULL && *text == '<') {
			text++;
			label = m_font->FUN_10005522(p_buttons[i].m_textPos.x, p_buttons[i].m_textPos.y, text, m_colors);
		}
		else {
			label = NULL;
		}

		item = new MenuEntry0x2a(
			p_buttons[i].m_left,
			p_buttons[i].m_top,
			p_buttons[i].m_right,
			p_buttons[i].m_bottom,
			i,
			m_drawRect,
			m_videoDriver,
			m_font,
			text,
			p_buttons[i].m_textPos,
			NULL,
			TRUE
		);
		item->m_label = label;
		ExpandCollection(m_items, item);
	}
}

// FUNCTION: MW2SHELL 0x1004883e
MenuList0x10d::~MenuList0x10d()
{
	MechS32 i;
	MenuEntry0x2a* item;

	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		delete item;
	}

	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_items->m_items);
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_items);
}

// FUNCTION: MW2SHELL 0x100488ed
void MenuList0x10d::FUN_100488ed(MechS32 p_id)
{
	Collection* retained;
	MechS32 i;
	MenuEntry0x2a* item;

	CreateCollection(&retained, 10, NULL, 4, NULL);
	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		if (item->m_id >= p_id) {
			delete item;
		}
		else {
			ExpandCollection(retained, item);
		}
		m_items->m_items[i] = NULL;
	}

	ClearCollection(m_items);
	FUN_1003c638(m_items, retained);
	DestroyCollection(retained);
}

// Stack-slot permutation: i and item exchange [ebp-N] slots with the original.
// FUNCTION: MW2SHELL 0x100489e9
MechS32 MenuList0x10d::FUN_100489e9(MechS32 p_x, MechS32 p_y)
{
	MechS32 id;
	MechS32 i;
	MenuEntry0x2a* item;

	id = -1;
	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		if (item->FUN_1004902a(p_x, p_y) == TRUE && item->m_enabled == TRUE) {
			id = item->m_id;
		}
	}

	return id;
}

// Stack-slot permutation: i and item exchange [ebp-N] slots with the original.
// FUNCTION: MW2SHELL 0x10048a7c
void MenuList0x10d::FUN_10048a7c()
{
	MechS32 i;
	MenuEntry0x2a* item;

	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		if (item->m_drawRect == TRUE) {
			item->FUN_10049003(m_videoDriver);
		}
	}
}

// FUNCTION: MW2SHELL 0x10048aec
void MenuList0x10d::FUN_10048aec(MechS32 p_id)
{
	MechS32 i;
	MenuEntry0x2a* item;

	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		if (item->m_id == p_id) {
			FUN_1003c3ba(m_items, item, FALSE);
			delete item;
		}
	}
}

// Adds one button.
// FUNCTION: MW2SHELL 0x10048b95
void MenuList0x10d::FUN_10048b95(MainMenuButton p_button, MechS32 p_id, MechU8 p_drawRect)
{
	EmberGlyph0x3e* label = NULL;
	MechChar* text = p_button.m_text;
	MenuEntry0x2a* item;

	if (text != NULL && *text == '<') {
		text++;
		label = m_font->FUN_10005522(p_button.m_textPos.x, p_button.m_textPos.y, text, m_colors);
	}

	item = new MenuEntry0x2a(
		p_button.m_left,
		p_button.m_top,
		p_button.m_right,
		p_button.m_bottom,
		p_id,
		p_drawRect,
		m_videoDriver,
		m_font,
		text,
		p_button.m_textPos,
		NULL,
		TRUE
	);
	item->m_label = label;
	ExpandCollection(m_items, item);
}

// FUNCTION: MW2SHELL 0x10048cc1
void MenuList0x10d::FUN_10048cc1(MechS32 p_id)
{
	MechS32 i;
	MenuEntry0x2a* item;

	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		if (item->m_id == p_id && !item->m_enabled) {
			item->m_enabled = TRUE;
			if (item->m_label != NULL) {
				m_videoDriver->FUN_100076e8(item->m_label, 1);
				item->m_label->FUN_10047425();
			}
		}
	}
}

// FUNCTION: MW2SHELL 0x10048d65
void MenuList0x10d::FUN_10048d65(MechS32 p_id)
{
	MechS32 i;
	MenuEntry0x2a* item;

	for (i = 0; i < m_items->m_count; i++) {
		item = (MenuEntry0x2a*) CollectionGet(m_items, i);
		if (item->m_id == p_id && item->m_enabled) {
			item->m_enabled = FALSE;
			if (item->m_hover != NULL) {
				delete item->m_hover;
				item->m_hover = NULL;
			}
			if (item->m_label != NULL) {
				item->m_label->Shutdown();
			}
		}
	}
}

// FUNCTION: MW2SHELL 0x10048e43
MenuEntry0x2a::MenuEntry0x2a(
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_right,
	MechS32 p_bottom,
	MechS32 p_id,
	MechU8 p_drawRect,
	VideoDriver* p_videoDriver,
	BrassLantern0x414* p_font,
	MechChar* p_text,
	POINT p_textPos,
	undefined* p_colors,
	MechU8 p_enabled
)
{
	m_id = p_id;
	m_drawRect = p_drawRect;
	m_text = p_text;
	m_textPos = p_textPos;
	m_hover = NULL;
	m_font = p_font;
	m_unk0x0c = -1;
	m_colors = p_colors;
	m_enabled = p_enabled;
	m_rect = new FrostPebble0x10(p_left, p_top, p_right, p_bottom);
	if (m_drawRect == TRUE) {
		FUN_10049003(p_videoDriver);
	}
}

// FUNCTION: MW2SHELL 0x10048f65
MenuEntry0x2a::~MenuEntry0x2a()
{
	if (m_hover != NULL) {
		delete m_hover;
	}
	if (m_label != NULL) {
		delete m_label;
	}
}

// FUNCTION: MW2SHELL 0x10049003
void MenuEntry0x2a::FUN_10049003(VideoDriver* p_videoDriver)
{
	m_rect->FUN_10049199(p_videoDriver);
}

// FUNCTION: MW2SHELL 0x1004902a
MechU8 MenuEntry0x2a::FUN_1004902a(MechS32 p_x, MechS32 p_y)
{
	if (m_enabled && m_rect->FUN_10049245(p_x, p_y)) {
		if (m_hover == NULL && m_text != NULL && strlen(m_text) != 0) {
			m_hover = m_font->FUN_10005522(m_textPos.x, m_textPos.y, m_text, m_colors);
		}
		return TRUE;
	}
	else {
		if (m_hover != NULL) {
			delete m_hover;
			m_hover = NULL;
			if (m_label != NULL) {
				m_label->FUN_10047425();
			}
		}

		return FALSE;
	}
}

// FUNCTION: MW2SHELL 0x10049145
FrostPebble0x10::FrostPebble0x10(undefined4 p_unk0x00, undefined4 p_unk0x04, undefined4 p_unk0x08, undefined4 p_unk0x0c)
{
	m_unk0x00 = p_unk0x00;
	m_unk0x04 = p_unk0x04;
	m_unk0x08 = p_unk0x08;
	m_unk0x0c = p_unk0x0c;
}

// FUNCTION: MW2SHELL 0x10049183
void FrostPebble0x10::FUN_10049183()
{
}

// FUNCTION: MW2SHELL 0x10049199
void FrostPebble0x10::FUN_10049199(VideoDriver* p_videoDriver)
{
	p_videoDriver->FUN_10006e51(m_unk0x00, m_unk0x0c, m_unk0x08, m_unk0x0c, 1);
	p_videoDriver->FUN_10006e51(m_unk0x00, m_unk0x04, m_unk0x08, m_unk0x04, 1);
	p_videoDriver->FUN_10006e51(m_unk0x08, m_unk0x04, m_unk0x08, m_unk0x0c, 1);
	p_videoDriver->FUN_10006e51(m_unk0x00, m_unk0x0c, m_unk0x00, m_unk0x04, 1);
}

// FUNCTION: MW2SHELL 0x10049245
MechU8 FrostPebble0x10::FUN_10049245(MechS32 p_x, MechS32 p_y)
{
	if (m_unk0x0c >= p_y && m_unk0x04 <= p_y && m_unk0x08 >= p_x && m_unk0x00 <= p_x) {
		return TRUE;
	}

	return FALSE;
}
