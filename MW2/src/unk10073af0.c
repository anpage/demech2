/* Menu actions (the eject menu) and the "dorcs" sequence's data files. */
#include "unk10073af0.h"

#include "blit.h"
#include "clock.h"
#include "config.h"
#include "displaybackend.h"
#include "gamekeys.h"
#include "menu.h"
#include "menupage.h"
#include "mss.h"
#include "palette.h"
#include "palettecolor.h"
#include "players.h"
#include "recttransition.h"
#include "refreshmode.h"
#include "render.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "slateheron.h"
#include "soundfx.h"
#include "types.h"
#include "unk100079d0.h"

#include <stdio.h>
#include <windows.h>

// The dorcs sequence (ShowDorcs): the view shrinks to a point (g_dorcsTransition), a picture
// shows, then another, and a menu.

// GLOBAL: MW2 0x100b1358
RenderTarget g_dorcsPoint = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100b1370
RenderTarget g_dorcsRectFrom = {NULL, 0x7d71, 0x7d71, 0x828f, 0x828f};

// GLOBAL: MW2 0x100b1388
RenderTarget g_dorcsRectTo = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100b13a0
RenderTarget g_dorcsRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100b13b8
RectTransitionState g_dorcsTransitionState = {0, 0, 0};

// GLOBAL: MW2 0x100b13c8
RectTransitionDef g_dorcsTransitionDef = {0xb5, &g_dorcsRectFrom, &g_dorcsRectTo, &g_dorcsRect};

// GLOBAL: MW2 0x100b13d8
RectTransition g_dorcsTransition = {&g_dorcsTransitionState, &g_dorcsTransitionDef};

// GLOBAL: MW2 0x100b13e0
void* g_dorcsGif = NULL;

// GLOBAL: MW2 0x100b13e4
PaletteColor* g_dorcsPalette = NULL;

// GLOBAL: MW2 0x100b13e8
MechU8* g_dorcsGifState = NULL;

// GLOBAL: MW2 0x100b13ec
MechS32 g_dorcsGifLoaded = 0;

// GLOBAL: MW2 0x100b13f0
MechS32 g_dorcsTime = 0;

// GLOBAL: MW2 0x100b13f4
MechS32 g_dorcsReverse = 1;

// A menu item's action: ejects the local player (game key 0x3b) without its sound.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10073af0
void FUN_10073af0(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	MechS32 p_index,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	MenuPage* p_page
)
{
	MechS32 saved;
	MechS32 digit;
	MechS32 selected;

	if (!p_page) {
		return;
	}

	digit = g_menuKey - '1' == p_index;
	selected = p_page->m_selected == p_index;
	if ((selected && g_menuKey == '\r') || digit) {
		g_difficulty->m_invulnerable = 0;
		saved = g_unk0x100ba624;
		g_unk0x100ba624 = 0;
		FUN_1005c78a(0x3b);
		g_unk0x100ba624 = saved;
		FreeMenus();
	}
}

// A menu item's action: ejects the local player's mech.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10073ba6
void FUN_10073ba6(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	MechS32 p_index,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	MenuPage* p_page
)
{
	MechS32 digit;
	MechS32 selected;
	Player* player;

	if (!p_page) {
		return;
	}

	digit = g_menuKey - '1' == p_index;
	selected = p_page->m_selected == p_index;
	if ((selected && g_menuKey == '\r') || digit) {
		g_unk0x100b1350 = 1;
		g_difficulty->m_invulnerable = 0;
		player = g_players[g_localPlayerId];
		EjectPlayer(player->m_mech, 0);
		FreeMenus();
	}
}

// Reads vfx/<p_name>.bin whole.
// FUNCTION: MW2 0x10073c62
void* ReadVfxBin(MechChar* p_name)
{
	void* data;
	MechChar path[256];

	sprintf(path, "%s/%s.%s", "vfx", p_name, "bin");
	data = FILE_read(path, NULL);
	return data;
}

// FUNCTION: MW2 0x10073cb5
void FUN_10073cb5(void)
{
	RequestMenuClose(6);
	RequestMenuClose(2);
	RequestMenuClose(4);
	RequestMenuClose(7);
	RequestMenuClose(8);
	RequestMenuClose(5);
}

// The dorcs sequence's state.
// GLOBAL: MW2 0x100bf084
MechS32 g_dorcsState;

// GLOBAL: MW2 0x100bf058
RenderTarget g_dorcsSavedTarget;

// GLOBAL: MW2 0x100bf070
RenderTarget g_dorcsGifTarget;

// GLOBAL: MW2 0x100c2cec
PaletteColor g_unk0x100c2cec;

// The dorcs sequence's frame draw callback: steps the sequence (g_dorcsState) each frame.
// FUNCTION: MW2 0x10073cfc
void UpdateDorcs(void)
{
	RenderTarget* rect;
	MechS32 index;
	RenderTarget saved;
	RenderTarget* target;
	MechS32 i;

	switch (g_dorcsState) {
	case 0:
		g_dorcsSavedTarget = g_currentRenderTarget;
		g_dorcsGifLoaded = 0;
		g_dorcsTransition.m_def->m_first = &g_dorcsRectFrom;
		if (!g_dorcsTransition.m_def->m_first->m_buffer) {
			target = g_dorcsTransition.m_def->m_first;
			ScaleRectToScreen(&g_mainPixelBuffer, target, target);
			target->m_buffer = &g_mainPixelBuffer;
			target = g_dorcsTransition.m_def->m_second;
			ScaleRectToScreen(&g_mainPixelBuffer, target, target);
			target->m_buffer = &g_mainPixelBuffer;
			target = g_dorcsTransition.m_def->m_out;
			target->m_buffer = &g_mainPixelBuffer;
			target = &g_dorcsPoint;
			target->m_buffer = &g_mainPixelBuffer;
			ScaleRectToScreen(&g_mainPixelBuffer, target, target);
		}

		g_dorcsTime = g_currentClock + 0x389;
		g_dorcsState = 1;
		if (g_dorcsPreviousDrawCallback) {
			g_dorcsPreviousDrawCallback();
		}
		break;
	case 1:
		if (g_currentClock < g_dorcsTime) {
			if (g_dorcsPreviousDrawCallback) {
				g_dorcsPreviousDrawCallback();
			}
			break;
		}

		g_dorcsTransition.m_def->m_duration = 0x279;
		g_dorcsReverse = 1;
		StartRectTransition(&g_dorcsTransition);
		g_dorcsState = 2;
	case 2:
		g_unk0x100a5a24 = g_unk0x100a5f18;
		g_unk0x100a5f18 = 0;
		FUN_10073cb5();
		rect = UpdateRectTransition(g_dorcsReverse, &g_dorcsTransition);
		if (rect) {
			saved = g_renderTargets[g_renderTargetIndex];
			g_renderTargets[g_renderTargetIndex] = *rect;
			index = g_renderTargetIndex;
			g_renderTargetIndex = -1;
			SelectRenderTarget(index);
			if (g_dorcsPreviousDrawCallback) {
				g_dorcsPreviousDrawCallback();
			}

			g_unk0x10176ebc = 1;
			g_renderTargets[g_renderTargetIndex] = saved;
			break;
		}
		else if (g_dorcsReverse == 1) {
			g_dorcsReverse = 0;
			StartRectTransition(&g_dorcsTransition);
			break;
		}
		else {
			g_dorcsTransition.m_def->m_duration = 0x2d4;
			g_dorcsTransition.m_def->m_first = &g_dorcsPoint;
			StartRectTransition(&g_dorcsTransition);
			g_dorcsState = 3;
		}
	case 3:
		g_unk0x100a5f18 = 0;
		FUN_10073cb5();
		rect = UpdateRectTransitionByAxis(1, &g_dorcsTransition);
		if (rect) {
			saved = g_renderTargets[g_renderTargetIndex];
			g_renderTargets[g_renderTargetIndex] = *rect;
			index = g_renderTargetIndex;
			g_renderTargetIndex = -1;
			SelectRenderTarget(index);
			FillView(&g_dorcsSavedTarget, 0);
			if (g_dorcsPreviousDrawCallback) {
				g_dorcsPreviousDrawCallback();
			}

			FUN_100570e9(&g_currentRenderTarget, 10);
			g_renderTargets[g_renderTargetIndex] = saved;
			g_currentRenderTarget = g_dorcsSavedTarget;
			break;
		}
		else {
			index = g_renderTargetIndex;
			g_renderTargetIndex = -1;
			SelectRenderTarget(index);
			g_dorcsState = 4;
		}
	case 4:
		g_unk0x100a5f18 = 0;
		FUN_10073cb5();
		g_dorcsGifTarget = g_currentRenderTarget;
		g_dorcsGifState = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x502e);
		if (g_dorcsGifState) {
			g_dorcsPalette =
				HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x100 * sizeof(PaletteColor));
			if (g_dorcsPalette) {
				g_currentDisplayBackend->m_setPalette(0, 0x100, g_dorcsPalette, 1);
				g_dorcsGif = ReadVfxBin("vfxjk");
				if (g_dorcsGif) {
					FUN_1005705e(&g_dorcsGifTarget, &g_dorcsGifTarget, g_dorcsGif);
					FillView(&g_currentRenderTarget, 0);
					BlitGif(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
					if (g_windowActive) {
						g_currentRefreshMode->m_flip();
					}

					ReadGifPalette(g_dorcsGif, (MechU8*) g_dorcsPalette);
					g_currentDisplayBackend->m_blendPalettes(g_dorcsPalette, 0xb5);
					g_dorcsGifLoaded = 1;
				}
			}
		}

		g_dorcsTime = g_currentClock + 0x10f;
		g_dorcsState = 5;
		break;
	case 5:
		g_unk0x100a5f18 = 0;
		FUN_10073cb5();
		if (g_currentClock < g_dorcsTime) {
			if (g_dorcsGifLoaded) {
				BlitGif(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
				if (g_windowActive) {
					g_currentRefreshMode->m_flip();
				}
			}
		}
		else {
			if (g_dorcsGif) {
				MEM_free_lock(g_dorcsGif);
			}

			g_dorcsGif = ReadVfxBin("vfxhd");
			if (g_dorcsGif && g_dorcsGifLoaded) {
				for (i = 0; i < 0x100; i++) {
					g_dorcsPalette[i] = g_unk0x100c2cec;
				}

				g_currentDisplayBackend->m_blendPalettes(g_dorcsPalette, 0x5a);
				FillView(&g_currentRenderTarget, 0);
				BlitGif(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
				if (g_windowActive) {
					g_currentRefreshMode->m_flip();
				}

				ReadGifPalette(g_dorcsGif, (MechU8*) g_dorcsPalette);
				g_currentDisplayBackend->m_blendPalettes(g_dorcsPalette, 0xb5);
			}
			else {
				g_dorcsGifLoaded = 0;
			}

			g_dorcsTime = g_currentClock + 0x10f;
			g_dorcsState = 6;
		}
		break;
	case 6:
		g_unk0x100a5f18 = 0;
		FUN_10073cb5();
		if (g_currentClock < g_dorcsTime) {
			if (g_dorcsGifLoaded) {
				BlitGif(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
			}
		}
		else {
			if (!g_dorcsGifLoaded) {
				ApplyPaletteResource(g_currentPalette);
			}

			RequestMenu(3);
			g_dorcsState = 7;
		}
		break;
	case 7:
		FillView(&g_currentRenderTarget, 0);
		if (g_dorcsGifLoaded) {
			BlitGif(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
		}

		if (!GetOpenMenu()) {
			g_dorcsTime = g_currentClock + 0x10f;
			g_dorcsState = -1;
		}
		break;
	case -1:
		g_unk0x100a5f18 = 0;
		FUN_10073cb5();
		if (g_currentClock < g_dorcsTime) {
			FillView(&g_currentRenderTarget, 0);
			if (g_dorcsGifLoaded) {
				BlitGif(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
			}
		}
		else {
			FillView(&g_currentRenderTarget, 0);
			if (g_windowActive) {
				g_currentRefreshMode->m_flip();
			}

			ApplyPaletteResource(g_currentPalette);
			if (g_dorcsGif) {
				MEM_free_lock(g_dorcsGif);
			}

			g_dorcsGif = NULL;
			if (g_dorcsPalette) {
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_dorcsPalette);
			}

			g_dorcsPalette = NULL;
			if (g_dorcsGifState) {
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_dorcsGifState);
			}

			g_dorcsGifState = NULL;
			g_unk0x100a6cc8.m_frameDrawCallback = g_dorcsPreviousDrawCallback;
			g_unk0x100a5a24 = 1;
			g_unk0x10176ebc = 1;
			g_dorcsState = 0;
		}
		break;
	}
}

// Starts the dorcs sequence: UpdateDorcs draws the frames in place of the frame draw callback.
// FUNCTION: MW2 0x100745b2
void ShowDorcs(void)
{
	g_dorcsPreviousDrawCallback = g_unk0x100a6cc8.m_frameDrawCallback;
	g_unk0x100a6cc8.m_frameDrawCallback = UpdateDorcs;
	g_dorcsState = 0;
}
