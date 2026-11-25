#include "lancemenu.h"

#include "ai.h"
#include "decomp.h"
#include "menu.h"
#include "menupage.h"
#include "navpoint.h"
#include "players.h"
#include "targeting.h"
#include "team.h"
#include "types.h"

// The order each AI slot was given last, an index into g_orderChoices (7: none); slot 0 is the
// whole star's.
// GLOBAL: MW2 0x100acaf0
// GLOBAL: MW2GBL 0x1009b790
// GLOBAL: MW2PREMERCS 0x100c5c10
// GLOBAL: MW2MATROX 0x100bd320
// GLOBAL: MW23DFX 0x10264468
// GLOBAL: MW2ATI 0x10198760
// GLOBAL: MW2S3 0x10065cb8
// GLOBAL: MW2POWERVR 0x101c3ad8
MechS32 g_lanceOrders[8] = {7, 7, 7, 7, 7, 0, 0, 0};

// Fills the page with an item per AI slot (from template 5) and a last one (template 6), marking
// the slots whose player is in AI state 12. With no slot, or every slot marked, the page has only
// templates 7 and 6.
// FUNCTION: MW2 0x10065f50
// FUNCTION: MW2GBL 0x10005290
// FUNCTION: MW2MATROX 0x1008e640
// FUNCTION: MW23DFX 0x10064390
// FUNCTION: MW2ATI 0x10061a00
// FUNCTION: MW2S3 0x100254a0
// FUNCTION: MW2POWERVR 0x1005c770
MechS32 PrepareCommandComputerPage(MenuDefinition* p_menu, MenuPage* p_page)
{
	MechS32 marked;
	MechS32 count;
	MechS32 slot;
	MechS32 i;
	MechS32 index;
	MechS32 src;

	marked = 0;
	if (!p_page) {
		return FALSE;
	}

	count = GetLocalStarSize() - 1;
	if (count) {
		src = 5;
		slot = count + 1;
		p_page->m_items[slot] = p_page->m_items[src];
		src = 6;
		slot++;
		p_page->m_items[slot] = p_page->m_items[src];
		p_page->m_itemCount = slot + 1;
		for (i = 0; i < count; i++) {
			index = FindStarSlotPlayer(i + 1);
			if (index < g_playerCount && g_players[index]->m_ai.m_state == 12) {
				p_page->m_items[i + 1].m_type = 1;
				marked++;
			}
		}
	}

	if (!count || marked == count) {
		p_page->m_items[0] = p_page->m_items[7];
		p_page->m_items[1] = p_page->m_items[6];
		p_page->m_itemCount = 2;
	}

	return TRUE;
}

// Unless the page's AI slot holds a player outside AI state 12, keeps only the page's last two
// items.
// FUNCTION: MW2 0x100660c2
// FUNCTION: MW2GBL 0x10005403
// FUNCTION: MW2MATROX 0x1008e7b2
// FUNCTION: MW23DFX 0x10064480
// FUNCTION: MW2ATI 0x10061af0
// FUNCTION: MW2S3 0x10025590
// FUNCTION: MW2POWERVR 0x1005c860
MechS32 PrepareCommandPointPage(MenuDefinition* p_menu, MenuPage* p_page)
{
	MechS32 count;
	MechS32 slot;
	MechS32 index;
	MechS32 src;
	MechU32 unk0x08;
	MechS32 valid;

	if (!p_page) {
		return FALSE;
	}

	unk0x08 = p_page->m_aiSlot;
	count = GetLocalStarSize() - 1;
	valid = unk0x08 <= count;
	if (valid) {
		index = FindStarSlotPlayer(unk0x08);
		valid = index < g_playerCount && g_players[index]->m_ai.m_state != 12;
	}

	if (!valid) {
		src = p_page->m_itemCount;
		slot = 0;
		p_page->m_items[slot] = p_page->m_items[src];
		src = p_page->m_itemCount - 1;
		slot++;
		p_page->m_items[slot] = p_page->m_items[src];
		p_page->m_itemCount = slot + 1;
	}

	return TRUE;
}

// FUNCTION: MW2 0x100661ef
// FUNCTION: MW2GBL 0x10005531
// FUNCTION: MW2PREMERCS 0x100828cf
// FUNCTION: MW2MATROX 0x1008e8e0
// FUNCTION: MW23DFX 0x10064510
// FUNCTION: MW2ATI 0x10061b80
// FUNCTION: MW2S3 0x10025620
// FUNCTION: MW2POWERVR 0x1005c8f0
MechS32 GetLanceOrder(MechS32 p_index)
{
	MechS32 state;

	state = 7;
	if (p_index < 8) {
		state = g_lanceOrders[p_index];
	}

	return state;
}

// FUNCTION: MW2 0x10066223
// FUNCTION: MW2GBL 0x10005565
// FUNCTION: MW2PREMERCS 0x10082903
// FUNCTION: MW2MATROX 0x1008e914
// FUNCTION: MW23DFX 0x10064530
// FUNCTION: MW2ATI 0x10061ba0
// FUNCTION: MW2S3 0x10025640
// FUNCTION: MW2POWERVR 0x1005c910
MechS32 GetFormation(MechS32 p_arg)
{
	return GetTeamFormation(g_localStar);
}

// FUNCTION: MW2 0x10066241
// FUNCTION: MW2GBL 0x10005583
// FUNCTION: MW2PREMERCS 0x10082921
// FUNCTION: MW2MATROX 0x1008e932
// FUNCTION: MW23DFX 0x10064540
// FUNCTION: MW2ATI 0x10061bb0
// FUNCTION: MW2S3 0x10025650
// FUNCTION: MW2POWERVR 0x1005c920
void SelectFormation(MechS32 p_formation, MechS32 p_value)
{
	g_lanceOrders[0] = 0;
	SetTeamFormation(g_localStar, p_formation);
	RequestMenuClose(1);
}

// Returns the AI state of the player in AI slot p_index, plus one (0: none).
// FUNCTION: MW2 0x10066272
// FUNCTION: MW2GBL 0x100055b4
// FUNCTION: MW2PREMERCS 0x10082952
// FUNCTION: MW2MATROX 0x1008e963
// FUNCTION: MW23DFX 0x10064570
// FUNCTION: MW2ATI 0x10061be0
// FUNCTION: MW2S3 0x10025680
// FUNCTION: MW2POWERVR 0x1005c950
MechS32 GetSlotAiState(MechS32 p_index)
{
	Player* player;
	PlayerAi* ai;
	MechS32 index;
	MechS32 state;

	state = -1;
	if (p_index < 8) {
		index = FindStarSlotPlayer(p_index);
		if (index != -1) {
			player = g_players[index];
			ai = &player->m_ai;
			if (ai) {
				state = ai->m_state;
			}
		}
	}

	return state + 1;
}

// Gives the control the page's AI slot.
// FUNCTION: MW2 0x100662df
// FUNCTION: MW2GBL 0x10005621
// FUNCTION: MW2PREMERCS 0x100829bf
// FUNCTION: MW2MATROX 0x1008e9d0
// FUNCTION: MW23DFX 0x100645a0
// FUNCTION: MW2ATI 0x10061c10
// FUNCTION: MW2S3 0x100256b0
// FUNCTION: MW2POWERVR 0x1005c980
void SetControlSlot(MenuPage* p_page, MenuControl* p_control)
{
	if (!p_page) {
		return;
	}

	if (!p_control) {
		return;
	}

	p_control->m_arg = p_page->m_aiSlot;
}

// Installs GetSlotGoalName as the suffix of the control's choices.
// FUNCTION: MW2 0x10066314
// FUNCTION: MW2GBL 0x10005656
// FUNCTION: MW2PREMERCS 0x100829f4
// FUNCTION: MW2MATROX 0x1008ea05
// FUNCTION: MW23DFX 0x100645b0
// FUNCTION: MW2ATI 0x10061c20
// FUNCTION: MW2S3 0x100256c0
// FUNCTION: MW2POWERVR 0x1005c990
void InstallGoalSuffix(MenuPage* p_page, MenuControl* p_control)
{
	MenuChoicesSuffixFn old;
	MenuChoices* choices;

	if (!p_page) {
		return;
	}

	if (!p_control) {
		return;
	}

	choices = p_control->m_data;
	if (!choices) {
		return;
	}

	old = choices->m_suffix;
	choices->m_suffix = GetSlotGoalName;
}

// The orders' actions: each records the order for the menu and gives it to the slot's player
// (OrderStarSlot).
// FUNCTION: MW2 0x10066369
// FUNCTION: MW2GBL 0x100056ab
// FUNCTION: MW2PREMERCS 0x10082a49
// FUNCTION: MW2MATROX 0x1008ea5a
// FUNCTION: MW23DFX 0x100645d0
// FUNCTION: MW2ATI 0x10061c40
// FUNCTION: MW2S3 0x100256e0
// FUNCTION: MW2POWERVR 0x1005c9b0
void OrderAttack(MechS32 p_index, MechS32 p_value)
{
	if (p_index < 8) {
		g_lanceOrders[p_index] = 2;
		OrderStarSlot(p_index, 2);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x100663a4
// FUNCTION: MW2GBL 0x100056e6
// FUNCTION: MW2PREMERCS 0x10082a84
// FUNCTION: MW2MATROX 0x1008ea95
// FUNCTION: MW23DFX 0x10064600
// FUNCTION: MW2ATI 0x10061c70
// FUNCTION: MW2S3 0x10025710
// FUNCTION: MW2POWERVR 0x1005c9e0
void OrderEngageAtWill(MechS32 p_index, MechS32 p_value)
{
	if (p_index < 8) {
		g_lanceOrders[p_index] = 1;
		OrderStarSlot(p_index, 3);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x100663df
// FUNCTION: MW2GBL 0x10005721
// FUNCTION: MW2PREMERCS 0x10082abf
// FUNCTION: MW2MATROX 0x1008ead0
// FUNCTION: MW23DFX 0x10064630
// FUNCTION: MW2ATI 0x10061ca0
// FUNCTION: MW2S3 0x10025740
// FUNCTION: MW2POWERVR 0x1005ca10
void OrderJoinFormation(MechS32 p_index, MechS32 p_value)
{
	if (p_index < 8) {
		g_lanceOrders[p_index] = 3;
		OrderStarSlot(p_index, 5);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x1006641a
// FUNCTION: MW2GBL 0x1000575c
// FUNCTION: MW2PREMERCS 0x10082afa
// FUNCTION: MW2MATROX 0x1008eb0b
// FUNCTION: MW23DFX 0x10064660
// FUNCTION: MW2ATI 0x10061cd0
// FUNCTION: MW2S3 0x10025770
// FUNCTION: MW2POWERVR 0x1005ca40
void OrderDefend(MechS32 p_index, MechS32 p_value)
{
	if (p_index < 8) {
		g_lanceOrders[p_index] = 4;
		OrderStarSlot(p_index, 7);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x10066455
// FUNCTION: MW2GBL 0x10005797
// FUNCTION: MW2PREMERCS 0x10082b35
// FUNCTION: MW2MATROX 0x1008eb46
// FUNCTION: MW23DFX 0x10064690
// FUNCTION: MW2ATI 0x10061d00
// FUNCTION: MW2S3 0x100257a0
// FUNCTION: MW2POWERVR 0x1005ca70
void OrderDisengage(MechS32 p_index, MechS32 p_value)
{
	if (p_index < 8) {
		g_lanceOrders[p_index] = 5;
		OrderStarSlot(p_index, 8);
	}

	RequestMenuClose(1);
}

// FUNCTION: MW2 0x10066490
// FUNCTION: MW2GBL 0x100057d2
// FUNCTION: MW2PREMERCS 0x10082b70
// FUNCTION: MW2MATROX 0x1008eb81
// FUNCTION: MW23DFX 0x100646c0
// FUNCTION: MW2ATI 0x10061d30
// FUNCTION: MW2S3 0x100257d0
// FUNCTION: MW2POWERVR 0x1005caa0
void OrderShutdown(MechS32 p_index, MechS32 p_value)
{
	if (p_index < 8) {
		g_lanceOrders[p_index] = 6;
		OrderStarSlot(p_index, 0xb);
	}

	RequestMenuClose(1);
}

// Returns the name of the goal of the control's AI player: a nav, a player or a game thing.
// FUNCTION: MW2 0x100664cb
// FUNCTION: MW2GBL 0x1000580d
// FUNCTION: MW2MATROX 0x1008ebbc
// FUNCTION: MW23DFX 0x100646f0
// FUNCTION: MW2ATI 0x10061d60
// FUNCTION: MW2S3 0x10025800
// FUNCTION: MW2POWERVR 0x1005cad0
MechChar* GetSlotGoalName(
	MenuDefinition* p_menu,
	MenuControl* p_control,
	MechS32 p_index,
	Point p_pos,
	MenuPage* p_page
)
{
	MechS16 target;
	Player* player;
	MechS32 index;
	MechS16 goal;

	index = FindStarSlotPlayer(p_control->m_arg);
	if (index >= g_playerCount) {
		return NULL;
	}

	player = g_players[index];
	if (player->m_ai.m_state == 5) {
		return NULL;
	}

	goal = player->m_ai.m_goal;
	target = goal & 0xff;
	switch (goal & 0xf00) {
	case 0x100:
		return g_navTable[target].m_name;
		break;
	case 0x400:
		return g_gameThings[target].m_name;
		break;
	case 0x200:
		return g_players[target]->m_name;
		break;
	default:
		return NULL;
		break;
	}
}
