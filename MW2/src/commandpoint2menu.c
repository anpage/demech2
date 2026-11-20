/* The menu of command point 2 (menu 7), like commandpointmenu.c's of command point 3. A
   data-only object: its data follows view.c's in 1.1 (commandmenu.c's follows it), settings.c's
   in the Matrox edition and commandmenu.c's in the 3Dfx edition. */

#include "commandmenu.h"
#include "menu.h"
#include "menupage.h"
#include "types.h"

#include <stddef.h>

// GLOBAL: MW2 0x100a7130
// GLOBAL: MW2MATROX 0x100a84a0
// GLOBAL: MW23DFX 0x10261748
// GLOBAL: MW2ATI 0x10195a40
PANE g_commandPoint2MenuTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a7148
// GLOBAL: MW2MATROX 0x100a84b8
// GLOBAL: MW23DFX 0x10261760
// GLOBAL: MW2ATI 0x10195a58
PANE g_commandPoint2MenuBackgroundTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a7160
// GLOBAL: MW2MATROX 0x100a84d0
// GLOBAL: MW23DFX 0x10261778
// GLOBAL: MW2ATI 0x10195a70
MenuDefinition g_commandPoint2Menu = {
	&g_commandPoint2MenuTarget,
	10,
	g_commandPoint2MenuPageStack,
	0,
	-1,
	NULL,
	&g_commandPoint2MenuBackgroundTarget,
	-1,
	NULL,
	225,
	219,
	1,
	NULL,
	14,
	14,
	8,
	{0, 0},
	{0, 0},
	{0x51f, 0},
	{0x51f, 0},
	{0x6666, 0},
	&g_commandPoint2Page
};

// GLOBAL: MW2 0x100ea800
// GLOBAL: MW2MATROX 0x101d4a10
// GLOBAL: MW23DFX 0x10153540
// GLOBAL: MW2ATI 0x10087550
MenuPage* g_commandPoint2MenuPageStack[8];
