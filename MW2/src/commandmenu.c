/* The lance command menus: the command computer (menu 1) and the pages of each command point, the
   formation and the orders to all (command point 2's menu is commandpoint2menu.c's, command point
   3's commandpointmenu.c's). A data-only object: its data follows commandpoint2menu.c's in 1.1,
   inputmap.c's in the Matrox edition and commandpointmenu.c's in the 3Dfx edition. */
#include "commandmenu.h"

#include "lancemenu.h"
#include "mainmenu.h"
#include "menu.h"
#include "menuchoices.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "targeting.h"
#include "types.h"

#include <stddef.h>

// GLOBAL: MW2 0x100a71d0
// GLOBAL: MW2GBL 0x1009f2b8
// GLOBAL: MW23DFX 0x10260628
// GLOBAL: MW2ATI 0x10194920
// GLOBAL: MW2S3 0x10062f48
// GLOBAL: MW2POWERVR 0x101bfc68
MechChar g_commandComputerTitle[] = "COMMAND COMPUTER";

// GLOBAL: MW2 0x100a71e8
// GLOBAL: MW2GBL 0x1009f2d0
// GLOBAL: MW23DFX 0x10260640
// GLOBAL: MW2ATI 0x10194938
// GLOBAL: MW2S3 0x10062f60
// GLOBAL: MW2POWERVR 0x101bfc80
MechChar g_commandAllItem[] = "Command All";

// GLOBAL: MW2 0x100a71f8
// GLOBAL: MW2GBL 0x1009f2e0
// GLOBAL: MW23DFX 0x10260650
// GLOBAL: MW2ATI 0x10194948
// GLOBAL: MW2S3 0x10062f70
// GLOBAL: MW2POWERVR 0x101bfc90
MechChar g_changeFormationItem[] = "Change Formation";

// GLOBAL: MW2 0x100a7210
// GLOBAL: MW2GBL 0x1009f2f8
// GLOBAL: MW23DFX 0x10260668
// GLOBAL: MW2ATI 0x10194960
// GLOBAL: MW2S3 0x10062f88
// GLOBAL: MW2POWERVR 0x101bfca8
MechChar g_noStarMatesText[] = "NO STAR MATES";

// GLOBAL: MW2 0x100a7220
// GLOBAL: MW2GBL 0x1009f308
// GLOBAL: MW23DFX 0x10260678
// GLOBAL: MW2ATI 0x10194970
// GLOBAL: MW2S3 0x10062f98
// GLOBAL: MW2POWERVR 0x101bfcb8
MechChar g_currentFormationLabel[] = "Current Form:";

// GLOBAL: MW2 0x100a7230
// GLOBAL: MW2GBL 0x1009f318
// GLOBAL: MW23DFX 0x10260688
// GLOBAL: MW2ATI 0x10194980
// GLOBAL: MW2S3 0x10062fa8
// GLOBAL: MW2POWERVR 0x101bfcc8
MechChar g_commandPoint2Item[] = "Command Point 2";

// GLOBAL: MW2 0x100a7240
// GLOBAL: MW2GBL 0x1009f328
// GLOBAL: MW23DFX 0x10260698
// GLOBAL: MW2ATI 0x10194990
// GLOBAL: MW2S3 0x10062fb8
// GLOBAL: MW2POWERVR 0x101bfcd8
MechChar g_commandPoint3Item[] = "Command Point 3";

// GLOBAL: MW2 0x100a7250
// GLOBAL: MW2GBL 0x1009f338
// GLOBAL: MW23DFX 0x102606a8
// GLOBAL: MW2ATI 0x101949a0
// GLOBAL: MW2S3 0x10062fc8
// GLOBAL: MW2POWERVR 0x101bfce8
MechChar g_commandPoint4Item[] = "Command Point 4";

// GLOBAL: MW2 0x100a7260
// GLOBAL: MW2GBL 0x1009f348
// GLOBAL: MW23DFX 0x102606b8
// GLOBAL: MW2ATI 0x101949b0
// GLOBAL: MW2S3 0x10062fd8
// GLOBAL: MW2POWERVR 0x101bfcf8
MechChar g_commandPoint5Item[] = "Command Point 5";

// GLOBAL: MW2 0x100a7270
// GLOBAL: MW2GBL 0x1009f358
// GLOBAL: MW23DFX 0x102606c8
// GLOBAL: MW2ATI 0x101949c0
// GLOBAL: MW2S3 0x10062fe8
// GLOBAL: MW2POWERVR 0x101bfd08
MechChar g_statusLabel[] = "Status:";

// GLOBAL: MW2 0x100a7278
// GLOBAL: MW2GBL 0x1009f360
// GLOBAL: MW23DFX 0x102606d0
// GLOBAL: MW2ATI 0x101949c8
// GLOBAL: MW2S3 0x10062ff0
// GLOBAL: MW2POWERVR 0x101bfd10
MechChar g_notAvailableText[] = "NOT AVAILABLE";

// GLOBAL: MW2 0x100a7288
// GLOBAL: MW2GBL 0x1009f370
// GLOBAL: MW23DFX 0x102606e0
// GLOBAL: MW2ATI 0x101949d8
// GLOBAL: MW2S3 0x10063000
// GLOBAL: MW2POWERVR 0x101bfd20
MechChar g_commandAllTitle[] = "COMMAND ALL";

// GLOBAL: MW2 0x100a7298
// GLOBAL: MW2GBL 0x1009f380
// GLOBAL: MW23DFX 0x102606f0
// GLOBAL: MW2ATI 0x101949e8
// GLOBAL: MW2S3 0x10063010
// GLOBAL: MW2POWERVR 0x101bfd30
MechChar g_changeFormationTitle[] = "CHANGE FORMATION";

// GLOBAL: MW2 0x100a72b0
// GLOBAL: MW2GBL 0x1009f398
// GLOBAL: MW23DFX 0x10260708
// GLOBAL: MW2ATI 0x10194a00
// GLOBAL: MW2S3 0x10063028
// GLOBAL: MW2POWERVR 0x101bfd48
MechChar g_commandPoint2Title[] = "COMMAND POINT 2";

// GLOBAL: MW2 0x100a72c0
// GLOBAL: MW2GBL 0x1009f3a8
// GLOBAL: MW23DFX 0x10260718
// GLOBAL: MW2ATI 0x10194a10
// GLOBAL: MW2S3 0x10063038
// GLOBAL: MW2POWERVR 0x101bfd58
MechChar g_commandPoint3Title[] = "COMMAND POINT 3";

// GLOBAL: MW2 0x100a72d0
// GLOBAL: MW2GBL 0x1009f3b8
// GLOBAL: MW23DFX 0x10260728
// GLOBAL: MW2ATI 0x10194a20
// GLOBAL: MW2S3 0x10063048
// GLOBAL: MW2POWERVR 0x101bfd68
MechChar g_commandPoint4Title[] = "COMMAND POINT 4";

// GLOBAL: MW2 0x100a72e0
// GLOBAL: MW2GBL 0x1009f3c8
// GLOBAL: MW23DFX 0x10260738
// GLOBAL: MW2ATI 0x10194a30
// GLOBAL: MW2S3 0x10063058
// GLOBAL: MW2POWERVR 0x101bfd78
MechChar g_commandPoint5Title[] = "COMMAND POINT 5";

// GLOBAL: MW2 0x100a72f0
// GLOBAL: MW2GBL 0x1009f3d8
// GLOBAL: MW23DFX 0x10260748
// GLOBAL: MW2ATI 0x10194a40
// GLOBAL: MW2S3 0x10063068
// GLOBAL: MW2POWERVR 0x101bfd88
MechChar g_echelonLeftText[] = "Echelon Left";

// GLOBAL: MW2 0x100a7300
// GLOBAL: MW2GBL 0x1009f3e8
// GLOBAL: MW23DFX 0x10260758
// GLOBAL: MW2ATI 0x10194a50
// GLOBAL: MW2S3 0x10063078
// GLOBAL: MW2POWERVR 0x101bfd98
MechChar g_echelonRightText[] = "Echelon Right";

// GLOBAL: MW2 0x100a7310
// GLOBAL: MW2GBL 0x1009f3f8
// GLOBAL: MW23DFX 0x10260768
// GLOBAL: MW2ATI 0x10194a60
// GLOBAL: MW2S3 0x10063088
// GLOBAL: MW2POWERVR 0x101bfda8
MechChar g_lineAbreastText[] = "Line Abreast";

// GLOBAL: MW2 0x100a7320
// GLOBAL: MW2GBL 0x1009f408
// GLOBAL: MW23DFX 0x10260778
// GLOBAL: MW2ATI 0x10194a70
// GLOBAL: MW2S3 0x10063098
// GLOBAL: MW2POWERVR 0x101bfdb8
MechChar g_lineAsternText[] = "Line Astern";

// GLOBAL: MW2 0x100a7330
// GLOBAL: MW2GBL 0x1009f418
// GLOBAL: MW23DFX 0x10260788
// GLOBAL: MW2ATI 0x10194a80
// GLOBAL: MW2S3 0x100630a8
// GLOBAL: MW2POWERVR 0x101bfdc8
MechChar g_vFormText[] = "V Form";

// GLOBAL: MW2 0x100a7338
// GLOBAL: MW2GBL 0x1009f420
// GLOBAL: MW23DFX 0x10260790
// GLOBAL: MW2ATI 0x10194a88
// GLOBAL: MW2S3 0x100630b0
// GLOBAL: MW2POWERVR 0x101bfdd0
MechChar g_wedgeText[] = "Wedge";

// GLOBAL: MW2 0x100a7340
// GLOBAL: MW2GBL 0x1009f428
// GLOBAL: MW23DFX 0x10260798
// GLOBAL: MW2ATI 0x10194a90
// GLOBAL: MW2S3 0x100630b8
// GLOBAL: MW2POWERVR 0x101bfdd8
MechChar g_noFormationText[] = "No Formation";

// GLOBAL: MW2 0x100a7350
// GLOBAL: MW2GBL 0x1009f438
// GLOBAL: MW23DFX 0x102607a8
// GLOBAL: MW2ATI 0x10194aa0
// GLOBAL: MW2S3 0x100630c8
// GLOBAL: MW2POWERVR 0x101bfde8
MechChar g_orderAttackText[] = "Attack";

// GLOBAL: MW2 0x100a7358
// GLOBAL: MW2GBL 0x1009f440
// GLOBAL: MW23DFX 0x102607b0
// GLOBAL: MW2ATI 0x10194aa8
// GLOBAL: MW2S3 0x100630d0
// GLOBAL: MW2POWERVR 0x101bfdf0
MechChar g_orderDefendText[] = "Defend";

// GLOBAL: MW2 0x100a7360
// GLOBAL: MW2GBL 0x1009f448
// GLOBAL: MW23DFX 0x102607b8
// GLOBAL: MW2ATI 0x10194ab0
// GLOBAL: MW2S3 0x100630d8
// GLOBAL: MW2POWERVR 0x101bfdf8
MechChar g_orderJoinFormationText[] = "Join Formation";

// GLOBAL: MW2 0x100a7370
// GLOBAL: MW2GBL 0x1009f458
// GLOBAL: MW23DFX 0x102607c8
// GLOBAL: MW2ATI 0x10194ac0
// GLOBAL: MW2S3 0x100630e8
// GLOBAL: MW2POWERVR 0x101bfe08
MechChar g_orderChangeFormationText[] = "Change Formation";

// GLOBAL: MW2 0x100a7388
// GLOBAL: MW2GBL 0x1009f470
// GLOBAL: MW23DFX 0x102607e0
// GLOBAL: MW2ATI 0x10194ad8
// GLOBAL: MW2S3 0x10063100
// GLOBAL: MW2POWERVR 0x101bfe20
MechChar g_orderDisengageText[] = "Disengage";

// GLOBAL: MW2 0x100a7398
// GLOBAL: MW2GBL 0x1009f480
// GLOBAL: MW23DFX 0x102607f0
// GLOBAL: MW2ATI 0x10194ae8
// GLOBAL: MW2S3 0x10063110
// GLOBAL: MW2POWERVR 0x101bfe30
MechChar g_orderEngageAtWillText[] = "Engage at Will";

// GLOBAL: MW2 0x100a73a8
// GLOBAL: MW2GBL 0x1009f490
// GLOBAL: MW23DFX 0x10260800
// GLOBAL: MW2ATI 0x10194af8
// GLOBAL: MW2S3 0x10063120
// GLOBAL: MW2POWERVR 0x101bfe40
MechChar g_orderShutdownText[] = "Shutdown";

// GLOBAL: MW2 0x100a73b8
// GLOBAL: MW2GBL 0x1009f4a0
// GLOBAL: MW23DFX 0x10260810
// GLOBAL: MW2ATI 0x10194b08
// GLOBAL: MW2S3 0x10063130
// GLOBAL: MW2POWERVR 0x101bfe50
MechChar g_orderNoneText[] = "No Cmd";

// GLOBAL: MW2 0x100a73c0
// GLOBAL: MW2GBL 0x1009f4a8
// GLOBAL: MW23DFX 0x10260818
// GLOBAL: MW2ATI 0x10194b10
// GLOBAL: MW2S3 0x10063138
// GLOBAL: MW2POWERVR 0x101bfe58
MechChar g_attackMyTargetItem[] = "Attack My Target";

// GLOBAL: MW2 0x100a73d8
// GLOBAL: MW2GBL 0x1009f4c0
// GLOBAL: MW23DFX 0x10260830
// GLOBAL: MW2ATI 0x10194b28
// GLOBAL: MW2S3 0x10063150
// GLOBAL: MW2POWERVR 0x101bfe70
MechChar g_defendMyTargetItem[] = "Defend My Target";

// GLOBAL: MW2 0x100a73f0
// GLOBAL: MW2GBL 0x1009f4d8
// GLOBAL: MW23DFX 0x10260848
// GLOBAL: MW2ATI 0x10194b40
// GLOBAL: MW2S3 0x10063168
// GLOBAL: MW2POWERVR 0x101bfe88
MechChar g_joinFormationItem[] = "Join Formation";

// GLOBAL: MW2 0x100a7400
MechChar g_disengageItem[] = "Disengage";

// GLOBAL: MW2 0x100a7410
// GLOBAL: MW2GBL 0x1009f4f8
// GLOBAL: MW23DFX 0x10260868
// GLOBAL: MW2ATI 0x10194b60
// GLOBAL: MW2S3 0x10063188
// GLOBAL: MW2POWERVR 0x101bfea8
MechChar g_engageAtWillItem[] = "Engage at Will";

// GLOBAL: MW2 0x100a7420
// GLOBAL: MW2GBL 0x1009f508
// GLOBAL: MW23DFX 0x10260878
// GLOBAL: MW2ATI 0x10194b70
// GLOBAL: MW2S3 0x10063198
// GLOBAL: MW2POWERVR 0x101bfeb8
MechChar g_shutdownItem[] = "Shutdown";

// GLOBAL: MW2 0x100a7430
// GLOBAL: MW2GBL 0x1009f518
// GLOBAL: MW23DFX 0x10260888
// GLOBAL: MW2ATI 0x10194b80
// GLOBAL: MW2S3 0x100631a8
// GLOBAL: MW2POWERVR 0x101bfec8
MechChar g_aiStateNoneText[] = "None ";

// GLOBAL: MW2 0x100a7438
// GLOBAL: MW2GBL 0x1009f520
// GLOBAL: MW23DFX 0x10260890
// GLOBAL: MW2ATI 0x10194b88
// GLOBAL: MW2S3 0x100631b0
// GLOBAL: MW2POWERVR 0x101bfed0
MechChar g_aiStateIdleText[] = "Idle ";

// GLOBAL: MW2 0x100a7440
// GLOBAL: MW2GBL 0x1009f528
// GLOBAL: MW23DFX 0x10260898
// GLOBAL: MW2ATI 0x10194b90
// GLOBAL: MW2S3 0x100631b8
// GLOBAL: MW2POWERVR 0x101bfed8
MechChar g_aiStateAvoidText[] = "Avoiding ";

// GLOBAL: MW2 0x100a7450
// GLOBAL: MW2GBL 0x1009f538
// GLOBAL: MW23DFX 0x102608a8
// GLOBAL: MW2ATI 0x10194ba0
// GLOBAL: MW2S3 0x100631c8
// GLOBAL: MW2POWERVR 0x101bfee8
MechChar g_aiStateTargetText[] = "Targeting ";

// GLOBAL: MW2 0x100a7460
// GLOBAL: MW2GBL 0x1009f548
// GLOBAL: MW23DFX 0x102608b8
// GLOBAL: MW2ATI 0x10194bb0
// GLOBAL: MW2S3 0x100631d8
// GLOBAL: MW2POWERVR 0x101bfef8
MechChar g_aiStateAttackText[] = "Engaging ";

// GLOBAL: MW2 0x100a7470
// GLOBAL: MW2GBL 0x1009f558
// GLOBAL: MW23DFX 0x102608c8
// GLOBAL: MW2ATI 0x10194bc0
// GLOBAL: MW2S3 0x100631e8
// GLOBAL: MW2POWERVR 0x101bff08
MechChar g_aiStateFleeText[] = "Disengaging ";

// GLOBAL: MW2 0x100a7480
// GLOBAL: MW2GBL 0x1009f568
// GLOBAL: MW23DFX 0x102608d8
// GLOBAL: MW2ATI 0x10194bd0
// GLOBAL: MW2S3 0x100631f8
// GLOBAL: MW2POWERVR 0x101bff18
MechChar g_aiStateFollowText[] = "In Formation ";

// GLOBAL: MW2 0x100a7490
// GLOBAL: MW2GBL 0x1009f578
// GLOBAL: MW23DFX 0x102608e8
// GLOBAL: MW2ATI 0x10194be0
// GLOBAL: MW2S3 0x10063208
// GLOBAL: MW2POWERVR 0x101bff28
MechChar g_aiStateReconText[] = "Reconning ";

// GLOBAL: MW2 0x100a74a0
// GLOBAL: MW2GBL 0x1009f588
// GLOBAL: MW23DFX 0x102608f8
// GLOBAL: MW2ATI 0x10194bf0
// GLOBAL: MW2S3 0x10063218
// GLOBAL: MW2POWERVR 0x101bff38
MechChar g_aiStatePatrolText[] = "Defending ";

// GLOBAL: MW2 0x100a74b0
// GLOBAL: MW2GBL 0x1009f598
// GLOBAL: MW23DFX 0x10260908
// GLOBAL: MW2ATI 0x10194c00
// GLOBAL: MW2S3 0x10063228
// GLOBAL: MW2POWERVR 0x101bff48
MechChar g_aiStateGoDirectText[] = "En Route ";

// GLOBAL: MW2 0x100a74c0
// GLOBAL: MW2GBL 0x1009f5a8
// GLOBAL: MW23DFX 0x10260918
// GLOBAL: MW2ATI 0x10194c10
// GLOBAL: MW2S3 0x10063238
// GLOBAL: MW2POWERVR 0x101bff58
MechChar g_aiState9Text[] = "Disengaging ";

// GLOBAL: MW2 0x100a74d0
// GLOBAL: MW2GBL 0x1009f5b8
// GLOBAL: MW23DFX 0x10260928
// GLOBAL: MW2ATI 0x10194c20
// GLOBAL: MW2S3 0x10063248
// GLOBAL: MW2POWERVR 0x101bff68
MechChar g_aiStateRestText[] = "Silent ";

// GLOBAL: MW2 0x100a74d8
// GLOBAL: MW2GBL 0x1009f5c0
// GLOBAL: MW23DFX 0x10260930
// GLOBAL: MW2ATI 0x10194c28
// GLOBAL: MW2S3 0x10063250
// GLOBAL: MW2POWERVR 0x101bff70
MechChar g_aiStateShutdownText[] = "Shutdown ";

// GLOBAL: MW2 0x100a74e8
// GLOBAL: MW2GBL 0x1009f5d0
// GLOBAL: MW23DFX 0x10260940
// GLOBAL: MW2ATI 0x10194c38
// GLOBAL: MW2S3 0x10063260
// GLOBAL: MW2POWERVR 0x101bff80
MechChar g_aiStateDeadText[] = "Destroyed ";

// GLOBAL: MW2 0x100a74f8
// GLOBAL: MW2GBL 0x1009f5e0
// GLOBAL: MW23DFX 0x10260950
// GLOBAL: MW2ATI 0x10194c48
// GLOBAL: MW2S3 0x10063270
// GLOBAL: MW2POWERVR 0x101bff90
MenuChoices g_formationChoices = {
	NULL,
	7,
	{g_echelonLeftText,
	 g_echelonRightText,
	 g_lineAbreastText,
	 g_lineAsternText,
	 g_vFormText,
	 g_wedgeText,
	 g_noFormationText}
};

// The orders, by g_lanceOrders.
// GLOBAL: MW2 0x100a7540
// GLOBAL: MW2GBL 0x1009f628
// GLOBAL: MW23DFX 0x10260998
// GLOBAL: MW2ATI 0x10194c90
// GLOBAL: MW2S3 0x100632b8
// GLOBAL: MW2POWERVR 0x101bffd8
MenuChoices g_orderChoices = {
	NULL,
	8,
	{g_orderChangeFormationText,
	 g_orderEngageAtWillText,
	 g_orderAttackText,
	 g_orderJoinFormationText,
	 g_orderDefendText,
	 g_orderDisengageText,
	 g_orderShutdownText,
	 g_orderNoneText}
};

// The AI states, by state plus one (GetSlotAiState), each followed by the player's goal.
// GLOBAL: MW2 0x100a7588
// GLOBAL: MW2GBL 0x1009f670
// GLOBAL: MW23DFX 0x102609e0
// GLOBAL: MW2ATI 0x10194cd8
// GLOBAL: MW2S3 0x10063300
// GLOBAL: MW2POWERVR 0x101c0020
MenuChoices g_aiStateChoices = {
	GetSlotGoalName,
	14,
	{g_aiStateNoneText,
	 g_aiStateIdleText,
	 g_aiStateAvoidText,
	 g_aiStateTargetText,
	 g_aiStateAttackText,
	 g_aiStateFleeText,
	 g_aiStateFollowText,
	 g_aiStateReconText,
	 g_aiStatePatrolText,
	 g_aiStateGoDirectText,
	 g_aiState9Text,
	 g_aiStateRestText,
	 g_aiStateShutdownText,
	 g_aiStateDeadText}
};

// GLOBAL: MW2 0x100a75d0
// GLOBAL: MW2GBL 0x1009f6b8
// GLOBAL: MW23DFX 0x10260a28
// GLOBAL: MW2ATI 0x10194d20
// GLOBAL: MW2S3 0x10063348
// GLOBAL: MW2POWERVR 0x101c0068
MenuChoices g_noChoices = {NULL, 0};

// GLOBAL: MW2 0x100a7618
// GLOBAL: MW2GBL 0x1009f700
// GLOBAL: MW23DFX 0x10260a70
// GLOBAL: MW2ATI 0x10194d68
// GLOBAL: MW2S3 0x10063390
// GLOBAL: MW2POWERVR 0x101c00b0
MenuControl g_formationControl = {3, 0, &g_formationChoices, 0, NULL, GetFormation, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7640
// GLOBAL: MW2GBL 0x1009f728
// GLOBAL: MW23DFX 0x10260a98
// GLOBAL: MW2ATI 0x10194d90
// GLOBAL: MW2S3 0x100633b8
// GLOBAL: MW2POWERVR 0x101c00d8
MenuControl g_commandAllStatusControl = {3, 0, &g_orderChoices, 0, NULL, GetLanceOrder, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7668
// GLOBAL: MW2GBL 0x1009f750
// GLOBAL: MW23DFX 0x10260ac0
// GLOBAL: MW2ATI 0x10194db8
// GLOBAL: MW2S3 0x100633e0
// GLOBAL: MW2POWERVR 0x101c0100
MenuControl g_commandPoint2StatusControl =
	{3, 0, &g_aiStateChoices, 1, InstallGoalSuffix, GetSlotAiState, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7690
// GLOBAL: MW2GBL 0x1009f778
// GLOBAL: MW23DFX 0x10260ae8
// GLOBAL: MW2ATI 0x10194de0
// GLOBAL: MW2S3 0x10063408
// GLOBAL: MW2POWERVR 0x101c0128
MenuControl g_commandPoint3StatusControl =
	{3, 0, &g_aiStateChoices, 2, InstallGoalSuffix, GetSlotAiState, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a76b8
// GLOBAL: MW2GBL 0x1009f7a0
// GLOBAL: MW23DFX 0x10260b10
// GLOBAL: MW2ATI 0x10194e08
// GLOBAL: MW2S3 0x10063430
// GLOBAL: MW2POWERVR 0x101c0150
MenuControl g_commandPoint4StatusControl =
	{3, 0, &g_aiStateChoices, 3, InstallGoalSuffix, GetSlotAiState, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a76e0
// GLOBAL: MW2GBL 0x1009f7c8
// GLOBAL: MW23DFX 0x10260b38
// GLOBAL: MW2ATI 0x10194e30
// GLOBAL: MW2S3 0x10063458
// GLOBAL: MW2POWERVR 0x101c0178
MenuControl g_commandPoint5StatusControl =
	{3, 0, &g_aiStateChoices, 4, InstallGoalSuffix, GetSlotAiState, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a7708
// GLOBAL: MW2GBL 0x1009f7f0
// GLOBAL: MW23DFX 0x10260b60
// GLOBAL: MW2ATI 0x10194e58
// GLOBAL: MW2S3 0x10063480
// GLOBAL: MW2POWERVR 0x101c01a0
MenuControl g_echelonLeftControl = {2, 0, &g_noChoices, 0, NULL, NULL, NULL, SelectFormation, NULL};

// GLOBAL: MW2 0x100a7730
// GLOBAL: MW2GBL 0x1009f818
// GLOBAL: MW23DFX 0x10260b88
// GLOBAL: MW2ATI 0x10194e80
// GLOBAL: MW2S3 0x100634a8
// GLOBAL: MW2POWERVR 0x101c01c8
MenuControl g_echelonRightControl = {2, 0, &g_noChoices, 1, NULL, NULL, NULL, SelectFormation, NULL};

// GLOBAL: MW2 0x100a7758
// GLOBAL: MW2GBL 0x1009f840
// GLOBAL: MW23DFX 0x10260bb0
// GLOBAL: MW2ATI 0x10194ea8
// GLOBAL: MW2S3 0x100634d0
// GLOBAL: MW2POWERVR 0x101c01f0
MenuControl g_lineAbreastControl = {2, 0, &g_noChoices, 2, NULL, NULL, NULL, SelectFormation, NULL};

// GLOBAL: MW2 0x100a7780
// GLOBAL: MW2GBL 0x1009f868
// GLOBAL: MW23DFX 0x10260bd8
// GLOBAL: MW2ATI 0x10194ed0
// GLOBAL: MW2S3 0x100634f8
// GLOBAL: MW2POWERVR 0x101c0218
MenuControl g_lineAsternControl = {2, 0, &g_noChoices, 3, NULL, NULL, NULL, SelectFormation, NULL};

// GLOBAL: MW2 0x100a77a8
// GLOBAL: MW2GBL 0x1009f890
// GLOBAL: MW23DFX 0x10260c00
// GLOBAL: MW2ATI 0x10194ef8
// GLOBAL: MW2S3 0x10063520
// GLOBAL: MW2POWERVR 0x101c0240
MenuControl g_vFormControl = {2, 0, &g_noChoices, 4, NULL, NULL, NULL, SelectFormation, NULL};

// GLOBAL: MW2 0x100a77d0
// GLOBAL: MW2GBL 0x1009f8b8
// GLOBAL: MW23DFX 0x10260c28
// GLOBAL: MW2ATI 0x10194f20
// GLOBAL: MW2S3 0x10063548
// GLOBAL: MW2POWERVR 0x101c0268
MenuControl g_wedgeControl = {2, 0, &g_noChoices, 5, NULL, NULL, NULL, SelectFormation, NULL};

// GLOBAL: MW2 0x100a77f8
// GLOBAL: MW2GBL 0x1009f8e0
// GLOBAL: MW23DFX 0x10260c50
// GLOBAL: MW2ATI 0x10194f48
// GLOBAL: MW2S3 0x10063570
// GLOBAL: MW2POWERVR 0x101c0290
MenuControl g_attackControl = {2, 0, &g_noChoices, 0, SetControlSlot, NULL, NULL, OrderAttack, NULL};

// GLOBAL: MW2 0x100a7820
// GLOBAL: MW2GBL 0x1009f908
// GLOBAL: MW23DFX 0x10260c78
// GLOBAL: MW2ATI 0x10194f70
// GLOBAL: MW2S3 0x10063598
// GLOBAL: MW2POWERVR 0x101c02b8
MenuControl g_engageAtWillControl = {2, 0, &g_noChoices, 0, SetControlSlot, NULL, NULL, OrderEngageAtWill, NULL};

// GLOBAL: MW2 0x100a7848
// GLOBAL: MW2GBL 0x1009f930
// GLOBAL: MW23DFX 0x10260ca0
// GLOBAL: MW2ATI 0x10194f98
// GLOBAL: MW2S3 0x100635c0
// GLOBAL: MW2POWERVR 0x101c02e0
MenuControl g_joinFormationControl = {2, 0, &g_noChoices, 0, SetControlSlot, NULL, NULL, OrderJoinFormation, NULL};

// GLOBAL: MW2 0x100a7870
// GLOBAL: MW2GBL 0x1009f958
// GLOBAL: MW23DFX 0x10260cc8
// GLOBAL: MW2ATI 0x10194fc0
// GLOBAL: MW2S3 0x100635e8
// GLOBAL: MW2POWERVR 0x101c0308
MenuControl g_defendControl = {2, 0, &g_noChoices, 0, SetControlSlot, NULL, NULL, OrderDefend, NULL};

// GLOBAL: MW2 0x100a7898
MenuControl g_disengageControl = {2, 0, &g_noChoices, 0, SetControlSlot, NULL, NULL, OrderDisengage, NULL};

// GLOBAL: MW2 0x100a78c0
// GLOBAL: MW2GBL 0x1009f9a8
// GLOBAL: MW23DFX 0x10260d18
// GLOBAL: MW2ATI 0x10195010
// GLOBAL: MW2S3 0x10063638
// GLOBAL: MW2POWERVR 0x101c0358
MenuControl g_shutdownControl = {2, 0, &g_noChoices, 0, SetControlSlot, NULL, NULL, OrderShutdown, NULL};

// GLOBAL: MW2 0x100a78e8
// GLOBAL: MW23DFX 0x10260d40
// GLOBAL: MW2ATI 0x10195038
// GLOBAL: MW2S3 0x10063660
// GLOBAL: MW2POWERVR 0x101c0380
MenuPage g_changeFormationPage = {
	0,
	g_changeFormationTitle,
	0,
	8,
	0,
	NULL,
	{{3, g_currentFormationLabel, RunMenuStatus, &g_formationControl, NULL},
	 {5, g_echelonLeftText, RunMenuStatus, &g_echelonLeftControl, NULL},
	 {5, g_echelonRightText, RunMenuStatus, &g_echelonRightControl, NULL},
	 {5, g_lineAbreastText, RunMenuStatus, &g_lineAbreastControl, NULL},
	 {5, g_lineAsternText, RunMenuStatus, &g_lineAsternControl, NULL},
	 {5, g_vFormText, RunMenuStatus, &g_vFormControl, NULL},
	 {5, g_wedgeText, RunMenuStatus, &g_wedgeControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7a40
// GLOBAL: MW23DFX 0x10260e98
// GLOBAL: MW2ATI 0x10195190
// GLOBAL: MW2S3 0x100637b8
// GLOBAL: MW2POWERVR 0x101c04d8
MenuPage g_commandAllPage = {
	0,
	g_commandAllTitle,
	0,
	7,
	0,
	NULL,
	{{3, g_statusLabel, RunMenuStatus, &g_commandAllStatusControl, NULL},
	 {5, g_attackMyTargetItem, RunMenuStatus, &g_attackControl, NULL},
	 {5, g_defendMyTargetItem, RunMenuStatus, &g_defendControl, NULL},
	 {5, g_joinFormationItem, RunMenuStatus, &g_joinFormationControl, NULL},
	 {5, g_engageAtWillItem, RunMenuStatus, &g_engageAtWillControl, NULL},
	 {5, g_shutdownItem, RunMenuStatus, &g_shutdownControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7b98
// GLOBAL: MW2MATROX 0x100ba8b0
// GLOBAL: MW23DFX 0x10260ff0
// GLOBAL: MW2ATI 0x101952e8
// GLOBAL: MW2S3 0x10063910
// GLOBAL: MW2POWERVR 0x101c0630
MenuPage g_commandPoint2Page = {
	0,
	g_commandPoint2Title,
	1,
	7,
	0,
	PrepareCommandPointPage,
	{{3, g_statusLabel, RunMenuStatus, &g_commandPoint2StatusControl, NULL},
	 {5, g_attackMyTargetItem, RunMenuStatus, &g_attackControl, NULL},
	 {5, g_defendMyTargetItem, RunMenuStatus, &g_defendControl, NULL},
	 {5, g_joinFormationItem, RunMenuStatus, &g_joinFormationControl, NULL},
	 {5, g_engageAtWillItem, RunMenuStatus, &g_engageAtWillControl, NULL},
	 {5, g_shutdownItem, RunMenuStatus, &g_shutdownControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL},
	 {3, g_notAvailableText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7cf0
// GLOBAL: MW2MATROX 0x100baa08
// GLOBAL: MW23DFX 0x10261148
// GLOBAL: MW2ATI 0x10195440
// GLOBAL: MW2S3 0x10063a68
// GLOBAL: MW2POWERVR 0x101c0788
MenuPage g_commandPoint3Page = {
	0,
	g_commandPoint3Title,
	2,
	7,
	0,
	PrepareCommandPointPage,
	{{3, g_statusLabel, RunMenuStatus, &g_commandPoint3StatusControl, NULL},
	 {5, g_attackMyTargetItem, RunMenuStatus, &g_attackControl, NULL},
	 {5, g_defendMyTargetItem, RunMenuStatus, &g_defendControl, NULL},
	 {5, g_joinFormationItem, RunMenuStatus, &g_joinFormationControl, NULL},
	 {5, g_engageAtWillItem, RunMenuStatus, &g_engageAtWillControl, NULL},
	 {5, g_shutdownItem, RunMenuStatus, &g_shutdownControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL},
	 {3, g_notAvailableText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7e48
// GLOBAL: MW23DFX 0x102612a0
// GLOBAL: MW2ATI 0x10195598
// GLOBAL: MW2S3 0x10063bc0
// GLOBAL: MW2POWERVR 0x101c08e0
MenuPage g_commandPoint4Page = {
	0,
	g_commandPoint4Title,
	3,
	7,
	0,
	PrepareCommandPointPage,
	{{3, g_statusLabel, RunMenuStatus, &g_commandPoint4StatusControl, NULL},
	 {5, g_attackMyTargetItem, RunMenuStatus, &g_attackControl, NULL},
	 {5, g_defendMyTargetItem, RunMenuStatus, &g_defendControl, NULL},
	 {5, g_joinFormationItem, RunMenuStatus, &g_joinFormationControl, NULL},
	 {5, g_engageAtWillItem, RunMenuStatus, &g_engageAtWillControl, NULL},
	 {5, g_shutdownItem, RunMenuStatus, &g_shutdownControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL},
	 {3, g_notAvailableText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a7fa0
// GLOBAL: MW23DFX 0x102613f8
// GLOBAL: MW2ATI 0x101956f0
// GLOBAL: MW2S3 0x10063d18
// GLOBAL: MW2POWERVR 0x101c0a38
MenuPage g_commandPoint5Page = {
	0,
	g_commandPoint5Title,
	4,
	7,
	0,
	PrepareCommandPointPage,
	{{3, g_statusLabel, RunMenuStatus, &g_commandPoint5StatusControl, NULL},
	 {5, g_attackMyTargetItem, RunMenuStatus, &g_attackControl, NULL},
	 {5, g_defendMyTargetItem, RunMenuStatus, &g_defendControl, NULL},
	 {5, g_joinFormationItem, RunMenuStatus, &g_joinFormationControl, NULL},
	 {5, g_engageAtWillItem, RunMenuStatus, &g_engageAtWillControl, NULL},
	 {5, g_shutdownItem, RunMenuStatus, &g_shutdownControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL},
	 {3, g_notAvailableText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a80f8
// GLOBAL: MW2MATROX 0x100bae10
// GLOBAL: MW23DFX 0x10261550
// GLOBAL: MW2ATI 0x10195848
// GLOBAL: MW2S3 0x10063e70
// GLOBAL: MW2POWERVR 0x101c0b90
MenuPage g_commandComputerPage = {
	0,
	g_commandComputerTitle,
	0,
	8,
	0,
	PrepareCommandComputerPage,
	{{0, g_changeFormationItem, RunMenuStatus, &g_formationControl, &g_changeFormationPage},
	 {0, g_commandPoint2Item, RunMenuStatus, &g_commandPoint2StatusControl, &g_commandPoint2Page},
	 {0, g_commandPoint3Item, RunMenuStatus, &g_commandPoint3StatusControl, &g_commandPoint3Page},
	 {0, g_commandPoint4Item, RunMenuStatus, &g_commandPoint4StatusControl, &g_commandPoint4Page},
	 {0, g_commandPoint5Item, RunMenuStatus, &g_commandPoint5StatusControl, &g_commandPoint5Page},
	 {0, g_commandAllItem, RunMenuStatus, &g_commandAllStatusControl, &g_commandAllPage},
	 {6, g_escToExitText, NULL, NULL, NULL},
	 {3, g_noStarMatesText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a8250
// GLOBAL: MW2GBL 0x100a0338
// GLOBAL: MW2MATROX 0x100baf68
// GLOBAL: MW23DFX 0x102616a8
// GLOBAL: MW2ATI 0x101959a0
// GLOBAL: MW2S3 0x10063fc8
// GLOBAL: MW2POWERVR 0x101c0ce8
PANE g_commandMenuTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a8268
// GLOBAL: MW2GBL 0x100a0350
// GLOBAL: MW2MATROX 0x100baf80
// GLOBAL: MW23DFX 0x102616c0
// GLOBAL: MW2ATI 0x101959b8
// GLOBAL: MW2S3 0x10063fe0
// GLOBAL: MW2POWERVR 0x101c0d00
PANE g_commandMenuBackgroundTarget = {NULL, 0x3852, 0x4ccd, 0x10000, 0x999a};

// GLOBAL: MW2 0x100a8280
// GLOBAL: MW2MATROX 0x100baf98
// GLOBAL: MW23DFX 0x102616d8
// GLOBAL: MW2ATI 0x101959d0
// GLOBAL: MW2S3 0x10063ff8
// GLOBAL: MW2POWERVR 0x101c0d18
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
// GLOBAL: MW2GBL 0x10137c20
// GLOBAL: MW2MATROX 0x1012f450
// GLOBAL: MW23DFX 0x10153560
// GLOBAL: MW2ATI 0x10087570
// GLOBAL: MW2S3 0x1014d990
// GLOBAL: MW2POWERVR 0x1007db50
MenuPage* g_commandMenuPageStack[8];
