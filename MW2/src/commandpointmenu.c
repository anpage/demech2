/* The menu of command point 3 (menu 8), the lance mate the third command key commands. A
   data-only object: its data follows bandpoly.c's in 1.1 and faceshade.c's in the Matrox edition,
   and commandmenu.c's follows it in the 3Dfx edition. */
#include "commandpointmenu.h"

#include "commandmenu.h"
#include "menu.h"
#include "menupage.h"
#include "targeting.h"
#include "types.h"

#include <stddef.h>

// GLOBAL: MW2 0x100a2350
// GLOBAL: MW2GBL 0x100a0c78
// GLOBAL: MW2PREMERCS 0x100c1cf0
// GLOBAL: MW2MATROX 0x100a4960
// GLOBAL: MW23DFX 0x10260588
// GLOBAL: MW2ATI 0x10194880
// GLOBAL: MW2S3 0x10062b70
// GLOBAL: MW2POWERVR 0x101bfbc8
PANE g_commandPoint3MenuTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a2368
// GLOBAL: MW2GBL 0x100a0c90
// GLOBAL: MW2PREMERCS 0x100c1d08
// GLOBAL: MW2MATROX 0x100a4978
// GLOBAL: MW23DFX 0x102605a0
// GLOBAL: MW2ATI 0x10194898
// GLOBAL: MW2S3 0x10062b88
// GLOBAL: MW2POWERVR 0x101bfbe0
PANE g_commandPoint3MenuBackgroundTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a2380
// GLOBAL: MW2GBL 0x100a0ca8
// GLOBAL: MW2MATROX 0x100a4990
// GLOBAL: MW23DFX 0x102605b8
// GLOBAL: MW2ATI 0x101948b0
// GLOBAL: MW2S3 0x10062ba0
// GLOBAL: MW2POWERVR 0x101bfbf8
MenuDefinition g_commandPoint3Menu = {
	&g_commandPoint3MenuTarget,
	10,
	g_commandPoint3MenuPageStack,
	0,
	-1,
	NULL,
	&g_commandPoint3MenuBackgroundTarget,
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
	&g_commandPoint3Page
};

// GLOBAL: MW2 0x10177060
// GLOBAL: MW2GBL 0x10135040
// GLOBAL: MW2PREMERCS 0x1014e600
// GLOBAL: MW2MATROX 0x10212db0
// GLOBAL: MW23DFX 0x10153580
// GLOBAL: MW2ATI 0x10087590
// GLOBAL: MW2S3 0x1014d9d0
// GLOBAL: MW2POWERVR 0x1007db70
MenuPage* g_commandPoint3MenuPageStack[8];
