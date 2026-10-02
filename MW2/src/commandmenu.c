/* The lance command menus: the command computer (menu 1), command point 2 (menu 7) and the
   pages of each command point, the formation and the orders to all. A data-only object: its data
   follows unk1004b980.c's. */
#include "commandmenu.h"

#include "mainmenu.h"
#include "menu.h"
#include "menuchoices.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "rendertarget.h"
#include "types.h"
#include "unk10065f50.h"

#include <stddef.h>

// GLOBAL: MW2 0x100a7130
Pane g_commandPoint2MenuTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a7148
Pane g_commandPoint2MenuBackgroundTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a7160
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

// GLOBAL: MW2 0x100a71d0
MechChar g_unk0x100a71d0[] = "COMMAND COMPUTER";

// GLOBAL: MW2 0x100a71e8
MechChar g_unk0x100a71e8[] = "Command All";

// GLOBAL: MW2 0x100a71f8
MechChar g_unk0x100a71f8[] = "Change Formation";

// GLOBAL: MW2 0x100a7210
MechChar g_unk0x100a7210[] = "NO STAR MATES";

// GLOBAL: MW2 0x100a7220
MechChar g_unk0x100a7220[] = "Current Form:";

// GLOBAL: MW2 0x100a7230
MechChar g_unk0x100a7230[] = "Command Point 2";

// GLOBAL: MW2 0x100a7240
MechChar g_unk0x100a7240[] = "Command Point 3";

// GLOBAL: MW2 0x100a7250
MechChar g_unk0x100a7250[] = "Command Point 4";

// GLOBAL: MW2 0x100a7260
MechChar g_unk0x100a7260[] = "Command Point 5";

// GLOBAL: MW2 0x100a7270
MechChar g_unk0x100a7270[] = "Status:";

// GLOBAL: MW2 0x100a7278
MechChar g_unk0x100a7278[] = "NOT AVAILABLE";

// GLOBAL: MW2 0x100a7288
MechChar g_unk0x100a7288[] = "COMMAND ALL";

// GLOBAL: MW2 0x100a7298
MechChar g_unk0x100a7298[] = "CHANGE FORMATION";

// GLOBAL: MW2 0x100a72b0
MechChar g_unk0x100a72b0[] = "COMMAND POINT 2";

// GLOBAL: MW2 0x100a72c0
MechChar g_unk0x100a72c0[] = "COMMAND POINT 3";

// GLOBAL: MW2 0x100a72d0
MechChar g_unk0x100a72d0[] = "COMMAND POINT 4";

// GLOBAL: MW2 0x100a72e0
MechChar g_unk0x100a72e0[] = "COMMAND POINT 5";

// GLOBAL: MW2 0x100a72f0
MechChar g_unk0x100a72f0[] = "Echelon Left";

// GLOBAL: MW2 0x100a7300
MechChar g_unk0x100a7300[] = "Echelon Right";

// GLOBAL: MW2 0x100a7310
MechChar g_unk0x100a7310[] = "Line Abreast";

// GLOBAL: MW2 0x100a7320
MechChar g_unk0x100a7320[] = "Line Astern";

// GLOBAL: MW2 0x100a7330
MechChar g_unk0x100a7330[] = "V Form";

// GLOBAL: MW2 0x100a7338
MechChar g_unk0x100a7338[] = "Wedge";

// GLOBAL: MW2 0x100a7340
MechChar g_unk0x100a7340[] = "No Formation";

// GLOBAL: MW2 0x100a7350
MechChar g_unk0x100a7350[] = "Attack";

// GLOBAL: MW2 0x100a7358
MechChar g_unk0x100a7358[] = "Defend";

// GLOBAL: MW2 0x100a7360
MechChar g_unk0x100a7360[] = "Join Formation";

// GLOBAL: MW2 0x100a7370
MechChar g_unk0x100a7370[] = "Change Formation";

// GLOBAL: MW2 0x100a7388
MechChar g_unk0x100a7388[] = "Disengage";

// GLOBAL: MW2 0x100a7398
MechChar g_unk0x100a7398[] = "Engage at Will";

// GLOBAL: MW2 0x100a73a8
MechChar g_unk0x100a73a8[] = "Shutdown";

// GLOBAL: MW2 0x100a73b8
MechChar g_unk0x100a73b8[] = "No Cmd";

// GLOBAL: MW2 0x100a73c0
MechChar g_unk0x100a73c0[] = "Attack My Target";

// GLOBAL: MW2 0x100a73d8
MechChar g_unk0x100a73d8[] = "Defend My Target";

// GLOBAL: MW2 0x100a73f0
MechChar g_unk0x100a73f0[] = "Join Formation";

// GLOBAL: MW2 0x100a7400
MechChar g_unk0x100a7400[] = "Disengage";

// GLOBAL: MW2 0x100a7410
MechChar g_unk0x100a7410[] = "Engage at Will";

// GLOBAL: MW2 0x100a7420
MechChar g_unk0x100a7420[] = "Shutdown";

// GLOBAL: MW2 0x100a7430
MechChar g_unk0x100a7430[] = "None ";

// GLOBAL: MW2 0x100a7438
MechChar g_unk0x100a7438[] = "Idle ";

// GLOBAL: MW2 0x100a7440
MechChar g_unk0x100a7440[] = "Avoiding ";

// GLOBAL: MW2 0x100a7450
MechChar g_unk0x100a7450[] = "Targeting ";

// GLOBAL: MW2 0x100a7460
MechChar g_unk0x100a7460[] = "Engaging ";

// GLOBAL: MW2 0x100a7470
MechChar g_unk0x100a7470[] = "Disengaging ";

// GLOBAL: MW2 0x100a7480
MechChar g_unk0x100a7480[] = "In Formation ";

// GLOBAL: MW2 0x100a7490
MechChar g_unk0x100a7490[] = "Reconning ";

// GLOBAL: MW2 0x100a74a0
MechChar g_unk0x100a74a0[] = "Defending ";

// GLOBAL: MW2 0x100a74b0
MechChar g_unk0x100a74b0[] = "En Route ";

// GLOBAL: MW2 0x100a74c0
MechChar g_unk0x100a74c0[] = "Disengaging ";

// GLOBAL: MW2 0x100a74d0
MechChar g_unk0x100a74d0[] = "Silent ";

// GLOBAL: MW2 0x100a74d8
MechChar g_unk0x100a74d8[] = "Shutdown ";

// GLOBAL: MW2 0x100a74e8
MechChar g_unk0x100a74e8[] = "Destroyed ";

// GLOBAL: MW2 0x100a74f8
MenuChoices g_unk0x100a74f8 = {
	NULL,
	7,
	{g_unk0x100a72f0,
	 g_unk0x100a7300,
	 g_unk0x100a7310,
	 g_unk0x100a7320,
	 g_unk0x100a7330,
	 g_unk0x100a7338,
	 g_unk0x100a7340}
};

// GLOBAL: MW2 0x100a7540
MenuChoices g_unk0x100a7540 = {
	NULL,
	8,
	{g_unk0x100a7370,
	 g_unk0x100a7398,
	 g_unk0x100a7350,
	 g_unk0x100a7360,
	 g_unk0x100a7358,
	 g_unk0x100a7388,
	 g_unk0x100a73a8,
	 g_unk0x100a73b8}
};

// GLOBAL: MW2 0x100a7588
MenuChoices g_unk0x100a7588 = {
	FUN_100664cb,
	14,
	{g_unk0x100a7430,
	 g_unk0x100a7438,
	 g_unk0x100a7440,
	 g_unk0x100a7450,
	 g_unk0x100a7460,
	 g_unk0x100a7470,
	 g_unk0x100a7480,
	 g_unk0x100a7490,
	 g_unk0x100a74a0,
	 g_unk0x100a74b0,
	 g_unk0x100a74c0,
	 g_unk0x100a74d0,
	 g_unk0x100a74d8,
	 g_unk0x100a74e8}
};

// GLOBAL: MW2 0x100a75d0
MenuChoices g_unk0x100a75d0 = {NULL, 0};

// GLOBAL: MW2 0x100a7618
MenuControl g_unk0x100a7618 = {3, 0, &g_unk0x100a74f8, 0, NULL, FUN_10066223, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7640
MenuControl g_unk0x100a7640 = {3, 0, &g_unk0x100a7540, 0, NULL, FUN_100661ef, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7668
MenuControl g_unk0x100a7668 = {3, 0, &g_unk0x100a7588, 1, FUN_10066314, FUN_10066272, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7690
MenuControl g_unk0x100a7690 = {3, 0, &g_unk0x100a7588, 2, FUN_10066314, FUN_10066272, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a76b8
MenuControl g_unk0x100a76b8 = {3, 0, &g_unk0x100a7588, 3, FUN_10066314, FUN_10066272, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a76e0
MenuControl g_unk0x100a76e0 = {3, 0, &g_unk0x100a7588, 4, FUN_10066314, FUN_10066272, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7708
MenuControl g_unk0x100a7708 = {2, 0, &g_unk0x100a75d0, 0, NULL, NULL, NULL, FUN_10066241, NULL};

// GLOBAL: MW2 0x100a7730
MenuControl g_unk0x100a7730 = {2, 0, &g_unk0x100a75d0, 1, NULL, NULL, NULL, FUN_10066241, NULL};

// GLOBAL: MW2 0x100a7758
MenuControl g_unk0x100a7758 = {2, 0, &g_unk0x100a75d0, 2, NULL, NULL, NULL, FUN_10066241, NULL};

// GLOBAL: MW2 0x100a7780
MenuControl g_unk0x100a7780 = {2, 0, &g_unk0x100a75d0, 3, NULL, NULL, NULL, FUN_10066241, NULL};

// GLOBAL: MW2 0x100a77a8
MenuControl g_unk0x100a77a8 = {2, 0, &g_unk0x100a75d0, 4, NULL, NULL, NULL, FUN_10066241, NULL};

// GLOBAL: MW2 0x100a77d0
MenuControl g_unk0x100a77d0 = {2, 0, &g_unk0x100a75d0, 5, NULL, NULL, NULL, FUN_10066241, NULL};

// GLOBAL: MW2 0x100a77f8
MenuControl g_unk0x100a77f8 = {2, 0, &g_unk0x100a75d0, 0, FUN_100662df, NULL, NULL, FUN_10066369, NULL};

// GLOBAL: MW2 0x100a7820
MenuControl g_unk0x100a7820 = {2, 0, &g_unk0x100a75d0, 0, FUN_100662df, NULL, NULL, FUN_100663a4, NULL};

// GLOBAL: MW2 0x100a7848
MenuControl g_unk0x100a7848 = {2, 0, &g_unk0x100a75d0, 0, FUN_100662df, NULL, NULL, FUN_100663df, NULL};

// GLOBAL: MW2 0x100a7870
MenuControl g_unk0x100a7870 = {2, 0, &g_unk0x100a75d0, 0, FUN_100662df, NULL, NULL, FUN_1006641a, NULL};

// GLOBAL: MW2 0x100a7898
MenuControl g_unk0x100a7898 = {2, 0, &g_unk0x100a75d0, 0, FUN_100662df, NULL, NULL, FUN_10066455, NULL};

// GLOBAL: MW2 0x100a78c0
MenuControl g_unk0x100a78c0 = {2, 0, &g_unk0x100a75d0, 0, FUN_100662df, NULL, NULL, FUN_10066490, NULL};

// GLOBAL: MW2 0x100a78e8
MenuPage g_changeFormationPage = {
	0,
	g_unk0x100a7298,
	0,
	8,
	0,
	NULL,
	{{3, g_unk0x100a7220, FUN_10072dab, &g_unk0x100a7618, NULL},
	 {5, g_unk0x100a72f0, FUN_10072dab, &g_unk0x100a7708, NULL},
	 {5, g_unk0x100a7300, FUN_10072dab, &g_unk0x100a7730, NULL},
	 {5, g_unk0x100a7310, FUN_10072dab, &g_unk0x100a7758, NULL},
	 {5, g_unk0x100a7320, FUN_10072dab, &g_unk0x100a7780, NULL},
	 {5, g_unk0x100a7330, FUN_10072dab, &g_unk0x100a77a8, NULL},
	 {5, g_unk0x100a7338, FUN_10072dab, &g_unk0x100a77d0, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7a40
MenuPage g_commandAllPage = {
	0,
	g_unk0x100a7288,
	0,
	7,
	0,
	NULL,
	{{3, g_unk0x100a7270, FUN_10072dab, &g_unk0x100a7640, NULL},
	 {5, g_unk0x100a73c0, FUN_10072dab, &g_unk0x100a77f8, NULL},
	 {5, g_unk0x100a73d8, FUN_10072dab, &g_unk0x100a7870, NULL},
	 {5, g_unk0x100a73f0, FUN_10072dab, &g_unk0x100a7848, NULL},
	 {5, g_unk0x100a7410, FUN_10072dab, &g_unk0x100a7820, NULL},
	 {5, g_unk0x100a7420, FUN_10072dab, &g_unk0x100a78c0, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7b98
MenuPage g_commandPoint2Page = {
	0,
	g_unk0x100a72b0,
	1,
	7,
	0,
	FUN_100660c2,
	{{3, g_unk0x100a7270, FUN_10072dab, &g_unk0x100a7668, NULL},
	 {5, g_unk0x100a73c0, FUN_10072dab, &g_unk0x100a77f8, NULL},
	 {5, g_unk0x100a73d8, FUN_10072dab, &g_unk0x100a7870, NULL},
	 {5, g_unk0x100a73f0, FUN_10072dab, &g_unk0x100a7848, NULL},
	 {5, g_unk0x100a7410, FUN_10072dab, &g_unk0x100a7820, NULL},
	 {5, g_unk0x100a7420, FUN_10072dab, &g_unk0x100a78c0, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL},
	 {3, g_unk0x100a7278, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7cf0
MenuPage g_commandPoint3Page = {
	0,
	g_unk0x100a72c0,
	2,
	7,
	0,
	FUN_100660c2,
	{{3, g_unk0x100a7270, FUN_10072dab, &g_unk0x100a7690, NULL},
	 {5, g_unk0x100a73c0, FUN_10072dab, &g_unk0x100a77f8, NULL},
	 {5, g_unk0x100a73d8, FUN_10072dab, &g_unk0x100a7870, NULL},
	 {5, g_unk0x100a73f0, FUN_10072dab, &g_unk0x100a7848, NULL},
	 {5, g_unk0x100a7410, FUN_10072dab, &g_unk0x100a7820, NULL},
	 {5, g_unk0x100a7420, FUN_10072dab, &g_unk0x100a78c0, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL},
	 {3, g_unk0x100a7278, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7e48
MenuPage g_commandPoint4Page = {
	0,
	g_unk0x100a72d0,
	3,
	7,
	0,
	FUN_100660c2,
	{{3, g_unk0x100a7270, FUN_10072dab, &g_unk0x100a76b8, NULL},
	 {5, g_unk0x100a73c0, FUN_10072dab, &g_unk0x100a77f8, NULL},
	 {5, g_unk0x100a73d8, FUN_10072dab, &g_unk0x100a7870, NULL},
	 {5, g_unk0x100a73f0, FUN_10072dab, &g_unk0x100a7848, NULL},
	 {5, g_unk0x100a7410, FUN_10072dab, &g_unk0x100a7820, NULL},
	 {5, g_unk0x100a7420, FUN_10072dab, &g_unk0x100a78c0, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL},
	 {3, g_unk0x100a7278, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7fa0
MenuPage g_commandPoint5Page = {
	0,
	g_unk0x100a72e0,
	4,
	7,
	0,
	FUN_100660c2,
	{{3, g_unk0x100a7270, FUN_10072dab, &g_unk0x100a76e0, NULL},
	 {5, g_unk0x100a73c0, FUN_10072dab, &g_unk0x100a77f8, NULL},
	 {5, g_unk0x100a73d8, FUN_10072dab, &g_unk0x100a7870, NULL},
	 {5, g_unk0x100a73f0, FUN_10072dab, &g_unk0x100a7848, NULL},
	 {5, g_unk0x100a7410, FUN_10072dab, &g_unk0x100a7820, NULL},
	 {5, g_unk0x100a7420, FUN_10072dab, &g_unk0x100a78c0, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL},
	 {3, g_unk0x100a7278, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a80f8
MenuPage g_commandComputerPage = {
	0,
	g_unk0x100a71d0,
	0,
	8,
	0,
	FUN_10065f50,
	{{0, g_unk0x100a71f8, FUN_10072dab, &g_unk0x100a7618, &g_changeFormationPage},
	 {0, g_unk0x100a7230, FUN_10072dab, &g_unk0x100a7668, &g_commandPoint2Page},
	 {0, g_unk0x100a7240, FUN_10072dab, &g_unk0x100a7690, &g_commandPoint3Page},
	 {0, g_unk0x100a7250, FUN_10072dab, &g_unk0x100a76b8, &g_commandPoint4Page},
	 {0, g_unk0x100a7260, FUN_10072dab, &g_unk0x100a76e0, &g_commandPoint5Page},
	 {0, g_unk0x100a71e8, FUN_10072dab, &g_unk0x100a7640, &g_commandAllPage},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL},
	 {3, g_unk0x100a7210, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a8250
Pane g_commandMenuTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a8268
Pane g_commandMenuBackgroundTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a8280
MenuDefinition g_commandMenu = {
	&g_commandMenuTarget,
	10,
	g_commandMenuPageStack,
	0,
	-1,
	NULL,
	&g_commandMenuBackgroundTarget,
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
	&g_commandComputerPage
};

// GLOBAL: MW2 0x100ea7e0
MenuPage* g_commandMenuPageStack[8];

// GLOBAL: MW2 0x100ea800
MenuPage* g_commandPoint2MenuPageStack[8];
