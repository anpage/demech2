/* The in-mission menu's audio page: the music, sound effects and voice volumes. A data-only
   object: its data follows unk10033280.c's. */
#include "audiomenu.h"

#include "audio.h"
#include "mainmenu.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "types.h"

#include <stddef.h>

// GLOBAL: MW2 0x100a5288
MechChar g_unk0x100a5288[] = "Audio Ctrl";

// GLOBAL: MW2 0x100a5298
MechChar g_unk0x100a5298[] = "SET AUDIO VOLUME";

// GLOBAL: MW2 0x100a52b0
MechChar g_unk0x100a52b0[] = "Betty Message";

// GLOBAL: MW2 0x100a52c0
MechChar g_unk0x100a52c0[] = "Sound Effects";

// GLOBAL: MW2 0x100a52d0
MechChar g_unk0x100a52d0[] = "Voice";

// GLOBAL: MW2 0x100a52d8
MechChar g_unk0x100a52d8[] = "Music";

// GLOBAL: MW2 0x100a52e0
MenuControl g_unk0x100a52e0 = {2, 0, g_unk0x100a1cc0, 0, NULL, FUN_10006760, FUN_10006845, FUN_100069c9, FUN_10006b3a};

// GLOBAL: MW2 0x100a5308
MenuControl g_unk0x100a5308 = {2, 0, g_unk0x100a1cc0, 1, NULL, FUN_10006760, FUN_10006845, FUN_100069c9, FUN_10006b3a};

// GLOBAL: MW2 0x100a5330
MenuControl g_unk0x100a5330 = {2, 0, g_unk0x100a1cc0, 2, NULL, FUN_10006760, FUN_10006845, FUN_100069c9, FUN_10006b3a};

// GLOBAL: MW2 0x100a5358
MenuPage g_audioPage = {
	0,
	g_unk0x100a5298,
	0,
	4,
	0,
	NULL,
	{{1, g_unk0x100a52d8, RunMenuSlider, &g_unk0x100a5330, NULL},
	 {1, g_unk0x100a52c0, RunMenuSlider, &g_unk0x100a52e0, NULL},
	 {1, g_unk0x100a52d0, RunMenuSlider, &g_unk0x100a5308, NULL},
	 {2, g_unk0x100a1ad8, NULL, NULL, NULL}}
};
