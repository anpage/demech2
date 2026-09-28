#include "menudata.h"

#include "campaignmission.h"
#include "decomp.h"
#include "formation.h"
#include "mainmenubutton.h"
#include "menuscreen.h"
#include "types.h"

// The menu screens' layouts and the campaigns' tables. The original links this data as an object
// of its own, between cockpitcontrols.cpp and options.cpp, with no code: the tables come first,
// then the strings they point to (from 0x10070020), which are defined first here.

extern MechS32 g_unk0x1006e1d8[6];
extern MechS32 g_unk0x1006e1f0[6];
extern MechS32 g_unk0x1006e208[6];
extern MechS32 g_unk0x1006e220[6];
extern MechS32 g_unk0x1006e238[6];
extern MechS32 g_unk0x1006e250[6];

// GLOBAL: MW2SHELL 0x10070094
MechChar g_unk0x10070094[0x10] = "CADET TRAINING";

// GLOBAL: MW2SHELL 0x100700a4
MechChar g_unk0x100700a4[0x18] = "ARCHIVE HOLOPROJECTOR";

// GLOBAL: MW2SHELL 0x100700bc
MechChar g_unk0x100700bc[0x0c] = "READY ROOM";

// GLOBAL: MW2SHELL 0x100700c8
MechChar g_unk0x100700c8[0x0c] = "REGISTER";

// GLOBAL: MW2SHELL 0x100700d4
MechChar g_unk0x100700d4[0x08] = "~EXIT";

// GLOBAL: MW2SHELL 0x100700dc
MechChar g_unk0x100700dc[0x14] = "<~NEW ALLEGIANCE";

// GLOBAL: MW2SHELL 0x100700f0
MechChar g_unk0x100700f0[0x04] = "";

// GLOBAL: MW2SHELL 0x100700f4
MechChar g_unk0x100700f4[0x04] = "";

// GLOBAL: MW2SHELL 0x100700f8
MechChar g_unk0x100700f8[0x04] = "";

// GLOBAL: MW2SHELL 0x100700fc
MechChar g_unk0x100700fc[0x04] = "";

// GLOBAL: MW2SHELL 0x10070100
MechChar g_unk0x10070100[0x04] = "";

// GLOBAL: MW2SHELL 0x10070104
MechChar g_unk0x10070104[0x04] = "";

// GLOBAL: MW2SHELL 0x10070108
MechChar g_unk0x10070108[0x04] = "";

// GLOBAL: MW2SHELL 0x1007010c
MechChar g_unk0x1007010c[0x04] = "";

// GLOBAL: MW2SHELL 0x10070110
MechChar g_unk0x10070110[0x04] = "";

// GLOBAL: MW2SHELL 0x10070114
MechChar g_unk0x10070114[0x04] = "";

// GLOBAL: MW2SHELL 0x10070118
MechChar g_unk0x10070118[0x0c] = "<~ACCEPT";

// GLOBAL: MW2SHELL 0x10070124
MechChar g_unk0x10070124[0x18] = "<~DELETE MECHWARRIOR";

// GLOBAL: MW2SHELL 0x1007013c
MechChar g_unk0x1007013c[0x18] = "<~LAUNCH OLD MISSION";

// GLOBAL: MW2SHELL 0x10070154
MechChar g_unk0x10070154[0x10] = "<~PILOT INFO";

// The clan hall archive screen.
// GLOBAL: MW2SHELL 0x10070164
MechChar g_unk0x10070164[0x08] = "~EXIT";

// GLOBAL: MW2SHELL 0x1007016c
MechChar g_unk0x1007016c[0x0c] = "~PREV PAGE";

// GLOBAL: MW2SHELL 0x10070178
MechChar g_unk0x10070178[0x0c] = "~NEXT PAGE";

// GLOBAL: MW2SHELL 0x10070184
MechChar g_unk0x10070184[0x08] = "~BACK";

// The training screen.
// GLOBAL: MW2SHELL 0x1007018c
MechChar g_unk0x1007018c[0x0c] = "CLAN HALL";

// GLOBAL: MW2SHELL 0x10070198
MechChar g_unk0x10070198[0x10] = "<~NAV COMPUTER";

// GLOBAL: MW2SHELL 0x100701a8
MechChar g_unk0x100701a8[0x10] = "<~MECH HANDLING";

// GLOBAL: MW2SHELL 0x100701b8
MechChar g_unk0x100701b8[0x10] = "<~WEAPONS USAGE";

// GLOBAL: MW2SHELL 0x100701c8
MechChar g_unk0x100701c8[0x0c] = "<~HUNTING";

// GLOBAL: MW2SHELL 0x100701d4
MechChar g_unk0x100701d4[0x10] = "<~INSPECTION";

// GLOBAL: MW2SHELL 0x100701e4
MechChar g_unk0x100701e4[0x08] = "<~TRIAL";

// GLOBAL: MW2SHELL 0x100701ec
MechChar g_unk0x100701ec[0x0c] = "CLAN HALL";

// GLOBAL: MW2SHELL 0x100701f8
MechChar g_unk0x100701f8[0x0c] = "~MECH LAB";

// GLOBAL: MW2SHELL 0x10070204
MechChar g_unk0x10070204[0x10] = "~STAR CONFIG";

// GLOBAL: MW2SHELL 0x10070214
MechChar g_unk0x10070214[0x14] = "~MISSION BRIEFING";

// GLOBAL: MW2SHELL 0x10070228
MechChar g_unk0x10070228[0x0c] = "<~YELLOW";

// GLOBAL: MW2SHELL 0x10070234
MechChar g_unk0x10070234[0x0c] = "<~ORANGE";

// GLOBAL: MW2SHELL 0x10070240
MechChar g_unk0x10070240[0x08] = "<~TEAL";

// GLOBAL: MW2SHELL 0x10070248
MechChar g_unk0x10070248[0x08] = "<~TAUPE";

// GLOBAL: MW2SHELL 0x10070250
MechChar g_unk0x10070250[0x08] = "<~JENNY";

// GLOBAL: MW2SHELL 0x10070258
MechChar g_unk0x10070258[0x08] = "<~SABLE";

// GLOBAL: MW2SHELL 0x10070260
MechChar g_unk0x10070260[0x08] = "<~GREY";

// GLOBAL: MW2SHELL 0x10070268
MechChar g_unk0x10070268[0x08] = "<~BROWN";

// GLOBAL: MW2SHELL 0x10070270
MechChar g_unk0x10070270[0x08] = "<~AMY";

// GLOBAL: MW2SHELL 0x10070278
MechChar g_unk0x10070278[0x0c] = "<~SILVER";

// GLOBAL: MW2SHELL 0x10070284
MechChar g_unk0x10070284[0x08] = "<~AQUA";

// GLOBAL: MW2SHELL 0x1007028c
MechChar g_unk0x1007028c[0x08] = "<~KIM";

// GLOBAL: MW2SHELL 0x10070294
MechChar g_unk0x10070294[0x08] = "<~CYAN";

// GLOBAL: MW2SHELL 0x1007029c
MechChar g_unk0x1007029c[0x0c] = "<~MAROON";

// GLOBAL: MW2SHELL 0x100702a8
MechChar g_unk0x100702a8[0x08] = "<~GOLD";

// GLOBAL: MW2SHELL 0x100702b0
MechChar g_unk0x100702b0[0x08] = "<~IRENE";

// GLOBAL: MW2SHELL 0x100702b8
MechChar g_unk0x100702b8[0x08] = "<~ABORT";

// GLOBAL: MW2SHELL 0x100702c0
MechChar g_unk0x100702c0[0x0c] = "<~SITUATION";

// GLOBAL: MW2SHELL 0x100702cc
MechChar g_unk0x100702cc[0x0c] = "<~LAUNCH";

// GLOBAL: MW2SHELL 0x100702d8
MechChar g_unk0x100702d8[0x08] = "<~SKIP";

// GLOBAL: MW2SHELL 0x100702e0
MechChar g_unk0x100702e0[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x100702e8
MechChar g_unk0x100702e8[0x0c] = "<~PREV PAGE";

// GLOBAL: MW2SHELL 0x100702f4
MechChar g_unk0x100702f4[0x0c] = "<~NEXT PAGE";

// GLOBAL: MW2SHELL 0x10070300
MechChar g_unk0x10070300[0x04] = "";

// GLOBAL: MW2SHELL 0x10070304
MechChar g_unk0x10070304[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x1007030c
MechChar g_unk0x1007030c[0x0c] = "<~AFTERMATH";

// GLOBAL: MW2SHELL 0x10070318
MechChar g_unk0x10070318[0x0c] = "<~REPLAY";

// GLOBAL: MW2SHELL 0x10070324
MechChar g_unk0x10070324[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x1007032c
MechChar g_unk0x1007032c[0x0c] = "<~PREV PAGE";

// GLOBAL: MW2SHELL 0x10070338
MechChar g_unk0x10070338[0x0c] = "<~NEXT PAGE";

// GLOBAL: MW2SHELL 0x10070344
MechChar g_unk0x10070344[0x04] = "";

// The custom battle's star screen.
// GLOBAL: MW2SHELL 0x10070348
MechChar g_unk0x10070348[0x10] = "<~EXIT CONFIG";

// GLOBAL: MW2SHELL 0x10070358
MechChar g_unk0x10070358[0x0c] = "~MECH LAB";

// GLOBAL: MW2SHELL 0x10070364
MechChar g_unk0x10070364[0x10] = "NEXT FORMATION";

// GLOBAL: MW2SHELL 0x10070374
MechChar g_unk0x10070374[0x10] = "PREV FORMATION";

// GLOBAL: MW2SHELL 0x10070384
MechChar g_unk0x10070384[0x10] = "ADD STARMATE";

// GLOBAL: MW2SHELL 0x10070394
MechChar g_unk0x10070394[0x10] = "DELETE STARMATE";

// GLOBAL: MW2SHELL 0x100703a4
MechChar g_unk0x100703a4[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x100703b0
MechChar g_unk0x100703b0[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x100703bc
MechChar g_unk0x100703bc[0x0c] = "CHANGE MECH";

// The mech bay screen of each campaign.
// GLOBAL: MW2SHELL 0x100703c8
MechChar g_unk0x100703c8[0x0c] = "<~EXIT LAB";

// GLOBAL: MW2SHELL 0x100703d4
MechChar g_unk0x100703d4[0x10] = "~STAR CONFIG";

// GLOBAL: MW2SHELL 0x100703e4
MechChar g_unk0x100703e4[0x10] = "NEXT CHASSIS";

// GLOBAL: MW2SHELL 0x100703f4
MechChar g_unk0x100703f4[0x10] = "PREV CHASSIS";

// GLOBAL: MW2SHELL 0x10070404
MechChar g_unk0x10070404[0x10] = "NEXT VARIANT";

// GLOBAL: MW2SHELL 0x10070414
MechChar g_unk0x10070414[0x10] = "PREV VARIANT";

// GLOBAL: MW2SHELL 0x10070424
MechChar g_unk0x10070424[0x0c] = "<~CUSTOMIZE";

// GLOBAL: MW2SHELL 0x10070430
MechChar g_unk0x10070430[0x10] = "<~ACCEPT MECH";

// GLOBAL: MW2SHELL 0x10070440
MechChar g_unk0x10070440[0x08] = "<~SAVE";

// GLOBAL: MW2SHELL 0x10070448
MechChar g_unk0x10070448[0x08] = "<~ABORT";

// GLOBAL: MW2SHELL 0x10070450
MechChar g_unk0x10070450[0x0c] = "<~DELETE";

// GLOBAL: MW2SHELL 0x1007045c
MechChar g_unk0x1007045c[0x10] = "CADET TRAINING";

// GLOBAL: MW2SHELL 0x1007046c
MechChar g_unk0x1007046c[0x18] = "ARCHIVE HOLOPROJECTOR";

// GLOBAL: MW2SHELL 0x10070484
MechChar g_unk0x10070484[0x0c] = "READY ROOM";

// GLOBAL: MW2SHELL 0x10070490
MechChar g_unk0x10070490[0x0c] = "REGISTER";

// GLOBAL: MW2SHELL 0x1007049c
MechChar g_unk0x1007049c[0x08] = "~EXIT";

// GLOBAL: MW2SHELL 0x100704a4
MechChar g_unk0x100704a4[0x14] = "<~NEW ALLEGIANCE";

// GLOBAL: MW2SHELL 0x100704b8
MechChar g_unk0x100704b8[0x04] = "";

// GLOBAL: MW2SHELL 0x100704bc
MechChar g_unk0x100704bc[0x04] = "";

// GLOBAL: MW2SHELL 0x100704c0
MechChar g_unk0x100704c0[0x04] = "";

// GLOBAL: MW2SHELL 0x100704c4
MechChar g_unk0x100704c4[0x04] = "";

// GLOBAL: MW2SHELL 0x100704c8
MechChar g_unk0x100704c8[0x04] = "";

// GLOBAL: MW2SHELL 0x100704cc
MechChar g_unk0x100704cc[0x04] = "";

// GLOBAL: MW2SHELL 0x100704d0
MechChar g_unk0x100704d0[0x04] = "";

// GLOBAL: MW2SHELL 0x100704d4
MechChar g_unk0x100704d4[0x04] = "";

// GLOBAL: MW2SHELL 0x100704d8
MechChar g_unk0x100704d8[0x04] = "";

// GLOBAL: MW2SHELL 0x100704dc
MechChar g_unk0x100704dc[0x04] = "";

// GLOBAL: MW2SHELL 0x100704e0
MechChar g_unk0x100704e0[0x0c] = "<~ACCEPT";

// GLOBAL: MW2SHELL 0x100704ec
MechChar g_unk0x100704ec[0x18] = "<~DELETE MECHWARRIOR";

// GLOBAL: MW2SHELL 0x10070504
MechChar g_unk0x10070504[0x18] = "<~LAUNCH OLD MISSION";

// GLOBAL: MW2SHELL 0x1007051c
MechChar g_unk0x1007051c[0x10] = "<~PILOT INFO";

// GLOBAL: MW2SHELL 0x1007052c
MechChar g_unk0x1007052c[0x08] = "~EXIT";

// GLOBAL: MW2SHELL 0x10070534
MechChar g_unk0x10070534[0x0c] = "~PREV PAGE";

// GLOBAL: MW2SHELL 0x10070540
MechChar g_unk0x10070540[0x0c] = "~NEXT PAGE";

// GLOBAL: MW2SHELL 0x1007054c
MechChar g_unk0x1007054c[0x08] = "~BACK";

// GLOBAL: MW2SHELL 0x10070554
MechChar g_unk0x10070554[0x0c] = "CLAN HALL";

// GLOBAL: MW2SHELL 0x10070560
MechChar g_unk0x10070560[0x10] = "<~NAV COMPUTER";

// GLOBAL: MW2SHELL 0x10070570
MechChar g_unk0x10070570[0x10] = "<~MECH HANDLING";

// GLOBAL: MW2SHELL 0x10070580
MechChar g_unk0x10070580[0x10] = "<~WEAPONS USAGE";

// GLOBAL: MW2SHELL 0x10070590
MechChar g_unk0x10070590[0x0c] = "<~HUNTING";

// GLOBAL: MW2SHELL 0x1007059c
MechChar g_unk0x1007059c[0x10] = "<~INSPECTION";

// GLOBAL: MW2SHELL 0x100705ac
MechChar g_unk0x100705ac[0x08] = "<~TRIAL";

// GLOBAL: MW2SHELL 0x100705b4
MechChar g_unk0x100705b4[0x0c] = "CLAN HALL";

// GLOBAL: MW2SHELL 0x100705c0
MechChar g_unk0x100705c0[0x0c] = "~MECH LAB";

// GLOBAL: MW2SHELL 0x100705cc
MechChar g_unk0x100705cc[0x10] = "~STAR CONFIG";

// GLOBAL: MW2SHELL 0x100705dc
MechChar g_unk0x100705dc[0x14] = "~MISSION BRIEFING";

// GLOBAL: MW2SHELL 0x100705f0
MechChar g_unk0x100705f0[0x08] = "<~PINK";

// GLOBAL: MW2SHELL 0x100705f8
MechChar g_unk0x100705f8[0x08] = "<~GREEN";

// GLOBAL: MW2SHELL 0x10070600
MechChar g_unk0x10070600[0x08] = "<~RED";

// GLOBAL: MW2SHELL 0x10070608
MechChar g_unk0x10070608[0x0c] = "<~FUCHSIA";

// GLOBAL: MW2SHELL 0x10070614
MechChar g_unk0x10070614[0x08] = "<~CINDY";

// GLOBAL: MW2SHELL 0x1007061c
MechChar g_unk0x1007061c[0x08] = "<~RUST";

// GLOBAL: MW2SHELL 0x10070624
MechChar g_unk0x10070624[0x08] = "<~UMBER";

// GLOBAL: MW2SHELL 0x1007062c
MechChar g_unk0x1007062c[0x08] = "<~TAN";

// GLOBAL: MW2SHELL 0x10070634
MechChar g_unk0x10070634[0x08] = "<~HEIDI";

// GLOBAL: MW2SHELL 0x1007063c
MechChar g_unk0x1007063c[0x08] = "<~PLUM";

// GLOBAL: MW2SHELL 0x10070644
MechChar g_unk0x10070644[0x08] = "<~WHITE";

// GLOBAL: MW2SHELL 0x1007064c
MechChar g_unk0x1007064c[0x08] = "<~JILL";

// GLOBAL: MW2SHELL 0x10070654
MechChar g_unk0x10070654[0x08] = "<~PUCE";

// GLOBAL: MW2SHELL 0x1007065c
MechChar g_unk0x1007065c[0x0c] = "<~BLONDE";

// GLOBAL: MW2SHELL 0x10070668
MechChar g_unk0x10070668[0x0c] = "<~BRONZE";

// GLOBAL: MW2SHELL 0x10070674
MechChar g_unk0x10070674[0x08] = "<~MARY";

// GLOBAL: MW2SHELL 0x1007067c
MechChar g_unk0x1007067c[0x08] = "<~ABORT";

// GLOBAL: MW2SHELL 0x10070684
MechChar g_unk0x10070684[0x0c] = "<~SITUATION";

// GLOBAL: MW2SHELL 0x10070690
MechChar g_unk0x10070690[0x0c] = "<~LAUNCH";

// GLOBAL: MW2SHELL 0x1007069c
MechChar g_unk0x1007069c[0x08] = "<~SKIP";

// GLOBAL: MW2SHELL 0x100706a4
MechChar g_unk0x100706a4[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x100706ac
MechChar g_unk0x100706ac[0x0c] = "<~PREV PAGE";

// GLOBAL: MW2SHELL 0x100706b8
MechChar g_unk0x100706b8[0x0c] = "<~NEXT PAGE";

// GLOBAL: MW2SHELL 0x100706c4
MechChar g_unk0x100706c4[0x04] = "";

// GLOBAL: MW2SHELL 0x100706c8
MechChar g_unk0x100706c8[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x100706d0
MechChar g_unk0x100706d0[0x0c] = "<~AFTERMATH";

// GLOBAL: MW2SHELL 0x100706dc
MechChar g_unk0x100706dc[0x0c] = "<~REPLAY";

// GLOBAL: MW2SHELL 0x100706e8
MechChar g_unk0x100706e8[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x100706f0
MechChar g_unk0x100706f0[0x0c] = "<~PREV PAGE";

// GLOBAL: MW2SHELL 0x100706fc
MechChar g_unk0x100706fc[0x0c] = "<~NEXT PAGE";

// GLOBAL: MW2SHELL 0x10070708
MechChar g_unk0x10070708[0x04] = "";

// GLOBAL: MW2SHELL 0x1007070c
MechChar g_unk0x1007070c[0x10] = "<~EXIT CONFIG";

// GLOBAL: MW2SHELL 0x1007071c
MechChar g_unk0x1007071c[0x0c] = "~MECH LAB";

// GLOBAL: MW2SHELL 0x10070728
MechChar g_unk0x10070728[0x10] = "NEXT FORMATION";

// GLOBAL: MW2SHELL 0x10070738
MechChar g_unk0x10070738[0x10] = "PREV FORMATION";

// GLOBAL: MW2SHELL 0x10070748
MechChar g_unk0x10070748[0x10] = "ADD STARMATE";

// GLOBAL: MW2SHELL 0x10070758
MechChar g_unk0x10070758[0x10] = "DELETE STARMATE";

// GLOBAL: MW2SHELL 0x10070768
MechChar g_unk0x10070768[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x10070774
MechChar g_unk0x10070774[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x10070780
MechChar g_unk0x10070780[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x1007078c
MechChar g_unk0x1007078c[0x0c] = "<~EXIT LAB";

// GLOBAL: MW2SHELL 0x10070798
MechChar g_unk0x10070798[0x10] = "~STAR CONFIG";

// GLOBAL: MW2SHELL 0x100707a8
MechChar g_unk0x100707a8[0x10] = "NEXT CHASSIS";

// GLOBAL: MW2SHELL 0x100707b8
MechChar g_unk0x100707b8[0x10] = "PREV CHASSIS";

// GLOBAL: MW2SHELL 0x100707c8
MechChar g_unk0x100707c8[0x10] = "NEXT VARIANT";

// GLOBAL: MW2SHELL 0x100707d8
MechChar g_unk0x100707d8[0x10] = "PREV VARIANT";

// GLOBAL: MW2SHELL 0x100707e8
MechChar g_unk0x100707e8[0x0c] = "<~CUSTOMIZE";

// GLOBAL: MW2SHELL 0x100707f4
MechChar g_unk0x100707f4[0x10] = "<~ACCEPT MECH";

// GLOBAL: MW2SHELL 0x10070804
MechChar g_unk0x10070804[0x08] = "<~SAVE";

// GLOBAL: MW2SHELL 0x1007080c
MechChar g_unk0x1007080c[0x08] = "<~ABORT";

// GLOBAL: MW2SHELL 0x10070814
MechChar g_unk0x10070814[0x0c] = "<~DELETE";

// GLOBAL: MW2SHELL 0x10070820
MechChar g_unk0x10070820[0x18] = "~TRIALS OF GRIEVANCE";

// GLOBAL: MW2SHELL 0x10070838
MechChar g_unk0x10070838[0x10] = "~WOLF CLAN HALL";

// GLOBAL: MW2SHELL 0x10070848
MechChar g_unk0x10070848[0x18] = "~JADE FALCON CLAN HALL";

// GLOBAL: MW2SHELL 0x10070860
MechChar g_unk0x10070860[0x08] = "~EXIT";

// The mission briefing screen.
// GLOBAL: MW2SHELL 0x1007088c
MechChar g_unk0x1007088c[0x04] = "";

// GLOBAL: MW2SHELL 0x10070890
MechChar g_unk0x10070890[0x08] = "<~EXIT";

// GLOBAL: MW2SHELL 0x10070898
MechChar g_unk0x10070898[0x04] = "";

// GLOBAL: MW2SHELL 0x1007089c
MechChar g_unk0x1007089c[0x04] = "";

// GLOBAL: MW2SHELL 0x100708a0
MechChar g_unk0x100708a0[0x04] = "";

// GLOBAL: MW2SHELL 0x100708a4
MechChar g_unk0x100708a4[0x04] = "";

// GLOBAL: MW2SHELL 0x100708a8
MechChar g_unk0x100708a8[0x04] = "";

// GLOBAL: MW2SHELL 0x100708ac
MechChar g_unk0x100708ac[0x04] = "";

// GLOBAL: MW2SHELL 0x100708b0
MechChar g_unk0x100708b0[0x04] = "";

// GLOBAL: MW2SHELL 0x100708b4
MechChar g_unk0x100708b4[0x04] = "";

// GLOBAL: MW2SHELL 0x100708b8
MechChar g_unk0x100708b8[0x04] = "";

// GLOBAL: MW2SHELL 0x100708bc
MechChar g_unk0x100708bc[0x04] = "";

// GLOBAL: MW2SHELL 0x100708c0
MechChar g_unk0x100708c0[0x04] = "";

// GLOBAL: MW2SHELL 0x100708c4
MechChar g_unk0x100708c4[0x04] = "";

// GLOBAL: MW2SHELL 0x100708c8
MechChar g_unk0x100708c8[0x04] = "";

// GLOBAL: MW2SHELL 0x100708cc
MechChar g_unk0x100708cc[0x04] = "";

// GLOBAL: MW2SHELL 0x100708d0
MechChar g_unk0x100708d0[0x04] = "";

// GLOBAL: MW2SHELL 0x100708d4
MechChar g_unk0x100708d4[0x04] = "";

// GLOBAL: MW2SHELL 0x100708d8
MechChar g_unk0x100708d8[0x04] = "";

// GLOBAL: MW2SHELL 0x100708dc
MechChar g_unk0x100708dc[0x04] = "";

// GLOBAL: MW2SHELL 0x100708e0
MechChar g_unk0x100708e0[0x04] = "";

// GLOBAL: MW2SHELL 0x100708e4
MechChar g_unk0x100708e4[0x04] = "";

// GLOBAL: MW2SHELL 0x100708e8
MechChar g_unk0x100708e8[0x04] = "";

// GLOBAL: MW2SHELL 0x100708ec
MechChar g_unk0x100708ec[0x04] = "";

// GLOBAL: MW2SHELL 0x100708f0
MechChar g_unk0x100708f0[0x04] = "";

// Tab stops for the "\T" text escape, in pixels from the left edge.
// GLOBAL: MW2SHELL 0x1006e150
MechS32 g_textTabStops[19] =
	{0, 36, 72, 108, 144, 180, 216, 252, 288, 324, 360, 396, 432, 468, 504, 540, 576, 612, 640};

// GLOBAL: MW2SHELL 0x1006e19c
char* g_unk0x1006e19c = "DATABASE.MW2";

// The clan hall archives, by campaign.
// GLOBAL: MW2SHELL 0x1006e1a0
MechChar* g_unk0x1006e1a0[2] = {"ARCHWO.MW2", "ARCHJF.MW2"};

// The star formations, as the mission briefing screen names them.
// GLOBAL: MW2SHELL 0x1006e1a8
Formation g_unk0x1006e1a8[6] = {
	{g_unk0x1006e1d8, "Echelon Left"},
	{g_unk0x1006e1f0, "Echelon Right"},
	{g_unk0x1006e208, "Line Abreast"},
	{g_unk0x1006e220, "Line Astern"},
	{g_unk0x1006e238, "V-Form"},
	{g_unk0x1006e250, "Wedge"},
};

// The positions of each formation's mechs.
// GLOBAL: MW2SHELL 0x1006e1d8
MechS32 g_unk0x1006e1d8[6] = {285, 176, 344, 222, 450, 320};

// GLOBAL: MW2SHELL 0x1006e1f0
MechS32 g_unk0x1006e1f0[6] = {520, 212, 342, 225, 143, 253};

// GLOBAL: MW2SHELL 0x1006e208
MechS32 g_unk0x1006e208[6] = {222, 199, 341, 216, 498, 242};

// GLOBAL: MW2SHELL 0x1006e220
MechS32 g_unk0x1006e220[6] = {385, 184, 343, 221, 264, 290};

// GLOBAL: MW2SHELL 0x1006e238
MechS32 g_unk0x1006e238[6] = {269, 176, 506, 205, 269, 283};

// GLOBAL: MW2SHELL 0x1006e250
MechS32 g_unk0x1006e250[6] = {376, 186, 139, 254, 452, 325};

// GLOBAL: MW2SHELL 0x1006e268
MainMenuButton g_unk0x1006e268[5] = {
	{185, 280, 240, 400, 197, 327, g_unk0x10070094},
	{320, 300, 470, 400, 246, 371, g_unk0x100700a4},
	{20, 245, 90, 411, 25, 307, g_unk0x100700bc},
	{95, 385, 180, 479, 80, 455, g_unk0x100700c8},
	{0, 0, 639, 40, 320, 15, g_unk0x100700d4},
};

// GLOBAL: MW2SHELL 0x1006e2f8
MainMenuButton g_unk0x1006e2f8[15] = {
	{466, 450, 619, 474, 543, 455, g_unk0x100700dc},
	{32, 83, 297, 116, 41, 91, g_unk0x100700f0},
	{32, 117, 297, 151, 41, 125, g_unk0x100700f4},
	{32, 152, 297, 186, 41, 160, g_unk0x100700f8},
	{32, 187, 297, 221, 41, 195, g_unk0x100700fc},
	{32, 222, 297, 256, 41, 230, g_unk0x10070100},
	{32, 257, 297, 291, 41, 265, g_unk0x10070104},
	{32, 292, 297, 326, 41, 300, g_unk0x10070108},
	{32, 327, 297, 361, 41, 335, g_unk0x1007010c},
	{32, 362, 297, 397, 41, 370, g_unk0x10070110},
	{32, 398, 297, 432, 41, 406, g_unk0x10070114},
	{294, 450, 393, 474, 344, 455, g_unk0x10070118},
	{20, 450, 221, 474, 121, 455, g_unk0x10070124},
	{373, 403, 562, 427, 468, 408, g_unk0x1007013c},
	{418, 403, 517, 427, 468, 408, g_unk0x10070154},
};

// GLOBAL: MW2SHELL 0x1006e4a0
MainMenuButton g_unk0x1006e4a0[4] = {
	{0, 440, 639, 479, 320, 455, g_unk0x10070164},
	{405, 372, 479, 405, 449, 354, g_unk0x1007016c},
	{480, 372, 556, 405, 511, 408, g_unk0x10070178},
	{67, 358, 120, 410, 92, 408, g_unk0x10070184},
};

// GLOBAL: MW2SHELL 0x1006e510
MainMenuButton g_unk0x1006e510[7] = {
	{5, 149, 57, 440, 18, 203, g_unk0x1007018c},
	{450, 13, 629, 37, 540, 18, g_unk0x10070198},
	{450, 38, 629, 62, 540, 43, g_unk0x100701a8},
	{450, 63, 629, 87, 540, 68, g_unk0x100701b8},
	{450, 88, 629, 112, 540, 93, g_unk0x100701c8},
	{450, 113, 629, 137, 540, 118, g_unk0x100701d4},
	{450, 138, 629, 162, 540, 143, g_unk0x100701e4},
};

// GLOBAL: MW2SHELL 0x1006e5d8
MainMenuButton g_unk0x1006e5d8[20] = {
	{0, 0, 130, 479, 56, 223, g_unk0x100701ec},      {300, 320, 559, 419, 450, 364, g_unk0x100701f8},
	{510, 420, 559, 469, 542, 455, g_unk0x10070204}, {140, 90, 399, 313, 265, 226, g_unk0x10070214},
	{400, 25, 519, 49, 460, 30, g_unk0x10070228},    {400, 55, 519, 79, 460, 60, g_unk0x10070234},
	{400, 85, 519, 109, 460, 90, g_unk0x10070240},   {400, 115, 519, 139, 460, 120, g_unk0x10070248},
	{400, 145, 519, 169, 460, 150, g_unk0x10070250}, {400, 175, 519, 199, 460, 180, g_unk0x10070258},
	{400, 205, 519, 229, 460, 210, g_unk0x10070260}, {400, 235, 519, 259, 460, 240, g_unk0x10070268},
	{520, 25, 639, 49, 580, 30, g_unk0x10070270},    {520, 55, 639, 79, 580, 60, g_unk0x10070278},
	{520, 85, 639, 109, 580, 90, g_unk0x10070284},   {520, 115, 639, 139, 580, 120, g_unk0x1007028c},
	{520, 145, 639, 169, 580, 150, g_unk0x10070294}, {520, 175, 639, 199, 580, 180, g_unk0x1007029c},
	{520, 205, 639, 229, 580, 210, g_unk0x100702a8}, {520, 235, 639, 259, 580, 240, g_unk0x100702b0},
};

// GLOBAL: MW2SHELL 0x1006e808
MainMenuButton g_unk0x1006e808[4] = {
	{430, 450, 529, 474, 480, 455, g_unk0x100702b8},
	{110, 450, 209, 474, 160, 455, g_unk0x100702c0},
	{270, 450, 369, 474, 320, 455, g_unk0x100702cc},
	{540, 450, 639, 474, 590, 455, g_unk0x100702d8},
};

// GLOBAL: MW2SHELL 0x1006e878
MainMenuButton g_unk0x1006e878[4] = {
	{110, 450, 209, 474, 160, 455, g_unk0x100702e0},
	{270, 450, 369, 474, 320, 455, g_unk0x100702e8},
	{430, 450, 529, 474, 480, 455, g_unk0x100702f4},
	{510, 550, 609, 574, 560, 555, g_unk0x10070300},
};

// GLOBAL: MW2SHELL 0x1006e8e8
MainMenuButton g_unk0x1006e8e8[3] = {
	{270, 450, 369, 474, 320, 455, g_unk0x10070304},
	{110, 450, 209, 474, 160, 455, g_unk0x1007030c},
	{430, 450, 529, 474, 480, 455, g_unk0x10070318},
};

// GLOBAL: MW2SHELL 0x1006e940
MainMenuButton g_unk0x1006e940[4] = {
	{110, 450, 209, 474, 160, 455, g_unk0x10070324},
	{270, 450, 369, 474, 320, 455, g_unk0x1007032c},
	{430, 450, 529, 474, 480, 455, g_unk0x10070338},
	{510, 550, 609, 574, 560, 555, g_unk0x10070344},
};

// GLOBAL: MW2SHELL 0x1006e9b0
MainMenuButton g_unk0x1006e9b0[9] = {
	{50, 445, 149, 469, 100, 450, g_unk0x10070348},
	{404, 414, 474, 474, 440, 460, g_unk0x10070358},
	{263, 425, 302, 469, 280, 465, g_unk0x10070364},
	{237, 425, 262, 469, 260, 465, g_unk0x10070374},
	{303, 425, 330, 469, 300, 465, g_unk0x10070384},
	{200, 425, 236, 469, 240, 465, g_unk0x10070394},
	{159, 195, 187, 229, 73, 234, g_unk0x100703a4},
	{391, 162, 419, 196, 305, 201, g_unk0x100703b0},
	{586, 218, 614, 252, 500, 257, g_unk0x100703bc},
};

// GLOBAL: MW2SHELL 0x1006eab0
MainMenuButton g_unk0x1006eab0[11] = {
	{50, 445, 149, 469, 100, 450, g_unk0x100703c8},
	{404, 414, 474, 474, 440, 460, g_unk0x100703d4},
	{303, 425, 330, 469, 300, 465, g_unk0x100703e4},
	{200, 425, 236, 469, 240, 465, g_unk0x100703f4},
	{263, 425, 302, 469, 280, 465, g_unk0x10070404},
	{237, 425, 262, 469, 260, 465, g_unk0x10070414},
	{50, 420, 149, 444, 100, 425, g_unk0x10070424},
	{50, 395, 149, 419, 100, 400, g_unk0x10070430},
	{50, 420, 149, 444, 100, 425, g_unk0x10070440},
	{50, 445, 149, 469, 100, 450, g_unk0x10070448},
	{490, 445, 589, 469, 540, 450, g_unk0x10070450},
};

// GLOBAL: MW2SHELL 0x1006ebe8
MainMenuButton g_unk0x1006ebe8[5] = {
	{66, 167, 152, 303, 69, 187, g_unk0x1007045c},
	{160, 290, 375, 322, 110, 340, g_unk0x1007046c},
	{523, 154, 636, 332, 450, 200, g_unk0x10070484},
	{397, 385, 492, 466, 397, 443, g_unk0x10070490},
	{0, 0, 639, 40, 320, 15, g_unk0x1007049c},
};

// GLOBAL: MW2SHELL 0x1006ec78
MainMenuButton g_unk0x1006ec78[15] = {
	{466, 450, 619, 474, 543, 455, g_unk0x100704a4},
	{32, 83, 297, 116, 41, 91, g_unk0x100704b8},
	{32, 117, 297, 151, 41, 125, g_unk0x100704bc},
	{32, 152, 297, 186, 41, 160, g_unk0x100704c0},
	{32, 187, 297, 221, 41, 195, g_unk0x100704c4},
	{32, 222, 297, 256, 41, 230, g_unk0x100704c8},
	{32, 257, 297, 291, 41, 265, g_unk0x100704cc},
	{32, 292, 297, 326, 41, 300, g_unk0x100704d0},
	{32, 327, 297, 361, 41, 335, g_unk0x100704d4},
	{32, 362, 297, 397, 41, 370, g_unk0x100704d8},
	{32, 398, 297, 432, 41, 406, g_unk0x100704dc},
	{294, 450, 393, 474, 344, 455, g_unk0x100704e0},
	{20, 450, 221, 474, 121, 455, g_unk0x100704ec},
	{373, 403, 562, 427, 468, 408, g_unk0x10070504},
	{418, 403, 517, 427, 468, 408, g_unk0x1007051c},
};

// GLOBAL: MW2SHELL 0x1006ee20
MainMenuButton g_unk0x1006ee20[4] = {
	{0, 440, 639, 479, 320, 455, g_unk0x1007052c},
	{405, 364, 479, 397, 449, 346, g_unk0x10070534},
	{480, 364, 556, 397, 511, 400, g_unk0x10070540},
	{67, 358, 120, 410, 92, 400, g_unk0x1007054c},
};

// GLOBAL: MW2SHELL 0x1006ee90
MainMenuButton g_unk0x1006ee90[7] = {
	{5, 149, 57, 440, 18, 203, g_unk0x10070554},
	{450, 13, 629, 37, 540, 18, g_unk0x10070560},
	{450, 38, 629, 62, 540, 43, g_unk0x10070570},
	{450, 63, 629, 87, 540, 68, g_unk0x10070580},
	{450, 88, 629, 112, 540, 93, g_unk0x10070590},
	{450, 113, 629, 137, 540, 118, g_unk0x1007059c},
	{450, 138, 629, 162, 540, 143, g_unk0x100705ac},
};

// GLOBAL: MW2SHELL 0x1006ef58
MainMenuButton g_unk0x1006ef58[20] = {
	{0, 0, 130, 479, 56, 223, g_unk0x100705b4},      {280, 340, 534, 439, 415, 370, g_unk0x100705c0},
	{432, 440, 482, 479, 464, 460, g_unk0x100705cc}, {140, 90, 399, 313, 277, 226, g_unk0x100705dc},
	{400, 25, 519, 49, 460, 30, g_unk0x100705f0},    {400, 55, 519, 79, 460, 60, g_unk0x100705f8},
	{400, 85, 519, 109, 460, 90, g_unk0x10070600},   {400, 115, 519, 139, 460, 120, g_unk0x10070608},
	{400, 145, 519, 169, 460, 150, g_unk0x10070614}, {400, 175, 519, 199, 460, 180, g_unk0x1007061c},
	{400, 205, 519, 229, 460, 210, g_unk0x10070624}, {400, 235, 519, 259, 460, 240, g_unk0x1007062c},
	{520, 25, 639, 49, 580, 30, g_unk0x10070634},    {520, 55, 639, 79, 580, 60, g_unk0x1007063c},
	{520, 85, 639, 109, 580, 90, g_unk0x10070644},   {520, 115, 639, 139, 580, 120, g_unk0x1007064c},
	{520, 145, 639, 169, 580, 150, g_unk0x10070654}, {520, 175, 639, 199, 580, 180, g_unk0x1007065c},
	{520, 205, 639, 229, 580, 210, g_unk0x10070668}, {520, 235, 639, 259, 580, 240, g_unk0x10070674},
};

// GLOBAL: MW2SHELL 0x1006f188
MainMenuButton g_unk0x1006f188[4] = {
	{430, 450, 529, 474, 480, 455, g_unk0x1007067c},
	{110, 450, 209, 474, 160, 455, g_unk0x10070684},
	{270, 450, 369, 474, 320, 455, g_unk0x10070690},
	{540, 450, 639, 474, 590, 455, g_unk0x1007069c},
};

// GLOBAL: MW2SHELL 0x1006f1f8
MainMenuButton g_unk0x1006f1f8[4] = {
	{110, 450, 209, 474, 160, 455, g_unk0x100706a4},
	{270, 450, 369, 474, 320, 455, g_unk0x100706ac},
	{430, 450, 529, 474, 480, 455, g_unk0x100706b8},
	{510, 550, 609, 574, 560, 555, g_unk0x100706c4},
};

// GLOBAL: MW2SHELL 0x1006f268
MainMenuButton g_unk0x1006f268[3] = {
	{270, 450, 369, 474, 320, 455, g_unk0x100706c8},
	{110, 450, 209, 474, 160, 455, g_unk0x100706d0},
	{430, 450, 529, 474, 480, 455, g_unk0x100706dc},
};

// GLOBAL: MW2SHELL 0x1006f2c0
MainMenuButton g_unk0x1006f2c0[4] = {
	{110, 450, 209, 474, 160, 455, g_unk0x100706e8},
	{270, 450, 369, 474, 320, 455, g_unk0x100706f0},
	{430, 450, 529, 474, 480, 455, g_unk0x100706fc},
	{510, 550, 609, 574, 560, 555, g_unk0x10070708},
};

// GLOBAL: MW2SHELL 0x1006f330
MainMenuButton g_unk0x1006f330[9] = {
	{50, 445, 149, 469, 100, 450, g_unk0x1007070c},
	{404, 414, 474, 474, 440, 460, g_unk0x1007071c},
	{263, 425, 302, 469, 280, 465, g_unk0x10070728},
	{237, 425, 262, 469, 260, 465, g_unk0x10070738},
	{303, 425, 330, 469, 300, 465, g_unk0x10070748},
	{200, 425, 236, 469, 240, 465, g_unk0x10070758},
	{125, 201, 153, 235, 39, 240, g_unk0x10070768},
	{358, 156, 386, 190, 272, 195, g_unk0x10070774},
	{573, 246, 601, 280, 487, 285, g_unk0x10070780},
};

// GLOBAL: MW2SHELL 0x1006f430
MainMenuButton g_unk0x1006f430[11] = {
	{50, 445, 149, 469, 100, 450, g_unk0x1007078c},
	{404, 414, 474, 474, 440, 460, g_unk0x10070798},
	{303, 425, 330, 469, 300, 465, g_unk0x100707a8},
	{200, 425, 236, 469, 240, 465, g_unk0x100707b8},
	{263, 425, 302, 469, 280, 465, g_unk0x100707c8},
	{237, 425, 262, 469, 260, 465, g_unk0x100707d8},
	{50, 420, 149, 444, 100, 425, g_unk0x100707e8},
	{50, 395, 149, 419, 100, 400, g_unk0x100707f4},
	{50, 420, 149, 444, 100, 425, g_unk0x10070804},
	{50, 445, 149, 469, 100, 450, g_unk0x1007080c},
	{490, 445, 589, 469, 540, 450, g_unk0x10070814},
};

// GLOBAL: MW2SHELL 0x1006f568
MainMenuButton g_mainMenuButtons[4] = {
	{0xdb, 0x126, 0x1aa, 0x1a3, 0x140, 0x18b, g_unk0x10070820},
	{0x1ab, 0xf5, 0x27a, 0x175, 0x20d, 0x176, g_unk0x10070838},
	{0x0a, 0xc5, 0xc8, 0x172, 0x7c, 0x176, g_unk0x10070848},
	{0, 0x1c2, 0x27f, 0x1df, 0x140, 0x1c7, g_unk0x10070860},
};

// GLOBAL: MW2SHELL 0x1006f618
MainMenuButton g_unk0x1006f618[0x19] = {
	{209, 371, 420, 452, 0, 0, g_unk0x1007088c},     {50, 445, 149, 469, 100, 450, g_unk0x10070890},
	{238, 67, 400, 102, 239, 69, g_unk0x10070898},   {13, 124, 103, 137, 29, 126, g_unk0x1007089c},
	{13, 138, 103, 151, 29, 140, g_unk0x100708a0},   {13, 152, 103, 165, 29, 154, g_unk0x100708a4},
	{13, 181, 103, 194, 29, 183, g_unk0x100708a8},   {4, 124, 12, 137, 29, 126, g_unk0x100708ac},
	{4, 138, 12, 151, 29, 140, g_unk0x100708b0},     {4, 152, 12, 165, 29, 154, g_unk0x100708b4},
	{4, 181, 12, 194, 29, 183, g_unk0x100708b8},     {13, 205, 160, 352, 0, 0, g_unk0x100708bc},
	{134, 122, 172, 166, 0, 0, g_unk0x100708c0},     {134, 168, 172, 200, 0, 0, g_unk0x100708c4},
	{481, 249, 572, 262, 498, 251, g_unk0x100708c8}, {481, 263, 572, 276, 498, 265, g_unk0x100708cc},
	{481, 277, 572, 290, 498, 279, g_unk0x100708d0}, {481, 306, 572, 319, 498, 308, g_unk0x100708d4},
	{472, 249, 480, 262, 498, 251, g_unk0x100708d8}, {472, 263, 480, 276, 498, 265, g_unk0x100708dc},
	{472, 277, 480, 290, 498, 279, g_unk0x100708e0}, {472, 306, 480, 319, 498, 308, g_unk0x100708e4},
	{482, 330, 629, 477, 0, 0, g_unk0x100708e8},     {583, 247, 621, 291, 0, 0, g_unk0x100708ec},
	{583, 293, 621, 325, 0, 0, g_unk0x100708f0},
};

// GLOBAL: MW2SHELL 0x1006fc90
CampaignMission g_unk0x1006fc90[17] = {
	{"yellSCN1", 0, "Pyre Light"},
	{"oranSCN1", 0, "Flame Tongue "},
	{"tealSCN1", 0, "Blade Splint"},
	{"taupSCN1", 0, "Temper Edge"},
	{"jennSCN1", 1, "Trial 1"},
	{"sablSCN1", 0, "Sable Flame"},
	{"greySCN1", 0, "Burning Chrome"},
	{"browSCN1", 0, "Scorching Sand"},
	{"amy_SCN1", 1, "Trial 2"},
	{"silvSCN1", 0, "Silver Staff"},
	{"aquaSCN1", 0, "Aquiline Fire"},
	{"kim_SCN1", 1, "Trial 3"},
	{"cyanSCN1", 0, "Cold Crescent"},
	{"maroSCN1", 0, "Velvet Hammer"},
	{"goldSCN1", 0, "Golden Spade"},
	{"irenSCN1", 1, "Trial 4"},
	{NULL, 0, "Retired"},
};

// GLOBAL: MW2SHELL 0x1006fd30
CampaignMission g_unk0x1006fd30[17] = {
	{"pinkSCN1", 0, "Silent Thunder"},
	{"greeSCN1", 0, "Arkham Bridge"},
	{"red_SCN1", 0, "Mirror Cage"},
	{"fuchSCN1", 0, "Bone Machine"},
	{"cindSCN1", 1, "Trial 1"},
	{"rustSCN1", 0, "Bouk Obelisk"},
	{"umbeSCN1", 0, "Umber Wall"},
	{"tan_SCN1", 0, "Rogue Chariot"},
	{"heidSCN1", 1, "Trial 2"},
	{"plumSCN1", 0, "Plum Wine"},
	{"whitSCN1", 0, "Rust Heart"},
	{"jillSCN1", 1, "Trial 3"},
	{"puceSCN1", 0, "Armor Veil"},
	{"blonSCN1", 0, "Iron Piston"},
	{"bronSCN1", 0, "Bronze Anvil"},
	{"marySCN1", 1, "Trial 4"},
	{NULL, 0, "Retired"},
};

// GLOBAL: MW2SHELL 0x1006fdd0
CampaignMission* g_campaignMissions[2] = {g_unk0x1006fc90, g_unk0x1006fd30};

// The training missions of each campaign, one per button from the second.
// GLOBAL: MW2SHELL 0x1006fdd8
MechChar* g_unk0x1006fdd8[6] = {"tnw1SCN1", "tnw2SCN1", "tnw3SCN1", "tnw4SCN1", "tnw5SCN1", "tnw6SCN1"};

// GLOBAL: MW2SHELL 0x1006fdf0
MechChar* g_unk0x1006fdf0[6] = {"tnj1SCN1", "tnj2SCN1", "tnj3SCN1", "tnj4SCN1", "tnj5SCN1", "tnj6SCN1"};

// GLOBAL: MW2SHELL 0x1006fe08
MechChar** g_unk0x1006fe08[2] = {g_unk0x1006fdd8, g_unk0x1006fdf0};

// GLOBAL: MW2SHELL 0x1006fe10
MenuScreen g_unk0x1006fe10[3] = {
	{g_unk0x1006e268, 5, 11, 0x24},
	{g_unk0x1006ebe8, 5, 18, 0x27},
	{NULL, 0, 0, 0},
};

// GLOBAL: MW2SHELL 0x1006fe40
MenuScreen g_unk0x1006fe40[3] = {
	{g_unk0x1006e2f8, 15, 17, -1},
	{g_unk0x1006ec78, 15, 24, -1},
	{NULL, 0, 0, 0},
};

// GLOBAL: MW2SHELL 0x1006fe70
MenuScreen g_unk0x1006fe70[3] = {
	{g_unk0x1006e4a0, 4, 12, -1},
	{g_unk0x1006ee20, 4, 19, -1},
	{NULL, 0, 0, 0},
};

// GLOBAL: MW2SHELL 0x1006fea0
MenuScreen g_unk0x1006fea0[3] = {
	{g_unk0x1006e9b0, 9, 15, -1},
	{g_unk0x1006f330, 9, 22, -1},
	{g_unk0x1006e9b0, 9, 10, -1},
};

// GLOBAL: MW2SHELL 0x1006fed0
MenuScreen g_unk0x1006fed0[3] = {
	{g_unk0x1006e5d8, 20, 14, 0x25},
	{g_unk0x1006ef58, 20, 21, 0x28},
	{NULL, 0, 0, 0},
};

// The debriefing screen of each campaign.
// GLOBAL: MW2SHELL 0x1006ff00
MenuScreen g_unk0x1006ff00[3] = {
	{g_unk0x1006e8e8, 3, 16, -1},
	{g_unk0x1006f268, 3, 23, -1},
	{g_unk0x1006e8e8, 3, 10, -1},
};

// The aftermath reader of each campaign.
// GLOBAL: MW2SHELL 0x1006ff30
MenuScreen g_unk0x1006ff30[3] = {
	{g_unk0x1006e940, 4, 16, -1},
	{g_unk0x1006f2c0, 4, 23, -1},
	{g_unk0x1006e940, 4, 10, -1},
};

// The briefing screen of each campaign. SKIP (the fourth button) is dropped for every pilot
// but FERRARI.
// GLOBAL: MW2SHELL 0x1006ff60
MenuScreen g_unk0x1006ff60[3] = {
	{g_unk0x1006e808, 4, 16, -1},
	{g_unk0x1006f188, 4, 23, -1},
	{g_unk0x1006e808, 4, 10, -1},
};

// The situation reader of each campaign.
// GLOBAL: MW2SHELL 0x1006ff90
MenuScreen g_unk0x1006ff90[3] = {
	{g_unk0x1006e878, 4, 16, -1},
	{g_unk0x1006f1f8, 4, 23, -1},
	{g_unk0x1006e878, 4, 10, -1},
};

// GLOBAL: MW2SHELL 0x1006ffc0
MenuScreen g_unk0x1006ffc0[3] = {
	{g_unk0x1006e510, 7, 13, 38},
	{g_unk0x1006ee90, 7, 20, 41},
	{NULL, 0, 0, 0},
};

// GLOBAL: MW2SHELL 0x1006fff0
MenuScreen g_unk0x1006fff0[3] = {
	{g_unk0x1006eab0, 11, 15, -1},
	{g_unk0x1006f430, 11, 22, -1},
	{g_unk0x1006eab0, 11, 10, -1},
};
