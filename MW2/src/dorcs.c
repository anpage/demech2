/* Menu actions (the eject menu) and the "dorcs" sequence's data files. */
#include "dorcs.h"

#include "clock.h"
#include "cockpit.h"
#include "config.h"
#include "displaybackend.h"
#include "gamekeys.h"
#include "hud.h"
#include "mechdamage.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "menutextbox.h"
#include "mss.h"
#include "palette.h"
#include "palettecolor.h"
#include "players.h"
#include "polydraw.h"
#include "recttransition.h"
#include "refreshmode.h"
#include "render.h"
#include "rendersettings.h"
#include "screenscale.h"
#include "simmain.h"
#include "soundfx.h"
#include "targeting.h"
#include "types.h"
#include "vfxa.h"

#include <stdio.h>
#include <windows.h>

// GLOBAL: MW2 0x100ae760
// GLOBAL: MW2GBL 0x100a0e10
// GLOBAL: MW2PREMERCS 0x100c3020
// GLOBAL: MW23DFX 0x10261878
// GLOBAL: MW2ATI 0x10195b70
// GLOBAL: MW2S3 0x10066a40
// GLOBAL: MW2POWERVR 0x101c0ee8
MechChar g_dorcsPageTitle[] = "MW2 Programmer Dorcs Page";

// GLOBAL: MW2 0x100ae780
// GLOBAL: MW2GBL 0x100a0e30
// GLOBAL: MW2PREMERCS 0x100c3040
// GLOBAL: MW23DFX 0x10261898
// GLOBAL: MW2ATI 0x10195b90
// GLOBAL: MW2S3 0x10066a60
// GLOBAL: MW2POWERVR 0x101c0f08
MechChar g_dorcsAboutItem[] = "About Dorcs";

// GLOBAL: MW2 0x100ae790
// GLOBAL: MW2GBL 0x100a0e40
// GLOBAL: MW2PREMERCS 0x100c3050
// GLOBAL: MW23DFX 0x102618a8
// GLOBAL: MW2ATI 0x10195ba0
// GLOBAL: MW2S3 0x10066a70
// GLOBAL: MW2POWERVR 0x101c0f18
MechChar g_dorcsClarkeName[] = "John A. Clarke";

// GLOBAL: MW2 0x100ae7a0
// GLOBAL: MW2GBL 0x100a0e50
// GLOBAL: MW2PREMERCS 0x100c3060
// GLOBAL: MW23DFX 0x102618b8
// GLOBAL: MW2ATI 0x10195bb0
// GLOBAL: MW2S3 0x10066a80
// GLOBAL: MW2POWERVR 0x101c0f28
MechChar g_dorcsDouglasName[] = "Michael H. Douglas";

// GLOBAL: MW2 0x100ae7b8
// GLOBAL: MW2GBL 0x100a0e68
// GLOBAL: MW2PREMERCS 0x100c3078
// GLOBAL: MW23DFX 0x102618d0
// GLOBAL: MW2ATI 0x10195bc8
// GLOBAL: MW2S3 0x10066a98
// GLOBAL: MW2POWERVR 0x101c0f40
MechChar g_dorcsEthertonName[] = "Scott T. Etherton";

// GLOBAL: MW2 0x100ae7d0
// GLOBAL: MW2GBL 0x100a0e80
// GLOBAL: MW2PREMERCS 0x100c3090
// GLOBAL: MW23DFX 0x102618e8
// GLOBAL: MW2ATI 0x10195be0
// GLOBAL: MW2S3 0x10066ab0
// GLOBAL: MW2POWERVR 0x101c0f58
MechChar g_dorcsHusebyName[] = "Sverre H. Huseby";

// GLOBAL: MW2 0x100ae7e8
// GLOBAL: MW2GBL 0x100a0e98
// GLOBAL: MW2PREMERCS 0x100c30a8
// GLOBAL: MW23DFX 0x10261900
// GLOBAL: MW2ATI 0x10195bf8
// GLOBAL: MW2S3 0x10066ac8
// GLOBAL: MW2POWERVR 0x101c0f70
MechChar g_dorcsKaminsName[] = "Dan Kamins";

// GLOBAL: MW2 0x100ae7f8
// GLOBAL: MW2GBL 0x100a0ea8
// GLOBAL: MW2PREMERCS 0x100c30b8
// GLOBAL: MW23DFX 0x10261910
// GLOBAL: MW2ATI 0x10195c08
// GLOBAL: MW2S3 0x10066ad8
// GLOBAL: MW2POWERVR 0x101c0f80
MechChar g_dorcsKeatingName[] = "John Keating";

// GLOBAL: MW2 0x100ae808
// GLOBAL: MW2GBL 0x100a0eb8
// GLOBAL: MW2PREMERCS 0x100c30c8
// GLOBAL: MW23DFX 0x10261920
// GLOBAL: MW2ATI 0x10195c18
// GLOBAL: MW2S3 0x10066ae8
// GLOBAL: MW2POWERVR 0x101c0f90
MechChar g_dorcsMilesName[] = "John Miles";

// GLOBAL: MW2 0x100ae818
// GLOBAL: MW2GBL 0x100a0ec8
// GLOBAL: MW2PREMERCS 0x100c30d8
// GLOBAL: MW23DFX 0x10261930
// GLOBAL: MW2ATI 0x10195c28
// GLOBAL: MW2S3 0x10066af8
// GLOBAL: MW2POWERVR 0x101c0fa0
MechChar g_dorcsMortenName[] = "Tim Morten";

// GLOBAL: MW2 0x100ae828
// GLOBAL: MW2GBL 0x100a0ed8
// GLOBAL: MW2PREMERCS 0x100c30e8
// GLOBAL: MW23DFX 0x10261940
// GLOBAL: MW2ATI 0x10195c38
// GLOBAL: MW2S3 0x10066b08
// GLOBAL: MW2POWERVR 0x101c0fb0
MechChar g_dorcsMortensenName[] = "Bob Mortensen";

// GLOBAL: MW2 0x100ae838
// GLOBAL: MW2GBL 0x100a0ee8
// GLOBAL: MW2PREMERCS 0x100c30f8
// GLOBAL: MW23DFX 0x10261950
// GLOBAL: MW2ATI 0x10195c48
// GLOBAL: MW2S3 0x10066b18
// GLOBAL: MW2POWERVR 0x101c0fc0
MechChar g_dorcsPetersonName[] = "Eric Peterson";

// GLOBAL: MW2 0x100ae848
// GLOBAL: MW2GBL 0x100a0ef8
// GLOBAL: MW2PREMERCS 0x100c3108
// GLOBAL: MW23DFX 0x10261960
// GLOBAL: MW2ATI 0x10195c58
// GLOBAL: MW2S3 0x10066b28
// GLOBAL: MW2POWERVR 0x101c0fd0
MechChar g_dorcsStanfillName[] = "Dan Stanfill";

// GLOBAL: MW2 0x100ae858
// GLOBAL: MW2GBL 0x100a0f08
// GLOBAL: MW2PREMERCS 0x100c3118
// GLOBAL: MW23DFX 0x10261970
// GLOBAL: MW2ATI 0x10195c68
// GLOBAL: MW2S3 0x10066b38
// GLOBAL: MW2POWERVR 0x101c0fe0
MechChar g_dorcsWhiteName[] = "David White";

// GLOBAL: MW2 0x100ae868
// GLOBAL: MW2GBL 0x100a0f18
// GLOBAL: MW2PREMERCS 0x100c3128
// GLOBAL: MW23DFX 0x10261980
// GLOBAL: MW2ATI 0x10195c78
// GLOBAL: MW2S3 0x10066b48
// GLOBAL: MW2POWERVR 0x101c0ff0
MechChar g_dorcsZobelName[] = "Dave Zobel";

// GLOBAL: MW2 0x100ae878
// GLOBAL: MW2GBL 0x100a0f28
// GLOBAL: MW2PREMERCS 0x100c3138
// GLOBAL: MW23DFX 0x10261990
// GLOBAL: MW2ATI 0x10195c88
// GLOBAL: MW2S3 0x10066b58
// GLOBAL: MW2POWERVR 0x101c1000
MechChar g_dorcsAboutText[] =
	"Hi MW2 Fans,\n"
	"\n"
	"Welcome to the MW2 Sim Programmer Dorcs Web page.  We hope you'll spend a few minutes here to learn a bit about "
	"us.  Basically, we are just a bunch of guys who love to code. We get positively giddi watching the source files "
	"stream in after a well timed dorcs co.  We spend hours debating age old questions such as \"Which came first wasm "
	"or unwasm?\", \"Why do our game designers insist on using DOS Edit?\" and \"What does booyow mean?\".  Uh oh, "
	"here comes the \"ALL CLEAR FOR DORCS!\" signal.  We better get back to coding.\n"
	"\n"
	"So long for now, \n"
	"\n"
	"MW2 Programmers";

// GLOBAL: MW2 0x100aeab8
// GLOBAL: MW2GBL 0x100a1168
// GLOBAL: MW2PREMERCS 0x100c3378
// GLOBAL: MW23DFX 0x10261bd0
// GLOBAL: MW2ATI 0x10195ec8
// GLOBAL: MW2S3 0x10066d98
// GLOBAL: MW2POWERVR 0x101c1240
MechChar g_dorcsClarkeText[] =
	"Hi MW2 fans,\n"
	"\n"
	"I can't believe it finally over.  My tour of duty has been 18 months long.  I am looking forward to spending lots "
	"of time with my wife Carol Codekas and our son John Taylor.  Taylor became the official MechBaby when he was born "
	"seven months ago.  The delivery went smoothly except that in all the excitement I dropped my laptop and lost a "
	"few days worth of source code.\n"
	"\n"
	"Thanks to Bill, Dr. Bob and HP for keeping me sane and sober. Thanks Carol for your patience and Grandma Clarke "
	"and Codekas for babysitting Taylor. Hi John Clarke Sr! I have one last announcement for old time's sake.\n"
	"\n"
	"ATTENTION ALL PROGRAMMERS! ALL CLEAR FOR DORCS CO! GET GIDDI TOO!";

// GLOBAL: MW2 0x100aed58
// GLOBAL: MW2GBL 0x100a1408
// GLOBAL: MW2PREMERCS 0x100c3618
// GLOBAL: MW23DFX 0x10261e70
// GLOBAL: MW2ATI 0x10196168
// GLOBAL: MW2S3 0x10067038
// GLOBAL: MW2POWERVR 0x101c14e0
MechChar g_dorcsDouglasText[] =
	"    If I had known six months ago how crazy working on MWII was going to be, I'm not sure I would have done it. "
	"But, after countless late nights and thousands of ornery bugs, we've finally got a killer game.\n"
	"    I'd like to thank John Spinale and Josh Resnick for bringing me on. They've put together the best development "
	"team anywhere, and I'm glad to have the opportunity to work with them.\n"
	"    Most of all, I'd like to thank my wife Rosaline for her infinite patience. Not everyone would understand "
	"postponing a honeymoon for a game.\n"
	"\n"
	"-- Michael H. Douglas     DEI/FEIF\n"
	"    mdouglas@activision.com\n"
	"\n"
	"P.S. - No, I'm not THAT Michael Douglas.";

// GLOBAL: MW2 0x100aefe0
// GLOBAL: MW2GBL 0x100a1690
// GLOBAL: MW2PREMERCS 0x100c38a0
// GLOBAL: MW23DFX 0x102620f8
// GLOBAL: MW2ATI 0x101963f0
// GLOBAL: MW2S3 0x100672c0
// GLOBAL: MW2POWERVR 0x101c1768
MechChar g_dorcsEthertonText[] = "I just love to code.  That's all there is to it.";

// GLOBAL: MW2 0x100af018
// GLOBAL: MW2GBL 0x100a16c8
// GLOBAL: MW2PREMERCS 0x100c38d8
// GLOBAL: MW23DFX 0x10262130
// GLOBAL: MW2ATI 0x10196428
// GLOBAL: MW2S3 0x100672f8
// GLOBAL: MW2POWERVR 0x101c17a0
MechChar g_dorcsHusebyText[] =
	"We don't know much about this guy except that he lives in Norway and probably has a thick Norwegian accent.  He "
	"wrote some handy freeware called GifSave and was kind enough to post it on the net.  You can thank him personally "
	"at sverrehu@ifi.uio.no for the cool high res screen shots.\n";

// GLOBAL: MW2 0x100af138
// GLOBAL: MW2GBL 0x100a17e8
// GLOBAL: MW2PREMERCS 0x100c39f8
// GLOBAL: MW23DFX 0x10262250
// GLOBAL: MW2ATI 0x10196548
// GLOBAL: MW2S3 0x10067418
// GLOBAL: MW2POWERVR 0x101c18c0
MechChar g_dorcsKaminsText[] =
	"  Thanks to all!\n"
	"  I'd take better advantage of my DORCSOpportunity (TM) here, except for the fact that I have 34 bugs to fix.\n"
	"  By the way, I wrote the NetDemo network shell.\n"
	"  If you liked it, I'd love to hear from you.\n"
	"--\n"
	"dkamins@husc.harvard.edu\n";

// GLOBAL: MW2 0x100af238
// GLOBAL: MW2GBL 0x100a18e8
// GLOBAL: MW2PREMERCS 0x100c3af8
// GLOBAL: MW23DFX 0x10262350
// GLOBAL: MW2ATI 0x10196648
// GLOBAL: MW2S3 0x10067518
// GLOBAL: MW2POWERVR 0x101c19c0
MechChar g_dorcsKeatingText[] =
	"John drew on his experience defending his turf and scrapping in the gutters as a kid in San Antonio's roughest "
	"neighborhood, \"Alamo Heights\", to create the battle AI for MechWarrior.  A Mac guy living in a hostile PC "
	"world, he is naturally short tempered and combative.  For the part of drill instructor John was influenced "
	"heavily by his big sister, Paula, as well as huge doses of MSG.  He welcomes your criticisms, offers of sympathy, "
	"or any reply from those lacking a \"Y\" chromosome at Marshall1@aol.com.";

// GLOBAL: MW2 0x100af438
// GLOBAL: MW2GBL 0x100a1ae8
// GLOBAL: MW2PREMERCS 0x100c3cf8
// GLOBAL: MW23DFX 0x10262550
// GLOBAL: MW2ATI 0x10196848
// GLOBAL: MW2S3 0x10067718
// GLOBAL: MW2POWERVR 0x101c1bc0
MechChar g_dorcsMilesText[] =
	"This guy wrote all the graphics and sound packages (along with John Lemberger) We could dorcs co his API with "
	"confidence because we knew we were getting high quality, efficient and reliable code.  Once we converted to the "
	"world of PANES, high res was easy.  The DLL loader just made us giddi.  We even used it for these menus.\n"
	"\n"
	"Thanks to to John for all the support.";

// GLOBAL: MW2 0x100af5a8
// GLOBAL: MW2GBL 0x100a1c58
// GLOBAL: MW2PREMERCS 0x100c3e68
// GLOBAL: MW23DFX 0x102626c0
// GLOBAL: MW2ATI 0x101969b8
// GLOBAL: MW2S3 0x10067888
// GLOBAL: MW2POWERVR 0x101c1d30
MechChar g_dorcsMortenText[] =
	"Tim's Liner Notes\n"
	"\n"
	"Is it done?  Can I leave my cube?  Where is everybody?\n"
	"\n"
	"I'm grateful for the friendship of all the people I've worked with on this project, from start to finish.  This "
	"project was a labor of love (and at times, war) for everyone who worked on it.\n"
	"\n"
	"Do you like camping?\n"
	"\n"
	"Tim Morten";

// GLOBAL: MW2 0x100af6d8
// GLOBAL: MW2GBL 0x100a1d88
// GLOBAL: MW2PREMERCS 0x100c3f98
// GLOBAL: MW23DFX 0x102627f0
// GLOBAL: MW2ATI 0x10196ae8
// GLOBAL: MW2S3 0x100679b8
// GLOBAL: MW2POWERVR 0x101c1e60
MechChar g_dorcsMortensenText[] =
	"I'm so HONORED!  Only three months on the project and I'm an official DORCS.\n"
	"\n"
	"Of course I'll have to thank my wife and kids, that is if I can ever remember their names....  Hmmmm, oh yeah.... "
	"Thanks Debra (8) and Timmy (4).  But mostly thanks to Helen (], with out your support and help I never would have "
	"been able to contribute as much as I have to this ROCKING GAME!\n"
	"\n"
	"Later,\n"
	"BobM";

// GLOBAL: MW2 0x100af858
// GLOBAL: MW2GBL 0x100a1f08
// GLOBAL: MW2PREMERCS 0x100c4118
// GLOBAL: MW23DFX 0x10262970
// GLOBAL: MW2ATI 0x10196c68
// GLOBAL: MW2S3 0x10067b38
// GLOBAL: MW2POWERVR 0x101c1fe0
MechChar g_dorcsPetersonText[] =
	"Eric no longer works here but he is really the grand-pappy of the Sim engine.  The original concept was his and "
	"he single-handedly wrote the first real mode version (including all tools and much of the art).  Without Eric's "
	"dedication to Warthink, we would'nt have a snowball's chance in heck of completing this game.\n"
	"\n"
	"By the way, if anyone sees Eric, would they mind asking him what \"booyow\" means?";

// GLOBAL: MW2 0x100af9e8
// GLOBAL: MW2GBL 0x100a2098
// GLOBAL: MW2PREMERCS 0x100c42a8
// GLOBAL: MW23DFX 0x10262b00
// GLOBAL: MW2ATI 0x10196df8
// GLOBAL: MW2S3 0x10067cc8
// GLOBAL: MW2POWERVR 0x101c2170
MechChar g_dorcsStanfillText[] =
	"We made it!\n"
	"\n"
	"I want to thank John Spinale, Josh Resnick, and Howard Marks for putting together such a stellar team for this "
	"project.\n"
	"\n"
	"But more than anything I want to thank my wife Kyung Ah and son D.J. for supporting me during the past seven "
	"months and for tolerating my constant absence while we built the coolest game ever!";

// GLOBAL: MW2 0x100afb30
// GLOBAL: MW2GBL 0x100a21e0
// GLOBAL: MW2PREMERCS 0x100c43f0
// GLOBAL: MW23DFX 0x10262c48
// GLOBAL: MW2ATI 0x10196f40
// GLOBAL: MW2S3 0x10067e10
// GLOBAL: MW2POWERVR 0x101c22b8
MechChar g_dorcsWhiteText[] = "I just love to code.  That's all there is to it.";

// GLOBAL: MW2 0x100afb68
// GLOBAL: MW2GBL 0x100a2218
// GLOBAL: MW2PREMERCS 0x100c4428
// GLOBAL: MW23DFX 0x10262c80
// GLOBAL: MW2ATI 0x10196f78
// GLOBAL: MW2S3 0x10067e48
// GLOBAL: MW2POWERVR 0x101c22f0
MechChar g_dorcsZobelText[] = "Dave Zobel was last seen plunging into the darkness beyond the Wall of Testosterone.";

// GLOBAL: MW2 0x100afbc0
// GLOBAL: MW2GBL 0x100a2270
// GLOBAL: MW2PREMERCS 0x100c4480
// GLOBAL: MW23DFX 0x10262cd8
// GLOBAL: MW2ATI 0x10196fd0
// GLOBAL: MW2S3 0x10067ea0
// GLOBAL: MW2POWERVR 0x101c2348
MechChar g_dorcsExitItem[] = "This is boring!";

// GLOBAL: MW2 0x100afbd0
// GLOBAL: MW2GBL 0x100a2280
// GLOBAL: MW2PREMERCS 0x100c4490
// GLOBAL: MW23DFX 0x10262ce8
// GLOBAL: MW2ATI 0x10196fe0
// GLOBAL: MW2S3 0x10067eb0
// GLOBAL: MW2POWERVR 0x101c2358
PANE g_dorcsTextRect = {NULL, 0, 0x2666, 0x10000, 0x10000};

// GLOBAL: MW2 0x100afbe8
// GLOBAL: MW2GBL 0x100a2298
// GLOBAL: MW2PREMERCS 0x100c44a8
// GLOBAL: MW23DFX 0x10262d00
// GLOBAL: MW2ATI 0x10196ff8
// GLOBAL: MW2S3 0x10067ec8
// GLOBAL: MW2POWERVR 0x101c2370
MenuTextBox g_dorcsAboutTextBox = {&g_dorcsTextRect, g_dorcsAboutText};

// GLOBAL: MW2 0x100afbf0
// GLOBAL: MW2GBL 0x100a22a0
// GLOBAL: MW2PREMERCS 0x100c44b0
// GLOBAL: MW23DFX 0x10262d08
// GLOBAL: MW2ATI 0x10197000
// GLOBAL: MW2S3 0x10067ed0
// GLOBAL: MW2POWERVR 0x101c2378
MenuTextBox g_dorcsClarkeTextBox = {&g_dorcsTextRect, g_dorcsClarkeText};

// GLOBAL: MW2 0x100afbf8
// GLOBAL: MW2GBL 0x100a22a8
// GLOBAL: MW2PREMERCS 0x100c44b8
// GLOBAL: MW23DFX 0x10262d10
// GLOBAL: MW2ATI 0x10197008
// GLOBAL: MW2S3 0x10067ed8
// GLOBAL: MW2POWERVR 0x101c2380
MenuTextBox g_dorcsDouglasTextBox = {&g_dorcsTextRect, g_dorcsDouglasText};

// GLOBAL: MW2 0x100afc00
// GLOBAL: MW2GBL 0x100a22b0
// GLOBAL: MW2PREMERCS 0x100c44c0
// GLOBAL: MW23DFX 0x10262d18
// GLOBAL: MW2ATI 0x10197010
// GLOBAL: MW2S3 0x10067ee0
// GLOBAL: MW2POWERVR 0x101c2388
MenuTextBox g_dorcsEthertonTextBox = {&g_dorcsTextRect, g_dorcsEthertonText};

// GLOBAL: MW2 0x100afc08
// GLOBAL: MW2GBL 0x100a22b8
// GLOBAL: MW2PREMERCS 0x100c44c8
// GLOBAL: MW23DFX 0x10262d20
// GLOBAL: MW2ATI 0x10197018
// GLOBAL: MW2S3 0x10067ee8
// GLOBAL: MW2POWERVR 0x101c2390
MenuTextBox g_dorcsHusebyTextBox = {&g_dorcsTextRect, g_dorcsHusebyText};

// GLOBAL: MW2 0x100afc10
// GLOBAL: MW2GBL 0x100a22c0
// GLOBAL: MW2PREMERCS 0x100c44d0
// GLOBAL: MW23DFX 0x10262d28
// GLOBAL: MW2ATI 0x10197020
// GLOBAL: MW2S3 0x10067ef0
// GLOBAL: MW2POWERVR 0x101c2398
MenuTextBox g_dorcsKaminsTextBox = {&g_dorcsTextRect, g_dorcsKaminsText};

// GLOBAL: MW2 0x100afc18
// GLOBAL: MW2GBL 0x100a22c8
// GLOBAL: MW2PREMERCS 0x100c44d8
// GLOBAL: MW23DFX 0x10262d30
// GLOBAL: MW2ATI 0x10197028
// GLOBAL: MW2S3 0x10067ef8
// GLOBAL: MW2POWERVR 0x101c23a0
MenuTextBox g_dorcsKeatingTextBox = {&g_dorcsTextRect, g_dorcsKeatingText};

// GLOBAL: MW2 0x100afc20
// GLOBAL: MW2GBL 0x100a22d0
// GLOBAL: MW2PREMERCS 0x100c44e0
// GLOBAL: MW23DFX 0x10262d38
// GLOBAL: MW2ATI 0x10197030
// GLOBAL: MW2S3 0x10067f00
// GLOBAL: MW2POWERVR 0x101c23a8
MenuTextBox g_dorcsMilesTextBox = {&g_dorcsTextRect, g_dorcsMilesText};

// GLOBAL: MW2 0x100afc28
// GLOBAL: MW2GBL 0x100a22d8
// GLOBAL: MW2PREMERCS 0x100c44e8
// GLOBAL: MW23DFX 0x10262d40
// GLOBAL: MW2ATI 0x10197038
// GLOBAL: MW2S3 0x10067f08
// GLOBAL: MW2POWERVR 0x101c23b0
MenuTextBox g_dorcsMortenTextBox = {&g_dorcsTextRect, g_dorcsMortenText};

// GLOBAL: MW2 0x100afc30
// GLOBAL: MW2GBL 0x100a22e0
// GLOBAL: MW2PREMERCS 0x100c44f0
// GLOBAL: MW23DFX 0x10262d48
// GLOBAL: MW2ATI 0x10197040
// GLOBAL: MW2S3 0x10067f10
// GLOBAL: MW2POWERVR 0x101c23b8
MenuTextBox g_dorcsMortensenTextBox = {&g_dorcsTextRect, g_dorcsMortensenText};

// GLOBAL: MW2 0x100afc38
// GLOBAL: MW2GBL 0x100a22e8
// GLOBAL: MW2PREMERCS 0x100c44f8
// GLOBAL: MW23DFX 0x10262d50
// GLOBAL: MW2ATI 0x10197048
// GLOBAL: MW2S3 0x10067f18
// GLOBAL: MW2POWERVR 0x101c23c0
MenuTextBox g_dorcsPetersonTextBox = {&g_dorcsTextRect, g_dorcsPetersonText};

// GLOBAL: MW2 0x100afc40
// GLOBAL: MW2GBL 0x100a22f0
// GLOBAL: MW2PREMERCS 0x100c4500
// GLOBAL: MW23DFX 0x10262d58
// GLOBAL: MW2ATI 0x10197050
// GLOBAL: MW2S3 0x10067f20
// GLOBAL: MW2POWERVR 0x101c23c8
MenuTextBox g_dorcsStanfillTextBox = {&g_dorcsTextRect, g_dorcsStanfillText};

// GLOBAL: MW2 0x100afc48
// GLOBAL: MW2GBL 0x100a22f8
// GLOBAL: MW2PREMERCS 0x100c4508
// GLOBAL: MW23DFX 0x10262d60
// GLOBAL: MW2ATI 0x10197058
// GLOBAL: MW2S3 0x10067f28
// GLOBAL: MW2POWERVR 0x101c23d0
MenuTextBox g_dorcsWhiteTextBox = {&g_dorcsTextRect, g_dorcsWhiteText};

// GLOBAL: MW2 0x100afc50
// GLOBAL: MW2GBL 0x100a2300
// GLOBAL: MW2PREMERCS 0x100c4510
// GLOBAL: MW23DFX 0x10262d68
// GLOBAL: MW2ATI 0x10197060
// GLOBAL: MW2S3 0x10067f30
// GLOBAL: MW2POWERVR 0x101c23d8
MenuTextBox g_dorcsZobelTextBox = {&g_dorcsTextRect, g_dorcsZobelText};

// GLOBAL: MW2 0x100afc58
// GLOBAL: MW2GBL 0x100a2308
// GLOBAL: MW2PREMERCS 0x100c4518
// GLOBAL: MW23DFX 0x10262d70
// GLOBAL: MW2ATI 0x10197068
// GLOBAL: MW2S3 0x10067f38
// GLOBAL: MW2POWERVR 0x101c23e0
MenuControl g_dorcsAboutControl = {2, 0, &g_dorcsAboutTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afc80
// GLOBAL: MW2GBL 0x100a2330
// GLOBAL: MW2PREMERCS 0x100c4540
// GLOBAL: MW23DFX 0x10262d98
// GLOBAL: MW2ATI 0x10197090
// GLOBAL: MW2S3 0x10067f60
// GLOBAL: MW2POWERVR 0x101c2408
MenuControl g_dorcsClarkeControl = {2, 0, &g_dorcsClarkeTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afca8
// GLOBAL: MW2GBL 0x100a2358
// GLOBAL: MW2PREMERCS 0x100c4568
// GLOBAL: MW23DFX 0x10262dc0
// GLOBAL: MW2ATI 0x101970b8
// GLOBAL: MW2S3 0x10067f88
// GLOBAL: MW2POWERVR 0x101c2430
MenuControl g_dorcsDouglasControl = {2, 0, &g_dorcsDouglasTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afcd0
// GLOBAL: MW2GBL 0x100a2380
// GLOBAL: MW2PREMERCS 0x100c4590
// GLOBAL: MW23DFX 0x10262de8
// GLOBAL: MW2ATI 0x101970e0
// GLOBAL: MW2S3 0x10067fb0
// GLOBAL: MW2POWERVR 0x101c2458
MenuControl g_dorcsEthertonControl = {2, 0, &g_dorcsEthertonTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afcf8
// GLOBAL: MW2GBL 0x100a23a8
// GLOBAL: MW2PREMERCS 0x100c45b8
// GLOBAL: MW23DFX 0x10262e10
// GLOBAL: MW2ATI 0x10197108
// GLOBAL: MW2S3 0x10067fd8
// GLOBAL: MW2POWERVR 0x101c2480
MenuControl g_dorcsHusebyControl = {2, 0, &g_dorcsHusebyTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afd20
// GLOBAL: MW2GBL 0x100a23d0
// GLOBAL: MW2PREMERCS 0x100c45e0
// GLOBAL: MW23DFX 0x10262e38
// GLOBAL: MW2ATI 0x10197130
// GLOBAL: MW2S3 0x10068000
// GLOBAL: MW2POWERVR 0x101c24a8
MenuControl g_dorcsKaminsControl = {2, 0, &g_dorcsKaminsTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afd48
// GLOBAL: MW2GBL 0x100a23f8
// GLOBAL: MW2PREMERCS 0x100c4608
// GLOBAL: MW23DFX 0x10262e60
// GLOBAL: MW2ATI 0x10197158
// GLOBAL: MW2S3 0x10068028
// GLOBAL: MW2POWERVR 0x101c24d0
MenuControl g_dorcsKeatingControl = {2, 0, &g_dorcsKeatingTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afd70
// GLOBAL: MW2GBL 0x100a2420
// GLOBAL: MW2PREMERCS 0x100c4630
// GLOBAL: MW23DFX 0x10262e88
// GLOBAL: MW2ATI 0x10197180
// GLOBAL: MW2S3 0x10068050
// GLOBAL: MW2POWERVR 0x101c24f8
MenuControl g_dorcsMilesControl = {2, 0, &g_dorcsMilesTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afd98
// GLOBAL: MW2GBL 0x100a2448
// GLOBAL: MW2PREMERCS 0x100c4658
// GLOBAL: MW23DFX 0x10262eb0
// GLOBAL: MW2ATI 0x101971a8
// GLOBAL: MW2S3 0x10068078
// GLOBAL: MW2POWERVR 0x101c2520
MenuControl g_dorcsMortenControl = {2, 0, &g_dorcsMortenTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afdc0
// GLOBAL: MW2GBL 0x100a2470
// GLOBAL: MW2PREMERCS 0x100c4680
// GLOBAL: MW23DFX 0x10262ed8
// GLOBAL: MW2ATI 0x101971d0
// GLOBAL: MW2S3 0x100680a0
// GLOBAL: MW2POWERVR 0x101c2548
MenuControl g_dorcsMortensenControl = {2, 0, &g_dorcsMortensenTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afde8
// GLOBAL: MW2GBL 0x100a2498
// GLOBAL: MW2PREMERCS 0x100c46a8
// GLOBAL: MW23DFX 0x10262f00
// GLOBAL: MW2ATI 0x101971f8
// GLOBAL: MW2S3 0x100680c8
// GLOBAL: MW2POWERVR 0x101c2570
MenuControl g_dorcsPetersonControl = {2, 0, &g_dorcsPetersonTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afe10
// GLOBAL: MW2GBL 0x100a24c0
// GLOBAL: MW2PREMERCS 0x100c46d0
// GLOBAL: MW23DFX 0x10262f28
// GLOBAL: MW2ATI 0x10197220
// GLOBAL: MW2S3 0x100680f0
// GLOBAL: MW2POWERVR 0x101c2598
MenuControl g_dorcsStanfillControl = {2, 0, &g_dorcsStanfillTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afe38
// GLOBAL: MW2GBL 0x100a24e8
// GLOBAL: MW2PREMERCS 0x100c46f8
// GLOBAL: MW23DFX 0x10262f50
// GLOBAL: MW2ATI 0x10197248
// GLOBAL: MW2S3 0x10068118
// GLOBAL: MW2POWERVR 0x101c25c0
MenuControl g_dorcsWhiteControl = {2, 0, &g_dorcsWhiteTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afe60
// GLOBAL: MW2GBL 0x100a2510
// GLOBAL: MW2PREMERCS 0x100c4720
// GLOBAL: MW23DFX 0x10262f78
// GLOBAL: MW2ATI 0x10197270
// GLOBAL: MW2S3 0x10068140
// GLOBAL: MW2POWERVR 0x101c25e8
MenuControl g_dorcsZobelControl = {2, 0, &g_dorcsZobelTextBox, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100afe88
// GLOBAL: MW2GBL 0x100a2538
// GLOBAL: MW2PREMERCS 0x100c4748
// GLOBAL: MW23DFX 0x10262fa0
// GLOBAL: MW2ATI 0x10197298
// GLOBAL: MW2S3 0x10068168
// GLOBAL: MW2POWERVR 0x101c2610
MenuPage g_dorcsAboutPage = {
	0,
	g_dorcsAboutItem,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsAboutControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100affe0
// GLOBAL: MW2GBL 0x100a2690
// GLOBAL: MW2PREMERCS 0x100c48a0
// GLOBAL: MW23DFX 0x102630f8
// GLOBAL: MW2ATI 0x101973f0
// GLOBAL: MW2S3 0x100682c0
// GLOBAL: MW2POWERVR 0x101c2768
MenuPage g_dorcsClarkePage = {
	0,
	g_dorcsClarkeName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsClarkeControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0138
// GLOBAL: MW2GBL 0x100a27e8
// GLOBAL: MW2PREMERCS 0x100c49f8
// GLOBAL: MW23DFX 0x10263250
// GLOBAL: MW2ATI 0x10197548
// GLOBAL: MW2S3 0x10068418
// GLOBAL: MW2POWERVR 0x101c28c0
MenuPage g_dorcsDouglasPage = {
	0,
	g_dorcsDouglasName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsDouglasControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0290
// GLOBAL: MW2GBL 0x100a2940
// GLOBAL: MW2PREMERCS 0x100c4b50
// GLOBAL: MW23DFX 0x102633a8
// GLOBAL: MW2ATI 0x101976a0
// GLOBAL: MW2S3 0x10068570
// GLOBAL: MW2POWERVR 0x101c2a18
MenuPage g_dorcsEthertonPage = {
	0,
	g_dorcsEthertonName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsEthertonControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b03e8
// GLOBAL: MW2GBL 0x100a2a98
// GLOBAL: MW2PREMERCS 0x100c4ca8
// GLOBAL: MW23DFX 0x10263500
// GLOBAL: MW2ATI 0x101977f8
// GLOBAL: MW2S3 0x100686c8
// GLOBAL: MW2POWERVR 0x101c2b70
MenuPage g_dorcsHusebyPage = {
	0,
	g_dorcsHusebyName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsHusebyControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0540
// GLOBAL: MW2GBL 0x100a2bf0
// GLOBAL: MW2PREMERCS 0x100c4e00
// GLOBAL: MW23DFX 0x10263658
// GLOBAL: MW2ATI 0x10197950
// GLOBAL: MW2S3 0x10068820
// GLOBAL: MW2POWERVR 0x101c2cc8
MenuPage g_dorcsKaminsPage = {
	0,
	g_dorcsKaminsName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsKaminsControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0698
// GLOBAL: MW2GBL 0x100a2d48
// GLOBAL: MW2PREMERCS 0x100c4f58
// GLOBAL: MW23DFX 0x102637b0
// GLOBAL: MW2ATI 0x10197aa8
// GLOBAL: MW2S3 0x10068978
// GLOBAL: MW2POWERVR 0x101c2e20
MenuPage g_dorcsKeatingPage = {
	0,
	g_dorcsKeatingName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsKeatingControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b07f0
// GLOBAL: MW2GBL 0x100a2ea0
// GLOBAL: MW2PREMERCS 0x100c50b0
// GLOBAL: MW23DFX 0x10263908
// GLOBAL: MW2ATI 0x10197c00
// GLOBAL: MW2S3 0x10068ad0
// GLOBAL: MW2POWERVR 0x101c2f78
MenuPage g_dorcsMilesPage = {
	0,
	g_dorcsMilesName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsMilesControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0948
// GLOBAL: MW2GBL 0x100a2ff8
// GLOBAL: MW2PREMERCS 0x100c5208
// GLOBAL: MW23DFX 0x10263a60
// GLOBAL: MW2ATI 0x10197d58
// GLOBAL: MW2S3 0x10068c28
// GLOBAL: MW2POWERVR 0x101c30d0
MenuPage g_dorcsMortenPage = {
	0,
	g_dorcsMortenName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsMortenControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0aa0
// GLOBAL: MW2GBL 0x100a3150
// GLOBAL: MW2PREMERCS 0x100c5360
// GLOBAL: MW23DFX 0x10263bb8
// GLOBAL: MW2ATI 0x10197eb0
// GLOBAL: MW2S3 0x10068d80
// GLOBAL: MW2POWERVR 0x101c3228
MenuPage g_dorcsMortensenPage = {
	0,
	g_dorcsMortensenName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsMortensenControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0bf8
// GLOBAL: MW2GBL 0x100a32a8
// GLOBAL: MW2PREMERCS 0x100c54b8
// GLOBAL: MW23DFX 0x10263d10
// GLOBAL: MW2ATI 0x10198008
// GLOBAL: MW2S3 0x10068ed8
// GLOBAL: MW2POWERVR 0x101c3380
MenuPage g_dorcsPetersonPage = {
	0,
	g_dorcsPetersonName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsPetersonControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0d50
// GLOBAL: MW2GBL 0x100a3400
// GLOBAL: MW2PREMERCS 0x100c5610
// GLOBAL: MW23DFX 0x10263e68
// GLOBAL: MW2ATI 0x10198160
// GLOBAL: MW2S3 0x10069030
// GLOBAL: MW2POWERVR 0x101c34d8
MenuPage g_dorcsStanfillPage = {
	0,
	g_dorcsStanfillName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsStanfillControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b0ea8
// GLOBAL: MW2GBL 0x100a3558
// GLOBAL: MW2PREMERCS 0x100c5768
// GLOBAL: MW23DFX 0x10263fc0
// GLOBAL: MW2ATI 0x101982b8
// GLOBAL: MW2S3 0x10069188
// GLOBAL: MW2POWERVR 0x101c3630
MenuPage g_dorcsWhitePage = {
	0,
	g_dorcsWhiteName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsWhiteControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b1000
// GLOBAL: MW2GBL 0x100a36b0
// GLOBAL: MW2PREMERCS 0x100c58c0
// GLOBAL: MW23DFX 0x10264118
// GLOBAL: MW2ATI 0x10198410
// GLOBAL: MW2S3 0x100692e0
// GLOBAL: MW2POWERVR 0x101c3788
MenuPage g_dorcsZobelPage = {
	0,
	g_dorcsZobelName,
	0,
	2,
	0,
	NULL,
	{{3, NULL, RunMenuTextBox, &g_dorcsZobelControl, NULL}, {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b1158
// GLOBAL: MW2GBL 0x100a3808
// GLOBAL: MW2PREMERCS 0x100c5a18
// GLOBAL: MW2MATROX 0x100b0e40
// GLOBAL: MW23DFX 0x10264270
// GLOBAL: MW2ATI 0x10198568
// GLOBAL: MW2S3 0x10069438
// GLOBAL: MW2POWERVR 0x101c38e0
MenuPage g_dorcsPage = {
	0,
	g_dorcsPageTitle,
	0,
	15,
	0,
	NULL,
	{{0, g_dorcsAboutItem, NULL, NULL, &g_dorcsAboutPage},
	 {0, g_dorcsClarkeName, NULL, NULL, &g_dorcsClarkePage},
	 {0, g_dorcsDouglasName, NULL, NULL, &g_dorcsDouglasPage},
	 {0, g_dorcsEthertonName, NULL, NULL, &g_dorcsEthertonPage},
	 {0, g_dorcsHusebyName, NULL, NULL, &g_dorcsHusebyPage},
	 {0, g_dorcsKaminsName, NULL, NULL, &g_dorcsKaminsPage},
	 {0, g_dorcsKeatingName, NULL, NULL, &g_dorcsKeatingPage},
	 {0, g_dorcsMilesName, NULL, NULL, &g_dorcsMilesPage},
	 {0, g_dorcsMortenName, NULL, NULL, &g_dorcsMortenPage},
	 {0, g_dorcsMortensenName, NULL, NULL, &g_dorcsMortensenPage},
	 {0, g_dorcsPetersonName, NULL, NULL, &g_dorcsPetersonPage},
	 {0, g_dorcsStanfillName, NULL, NULL, &g_dorcsStanfillPage},
	 {0, g_dorcsWhiteName, NULL, NULL, &g_dorcsWhitePage},
	 {0, g_dorcsZobelName, NULL, NULL, &g_dorcsZobelPage},
	 {2, g_dorcsExitItem, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100b12b0
// GLOBAL: MW2GBL 0x100a3960
// GLOBAL: MW2PREMERCS 0x100c5b70
// GLOBAL: MW2MATROX 0x100b0f98
// GLOBAL: MW23DFX 0x102643c8
// GLOBAL: MW2ATI 0x101986c0
// GLOBAL: MW2S3 0x10069590
// GLOBAL: MW2POWERVR 0x101c3a38
PANE g_dorcsMenuTarget = {NULL, 0x199a, 0x199a, 0xe666, 0xe666};

// GLOBAL: MW2 0x100b12c8
// GLOBAL: MW2GBL 0x100a3978
// GLOBAL: MW2PREMERCS 0x100c5b88
// GLOBAL: MW2MATROX 0x100b0fb0
// GLOBAL: MW23DFX 0x102643e0
// GLOBAL: MW2ATI 0x101986d8
// GLOBAL: MW2S3 0x100695a8
// GLOBAL: MW2POWERVR 0x101c3a50
PANE g_dorcsMenuBackgroundTarget = {NULL, 0x199a, 0x199a, 0xe666, 0xe666};

// GLOBAL: MW2 0x100b12e0
// GLOBAL: MW2GBL 0x100a3990
// GLOBAL: MW2PREMERCS 0x100c5ba0
// GLOBAL: MW2MATROX 0x100b0fc8
// GLOBAL: MW23DFX 0x102643f8
// GLOBAL: MW2ATI 0x101986f0
// GLOBAL: MW2S3 0x100695c0
// GLOBAL: MW2POWERVR 0x101c3a68
MenuDefinition g_dorcsMenu = {
	&g_dorcsMenuTarget,
	37,
	g_dorcsMenuPageStack,
	0,
	-1,
	NULL,
	&g_dorcsMenuBackgroundTarget,
	-1,
	NULL,
	225,
	219,
	1,
	NULL,
	16,
	1,
	15,
	{0, 0},
	{0, 0},
	{0x51f, 0},
	{0x51f, 0},
	{0xc000, 0},
	&g_dorcsPage
};

// GLOBAL: MW2 0x100b1350
// GLOBAL: MW2GBL 0x100a3a00
// GLOBAL: MW2PREMERCS 0x100c2f50
// GLOBAL: MW2MATROX 0x100b1038
// GLOBAL: MW23DFX 0x102617e8
// GLOBAL: MW2ATI 0x10195ae0
// GLOBAL: MW2S3 0x10069630
// GLOBAL: MW2POWERVR 0x101c0e54
MechS32 g_fledToWindows = 0;

// The frame draw callback ShowDorcs replaces.
// GLOBAL: MW2 0x100b1354
// GLOBAL: MW2GBL 0x100a3a04
// GLOBAL: MW2PREMERCS 0x100c2f54
void (*g_dorcsPreviousDrawCallback)(void) = DrawScene;

// GLOBAL: MW2 0x100c2d00
// GLOBAL: MW2GBL 0x10134ed0
// GLOBAL: MW2PREMERCS 0x1014e590
// GLOBAL: MW2MATROX 0x1012fc50
// GLOBAL: MW23DFX 0x10153510
// GLOBAL: MW2ATI 0x10087530
// GLOBAL: MW2S3 0x100abc20
// GLOBAL: MW2POWERVR 0x1007db10
MenuPage* g_dorcsMenuPageStack[8];

// The dorcs sequence (ShowDorcs): the view shrinks to a point (g_dorcsTransition), a picture
// shows, then another, and a menu.

// GLOBAL: MW2 0x100b1358
// GLOBAL: MW2GBL 0x100a3a08
// GLOBAL: MW2PREMERCS 0x100c2f58
PANE g_dorcsPoint = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100b1370
// GLOBAL: MW2GBL 0x100a3a20
// GLOBAL: MW2PREMERCS 0x100c2f70
PANE g_dorcsRectFrom = {NULL, 0x7d71, 0x7d71, 0x828f, 0x828f};

// GLOBAL: MW2 0x100b1388
// GLOBAL: MW2GBL 0x100a3a38
// GLOBAL: MW2PREMERCS 0x100c2f88
PANE g_dorcsRectTo = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100b13a0
// GLOBAL: MW2GBL 0x100a3a50
// GLOBAL: MW2PREMERCS 0x100c2fa0
PANE g_dorcsRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100b13b8
// GLOBAL: MW2GBL 0x100a3a68
// GLOBAL: MW2PREMERCS 0x100c2fb8
RectTransitionState g_dorcsTransitionState = {0, 0, 0};

// GLOBAL: MW2 0x100b13c8
// GLOBAL: MW2GBL 0x100a3a78
// GLOBAL: MW2PREMERCS 0x100c2fc8
RectTransitionDef g_dorcsTransitionDef = {0xb5, &g_dorcsRectFrom, &g_dorcsRectTo, &g_dorcsRect};

// GLOBAL: MW2 0x100b13d8
// GLOBAL: MW2GBL 0x100a3a88
// GLOBAL: MW2PREMERCS 0x100c2fd8
RectTransition g_dorcsTransition = {&g_dorcsTransitionState, &g_dorcsTransitionDef};

// GLOBAL: MW2 0x100b13e0
// GLOBAL: MW2GBL 0x100a3a90
// GLOBAL: MW2PREMERCS 0x100c2fe0
void* g_dorcsGif = NULL;

// GLOBAL: MW2 0x100b13e4
// GLOBAL: MW2GBL 0x100a3a94
// GLOBAL: MW2PREMERCS 0x100c2fe4
PaletteColor* g_dorcsPalette = NULL;

// GLOBAL: MW2 0x100b13e8
// GLOBAL: MW2GBL 0x100a3a98
// GLOBAL: MW2PREMERCS 0x100c2fe8
MechU8* g_dorcsGifState = NULL;

// GLOBAL: MW2 0x100b13ec
// GLOBAL: MW2GBL 0x100a3a9c
// GLOBAL: MW2PREMERCS 0x100c2fec
MechS32 g_dorcsGifLoaded = 0;

// GLOBAL: MW2 0x100b13f0
// GLOBAL: MW2GBL 0x100a3aa0
// GLOBAL: MW2PREMERCS 0x100c2ff0
MechS32 g_dorcsTime = 0;

// GLOBAL: MW2 0x100b13f4
// GLOBAL: MW2GBL 0x100a3aa4
// GLOBAL: MW2PREMERCS 0x100c2ff4
MechS32 g_dorcsReverse = 1;

// A menu item's action: ejects the local player (1.1's game key 0x3b) without its sound.
// FUNCTION: MW2 0x10073af0
// FUNCTION: MW2GBL 0x100234a0
// FUNCTION: MW2PREMERCS 0x10081b40
// FUNCTION: MW2MATROX 0x100758c0
// FUNCTION: MW23DFX 0x10064280
// FUNCTION: MW2ATI 0x10061930
// FUNCTION: MW2S3 0x1002b1a0
// FUNCTION: MW2POWERVR 0x1005c6a0
void AbortMissionAction(MenuDefinition* p_menu, MenuControl* p_control, MechS32 p_index, Point p_pos, MenuPage* p_page)
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
		saved = g_hostileAtmosphere;
		g_hostileAtmosphere = 0;
		RunGameKey(GAME_KEY_ACTION(0x3b));
		g_hostileAtmosphere = saved;
		FreeMenus();
	}
}

// A menu item's action: ejects the local player's mech.
// FUNCTION: MW2 0x10073ba6
// FUNCTION: MW2GBL 0x10023556
// FUNCTION: MW2PREMERCS 0x10081bf6
// FUNCTION: MW2MATROX 0x10075976
// FUNCTION: MW23DFX 0x100642e0
// FUNCTION: MW2ATI 0x10061990
// FUNCTION: MW2S3 0x1002b200
// FUNCTION: MW2POWERVR 0x1005c700
void FleeToWindowsAction(MenuDefinition* p_menu, MenuControl* p_control, MechS32 p_index, Point p_pos, MenuPage* p_page)
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
		g_fledToWindows = 1;
		g_difficulty->m_invulnerable = 0;
		player = g_players[g_localPlayerId];
		EjectPlayer(player->m_mech, 0);
		FreeMenus();
	}
}

// Reads vfx/<p_name>.bin whole.
// FUNCTION: MW2 0x10073c62
// FUNCTION: MW2GBL 0x10023612
// FUNCTION: MW2PREMERCS 0x10081cb2
void* ReadVfxBin(MechChar* p_name)
{
	void* data;
	MechChar path[256];

	sprintf(path, "%s/%s.%s", "vfx", p_name, "bin");
	data = FILE_read(path, NULL);
	return data;
}

// FUNCTION: MW2 0x10073cb5
// FUNCTION: MW2GBL 0x10023665
// FUNCTION: MW2PREMERCS 0x10081d05
// FUNCTION: MW2MATROX 0x10075a32
// FUNCTION: MW23DFX 0x10064350
void CloseInGameMenus(void)
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
// GLOBAL: MW2GBL 0x100b917c
// GLOBAL: MW2PREMERCS 0x100cc944
MechS32 g_dorcsState;

// GLOBAL: MW2 0x100bf058
// GLOBAL: MW2GBL 0x100b9150
// GLOBAL: MW2PREMERCS 0x100cc918
PANE g_dorcsSavedTarget;

// GLOBAL: MW2 0x100bf070
// GLOBAL: MW2GBL 0x100b9168
// GLOBAL: MW2PREMERCS 0x100cc930
PANE g_dorcsGifTarget;

// GLOBAL: MW2 0x100c2cec
// GLOBAL: MW2GBL 0x10134ec0
// GLOBAL: MW2PREMERCS 0x1014e5b0
PaletteColor g_dorcsBlack;

// The dorcs sequence's frame draw callback: steps the sequence (g_dorcsState) each frame.
// FUNCTION: MW2 0x10073cfc
// FUNCTION: MW2GBL 0x100236ac
// FUNCTION: MW2PREMERCS 0x10081d4c
void UpdateDorcs(void)
{
	PANE* rect;
	MechS32 index;
	PANE saved;
	PANE* target;
	MechS32 i;

	switch (g_dorcsState) {
	case 0:
		g_dorcsSavedTarget = g_currentPane;
		g_dorcsGifLoaded = 0;
		g_dorcsTransition.m_def->m_first = &g_dorcsRectFrom;
		if (!g_dorcsTransition.m_def->m_first->m_window) {
			target = g_dorcsTransition.m_def->m_first;
			ScaleRectToScreen(&g_mainPixelBuffer, target, target);
			target->m_window = &g_mainPixelBuffer;
			target = g_dorcsTransition.m_def->m_second;
			ScaleRectToScreen(&g_mainPixelBuffer, target, target);
			target->m_window = &g_mainPixelBuffer;
			target = g_dorcsTransition.m_def->m_out;
			target->m_window = &g_mainPixelBuffer;
			target = &g_dorcsPoint;
			target->m_window = &g_mainPixelBuffer;
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
		g_savedShowHud = g_showHud;
		g_showHud = 0;
		CloseInGameMenus();
		rect = UpdateRectTransition(g_dorcsReverse, &g_dorcsTransition);
		if (rect) {
			saved = g_panes[g_paneIndex];
			g_panes[g_paneIndex] = *rect;
			index = g_paneIndex;
			g_paneIndex = -1;
			SelectPane(index);
			if (g_dorcsPreviousDrawCallback) {
				g_dorcsPreviousDrawCallback();
			}

			g_stretchPending = 1;
			g_panes[g_paneIndex] = saved;
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
		g_showHud = 0;
		CloseInGameMenus();
		rect = UpdateRectTransitionByAxis(1, &g_dorcsTransition);
		if (rect) {
			saved = g_panes[g_paneIndex];
			g_panes[g_paneIndex] = *rect;
			index = g_paneIndex;
			g_paneIndex = -1;
			SelectPane(index);
			VFX_pane_wipe(&g_dorcsSavedTarget, 0);
			if (g_dorcsPreviousDrawCallback) {
				g_dorcsPreviousDrawCallback();
			}

			OutlinePane(&g_currentPane, 10);
			g_panes[g_paneIndex] = saved;
			g_currentPane = g_dorcsSavedTarget;
			break;
		}
		else {
			index = g_paneIndex;
			g_paneIndex = -1;
			SelectPane(index);
			g_dorcsState = 4;
		}
	case 4:
		g_showHud = 0;
		CloseInGameMenus();
		g_dorcsGifTarget = g_currentPane;
		g_dorcsGifState = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x502e);
		if (g_dorcsGifState) {
			g_dorcsPalette =
				HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x100 * sizeof(PaletteColor));
			if (g_dorcsPalette) {
				g_currentDisplayBackend->m_setPalette(0, 0x100, g_dorcsPalette, 1);
				g_dorcsGif = ReadVfxBin("vfxjk");
				if (g_dorcsGif) {
					FitRectToGif(&g_dorcsGifTarget, &g_dorcsGifTarget, g_dorcsGif);
					VFX_pane_wipe(&g_currentPane, 0);
					VFX_GIF_draw(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
					if (g_windowActive) {
						g_currentRefreshMode->m_flip();
					}

					VFX_GIF_palette(g_dorcsGif, (MechU8*) g_dorcsPalette);
					g_currentDisplayBackend->m_blendPalettes(g_dorcsPalette, 0xb5);
					g_dorcsGifLoaded = 1;
				}
			}
		}

		g_dorcsTime = g_currentClock + 0x10f;
		g_dorcsState = 5;
		break;
	case 5:
		g_showHud = 0;
		CloseInGameMenus();
		if (g_currentClock < g_dorcsTime) {
			if (g_dorcsGifLoaded) {
				VFX_GIF_draw(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
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
					g_dorcsPalette[i] = g_dorcsBlack;
				}

				g_currentDisplayBackend->m_blendPalettes(g_dorcsPalette, 0x5a);
				VFX_pane_wipe(&g_currentPane, 0);
				VFX_GIF_draw(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
				if (g_windowActive) {
					g_currentRefreshMode->m_flip();
				}

				VFX_GIF_palette(g_dorcsGif, (MechU8*) g_dorcsPalette);
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
		g_showHud = 0;
		CloseInGameMenus();
		if (g_currentClock < g_dorcsTime) {
			if (g_dorcsGifLoaded) {
				VFX_GIF_draw(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
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
		VFX_pane_wipe(&g_currentPane, 0);
		if (g_dorcsGifLoaded) {
			VFX_GIF_draw(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
		}

		if (!GetOpenMenu()) {
			g_dorcsTime = g_currentClock + 0x10f;
			g_dorcsState = -1;
		}
		break;
	case -1:
		g_showHud = 0;
		CloseInGameMenus();
		if (g_currentClock < g_dorcsTime) {
			VFX_pane_wipe(&g_currentPane, 0);
			if (g_dorcsGifLoaded) {
				VFX_GIF_draw(&g_dorcsGifTarget, g_dorcsGif, g_dorcsGifState);
			}
		}
		else {
			VFX_pane_wipe(&g_currentPane, 0);
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
			g_renderSettings.m_frameDrawCallback = g_dorcsPreviousDrawCallback;
			g_savedShowHud = 1;
			g_stretchPending = 1;
			g_dorcsState = 0;
		}
		break;
	}
}

// Starts the dorcs sequence: UpdateDorcs draws the frames in place of the frame draw callback.
// FUNCTION: MW2 0x100745b2
// FUNCTION: MW2GBL 0x10023f62
// FUNCTION: MW2PREMERCS 0x10082602
void ShowDorcs(void)
{
	g_dorcsPreviousDrawCallback = g_renderSettings.m_frameDrawCallback;
	g_renderSettings.m_frameDrawCallback = UpdateDorcs;
	g_dorcsState = 0;
}
