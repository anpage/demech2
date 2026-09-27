#include "brasslantern0x414.h"
#include "collection.h"
#include "emberglyph0x3e.h"
#include "frostpebble0x10.h"
#include "shellmain.h"
#include "videodriver.h"

#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(FrostPebble0x10, 0x10)

// The menu's collection owns 0x2a-byte entries with a hit rectangle and two text glyphs.
#pragma pack(1)
struct MenuEntry0x2a {
	MechChar* m_text;          // 0x00
	MechS32 m_left;            // 0x04
	MechS32 m_top;             // 0x08
	undefined m_unk0x0c[4];    // 0x0c
	BrassLantern0x414* m_font; // 0x10
	undefined* m_colors;       // 0x14
	FrostPebble0x10* m_rect;   // 0x18
	EmberGlyph0x3e* m_hover;   // 0x1c
	EmberGlyph0x3e* m_label;   // 0x20
	MechS32 m_id;              // 0x24
	MechU8 m_drawRect;         // 0x28
	MechU8 m_enabled;          // 0x29

	~MenuEntry0x2a();
	void FUN_10049003(VideoDriver* p_videoDriver);
	MechU8 FUN_1004902a(MechS32 p_x, MechS32 p_y);
};
#pragma pack()

DECOMP_SIZE_ASSERT(MenuEntry0x2a, 0x2a)

#pragma pack(1)
class MenuList0x10d {
public:
	void FUN_1004883e();
	void FUN_100488ed(MechS32 p_id);
	MechS32 FUN_100489e9(MechS32 p_x, MechS32 p_y);
	void FUN_10048a7c();
	void FUN_10048aec(MechS32 p_id);
	void FUN_10048cc1(MechS32 p_id);
	void FUN_10048d65(MechS32 p_id);

	Collection* m_items;               // 0x00
	VideoDriver* m_videoDriver;        // 0x04
	BrassLantern0x414* m_font;         // 0x08
	undefined m_unk0x0c[0x10d - 0x0c]; // 0x0c
};
#pragma pack()

DECOMP_SIZE_ASSERT(MenuList0x10d, 0x10d)

// FUNCTION: MW2SHELL 0x1004883e
void MenuList0x10d::FUN_1004883e()
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
			m_hover = m_font->FUN_10005522(m_left, m_top, m_text, m_colors);
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
