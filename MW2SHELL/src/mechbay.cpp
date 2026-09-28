#include "mechbay.h"

#include "audiosample.h"
#include "buttonmenu.h"
#include "customstar.h"
#include "debugprint.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "mainmenubutton.h"
#include "mechchassis.h"
#include "mechvariant.h"
#include "menudata.h"
#include "menuscreen.h"
#include "missionui.h"
#include "mousestate.h"
#include "options.h"
#include "projectarchive.h"
#include "refreshmode.h"
#include "screenfield.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "textglyph.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk1003bf90.h"
#include "video.h"
#include "videodriver.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The mech bay: the variant being edited, its engine, weapons and armor, and the fields that
// show them.

// An engine: its rating, its weight (in 1/100 t) and its maker. Ids from 10000 up are XL
// engines at half the weight.
// SIZE 0x0c
struct EngineType {
	MechS32 m_rating; // 0x00
	MechS32 m_weight; // 0x04
	MechChar* m_name; // 0x08
};

// A weapon; the weight (in 1/100 t) is at 0x18. The table holds negative values in the fields
// up to 0x14, so those are signed.
// SIZE 0x28
struct Weapon {
	MechS32 m_unk0x00;    // 0x00
	MechS32 m_unk0x04;    // 0x04
	MechS32 m_unk0x08;    // 0x08
	MechS32 m_unk0x0c;    // 0x0c
	MechS32 m_unk0x10;    // 0x10
	MechS32 m_unk0x14;    // 0x14
	MechS32 m_unk0x18;    // 0x18
	undefined4 m_unk0x1c; // 0x1c
	MechS32 m_unk0x20;    // 0x20 — rounds per ton of ammo, 0 for weapons without ammo
	MechChar* m_name;     // 0x24
};

// SIZE 0x10
// The internal structure of a weight class (20 to 110 tons in steps of 5) per location.
struct InternalStructure {
	MechS32 m_unk0x00; // 0x00 — center torso
	MechS32 m_unk0x04; // 0x04 — side torsos
	MechS32 m_unk0x08; // 0x08 — arms
	MechS32 m_unk0x0c; // 0x0c — legs
};

// SIZE 0x08
// A piece of equipment other than a weapon: ids come in steps of 50 from 5000.
struct Equipment {
	MechS32 m_unk0x00; // 0x00 — id
	MechChar* m_name;  // 0x04
};

// SIZE 0x7a8
// The variant being edited. 0x10007e90 copies an m_unk0x4a0 entry through the struct base
// (0x1005c640 + 0x4a0), which places the start; FUN_1000befe copies the whole struct to the
// backup at 0x1005cde8 (0x7a8 bytes further), which bounds it.
struct MekVariant {
	// SIZE 0x08
	struct Slot {
		MechS32 m_unk0x00; // 0x00 — item id, -1 when free
		MechS32 m_unk0x04; // 0x04
	};

	// SIZE 0x10
	// One location's armor. The tab callbacks index the array through its base at 0x710.
	struct Armor {
		MechS32 m_unk0x00; // 0x00 — internal structure
		MechS32 m_unk0x04; // 0x04
		MechS32 m_unk0x08; // 0x08 — front armor
		MechS32 m_unk0x0c; // 0x0c — rear armor, negative when the location has none
	};

	MechChar m_unk0x00[0x100];           // 0x00 — '~' and the mech's name
	MechChar m_unk0x100[0x40];           // 0x100 — variant name
	undefined m_unk0x140[0x200 - 0x140]; // 0x140
	MechS32 m_unk0x200;                  // 0x200 — maximum weight
	MechS32 m_unk0x204;                  // 0x204 — total weight
	MechS32 m_unk0x208;                  // 0x208 — weight left
	MechS32 m_unk0x20c;                  // 0x20c — engine id
	MechS32 m_unk0x210;                  // 0x210 — engine rating
	MechS32 m_unk0x214;                  // 0x214 — engine weight
	MechS32 m_unk0x218;                  // 0x218
	MechS32 m_unk0x21c;                  // 0x21c
	MechS32 m_unk0x220;                  // 0x220
	MechS32 m_unk0x224;                  // 0x224
	MechS32 m_unk0x228;                  // 0x228
	MechS32 m_unk0x22c;                  // 0x22c
	MechS32 m_unk0x230;                  // 0x230
	MechS32 m_unk0x234;                  // 0x234
	MechS32 m_unk0x238;                  // 0x238
	undefined4 m_unk0x23c;               // 0x23c
	MechS32 m_unk0x240;                  // 0x240
	MechS32 m_unk0x244;                  // 0x244
	undefined4 m_unk0x248;               // 0x248
	MechS32 m_unk0x24c;                  // 0x24c
	MechS32 m_unk0x250;                  // 0x250
	undefined4 m_unk0x254;               // 0x254
	MechS32 m_unk0x258;                  // 0x258
	MechS32 m_unk0x25c;                  // 0x25c
	MechS32 m_unk0x260;                  // 0x260
	undefined4 m_unk0x264;               // 0x264
	MechS32 m_unk0x268;                  // 0x268
	undefined4 m_unk0x26c;               // 0x26c
	MechS32 m_unk0x270;                  // 0x270
	undefined4 m_unk0x274;               // 0x274
	MechS32 m_unk0x278;                  // 0x278
	MechS32 m_unk0x27c;                  // 0x27c
	MechS32 m_unk0x280;                  // 0x280
	MechS32 m_unk0x284[25];              // 0x284 — ids, -1 past the last
	MechS32 m_unk0x2e8[10];              // 0x2e8 — ids, -1 past the last
	MechS32 m_unk0x310;                  // 0x310 — highlighted item id, -1 for none
	MechS32 m_unk0x314;                  // 0x314 — selected location
	MechS32 m_unk0x318[8][12];           // 0x318 — item ids per location and slot, 0 when free
	undefined4 m_unk0x498;               // 0x498
	MechS32 m_unk0x49c;                  // 0x49c — used entries of m_unk0x4a0
	Slot m_unk0x4a0[78];                 // 0x4a0
	Armor m_unk0x710[8];                 // 0x710
	MechS32 m_unk0x790;                  // 0x790
	MechS32 m_unk0x794;                  // 0x794
	MechS32 m_unk0x798;                  // 0x798
	MechS32 m_unk0x79c;                  // 0x79c
	MechS32 m_unk0x7a0;                  // 0x7a0
	MechS32 m_unk0x7a4;                  // 0x7a4 — items 5001 and up to add or remove together
};

// SIZE 0x28
// One location's critical slots in the image of a .mek file.
struct MekCriticalSlots {
	MechS32 m_unk0x00;     // 0x00 — front armor
	MechS32 m_unk0x04;     // 0x04 — rear armor
	MechS32 m_unk0x08;     // 0x08 — internal structure
	MechU16 m_unk0x0c[12]; // 0x0c — item ids
	MechS16 m_unk0x24;     // 0x24 — slots in use
	undefined2 m_unk0x26;  // 0x26
};

// SIZE 0x18
// The header of a .mek file.
struct MekHeader {
	MechS32 m_unk0x00; // 0x00 — maximum weight in tons
	MechS32 m_unk0x04; // 0x04
	MechS32 m_unk0x08; // 0x08
	MechS32 m_unk0x0c; // 0x0c
	MechS32 m_unk0x10; // 0x10 — weapons
	MechS32 m_unk0x14; // 0x14 — ammunition entries
};

DECOMP_SIZE_ASSERT(ScreenField, 0x2c)
DECOMP_SIZE_ASSERT(EngineType, 0x0c)
DECOMP_SIZE_ASSERT(Weapon, 0x28)
DECOMP_SIZE_ASSERT(MekVariant::Slot, 0x08)
DECOMP_SIZE_ASSERT(MekVariant::Armor, 0x10)
DECOMP_SIZE_ASSERT(MekVariant, 0x7a8)
DECOMP_SIZE_ASSERT(MechChassis, 0x18)
DECOMP_SIZE_ASSERT(MekCriticalSlots, 0x28)
DECOMP_SIZE_ASSERT(Equipment, 0x08)
DECOMP_SIZE_ASSERT(InternalStructure, 0x10)
DECOMP_SIZE_ASSERT(MekHeader, 0x18)

MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechU32 p_unk0x10,
	MechU32 p_unk0x14
);

// GLOBAL: MW2SHELL 0x1005c438
Equipment g_unk0x1005c438[23] = {
	{5000, "MASC"},
	{5050, "Targeting Computer"},
	{5100, "ECM"},
	{5150, "Artemis IV"},
	{5200, "Beagle Active Probe"},
	{5250, "TAG"},
	{5300, "Shoulder"},
	{5350, "Upper Arm Actuator"},
	{5400, "Lower Arm Actuator"},
	{5450, "Hand Actuator"},
	{5500, "Hip"},
	{5550, "Upper Leg Actuator"},
	{5600, "Lower Leg Actuator"},
	{5650, "Foot Actuator"},
	{5700, "Sensors"},
	{5750, "Cockpit"},
	{5800, "Gyro"},
	{5850, "Engine"},
	{5900, "Life Support"},
	{6000, "Heat Sink"},
	{7000, "Jump Jet"},
	{8000, "Endo Steel"},
	{9000, "Ferro-Fibrous"},
};

// GLOBAL: MW2SHELL 0x1005c4f0
MechChar* g_unk0x1005c4f0[8] =
	{"Head", "Right Torso", "Center Torso", "Left Torso", "Right Arm", "Left Arm", "Right Leg", "Left Leg"};

// GLOBAL: MW2SHELL 0x1005c510
InternalStructure g_unk0x1005c510[19] = {
	{4, 3, 1, 2},     {5, 4, 2, 3},     {6, 5, 3, 4},     {8, 6, 4, 6},     {10, 7, 5, 7},
	{11, 8, 6, 8},    {12, 10, 6, 10},  {14, 11, 7, 11},  {16, 12, 8, 12},  {18, 13, 9, 13},
	{20, 14, 10, 14}, {21, 15, 10, 15}, {22, 15, 11, 15}, {23, 16, 12, 16}, {25, 17, 13, 17},
	{27, 18, 14, 18}, {29, 19, 15, 19}, {30, 20, 16, 20}, {31, 21, 17, 21},
};

// GLOBAL: MW2SHELL 0x1005c640
MekVariant g_unk0x1005c640 = {0};

// The variant as loaded, before any change.
// GLOBAL: MW2SHELL 0x1005cde8
MekVariant g_unk0x1005cde8 = {0};

// GLOBAL: MW2SHELL 0x1005d590
EngineType g_unk0x1005d590[] = {
	{10, 50, "Omni"},      {15, 50, "GM"},          {20, 50, "Pitban"},    {25, 50, "Omni"},
	{30, 100, "Nissan"},   {35, 100, "VOX"},        {40, 100, "GM"},       {45, 100, "GM"},
	{50, 150, "DAV"},      {55, 150, "VOX"},        {60, 150, "Leenex"},   {65, 200, "Nissan"},
	{70, 200, "Omni"},     {75, 200, "GM"},         {80, 250, "VOX"},      {85, 250, "DAV"},
	{90, 300, "DAV"},      {95, 300, "Nissan"},     {100, 300, "Hermes"},  {105, 350, "DAV"},
	{110, 350, "GM"},      {115, 400, "GM"},        {120, 400, "GM"},      {125, 400, "Vlar"},
	{130, 450, "Magna"},   {135, 450, "Hermes"},    {140, 500, "Leenex"},  {145, 500, "Omni"},
	{150, 550, "GM"},      {155, 550, "GM"},        {160, 600, "LTV"},     {165, 600, "VOX"},
	{170, 600, "DAV"},     {175, 700, "Omni"},      {180, 700, "GM"},      {185, 750, "GM"},
	{190, 750, "DAV"},     {195, 800, "Nissan"},    {200, 850, "Nissan"},  {205, 850, "Vlar"},
	{210, 900, "GM"},      {215, 850, "Core Tex"},  {220, 1000, "DAV"},    {225, 1000, "VOX"},
	{230, 1050, "Leenex"}, {235, 1100, "GM"},       {240, 1150, "Pitban"}, {245, 1200, "Magna"},
	{250, 1250, "Magna"},  {255, 1300, "Strand"},   {260, 1350, "Magna"},  {265, 1400, "Vlar"},
	{270, 1450, "GM"},     {275, 1550, "Core Tex"}, {280, 1600, "VOX"},    {285, 1650, "Pitban"},
	{290, 1750, "Omni"},   {295, 1800, "GM"},       {300, 1900, "Vlar"},   {305, 1950, "GM"},
	{310, 2050, "Magna"},  {315, 2150, "GM"},       {320, 2250, "Pitban"}, {325, 2350, "VOX"},
	{330, 2400, "VOX"},    {335, 2550, "Leenex"},   {340, 2700, "VOX"},    {345, 2850, "Vlar"},
	{350, 2950, "Magna"},  {355, 3150, "LTV"},      {360, 3300, "Hermes"}, {365, 3450, "Hermes"},
	{370, 3650, "Magna"},  {375, 3850, "GM"},       {380, 4100, "GM"},     {385, 4350, "LTV"},
	{390, 4600, "Magna"},  {395, 4900, "Hermes"},   {400, 5250, "LTV"},    {-1, 0, NULL},
};

// GLOBAL: MW2SHELL 0x1005d950
Weapon g_unk0x1005d950[] = {
	{6, -1, -1, 7, 14, 1000, 500, 4, 6, "LRM 20"},
	{5, -1, -1, 7, 14, 1000, 350, 2, 8, "LRM 15"},
	{4, -1, -1, 7, 14, 1000, 250, 1, 12, "LRM 10"},
	{2, -1, -1, 7, 14, 1000, 100, 1, 24, "LRM 5"},
	{4, -2, -1, 3, 6, 497, 150, 1, 15, "SRM 6"},
	{3, -2, -1, 3, 6, 497, 100, 1, 25, "SRM 4"},
	{2, -2, -1, 3, 6, 497, 50, 1, 50, "SRM 2"},
	{4, -2, -1, 4, 8, 497, 300, 2, 15, "Streak SRM-6"},
	{3, -2, -1, 4, 8, 497, 200, 1, 25, "Streak SRM-4"},
	{2, -2, -1, 4, 8, 497, 100, 1, 50, "Streak SRM-2"},
	{0, 2, -1, 1, 2, 175, 25, 1, 200, "Machine Gun"},
	{1, 15, 2, 7, 15, 1820, 1200, 6, 8, "Gauss Rifle"},
	{1, 2, 4, 10, 20, 800, 500, 3, 45, "LB 2-X AC"},
	{1, 5, 3, 8, 15, 700, 700, 4, 20, "LB 5-X AC"},
	{2, 10, -1, 6, 12, 600, 1000, 5, 10, "LB 10-X AC"},
	{6, 20, -1, 4, 8, 450, 1200, 9, 5, "LB 20-X AC"},
	{1, 2, 2, 9, 18, 700, 500, 2, 45, "Ultra AC/2"},
	{1, 5, -1, 7, 14, 600, 700, 3, 20, "Ultra AC/5"},
	{3, 10, -1, 6, 12, 500, 1000, 4, 10, "Ultra AC/10"},
	{7, 20, -1, 4, 8, 400, 1200, 8, 5, "Ultra AC/20"},
	{3, 2, -1, 1, 2, 3, 50, 1, 0, "Flamer"},
	{15, 15, -1, 7, 14, 746, 600, 2, 0, "ER PPC"},
	{12, 10, -1, 8, 15, 1019, 400, 1, 0, "ER Laser (Large)"},
	{5, 7, -1, 5, 10, 510, 100, 1, 0, "ER Laser (Medium)"},
	{2, 5, -1, 2, 4, 255, 50, 1, 0, "ER Laser (Small)"},
	{10, 10, -1, 6, 14, 815, 600, 2, 0, "Pulse Laser (Large)"},
	{4, 7, -1, 4, 8, 408, 200, 1, 0, "Pulse Laser (Medium)"},
	{2, 3, -1, 2, 4, 204, 100, 1, 0, "Pulse Laser (Small)"},
	{0, 0, -1, 4, 8, 12, 200, 1, 0, "Narc Missile Beacon"},
	{1, 0, -1, -1, -1, -1, 50, 1, 0, "Anti-Missile System"},
	{0, 0, -1, -1, -1, -1, 0, 1, 0, "Nuke"},
};

// GLOBAL: MW2SHELL 0x1005de28
ScreenField* g_unk0x1005de28 = NULL;

// GLOBAL: MW2SHELL 0x1005de2c
ScreenField* g_unk0x1005de2c = NULL;

// GLOBAL: MW2SHELL 0x1005de30
MechS32 g_unk0x1005de30 = 0;

// GLOBAL: MW2SHELL 0x1005de38
MechS32 g_unk0x1005de38[8] = {75, 46, 86, 126, 10, 162, 39, 109};

// GLOBAL: MW2SHELL 0x1005de58
MechS32 g_unk0x1005de58[8] = {171, 206, 206, 206, 204, 204, 310, 310};

// GLOBAL: MW2SHELL 0x1005de78
AudioSample* g_unk0x1005de78 = NULL;

// GLOBAL: MW2SHELL 0x10061560
MechChassis g_unk0x10061560[] = {
	{"ds", "frm", "firemoth", "Firemoth", 20, 85},
	{"kf", "ktf", "kitfox", "Kit Fox", 30, 89},
	{"jn", "jnr", "jenner", "Jenner IIC", 35, 88},
	{"bh", "nva", "nova", "Nova", 50, 92},
	{"sc", "stm", "strmcrow", "Stormcrow", 55, 94},
	{"md", "mdg", "maddog", "Mad Dog", 60, 90},
	{"lo", "hlb", "hellbrgr", "Hellbringer", 65, 87},
	{"rf", "rfl", "rifleman", "Rifleman IIC", 65, 93},
	{"su", "smn", "summoner", "Summoner", 70, 95},
	{"mc", "tbr", "timbrwlf", "Timber Wolf", 75, 96},
	{"mw", "grg", "gargoyle", "Gargoyle", 80, 86},
	{"wh", "whm", "warhammr", "Warhammer IIC", 80, 97},
	{"mr", "mrd", "marauder", "Marauder IIC ", 85, 91},
	{"ms", "whk", "warhawk", "Warhawk", 85, 98},
	{"da", "drw", "direwolf", "Dire Wolf", 100, 83},
	{"el", "ele", "elementl", "Elemental", 100, 84},
	{"ta", "tar", "tarantul", "Tarantula", 100, -1},
	{"bm", "btm", "btllmstr", "Battle Master IIC", 100, 99},
	{NULL, NULL, NULL, NULL, 0, 0},
};

// GLOBAL: MW2SHELL 0x10061728
MechS32 g_unk0x10061728 = 15;

// GLOBAL: MW2SHELL 0x10061730
MechChar g_unk0x10061730[] = "awomp%s";

// GLOBAL: MW2SHELL 0x10061738
MechChar g_unk0x10061738[] = "ajfmp%s";

// GLOBAL: MW2SHELL 0x10061740
MechChar g_unk0x10061740[] = "aiamp%s";

// GLOBAL: MW2SHELL 0x10061748
MechChar* g_unk0x10061748 = g_unk0x10061730;

// GLOBAL: MW2SHELL 0x1006176c
MechS32 g_unk0x1006176c = 7;

// GLOBAL: MW2SHELL 0x10061770
AudioSample* g_unk0x10061770 = NULL;

// GLOBAL: MW2SHELL 0x10061774
MechS32 g_unk0x10061774 = 0;

// GLOBAL: MW2SHELL 0x10061778
ButtonMenu* g_unk0x10061778 = NULL;

// GLOBAL: MW2SHELL 0x1006177c
AudioSample* g_unk0x1006177c = NULL;

// The wParam the screen was opened with.
// GLOBAL: MW2SHELL 0x10061780
MechS32 g_unk0x10061780 = 0x404;

// Set once the bay's and the customize screen's quick tips have been shown.
// GLOBAL: MW2SHELL 0x10061784
MechS32 g_unk0x10061784 = 0;

// GLOBAL: MW2SHELL 0x10061788
MechS32 g_unk0x10061788 = 0;

// GLOBAL: MW2SHELL 0x1006178c
MechS32 g_unk0x1006178c = 1;

// GLOBAL: MW2SHELL 0x10079a98
undefined4 g_unk0x10079a98;

// GLOBAL: MW2SHELL 0x10079a9c
undefined4 g_unk0x10079a9c;

// GLOBAL: MW2SHELL 0x10079aa0
MechS32 g_unk0x10079aa0;

// GLOBAL: MW2SHELL 0x10079aa8
MechChar g_unk0x10079aa8[0x20];

// GLOBAL: MW2SHELL 0x10079ac8
AudioSample* g_unk0x10079ac8;

// The pilot's callsign and the star's maximum weight.
// GLOBAL: MW2SHELL 0x10079ad0
MechChar g_unk0x10079ad0[0x80];

// GLOBAL: MW2SHELL 0x10079b50
MechChar g_szTempBuffer[0x100];

// The image of a .mek file, besides g_unk0x1007c820: its header, the ammunition (id and
// rounds), the weapons (id and -1) and the variant name.
// GLOBAL: MW2SHELL 0x10079c50
MekVariant::Slot g_unk0x10079c50[25];

// GLOBAL: MW2SHELL 0x10079d18
AudioSample* g_unk0x10079d18;

// GLOBAL: MW2SHELL 0x10079d20
MekHeader g_unk0x10079d20;

// GLOBAL: MW2SHELL 0x10079d38
MechChar g_unk0x10079d38[0x10];

// GLOBAL: MW2SHELL 0x10079d48
MechChar g_unk0x10079d48[0x32];

// GLOBAL: MW2SHELL 0x10079d80
MechChar g_unk0x10079d80[200][13];

// GLOBAL: MW2SHELL 0x1007a7a8
MekVariant::Slot g_unk0x1007a7a8[10];

// GLOBAL: MW2SHELL 0x1007a7f8
MechS32 g_unk0x1007a7f8;

// GLOBAL: MW2SHELL 0x1007a800
MechChar g_unk0x1007a800[0x20];

// GLOBAL: MW2SHELL 0x1007a820
undefined g_unk0x1007a820[0x800];

// GLOBAL: MW2SHELL 0x1007c820
MekCriticalSlots g_unk0x1007c820[8];

// GLOBAL: MW2SHELL 0x1007c960
undefined g_unk0x1007c960[0x100];

// GLOBAL: MW2SHELL 0x1007ca60
undefined g_unk0x1007ca60[0x100];

// The path of the .mek file being saved.
// GLOBAL: MW2SHELL 0x1007cb60
undefined g_unk0x1007cb60[0x100];

// GLOBAL: MW2SHELL 0x1007cc60
MechChar g_unk0x1007cc60[0x20];

// GLOBAL: MW2SHELL 0x1007cc80
MechS32 g_unk0x1007cc80;

// FUNCTION: MW2SHELL 0x10007850
MechS32 FUN_10007850(MechS32 p_value)
{
	MechS32 remainder;

	remainder = p_value % 100;
	return remainder ? p_value + 100 - remainder : p_value;
}

// FUNCTION: MW2SHELL 0x1000788c
MechS32 FUN_1000788c(MechS32 p_value)
{
	return p_value - p_value % 100;
}

// FUNCTION: MW2SHELL 0x100078ae
MechS32 FUN_100078ae(MechS32 p_value)
{
	return FUN_1000788c(p_value + 50);
}

// Stack-slot permutation: top and height.
// FUNCTION: MW2SHELL 0x100078cd
void FUN_100078cd(ScreenField* p_tabs)
{
	MechS32 top;
	MechS32 height;

	top = 0;
	height = 0;
	if (p_tabs == NULL) {
		return;
	}

	while (p_tabs->m_left != -1) {
		if (p_tabs->m_top < 0) {
			p_tabs->m_top = ((p_tabs->m_top & 0xff0) >> 4) * height + (p_tabs->m_top & 0xf) + top;
		}

		if (p_tabs->m_unk0x1c != NULL) {
			p_tabs->m_glyph = p_tabs->m_unk0x1c(p_tabs);
		}
		else {
			p_tabs->m_glyph = NULL;
		}

		if (p_tabs->m_glyph != NULL) {
			if (p_tabs->m_width == -1) {
				p_tabs->m_width = p_tabs->m_glyph->m_width;
			}
			if (p_tabs->m_height == -1) {
				p_tabs->m_height = p_tabs->m_glyph->m_height;
			}
		}

		if (p_tabs->m_width == -1) {
			p_tabs->m_width = 0;
		}
		if (p_tabs->m_height == -1) {
			p_tabs->m_height = height;
		}

		top = p_tabs->m_top;
		height = p_tabs->m_height;
		p_tabs++;
	}
}

// FUNCTION: MW2SHELL 0x100079f8
void FUN_100079f8(ScreenField* p_tabs)
{
	ScreenField* tab;

	if (p_tabs == NULL) {
		return;
	}

	for (tab = p_tabs; tab->m_left != -1; tab++) {
		if (tab->m_glyph != NULL) {
			delete tab->m_glyph;
			tab->m_glyph = NULL;
		}
	}

	for (tab = p_tabs; tab->m_left != -1; tab++) {
		if (tab->m_unk0x1c != NULL) {
			tab->m_glyph = tab->m_unk0x1c(tab);
		}
	}
}

// FUNCTION: MW2SHELL 0x10007ac8
void FUN_10007ac8(ScreenField* p_tabs)
{
	if (p_tabs == NULL) {
		return;
	}

	while (p_tabs->m_left != -1) {
		if (p_tabs->m_glyph != NULL) {
			delete p_tabs->m_glyph;
			p_tabs->m_glyph = NULL;
		}

		p_tabs++;
	}
}

// The original loads the index j before i in m_unk0x318[i][j]; declaring j first didn't flip it.
// FUNCTION: MW2SHELL 0x10007b4d
MechS32 FUN_10007b4d(MechS32 p_id)
{
	MechS32 count;
	MechS32 j;
	MechS32 i;

	count = 0;
	if (p_id <= 0) {
		return count;
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 12; j++) {
			if (g_unk0x1005c640.m_unk0x318[i][j] == p_id) {
				g_unk0x1005c640.m_unk0x318[i][j] = 0;
				count++;
			}
		}
	}

	return count;
}

// FUNCTION: MW2SHELL 0x10007bee
MechS32 FUN_10007bee(MechS32 p_location, MechS32 p_id, MechS32 p_count)
{
	MechS32 i;

	if (p_id <= 0) {
		return FALSE;
	}

	for (i = 0; p_count && i < 12; i++) {
		if (g_unk0x1005c640.m_unk0x318[p_location][i] == 0) {
			g_unk0x1005c640.m_unk0x318[p_location][i] = p_id;
			p_count--;
		}
	}

	if (p_count) {
		for (i = 0; i < 12; i++) {
			if (g_unk0x1005c640.m_unk0x318[p_location][i] == p_id) {
				g_unk0x1005c640.m_unk0x318[p_location][i] = 0;
			}
		}

		return FALSE;
	}

	return TRUE;
}

// FUNCTION: MW2SHELL 0x10007cd4
void FUN_10007cd4(MechS32 p_id)
{
	MechS32 j;
	MechS32 i;

	if (p_id <= 0) {
		return;
	}

	j = 0;
	for (i = 0; i < 78; i++) {
		if (g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 == p_id) {
			g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = -1;
			g_unk0x1005c640.m_unk0x49c--;
		}

		if (g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 != -1) {
			if (g_unk0x1005c640.m_unk0x4a0[j].m_unk0x00 == -1) {
				g_unk0x1005c640.m_unk0x4a0[j] = g_unk0x1005c640.m_unk0x4a0[i];
				g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = -1;
			}
			j++;
		}
	}
}

// FUNCTION: MW2SHELL 0x10007d9e
void FUN_10007d9e(MechS32 p_id, MechS32 p_unk0x04)
{
	if (p_id <= 0) {
		return;
	}

	if (g_unk0x1005c640.m_unk0x49c < 78) {
		g_unk0x1005c640.m_unk0x4a0[g_unk0x1005c640.m_unk0x49c].m_unk0x00 = p_id;
		g_unk0x1005c640.m_unk0x4a0[g_unk0x1005c640.m_unk0x49c].m_unk0x04 = p_unk0x04;
		g_unk0x1005c640.m_unk0x49c++;
	}
}

// FUNCTION: MW2SHELL 0x10007df0
void FUN_10007df0(MechS32 p_id)
{
	MechS32 count;

	if (p_id <= 0) {
		return;
	}

	count = FUN_10007b4d(p_id);
	if (count) {
		FUN_10007d9e(p_id, count);
	}
}

// FUNCTION: MW2SHELL 0x10007e3b
MechS32 FUN_10007e3b(MechS32 p_location, MechS32 p_id, MechS32 p_count)
{
	if (p_id <= 0) {
		return FALSE;
	}

	if (FUN_10007bee(p_location, p_id, p_count)) {
		FUN_10007cd4(p_id);
		return TRUE;
	}

	return FALSE;
}

// Stack-slot permutation: id, group, index, i and j.
// FUNCTION: MW2SHELL 0x10007e90
void FUN_10007e90(MechS32 p_id)
{
	MechS32 id;
	MechS32 group;
	MechS32 index;
	MechS32 i;
	MechS32 j;

	group = p_id / 100;
	index = p_id % 100;
	if (p_id <= 0) {
		return;
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 12; j++) {
			id = g_unk0x1005c640.m_unk0x318[i][j];
			if (id == p_id) {
				id = 0;
			}
			else if (id > 0 && (id < 5000 || id >= 10000)) {
				if (id / 100 == group && id % 100 > index) {
					id--;
				}
				else if (p_id < 5000 && id >= 10000) {
					if ((id - 10000) / 100 == p_id) {
						id = 0;
					}
					else if ((id - 10000) / 10000 == group && id / 100 % 100 > index) {
						id -= 100;
					}
				}
			}

			g_unk0x1005c640.m_unk0x318[i][j] = id;
		}
	}

	for (i = 0, j = 0; i < 78; i++) {
		id = g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00;
		if (id == p_id) {
			id = -1;
			g_unk0x1005c640.m_unk0x49c--;
		}
		else if (id > 0 && (id < 5000 || id >= 10000)) {
			if (id / 100 == group && id % 100 > index) {
				id--;
			}
			else if (p_id < 5000 && id >= 10000) {
				if ((id - 10000) / 100 == p_id) {
					id = -1;
					g_unk0x1005c640.m_unk0x49c--;
				}
				else if ((id - 10000) / 10000 == group && id / 100 % 100 > index) {
					id -= 100;
				}
			}
		}

		g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = id;
		if (id != -1) {
			g_unk0x1005c640.m_unk0x4a0[j] = g_unk0x1005c640.m_unk0x4a0[i];
			j++;
		}
	}

	while (j < 78) {
		g_unk0x1005c640.m_unk0x4a0[j].m_unk0x00 = -1;
		j++;
	}
}

// FUNCTION: MW2SHELL 0x1000819f
MechS32 FUN_1000819f(MechS32 p_id)
{
	MechS32 i;

	p_id -= p_id % 100;
	p_id++;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			if (p_id % 100 > 10) {
				return 0;
			}

			g_unk0x1005c640.m_unk0x284[i] = p_id;
			return p_id;
		}

		if (g_unk0x1005c640.m_unk0x284[i] / 100 == p_id / 100) {
			p_id++;
		}
	}

	return 0;
}

// Stack-slot permutation: count and i.
// FUNCTION: MW2SHELL 0x10008254
MechS32 FUN_10008254(MechS32 p_id)
{
	MechS32 count;
	MechS32 i;

	count = 0;
	p_id = (p_id * 100 + 10000) / 100;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			break;
		}

		if (g_unk0x1005c640.m_unk0x284[i] / 100 == p_id) {
			count++;
		}
	}

	return count;
}

// FUNCTION: MW2SHELL 0x100082de
void FUN_100082de(MechS32 p_id)
{
	MechS32 j;
	MechS32 i;

	j = 0;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == p_id) {
			g_unk0x1005c640.m_unk0x284[i] = -1;
		}

		if (g_unk0x1005c640.m_unk0x284[i] != -1) {
			if (g_unk0x1005c640.m_unk0x284[j] == -1) {
				g_unk0x1005c640.m_unk0x284[j] = g_unk0x1005c640.m_unk0x284[i];
				g_unk0x1005c640.m_unk0x284[i] = -1;
			}

			if (g_unk0x1005c640.m_unk0x284[j] / 100 == p_id / 100 && g_unk0x1005c640.m_unk0x284[j] % 100 > p_id % 100) {
				g_unk0x1005c640.m_unk0x284[j]--;
			}

			j++;
		}
	}
}

// FUNCTION: MW2SHELL 0x100083d6
MechS32 FUN_100083d6(MechS32 p_id)
{
	MechS32 i;

	p_id -= p_id % 100;
	p_id++;
	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] == -1) {
			g_unk0x1005c640.m_unk0x2e8[i] = p_id;
			return p_id;
		}

		if (g_unk0x1005c640.m_unk0x2e8[i] / 100 == p_id / 100) {
			p_id++;
		}
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10008470
void FUN_10008470(MechS32 p_id)
{
	MechS32 j;
	MechS32 i;

	j = 0;
	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] == p_id) {
			g_unk0x1005c640.m_unk0x2e8[i] = -1;
		}

		if (g_unk0x1005c640.m_unk0x2e8[i] != -1) {
			if (g_unk0x1005c640.m_unk0x2e8[j] == -1) {
				g_unk0x1005c640.m_unk0x2e8[j] = g_unk0x1005c640.m_unk0x2e8[i];
				g_unk0x1005c640.m_unk0x2e8[i] = -1;
			}

			if (g_unk0x1005c640.m_unk0x2e8[j] / 100 == p_id / 100 && g_unk0x1005c640.m_unk0x2e8[j] % 100 > p_id % 100) {
				g_unk0x1005c640.m_unk0x2e8[j]--;
			}

			j++;
		}
	}

	p_id = p_id * 100 + 10001;
	j = 0;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] / 100 == p_id / 100) {
			g_unk0x1005c640.m_unk0x284[i] = -1;
		}

		if (g_unk0x1005c640.m_unk0x284[i] != -1) {
			if (g_unk0x1005c640.m_unk0x284[j] == -1) {
				g_unk0x1005c640.m_unk0x284[j] = g_unk0x1005c640.m_unk0x284[i];
				g_unk0x1005c640.m_unk0x284[i] = -1;
			}

			if (g_unk0x1005c640.m_unk0x284[j] / 10000 == p_id / 10000 &&
				g_unk0x1005c640.m_unk0x284[j] / 100 % 100 > p_id / 100 % 100) {
				g_unk0x1005c640.m_unk0x284[j] -= 100;
			}

			j++;
		}
	}
}

// Operand order: the original adds m_unk0x214 last in the m_unk0x204 sum; it follows the unit's
// symbol table.
// FUNCTION: MW2SHELL 0x10008686
void FUN_10008686()
{
	g_unk0x1005c640.m_unk0x220 = FUN_10007850((MechS32) (g_unk0x1005c640.m_unk0x210 / 100.0 * 100.0));
	g_unk0x1005c640.m_unk0x244 = g_unk0x1005c640.m_unk0x200 / (g_unk0x1005c640.m_unk0x240 ? 20 : 10);
	g_unk0x1005c640.m_unk0x224 = 300;
	g_unk0x1005c640.m_unk0x204 = g_unk0x1005c640.m_unk0x238 + g_unk0x1005c640.m_unk0x22c + g_unk0x1005c640.m_unk0x270 +
								 g_unk0x1005c640.m_unk0x260 + g_unk0x1005c640.m_unk0x250 + g_unk0x1005c640.m_unk0x220 +
								 g_unk0x1005c640.m_unk0x244 + g_unk0x1005c640.m_unk0x224 + g_unk0x1005c640.m_unk0x268 +
								 g_unk0x1005c640.m_unk0x214;
	g_unk0x1005c640.m_unk0x208 = g_unk0x1005c640.m_unk0x200 - g_unk0x1005c640.m_unk0x204;
}

// FUNCTION: MW2SHELL 0x10008739
void FUN_10008739()
{
	g_unk0x1005c640.m_unk0x278 = g_unk0x1005c640.m_unk0x210 * 100 / g_unk0x1005c640.m_unk0x200;
	g_unk0x1005c640.m_unk0x27c = (g_unk0x1005c640.m_unk0x278 * 3 + 2) / 2;
}

// FUNCTION: MW2SHELL 0x10008776
void FUN_10008776()
{
}

// FUNCTION: MW2SHELL 0x10008786
void FUN_10008786()
{
	MechS32 count;

	count = g_unk0x1005c640.m_unk0x230;
	g_unk0x1005c640.m_unk0x230 = g_unk0x1005c640.m_unk0x22c / 100 + 10 - g_unk0x1005c640.m_unk0x210 / 25;
	if (g_unk0x1005c640.m_unk0x230 < 0) {
		g_unk0x1005c640.m_unk0x230 = 0;
	}

	if (count >= 0 && g_unk0x1005c640.m_unk0x230 != count) {
		while (g_unk0x1005c640.m_unk0x230 < count) {
			FUN_10007e90(count + 6000);
			count--;
		}

		while (count < g_unk0x1005c640.m_unk0x230) {
			count++;
			FUN_10007d9e(count + 6000, g_unk0x1005c640.m_unk0x228);
		}
	}
}

// FUNCTION: MW2SHELL 0x1000884c
void FUN_1000884c()
{
	g_unk0x1005c640.m_unk0x210 = g_unk0x1005d590[g_unk0x1005c640.m_unk0x20c % 10000].m_rating;
	g_unk0x1005c640.m_unk0x214 = g_unk0x1005d590[g_unk0x1005c640.m_unk0x20c % 10000].m_weight;
	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x214 /= 2;
	}

	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x218 = 7;
	}
	else {
		g_unk0x1005c640.m_unk0x218 = 0;
	}

	g_unk0x1005c640.m_unk0x21c = g_unk0x1005c640.m_unk0x200 - g_unk0x1005c640.m_unk0x214;
}

// FUNCTION: MW2SHELL 0x100088ec
void FUN_100088ec()
{
	FUN_10008739();
	while (g_unk0x1005c640.m_unk0x280 > g_unk0x1005c640.m_unk0x278) {
		FUN_10007e90(g_unk0x1005c640.m_unk0x280 + 7000);
		g_unk0x1005c640.m_unk0x280--;
	}

	if (g_unk0x1005c640.m_unk0x200 <= 5500) {
		g_unk0x1005c640.m_unk0x234 = 50;
	}
	else if (g_unk0x1005c640.m_unk0x200 <= 8500) {
		g_unk0x1005c640.m_unk0x234 = 100;
	}
	else {
		g_unk0x1005c640.m_unk0x234 = 200;
	}

	g_unk0x1005c640.m_unk0x238 = g_unk0x1005c640.m_unk0x280 * g_unk0x1005c640.m_unk0x234;
}

// FUNCTION: MW2SHELL 0x10008989
void FUN_10008989()
{
	FUN_10008686();
	FUN_10008739();
	FUN_10008776();
}

// FUNCTION: MW2SHELL 0x100089a8
void FUN_100089a8()
{
	MechS32 i;

	g_unk0x1005c640.m_unk0x260 = 0;
	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] != -1) {
			g_unk0x1005c640.m_unk0x260 += g_unk0x1005d950[g_unk0x1005c640.m_unk0x2e8[i] / 100].m_unk0x18;
		}
	}
}

// FUNCTION: MW2SHELL 0x10008a16
void FUN_10008a16()
{
	MechS32 i;

	if (!g_unk0x1005c640.m_unk0x24c) {
		g_unk0x1005c640.m_unk0x258 = (g_unk0x1005c640.m_unk0x250 * 16 + 49) / 100;
	}
	else {
		g_unk0x1005c640.m_unk0x258 = ((MechS32) (g_unk0x1005c640.m_unk0x250 * 16 * 1.2) + 49) / 100;
	}

	g_unk0x1005c640.m_unk0x25c = 0;
	for (i = 0; i < 8; i++) {
		g_unk0x1005c640.m_unk0x25c += g_unk0x1005c640.m_unk0x710[i].m_unk0x08;
		if (g_unk0x1005c640.m_unk0x710[i].m_unk0x0c >= 0) {
			g_unk0x1005c640.m_unk0x25c += g_unk0x1005c640.m_unk0x710[i].m_unk0x0c;
		}
	}

	while (g_unk0x1005c640.m_unk0x25c > g_unk0x1005c640.m_unk0x258) {
		if (g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x08 > 0) {
			g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x08--;
			g_unk0x1005c640.m_unk0x25c--;
		}

		if (g_unk0x1005c640.m_unk0x25c <= g_unk0x1005c640.m_unk0x258) {
			break;
		}

		if (g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x0c > 0) {
			g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x0c--;
			g_unk0x1005c640.m_unk0x25c--;
		}

		g_unk0x1005de30++;
		if (g_unk0x1005de30 >= 8) {
			g_unk0x1005de30 = 0;
		}
	}
}

// FUNCTION: MW2SHELL 0x10008b76
void FUN_10008b76()
{
	MechS32 i;

	g_unk0x1005c640.m_unk0x268 = 0;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			break;
		}

		g_unk0x1005c640.m_unk0x268 += 100;
	}
}

// FUNCTION: MW2SHELL 0x10008bce
TextGlyph* FUN_10008bce(ScreenField* p_tab)
{
	// An empty test of the unused parameter: the original compiles it to a cmp with a zero-length je.
	if (p_tab) {
	}

	FUN_10017698(12, g_unk0x1005c640.m_unk0x314);
	FUN_10016f82(
		12,
		g_unk0x1005de38[g_unk0x1005c640.m_unk0x314] + 0xd8,
		g_unk0x1005de58[g_unk0x1005c640.m_unk0x314] + 0x34
	);
	FUN_10016d27(12);
	return NULL;
}

// FUNCTION: MW2SHELL 0x10008c30
TextGlyph* FUN_10008c30(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d.%d%d T", value / 100, value / 10 % 10, value % 10);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008cb0
TextGlyph* FUN_10008cb0(ScreenField* p_tab)
{
	MechS32 value;
	undefined* colors;

	value = *(MechS32*) p_tab->m_unk0x24;
	colors = p_tab->m_unk0x14;
	if (value > g_unk0x1005c640.m_unk0x200) {
		colors = g_unk0x1007c960;
	}

	sprintf(g_szTempBuffer, "%d.%d%d T", value / 100, value / 10 % 10, value % 10);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, colors);
}

// FUNCTION: MW2SHELL 0x10008d4c
TextGlyph* FUN_10008d4c(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d.%d%d T", value / 100, value / 10 % 10, value % 10);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008dcc
TextGlyph* FUN_10008dcc(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, value >= 10000 ? "%dXL" : "%d", g_unk0x1005d590[value % 10000].m_rating);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008e4f
TextGlyph* FUN_10008e4f(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value >= 10000 ? "XL" : "Std"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008eaa
TextGlyph* FUN_10008eaa(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%s", g_unk0x1005d590[value % 10000].m_name);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008f14
TextGlyph* FUN_10008f14(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%1.1f kph", value * 10.8);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008f7d
TextGlyph* FUN_10008f7d(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d m", value * 30);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008fdd
TextGlyph* FUN_10008fdd(ScreenField* p_tab)
{
	MechS32 value;
	MechS32 count;

	value = *(MechS32*) p_tab->m_unk0x24;
	count = (g_unk0x1005c640.m_unk0x22c + 50) / 100 + 10;
	if (value == 1) {
		sprintf(g_szTempBuffer, "%d", count);
	}
	else {
		sprintf(g_szTempBuffer, "%d (%d)", count, count * 2);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009076
TextGlyph* FUN_10009076(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value == 1 ? "Single" : "Double"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100090ce
TextGlyph* FUN_100090ce(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d", value);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009126
TextGlyph* FUN_10009126(ScreenField* p_tab)
{
	MechChar* text;

	if (p_tab->m_unk0x24 == NULL) {
		return NULL;
	}

	text = *(MechChar**) p_tab->m_unk0x24;
	if (text == NULL) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000918c
TextGlyph* FUN_1000918c(ScreenField* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	if (text == NULL) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100091dc
TextGlyph* FUN_100091dc(ScreenField* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	if (text == NULL) {
		return NULL;
	}

	return g_unk0x10071214->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000922c
TextGlyph* FUN_1000922c(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value ? "Endo-S" : "Std"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009284
TextGlyph* FUN_10009284(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value ? "Ferro-F" : "Std"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100092dc
TextGlyph* FUN_100092dc(ScreenField* p_tab)
{
	MekVariant::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[g_unk0x1005c640.m_unk0x314];
	if (p_tab->m_unk0x24) {
		if (armor->m_unk0x0c >= 0) {
			sprintf(g_szTempBuffer, "~%d", armor->m_unk0x0c);
		}
		else {
			strcpy(g_szTempBuffer, "~--");
		}
	}
	else {
		sprintf(g_szTempBuffer, "~%d", armor->m_unk0x08);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100093a7
TextGlyph* FUN_100093a7(ScreenField* p_tab)
{
	MekVariant::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[(MechS32) p_tab->m_unk0x24];
	if (armor->m_unk0x0c >= 0) {
		sprintf(g_szTempBuffer, "~%d/%d", armor->m_unk0x08, armor->m_unk0x0c);
	}
	else {
		sprintf(g_szTempBuffer, "~%d", armor->m_unk0x08);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000943f
TextGlyph* FUN_1000943f(ScreenField* p_tab)
{
	MechS32 location;

	location = (MechS32) p_tab->m_unk0x24;
	if (location == -1) {
		return NULL;
	}

	sprintf(g_szTempBuffer, "%s (%d)", g_unk0x1005c4f0[location], g_unk0x1005c640.m_unk0x710[location].m_unk0x04);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100094ba
TextGlyph* FUN_100094ba(ScreenField* p_tab)
{
	MechS32 location;

	location = *(MechS32*) p_tab->m_unk0x24;
	if (location == -1) {
		return NULL;
	}

	sprintf(g_szTempBuffer, "%s (%d)", g_unk0x1005c4f0[location], g_unk0x1005c640.m_unk0x710[location].m_unk0x04);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009537
TextGlyph* FUN_10009537(ScreenField* p_tab)
{
	MechS32 value;
	MechS32 count;
	undefined* colors;

	value = *(MechS32*) p_tab->m_unk0x24;
	colors = p_tab->m_unk0x14;
	if (value == -1) {
		strcpy(g_szTempBuffer, "-");
	}
	else {
		if (g_unk0x1005d950[value / 100].m_unk0x20) {
			count = FUN_10008254(value);
			sprintf(
				g_szTempBuffer,
				"%s #%d (ammo %dT/%d)",
				g_unk0x1005d950[value / 100].m_name,
				value % 100,
				count,
				g_unk0x1005d950[value / 100].m_unk0x20 * count
			);
		}
		else {
			sprintf(g_szTempBuffer, "%s #%d", g_unk0x1005d950[value / 100].m_name, value % 100);
		}

		if (g_unk0x1005c640.m_unk0x310 == value) {
			colors = g_unk0x1007ca60;
		}
	}

	if (p_tab->m_left >= 0x1b4) {
		return g_unk0x1007120c->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_szTempBuffer, colors);
	}
	else {
		return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, colors);
	}
}

// FUNCTION: MW2SHELL 0x100096bd
TextGlyph* FUN_100096bd(ScreenField* p_tab)
{
	MechS32 index;
	undefined* colors;

	index = (MechS32) p_tab->m_unk0x24;
	colors = p_tab->m_unk0x14;
	if (index == -1) {
		return NULL;
	}

	if (g_unk0x1005c640.m_unk0x310 == index * 100) {
		colors = g_unk0x1007ca60;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_unk0x1005d950[index].m_name, colors);
}

// FUNCTION: MW2SHELL 0x1000973c
TextGlyph* FUN_1000973c(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	if (value == -1) {
		return NULL;
	}

	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, g_unk0x1005d950[value / 100].m_name, p_tab->m_unk0x14);
}

// Draws one statistic (p_tab->m_unk0x24) of the highlighted weapon.
// FUNCTION: MW2SHELL 0x100097a0
TextGlyph* FUN_100097a0(ScreenField* p_tab)
{
	MechS32 weapon;

	if (g_unk0x1005c640.m_unk0x310 == -1) {
		return NULL;
	}

	weapon = g_unk0x1005c640.m_unk0x310 / 100;
	switch ((MechS32) p_tab->m_unk0x24) {
	case 0:
		if (g_unk0x1005c640.m_unk0x310 % 100) {
			sprintf(g_szTempBuffer, "%s #%d", g_unk0x1005d950[weapon].m_name, g_unk0x1005c640.m_unk0x310 % 100);
		}
		else {
			sprintf(g_szTempBuffer, "%s", g_unk0x1005d950[weapon].m_name);
		}
		break;
	case 1:
		sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x00);
		break;
	case 2:
		if (g_unk0x1005d950[weapon].m_unk0x04 == 0) {
			strcpy(g_szTempBuffer, "-");
		}
		else if (g_unk0x1005d950[weapon].m_unk0x04 < 0) {
			sprintf(g_szTempBuffer, "%d/missile", -g_unk0x1005d950[weapon].m_unk0x04);
		}
		else {
			sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x04);
		}
		break;
	case 3:
		if (g_unk0x1005d950[weapon].m_unk0x08 >= 0) {
			sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x08);
		}
		else {
			strcpy(g_szTempBuffer, "-");
		}
		break;
	case 4:
		if (g_unk0x1005d950[weapon].m_unk0x0c == -1) {
			strcpy(g_szTempBuffer, "-");
		}
		else if (g_unk0x1005d950[weapon].m_unk0x0c == 1) {
			strcpy(g_szTempBuffer, "1");
		}
		else {
			sprintf(g_szTempBuffer, "1-%d", g_unk0x1005d950[weapon].m_unk0x0c);
		}
		break;
	case 5:
		if (g_unk0x1005d950[weapon].m_unk0x10 == -1) {
			strcpy(g_szTempBuffer, "-");
		}
		else if (g_unk0x1005d950[weapon].m_unk0x0c + 1 == g_unk0x1005d950[weapon].m_unk0x10) {
			sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x10);
		}
		else {
			sprintf(g_szTempBuffer, "%d-%d", g_unk0x1005d950[weapon].m_unk0x0c + 1, g_unk0x1005d950[weapon].m_unk0x10);
		}
		break;
	case 6:
		if (g_unk0x1005d950[weapon].m_unk0x14 == -1) {
			strcpy(g_szTempBuffer, "-");
		}
		else {
			sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x14);
		}
		break;
	case 7:
		if (g_unk0x1005d950[weapon].m_unk0x18 % 10) {
			sprintf(
				g_szTempBuffer,
				"%d.%d%dT",
				g_unk0x1005d950[weapon].m_unk0x18 / 100,
				g_unk0x1005d950[weapon].m_unk0x18 / 10 % 10,
				g_unk0x1005d950[weapon].m_unk0x18 % 10
			);
		}
		else if (g_unk0x1005d950[weapon].m_unk0x18 % 100) {
			sprintf(
				g_szTempBuffer,
				"%d.%dT",
				g_unk0x1005d950[weapon].m_unk0x18 / 100,
				g_unk0x1005d950[weapon].m_unk0x18 / 10 % 10
			);
		}
		else {
			sprintf(g_szTempBuffer, "%dT", g_unk0x1005d950[weapon].m_unk0x18 / 100);
		}
		break;
	case 8:
		sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x1c);
		break;
	case 9:
		if (g_unk0x1005d950[weapon].m_unk0x20) {
			sprintf(g_szTempBuffer, "%d", g_unk0x1005d950[weapon].m_unk0x20);
		}
		else {
			strcpy(g_szTempBuffer, "-");
		}
		break;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009d50
TextGlyph* FUN_10009d50(ScreenField* p_tab)
{
	MechS32 location;

	location = *(MechS32*) p_tab->m_unk0x24;
	if (location == -1) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_unk0x1005c4f0[location], p_tab->m_unk0x14);
}

// Draws the item in one critical slot (p_tab->m_unk0x24) of the selected location: a weapon, a
// piece of equipment or ammunition (ids from 10000 up).
// Not 100%: the stack slots of id and i are permuted.
// FUNCTION: MW2SHELL 0x10009da9
TextGlyph* FUN_10009da9(ScreenField* p_tab)
{
	MechS32 id;
	MechS32 i;

	if (g_unk0x1005c640.m_unk0x314 == -1) {
		return NULL;
	}

	id = g_unk0x1005c640.m_unk0x318[g_unk0x1005c640.m_unk0x314][(MechS32) p_tab->m_unk0x24];
	if (id == -1) {
		return NULL;
	}

	id &= ~0x80000000;
	if (id == 0) {
		strcpy(g_szTempBuffer, "-");
	}
	else if (id < 5000) {
		sprintf(g_szTempBuffer, "%s #%d", g_unk0x1005d950[id / 100].m_name, id % 100);
	}
	else if (id < 10000) {
		for (i = 22; i >= 0; i--) {
			if (g_unk0x1005c438[i].m_unk0x00 / 50 == id / 50) {
				sprintf(g_szTempBuffer, "%s", g_unk0x1005c438[i].m_name);
				break;
			}
		}
		if (i < 0) {
			sprintf(g_szTempBuffer, "BAD CRITICAL %d", id);
		}
	}
	else {
		id -= 10000;
		sprintf(g_szTempBuffer, "Ammo (%s #%d) #%d", g_unk0x1005d950[id / 10000].m_name, id / 100 % 100, id % 100);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// Draws one entry (p_tab->m_unk0x24) of the items list with its count, like FUN_10009da9.
// Not 100%: the stack slots of id, count and i are permuted.
// FUNCTION: MW2SHELL 0x10009f92
TextGlyph* FUN_10009f92(ScreenField* p_tab)
{
	MechS32 id;
	MechS32 count;
	MechS32 i;

	if (g_unk0x1005c640.m_unk0x314 == -1) {
		return NULL;
	}

	id = g_unk0x1005c640.m_unk0x4a0[(MechS32) p_tab->m_unk0x24].m_unk0x00;
	count = g_unk0x1005c640.m_unk0x4a0[(MechS32) p_tab->m_unk0x24].m_unk0x04;
	if (id == -1 || id == 0 || count == 0) {
		return NULL;
	}
	else if (id < 5000) {
		sprintf(g_szTempBuffer, "%s #%d (%d)", g_unk0x1005d950[id / 100].m_name, id % 100, count);
	}
	else if (id < 10000) {
		for (i = 22; i >= 0; i--) {
			if (g_unk0x1005c438[i].m_unk0x00 / 50 == id / 50) {
				sprintf(g_szTempBuffer, "%s (%d)", g_unk0x1005c438[i].m_name, count);
				break;
			}
		}
		if (i < 0) {
			sprintf(g_szTempBuffer, "BAD CRITICAL %d", id);
		}
	}
	else {
		id -= 10000;
		sprintf(
			g_szTempBuffer,
			"Ammo (%s #%d) #%d (%d)",
			g_unk0x1005d950[id / 10000].m_name,
			id / 100 % 100,
			id % 100,
			count
		);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a161
TextGlyph* FUN_1000a161(ScreenField* p_tab)
{
	if (g_unk0x1005c640.m_unk0x4a0[(MechS32) p_tab->m_unk0x24].m_unk0x00 <= 0) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, "More...", p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a1b0
TextGlyph* FUN_1000a1b0(ScreenField* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value ? "Yes" : "No"), p_tab->m_unk0x14);
}

// Stack-slot permutation: id and text.
// FUNCTION: MW2SHELL 0x1000a208
TextGlyph* FUN_1000a208(ScreenField* p_tab)
{
	MechS32 id;
	MechChar* text;

	id = (MechS32) p_tab->m_unk0x24;
	switch (id) {
	case 5000:
		text = "MASC";
		break;
	case 5401:
		text = "Right Lower Arm Actuator";
		break;
	case 5402:
		text = "Left Lower Arm Actuator";
		break;
	case 5451:
		text = "Right Hand Actuator";
		break;
	case 5452:
		text = "Left Hand Actuator";
		break;
	default:
		text = "";
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a2eb
void FUN_1000a2eb(ScreenField* p_tab)
{
	ScreenField* tabs;

	tabs = p_tab->m_unk0x28;
	FUN_10016cc0(10, 0x20, 0x20);
	FUN_10016cc0(11, 0x20, 0x20);
	FUN_10016cc0(12, 0x20, 0x20);
	FUN_10016cc0(13, 0x20, 0x20);
	FUN_10016d27(14);
	FUN_10007ac8(g_unk0x1005de28);
	g_unk0x1005de28 = tabs;
	FUN_100078cd(g_unk0x1005de28);
}

// FUNCTION: MW2SHELL 0x1000a36d
void FUN_1000a36d(ScreenField* p_tab)
{
	g_unk0x1005c640.m_unk0x310 = -1;
	FUN_1000a2eb(p_tab);
	FUN_10016d27(11);
	FUN_10016d27(13);
	FUN_10016cc0(14, 0x20, 0x20);
}

// FUNCTION: MW2SHELL 0x1000a3b5
void FUN_1000a3b5(ScreenField* p_tab)
{
	FUN_1000a2eb(p_tab);
	FUN_10016d27(10);
	FUN_10016d27(12);
	FUN_10016cc0(14, 0x20, 0x20);
}

// FUNCTION: MW2SHELL 0x1000a3f3
void FUN_1000a3f3(ScreenField* p_tab)
{
	FUN_1000a2eb(p_tab);
	FUN_10016d27(10);
	FUN_10016d27(11);
	FUN_10016d27(12);
	FUN_10016cc0(14, 0x20, 0x20);
}

// FUNCTION: MW2SHELL 0x1000a43b
void FUN_1000a43b(ScreenField* p_tab)
{
	if (p_tab->m_glyph != NULL) {
		delete p_tab->m_glyph;
	}

	EditTextField(
		g_unk0x1007120c,
		p_tab->m_left,
		p_tab->m_top,
		(MechChar*) p_tab->m_unk0x24,
		p_tab->m_unk0x14,
		0x1c,
		p_tab->m_width
	);
	p_tab->m_glyph =
		g_unk0x1007120c->FUN_1000544e(p_tab->m_left, p_tab->m_top, (MechChar*) p_tab->m_unk0x24, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a4f0
void FUN_1000a4f0(ScreenField* p_tab)
{
	MechS32 rating;

	if (p_tab) {
	}

	rating = ((g_unk0x1005c640.m_unk0x278 + 1) * (g_unk0x1005c640.m_unk0x200 / 100) * 5 + 4) / 5;
	if (rating < 10) {
		return;
	}
	if (rating > 400) {
		return;
	}

	rating = (rating - 10) / 5;
	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x20c = rating + 10000;
	}
	else {
		g_unk0x1005c640.m_unk0x20c = rating;
	}

	FUN_1000884c();
	FUN_10008786();
	FUN_100088ec();
	FUN_10008989();
}

// The rating product differs only in evaluation order: the original divides m_unk0x200 before
// loading m_unk0x278 - 1, as FUN_1000a4f0 does; the order follows the unit's symbol table (it has
// flipped between the two functions as declarations moved).
// FUNCTION: MW2SHELL 0x1000a5a3
void FUN_1000a5a3(ScreenField* p_tab)
{
	MechS32 rating;

	if (p_tab) {
	}

	if (!g_unk0x1005c640.m_unk0x278) {
		return;
	}

	rating = ((g_unk0x1005c640.m_unk0x278 - 1) * (g_unk0x1005c640.m_unk0x200 / 100) * 5 + 4) / 5;
	if (rating < 10) {
		return;
	}
	if (rating > 400) {
		return;
	}

	rating = (rating - 10) / 5;
	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x20c = rating + 10000;
	}
	else {
		g_unk0x1005c640.m_unk0x20c = rating;
	}

	FUN_1000884c();
	FUN_10008786();
	FUN_100088ec();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a668
void FUN_1000a668(ScreenField* p_tab)
{
	MechS32 i;

	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x20c -= 10000;
		for (i = 0; i < 12; i++) {
			if (g_unk0x1005c640.m_unk0x318[1][i] == 5850) {
				g_unk0x1005c640.m_unk0x318[1][i] = 0;
			}
			if (g_unk0x1005c640.m_unk0x318[3][i] == 5850) {
				g_unk0x1005c640.m_unk0x318[3][i] = 0;
			}
		}
	}
	else {
		g_unk0x1005c640.m_unk0x20c += 10000;
		if (g_unk0x1005c640.m_unk0x318[3][0] > 0) {
			FUN_10007df0(g_unk0x1005c640.m_unk0x318[3][0]);
		}
		if (g_unk0x1005c640.m_unk0x318[3][1] > 0) {
			FUN_10007df0(g_unk0x1005c640.m_unk0x318[3][1]);
		}
		if (g_unk0x1005c640.m_unk0x318[1][0] > 0) {
			FUN_10007df0(g_unk0x1005c640.m_unk0x318[1][0]);
		}
		if (g_unk0x1005c640.m_unk0x318[1][1] > 0) {
			FUN_10007df0(g_unk0x1005c640.m_unk0x318[1][1]);
		}

		g_unk0x1005c640.m_unk0x318[3][0] = 5850;
		g_unk0x1005c640.m_unk0x318[3][1] = 5850;
		g_unk0x1005c640.m_unk0x318[1][0] = 5850;
		g_unk0x1005c640.m_unk0x318[1][1] = 5850;
	}

	FUN_1000884c();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a7ae
void FUN_1000a7ae(ScreenField* p_tab)
{
	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x280 >= g_unk0x1005c640.m_unk0x278) {
		return;
	}

	g_unk0x1005c640.m_unk0x280++;
	FUN_10007d9e(g_unk0x1005c640.m_unk0x280 + 7000, 1);
	FUN_100088ec();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a803
void FUN_1000a803(ScreenField* p_tab)
{
	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x280) {
		FUN_10007e90(g_unk0x1005c640.m_unk0x280 + 7000);
		g_unk0x1005c640.m_unk0x280--;
	}

	FUN_100088ec();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a84d
void FUN_1000a84d(ScreenField* p_tab)
{
	if (p_tab) {
	}

	g_unk0x1005c640.m_unk0x22c += 100;
	FUN_10008786();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a878
void FUN_1000a878(ScreenField* p_tab)
{
	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x22c) {
		g_unk0x1005c640.m_unk0x22c -= 100;
	}

	FUN_10008786();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a8b0
void FUN_1000a8b0(ScreenField* p_tab)
{
	MechS32 i;

	if (p_tab) {
	}

	g_unk0x1005c640.m_unk0x228 = 3 - g_unk0x1005c640.m_unk0x228;
	FUN_10008786();
	for (i = 1; i <= g_unk0x1005c640.m_unk0x230; i++) {
		FUN_10007e90(i + 6000);
	}
	for (i = 1; i <= g_unk0x1005c640.m_unk0x230; i++) {
		FUN_10007d9e(i + 6000, g_unk0x1005c640.m_unk0x228);
	}

	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a955
void FUN_1000a955(ScreenField* p_tab)
{
	if (p_tab) {
	}

	g_unk0x1005c640.m_unk0x250 += 50;
	FUN_10008a16();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a980
void FUN_1000a980(ScreenField* p_tab)
{
	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x250) {
		g_unk0x1005c640.m_unk0x250 -= 50;
	}

	FUN_10008a16();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000a9b8
void FUN_1000a9b8(ScreenField* p_tab)
{
	if (p_tab) {
	}

	g_unk0x1005c640.m_unk0x24c = 1 - g_unk0x1005c640.m_unk0x24c;
	if (!g_unk0x1005c640.m_unk0x24c) {
		FUN_10007e90(9001);
		FUN_10007e90(9002);
		FUN_10007e90(9003);
		FUN_10007e90(9004);
		FUN_10007e90(9005);
		FUN_10007e90(9006);
		FUN_10007e90(9007);
	}
	else {
		FUN_10007d9e(9001, 1);
		FUN_10007d9e(9002, 1);
		FUN_10007d9e(9003, 1);
		FUN_10007d9e(9004, 1);
		FUN_10007d9e(9005, 1);
		FUN_10007d9e(9006, 1);
		FUN_10007d9e(9007, 1);
	}

	FUN_10008a16();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000aac2
void FUN_1000aac2(ScreenField* p_tab)
{
	if (p_tab) {
	}

	g_unk0x1005c640.m_unk0x240 = 1 - g_unk0x1005c640.m_unk0x240;
	if (!g_unk0x1005c640.m_unk0x240) {
		FUN_10007e90(8001);
		FUN_10007e90(8002);
		FUN_10007e90(8003);
		FUN_10007e90(8004);
		FUN_10007e90(8005);
		FUN_10007e90(8006);
		FUN_10007e90(8007);
	}
	else {
		FUN_10007d9e(8001, 1);
		FUN_10007d9e(8002, 1);
		FUN_10007d9e(8003, 1);
		FUN_10007d9e(8004, 1);
		FUN_10007d9e(8005, 1);
		FUN_10007d9e(8006, 1);
		FUN_10007d9e(8007, 1);
	}

	FUN_10008a16();
	FUN_10008989();
	FUN_100079f8(g_unk0x1005de28);
	FUN_100079f8(g_unk0x1005de2c);
}

// FUNCTION: MW2SHELL 0x1000abe8
void FUN_1000abe8(ScreenField* p_tab)
{
	MechS32 id;

	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x310 < 0) {
		return;
	}

	id = FUN_100083d6(g_unk0x1005c640.m_unk0x310);
	if (id) {
		FUN_10007d9e(id, g_unk0x1005d950[id / 100].m_unk0x1c);
		if (FUN_10008254(id) >= 10) {
			return;
		}

		if (g_unk0x1005d950[id / 100].m_unk0x20) {
			id = FUN_1000819f(id * 100 + 10000);
			FUN_10007d9e(id, 1);
		}

		FUN_100089a8();
		FUN_10008b76();
		FUN_10008989();
	}
}

// FUNCTION: MW2SHELL 0x1000acc4
void FUN_1000acc4(ScreenField* p_tab)
{
	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x310 < 0) {
		return;
	}
	if (!(g_unk0x1005c640.m_unk0x310 % 100)) {
		return;
	}

	FUN_10008470(g_unk0x1005c640.m_unk0x310);
	FUN_10007e90(g_unk0x1005c640.m_unk0x310);
	g_unk0x1005c640.m_unk0x310 = g_unk0x1005c640.m_unk0x2e8[0];
	FUN_100089a8();
	FUN_10008b76();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000ad3f
void FUN_1000ad3f(ScreenField* p_tab)
{
	MechS32 id;

	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x310 < 0) {
		return;
	}
	if (!(g_unk0x1005c640.m_unk0x310 % 100)) {
		return;
	}
	if (FUN_10008254(g_unk0x1005c640.m_unk0x310) >= 10) {
		return;
	}

	if (g_unk0x1005d950[g_unk0x1005c640.m_unk0x310 / 100].m_unk0x20) {
		id = FUN_1000819f(g_unk0x1005c640.m_unk0x310 * 100 + 10000);
		FUN_10007d9e(id, 1);
		FUN_10008b76();
		FUN_10008989();
	}
}

// FUNCTION: MW2SHELL 0x1000adf9
void FUN_1000adf9(ScreenField* p_tab)
{
	MechS32 count;

	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x310 < 0) {
		return;
	}
	if (!(g_unk0x1005c640.m_unk0x310 % 100)) {
		return;
	}

	count = FUN_10008254(g_unk0x1005c640.m_unk0x310);
	if (!count) {
		return;
	}

	FUN_100082de(g_unk0x1005c640.m_unk0x310 * 100 + count + 10000);
	FUN_10007e90(g_unk0x1005c640.m_unk0x310 * 100 + count + 10000);
	FUN_10008b76();
	FUN_10008989();
}

// FUNCTION: MW2SHELL 0x1000aeaa
void FUN_1000aeaa(ScreenField* p_tab)
{
	MechS32 id;

	id = *(MechS32*) p_tab->m_unk0x24;
	if (id < 0) {
		return;
	}

	if (g_unk0x1005c640.m_unk0x310 == id && g_pMouseState->GetDoubleClicked()) {
		FUN_1000acc4(p_tab);
	}

	g_unk0x1005c640.m_unk0x310 = *(MechS32*) p_tab->m_unk0x24;
}

// FUNCTION: MW2SHELL 0x1000af16
void FUN_1000af16(ScreenField* p_tab)
{
	if (p_tab) {
	}

	if ((MechS32) p_tab->m_unk0x24 * 100 == g_unk0x1005c640.m_unk0x310) {
		if (g_pMouseState->GetDoubleClicked()) {
			FUN_1000abe8(p_tab);
		}
	}
	else {
		g_unk0x1005c640.m_unk0x310 = (MechS32) p_tab->m_unk0x24 * 100;
	}
}

// FUNCTION: MW2SHELL 0x1000af87
void FUN_1000af87(ScreenField* p_tab)
{
	if (p_tab) {
	}

	g_unk0x1005de78->Start();
	g_unk0x1005c640.m_unk0x314++;
	if (g_unk0x1005c640.m_unk0x314 >= 8) {
		g_unk0x1005c640.m_unk0x314 = 0;
	}
}

// FUNCTION: MW2SHELL 0x1000afc9
void FUN_1000afc9(ScreenField* p_tab)
{
	g_unk0x1005de78->Start();
	g_unk0x1005c640.m_unk0x314 = (MechS32) p_tab->m_unk0x24;
}

// Stack-slot permutation: id and count.
// FUNCTION: MW2SHELL 0x1000afef
void FUN_1000afef(ScreenField* p_tab)
{
	MechS32 id;
	MechS32 count;

	id = g_unk0x1005c640.m_unk0x4a0[(MechS32) p_tab->m_unk0x24].m_unk0x00;
	count = g_unk0x1005c640.m_unk0x4a0[(MechS32) p_tab->m_unk0x24].m_unk0x04;
	if (id < 0) {
		return;
	}

	if (g_unk0x1005c640.m_unk0x314 >= 0) {
		if (id - id % 100 == 7000 &&
			(g_unk0x1005c640.m_unk0x314 == 0 || g_unk0x1005c640.m_unk0x314 == 4 || g_unk0x1005c640.m_unk0x314 == 5)) {
			ShowDialog("Jump Jets may only|be assigned to torso|or leg sections.#Ok", 0);
			return;
		}

		if (!FUN_10007e3b(g_unk0x1005c640.m_unk0x314, id, count)) {
			ShowDialog("Insufficient criticals|for item placement.#Ok", 0);
		}
	}
}

// FUNCTION: MW2SHELL 0x1000b0c2
void FUN_1000b0c2(ScreenField* p_tab)
{
	if (p_tab) {
	}
}

// FUNCTION: MW2SHELL 0x1000b0dc
void FUN_1000b0dc(ScreenField* p_tab)
{
	MechS32 id;

	if (g_unk0x1005c640.m_unk0x314 < 0) {
		return;
	}

	id = g_unk0x1005c640.m_unk0x318[g_unk0x1005c640.m_unk0x314][(MechS32) p_tab->m_unk0x24];
	if (id < 0) {
		return;
	}

	if (id >= 5300 && id < 6000) {
		ShowDialog("Selected critical|can not be removed.#Ok", 0);
		return;
	}

	FUN_10007df0(id);
}

// Comparison operand order: the original loads m_unk0x25c into eax and compares m_unk0x258 with
// it, the reverse of ours; flipping the source didn't change it.
// FUNCTION: MW2SHELL 0x1000b166
void FUN_1000b166(ScreenField* p_tab)
{
	MekVariant::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[g_unk0x1005c640.m_unk0x314];
	if (p_tab) {
	}

	if (g_unk0x1005c640.m_unk0x25c >= g_unk0x1005c640.m_unk0x258) {
		if (armor->m_unk0x0c > 0) {
			armor->m_unk0x08++;
			armor->m_unk0x0c--;
		}
	}
	else if (armor->m_unk0x0c < 0) {
		if (armor->m_unk0x04 > armor->m_unk0x08) {
			armor->m_unk0x08++;
			g_unk0x1005c640.m_unk0x25c++;
		}
	}
	else if (armor->m_unk0x08 + armor->m_unk0x0c < armor->m_unk0x04) {
		armor->m_unk0x08++;
		g_unk0x1005c640.m_unk0x25c++;
	}
	else if (armor->m_unk0x0c) {
		armor->m_unk0x08++;
		armor->m_unk0x0c--;
	}
}

// FUNCTION: MW2SHELL 0x1000b239
void FUN_1000b239(ScreenField* p_tab)
{
	MekVariant::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[g_unk0x1005c640.m_unk0x314];
	if (p_tab) {
	}

	if (armor->m_unk0x08 > 0) {
		armor->m_unk0x08--;
		g_unk0x1005c640.m_unk0x25c--;
	}
}

// Comparison operand order: the original loads m_unk0x25c into eax and compares m_unk0x258 with
// it, the reverse of ours; flipping the source didn't change it.
// FUNCTION: MW2SHELL 0x1000b284
void FUN_1000b284(ScreenField* p_tab)
{
	MekVariant::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[g_unk0x1005c640.m_unk0x314];
	if (p_tab) {
	}

	if (armor->m_unk0x0c < 0) {
		return;
	}

	if (g_unk0x1005c640.m_unk0x25c >= g_unk0x1005c640.m_unk0x258) {
		if (armor->m_unk0x08 > 0) {
			armor->m_unk0x08--;
			armor->m_unk0x0c++;
		}
	}
	else if (armor->m_unk0x08 + armor->m_unk0x0c < armor->m_unk0x04) {
		armor->m_unk0x0c++;
		g_unk0x1005c640.m_unk0x25c++;
	}
	else if (armor->m_unk0x08) {
		armor->m_unk0x08--;
		armor->m_unk0x0c++;
	}
}

// FUNCTION: MW2SHELL 0x1000b339
void FUN_1000b339(ScreenField* p_tab)
{
	MekVariant::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[g_unk0x1005c640.m_unk0x314];
	if (p_tab) {
	}

	if (armor->m_unk0x0c > 0) {
		armor->m_unk0x0c--;
		g_unk0x1005c640.m_unk0x25c--;
	}
}

// Toggles the flag at m_unk0x24 and adds or removes the item id held in m_unk0x28. The arm
// actuators (5401, 5402, 5451, 5452) take a fixed slot in the arms, replacing what is there.
// FUNCTION: MW2SHELL 0x1000b384
void FUN_1000b384(ScreenField* p_tab)
{
	MechS32* flag;
	MechS32 i;

	flag = (MechS32*) p_tab->m_unk0x24;
	if (*flag == 0) {
		*flag = 1;
	}
	else {
		*flag = 0;
	}

	if (*flag) {
		switch ((MechS32) p_tab->m_unk0x28) {
		case 5000:
			for (i = 1; i <= g_unk0x1005c640.m_unk0x7a4; i++) {
				FUN_10007d9e(i + 5000, 1);
			}
			g_unk0x1005c640.m_unk0x270 = g_unk0x1005c640.m_unk0x7a4 * 100;
			FUN_10008989();
			break;
		case 5401:
			if (g_unk0x1005c640.m_unk0x318[4][2] > 0) {
				FUN_10007df0(g_unk0x1005c640.m_unk0x318[4][2]);
			}
			g_unk0x1005c640.m_unk0x318[4][2] = 5401;
			break;
		case 5451:
			if (g_unk0x1005c640.m_unk0x318[4][3] > 0) {
				FUN_10007df0(g_unk0x1005c640.m_unk0x318[4][3]);
			}
			g_unk0x1005c640.m_unk0x318[4][3] = 5451;
			break;
		case 5402:
			if (g_unk0x1005c640.m_unk0x318[5][2] > 0) {
				FUN_10007df0(g_unk0x1005c640.m_unk0x318[5][2]);
			}
			g_unk0x1005c640.m_unk0x318[5][2] = 5402;
			break;
		case 5452:
			if (g_unk0x1005c640.m_unk0x318[5][3] > 0) {
				FUN_10007df0(g_unk0x1005c640.m_unk0x318[5][3]);
			}
			g_unk0x1005c640.m_unk0x318[5][3] = 5452;
			break;
		}
	}
	else {
		switch ((MechS32) p_tab->m_unk0x28) {
		case 5000:
			for (i = 1; i <= g_unk0x1005c640.m_unk0x7a4; i++) {
				FUN_10007e90(i + 5000);
			}
			g_unk0x1005c640.m_unk0x270 = 0;
			FUN_10008989();
			break;
		case 5401:
		case 5402:
		case 5451:
		case 5452:
			FUN_10007e90((MechS32) p_tab->m_unk0x28);
			break;
		}
	}
}

// Returns the first tab with a click callback that contains the point.
// Operand order: the original loads m_width before m_left; swapping them in the source didn't
// change it.
// FUNCTION: MW2SHELL 0x1000b5ed
ScreenField* FUN_1000b5ed(ScreenField* p_tabs, MechS32 p_x, MechS32 p_y)
{
	if (p_tabs) {
		while (p_tabs->m_left != -1) {
			if (p_tabs->m_unk0x20 != NULL && p_tabs->m_left <= p_x && p_x < p_tabs->m_left + p_tabs->m_width &&
				p_tabs->m_top <= p_y && p_y < p_tabs->m_top + p_tabs->m_height) {
				return p_tabs;
			}

			p_tabs++;
		}
	}

	return NULL;
}

// FUNCTION: MW2SHELL 0x1000b679
MechS32 FUN_1000b679(MechS32 p_id, MechS32 p_location, MechS32* p_slot)
{
	MechS32 i;

	for (i = 0; i < g_unk0x1007c820[p_location].m_unk0x24; i++) {
		if (g_unk0x1007c820[p_location].m_unk0x0c[i] == p_id) {
			if (p_slot != NULL) {
				*p_slot = i;
			}
			return TRUE;
		}
	}

	return FALSE;
}

// Stack-slot permutation: location and slot.
// FUNCTION: MW2SHELL 0x1000b6f4
MechS32 FUN_1000b6f4(MechS32 p_id, MechS32* p_location, MechS32* p_slot)
{
	MechS32 location;
	MechS32 slot;

	for (location = 0; location < 8; location++) {
		if (FUN_1000b679(p_id, location, &slot)) {
			if (p_location != NULL) {
				*p_location = location;
			}
			if (p_slot != NULL) {
				*p_slot = slot;
			}
			return TRUE;
		}
	}

	return FALSE;
}

// Saves the variant being edited to the .mek file p_name in the mek directory, creating the
// directory if needed. Returns 1, or 0 on failure.
// Not 100%: m_unk0x318[i][j] loads the index before the base in the two tests of the slot loop.
// FUNCTION: MW2SHELL 0x1000b771
MechS32 FUN_1000b771(MechChar* p_name)
{
	MechS32 ammo;
	MechS32 i;
	MechS32 j;
	MechS32 k;
	FILE* file;

	memset(&g_unk0x10079d20, 0, sizeof(g_unk0x10079d20));
	memset(g_unk0x1007c820, 0, sizeof(g_unk0x1007c820));
	memset(g_unk0x1007a7a8, 0, sizeof(g_unk0x1007a7a8));
	memset(g_unk0x10079c50, 0, sizeof(g_unk0x10079c50));
	memset(g_unk0x10079d48, 0, sizeof(g_unk0x10079d48));

	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] == -1) {
			break;
		}
		g_unk0x1007a7a8[i].m_unk0x00 = g_unk0x1005c640.m_unk0x2e8[i];
		g_unk0x1007a7a8[i].m_unk0x04 = -1;
	}
	g_unk0x10079d20.m_unk0x10 = i;

	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			break;
		}
		ammo = (g_unk0x1005c640.m_unk0x284[i] - 10000) / 10000;
		ammo = ammo * 100 + 10001;
		for (j = 0; j < 25; j++) {
			if (g_unk0x10079c50[j].m_unk0x00 == 0) {
				break;
			}
			if (g_unk0x10079c50[j].m_unk0x00 == ammo) {
				ammo++;
			}
		}
		g_unk0x10079c50[i].m_unk0x00 = ammo;
		g_unk0x10079c50[i].m_unk0x04 = (g_unk0x1005c640.m_unk0x284[i] - 10000) / 100;
	}
	g_unk0x10079d20.m_unk0x14 = i;

	for (i = 0; i < 8; i++) {
		g_unk0x1007c820[i].m_unk0x26 = 1;
		g_unk0x1007c820[i].m_unk0x24 = 12;
		for (j = 0; j < 12; j++) {
			if (g_unk0x1005c640.m_unk0x318[i][j] == -1) {
				g_unk0x1007c820[i].m_unk0x24 = 6;
				g_unk0x1007c820[i].m_unk0x0c[j] = 0;
			}
			else if (g_unk0x1005c640.m_unk0x318[i][j] < 10000) {
				g_unk0x1007c820[i].m_unk0x0c[j] = g_unk0x1005c640.m_unk0x318[i][j];
			}
			else {
				for (k = 0; k < 25; k++) {
					if (g_unk0x1005c640.m_unk0x318[i][j] == g_unk0x1005c640.m_unk0x284[k]) {
						g_unk0x1007c820[i].m_unk0x0c[j] = g_unk0x10079c50[k].m_unk0x00;
						break;
					}
				}
			}
		}
	}

	g_unk0x1007c820[0].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[0].m_unk0x00;
	g_unk0x1007c820[1].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[1].m_unk0x00;
	g_unk0x1007c820[2].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[2].m_unk0x00;
	g_unk0x1007c820[3].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[3].m_unk0x00;
	g_unk0x1007c820[4].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[4].m_unk0x00;
	g_unk0x1007c820[5].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[5].m_unk0x00;
	g_unk0x1007c820[6].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[6].m_unk0x00;
	g_unk0x1007c820[7].m_unk0x08 = (MechS16) g_unk0x1005c640.m_unk0x710[7].m_unk0x00;
	g_unk0x1007c820[0].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[0].m_unk0x08;
	g_unk0x1007c820[1].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[1].m_unk0x08;
	g_unk0x1007c820[2].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[2].m_unk0x08;
	g_unk0x1007c820[3].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[3].m_unk0x08;
	g_unk0x1007c820[4].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[4].m_unk0x08;
	g_unk0x1007c820[5].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[5].m_unk0x08;
	g_unk0x1007c820[6].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[6].m_unk0x08;
	g_unk0x1007c820[7].m_unk0x00 = (MechS16) g_unk0x1005c640.m_unk0x710[7].m_unk0x08;
	g_unk0x1007c820[1].m_unk0x04 = (MechS16) g_unk0x1005c640.m_unk0x710[1].m_unk0x0c;
	g_unk0x1007c820[2].m_unk0x04 = (MechS16) g_unk0x1005c640.m_unk0x710[2].m_unk0x0c;
	g_unk0x1007c820[3].m_unk0x04 = (MechS16) g_unk0x1005c640.m_unk0x710[3].m_unk0x0c;

	g_unk0x10079d20.m_unk0x00 = (g_unk0x1005c640.m_unk0x200 + 50) / 100;
	g_unk0x10079d20.m_unk0x04 = g_unk0x1005c640.m_unk0x278;
	g_unk0x10079d20.m_unk0x08 = g_unk0x1005c640.m_unk0x280;
	g_unk0x10079d20.m_unk0x0c = ((g_unk0x1005c640.m_unk0x22c + 50) / 100 + 10) * g_unk0x1005c640.m_unk0x228;
	strcpy(g_unk0x10079d48, g_unk0x1005c640.m_unk0x100);

	sprintf(g_unk0x1007cc60, "mek\\%s", p_name);
	file = fopen(g_unk0x1007cc60, "wb");
	if (file == NULL) {
		sprintf(g_unk0x1007cc60, "mek\\");
		if (!CreateDirectory(g_unk0x1007cc60, NULL)) {
			DebugPrint("CreateDirectory mek failed: %d\n", GetLastError());
			return 0;
		}

		sprintf(g_unk0x1007cc60, "mek\\%s", p_name);
		file = fopen(g_unk0x1007cc60, "wb");
		if (file == NULL) {
			return 0;
		}
	}

	fwrite(&g_unk0x10079d20, 0x18, 1, file);
	fwrite(g_unk0x1007c820, 0x28, 8, file);
	fwrite(g_unk0x1007a7a8, 8, g_unk0x10079d20.m_unk0x10, file);
	fwrite(g_unk0x10079c50, 8, g_unk0x10079d20.m_unk0x14, file);
	fwrite(g_unk0x10079d48, 0x32, 1, file);
	fclose(file);
	return 1;
}

// Returns the image of a .mek file: a user variant from the mek directory, or a standard one
// ("…std") from the project file.
// FUNCTION: MW2SHELL 0x1000bce5
void* FUN_1000bce5(MechChar* p_name)
{
	FILE* file;
	MechChar path[0x20];

	if (_strnicmp(p_name + 5, "std", 3)) {
		strcpy(path, "mek\\");
		strcat(path, p_name);
		if (strchr(p_name, '.') == NULL) {
			strcat(path, ".mek");
		}

		file = fopen(path, "rb");
		if (file == NULL) {
			return NULL;
		}

		fread(g_unk0x1007a820, 1, sizeof(g_unk0x1007a820), file);
		fclose(file);
		return g_unk0x1007a820;
	}

	return g_projectArchive->GetResourceByName(p_name, 6, "MEK");
}

// FUNCTION: MW2SHELL 0x1000be09
void FUN_1000be09(MechChar* p_name)
{
	if (!_strnicmp(p_name + 5, "std", 3)) {
		g_projectArchive->ReleaseResourceByName(p_name, 6, "MEK");
	}
}

// Copies p_size bytes and returns the source position after them.
// FUNCTION: MW2SHELL 0x1000be4d
undefined* FUN_1000be4d(undefined* p_dst, undefined* p_src, MechU32 p_size)
{
	memcpy(p_dst, p_src, p_size);
	return p_src + p_size;
}

// FUNCTION: MW2SHELL 0x1000be7a
void FUN_1000be7a(MechS32 p_location, MechS32 p_slot, MechS32 p_id)
{
	MechS32 count;
	MechS32 id;

	id = g_unk0x1005c640.m_unk0x318[p_location][p_slot];
	if (id) {
		count = FUN_10007b4d(id);
	}

	g_unk0x1005c640.m_unk0x318[p_location][p_slot] = p_id;
	if (!FUN_10007bee(p_location, id, count)) {
		FUN_10007d9e(id, count);
	}
}

// Loads the .mek file p_name into the variant being edited, keeping the previous variant in
// g_unk0x1005cde8, and derives the engine, armor and internal structure from it.
// Not 100%: the stack slots of the locals are permuted, and m_unk0x0c[j] of the crate loads the
// base before the index.
// FUNCTION: MW2SHELL 0x1000befe
void FUN_1000befe(MechChar* p_name)
{
	MechS32 id;
	undefined* data;
	MechS32 i;
	MechS32 j;
	MechS32 k;

	data = (undefined*) FUN_1000bce5(p_name);
	if (data == NULL) {
		return;
	}

	g_unk0x1005cde8 = g_unk0x1005c640;
	memset(&g_unk0x10079d20, 0, sizeof(g_unk0x10079d20));
	memset(g_unk0x1007c820, 0, sizeof(g_unk0x1007c820));
	memset(g_unk0x1007a7a8, 0, sizeof(g_unk0x1007a7a8));
	memset(g_unk0x10079c50, 0, sizeof(g_unk0x10079c50));
	memset(g_unk0x10079d48, 0, sizeof(g_unk0x10079d48));

	data = FUN_1000be4d((undefined*) &g_unk0x10079d20, data, 0x18);
	data = FUN_1000be4d((undefined*) g_unk0x1007c820, data, 0x140);
	FUN_1000be4d(
		(undefined*) g_unk0x1007a7a8,
		data,
		(g_unk0x10079d20.m_unk0x10 < 10 ? g_unk0x10079d20.m_unk0x10 : 10) * 8
	);
	data += g_unk0x10079d20.m_unk0x10 * 8;
	FUN_1000be4d(
		(undefined*) g_unk0x10079c50,
		data,
		(g_unk0x10079d20.m_unk0x14 < 25 ? g_unk0x10079d20.m_unk0x14 : 25) * 8
	);
	data += g_unk0x10079d20.m_unk0x14 * 8;
	data = FUN_1000be4d((undefined*) g_unk0x10079d48, data, 0x32);
	FUN_1000be09(p_name);

	memset(&g_unk0x1005c640, 0, sizeof(g_unk0x1005c640));
	for (i = 0; i < 78; i++) {
		g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = -1;
	}

	for (i = 0; i < 10; i++) {
		if (i < g_unk0x10079d20.m_unk0x10) {
			g_unk0x1005c640.m_unk0x2e8[i] = g_unk0x1007a7a8[i].m_unk0x00;
		}
		else {
			g_unk0x1005c640.m_unk0x2e8[i] = -1;
		}
	}

	for (i = 0; i < 25; i++) {
		g_unk0x1005c640.m_unk0x284[i] = -1;
	}
	for (i = 0; i < g_unk0x10079d20.m_unk0x14; i++) {
		FUN_1000819f(g_unk0x10079c50[i].m_unk0x04 * 100 + 10001);
		g_unk0x1005c640.m_unk0x268 += 100;
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 12; j++) {
			if (g_unk0x1007c820[i].m_unk0x24 > j) {
				id = g_unk0x1007c820[i].m_unk0x0c[j];
				if (id >= 10000) {
					for (k = 0; k < 25; k++) {
						if (g_unk0x10079c50[k].m_unk0x00 == id) {
							id = g_unk0x1005c640.m_unk0x284[k];
							break;
						}
					}
				}
				g_unk0x1005c640.m_unk0x318[i][j] = id;
			}
			else {
				g_unk0x1005c640.m_unk0x318[i][j] = -1;
			}
		}
	}

	strcpy(g_unk0x1005c640.m_unk0x100, g_unk0x10079d48);
	g_unk0x1005c640.m_unk0x200 = g_unk0x10079d20.m_unk0x00 * 100;
	g_unk0x1005c640.m_unk0x278 = g_unk0x10079d20.m_unk0x04;
	g_unk0x1005c640.m_unk0x280 = g_unk0x10079d20.m_unk0x08;
	g_unk0x1005c640.m_unk0x22c = (g_unk0x10079d20.m_unk0x0c - 10) * 100;
	g_unk0x1005c640.m_unk0x210 = (g_unk0x10079d20.m_unk0x04 * g_unk0x10079d20.m_unk0x00 * 5 + 4) / 5;
	if (g_unk0x1005c640.m_unk0x210 < 10) {
		g_unk0x1005c640.m_unk0x210 = 10;
	}
	if (g_unk0x1005c640.m_unk0x210 > 400) {
		g_unk0x1005c640.m_unk0x210 = 400;
	}
	g_unk0x1005c640.m_unk0x20c = (g_unk0x1005c640.m_unk0x210 - 10) / 5;
	if (FUN_1000b679(5850, 1, NULL)) {
		g_unk0x1005c640.m_unk0x20c += 10000;
	}

	if (FUN_1000b6f4(8001, NULL, NULL)) {
		g_unk0x1005c640.m_unk0x240 = 1;
	}
	if (FUN_1000b6f4(9001, NULL, NULL)) {
		g_unk0x1005c640.m_unk0x24c = 1;
	}

	g_unk0x1005c640.m_unk0x258 = 0;
	for (i = 0; i < 8; i++) {
		g_unk0x1005c640.m_unk0x258 += g_unk0x1007c820[i].m_unk0x04 + g_unk0x1007c820[i].m_unk0x00;
	}
	if (g_unk0x1005c640.m_unk0x24c == 1) {
		g_unk0x1005c640.m_unk0x250 = (MechS32) (g_unk0x1005c640.m_unk0x258 / 19.2 * 100.0);
	}
	else {
		g_unk0x1005c640.m_unk0x250 = (MechS32) (g_unk0x1005c640.m_unk0x258 / 16.0 * 100.0);
	}
	g_unk0x1005c640.m_unk0x250 = FUN_100078ae(g_unk0x1005c640.m_unk0x250 * 2) / 2;
	g_unk0x1005c640.m_unk0x25c = g_unk0x1005c640.m_unk0x258;

	i = (g_unk0x1005c640.m_unk0x200 / 100 - 10) / 5;
	g_unk0x1005c640.m_unk0x710[0].m_unk0x00 = 3;
	g_unk0x1005c640.m_unk0x710[1].m_unk0x00 = g_unk0x1005c510[i].m_unk0x04;
	g_unk0x1005c640.m_unk0x710[2].m_unk0x00 = g_unk0x1005c510[i].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[3].m_unk0x00 = g_unk0x1005c510[i].m_unk0x04;
	g_unk0x1005c640.m_unk0x710[4].m_unk0x00 = g_unk0x1005c510[i].m_unk0x08;
	g_unk0x1005c640.m_unk0x710[5].m_unk0x00 = g_unk0x1005c510[i].m_unk0x08;
	g_unk0x1005c640.m_unk0x710[6].m_unk0x00 = g_unk0x1005c510[i].m_unk0x0c;
	g_unk0x1005c640.m_unk0x710[7].m_unk0x00 = g_unk0x1005c510[i].m_unk0x0c;
	g_unk0x1005c640.m_unk0x710[0].m_unk0x04 = 9;
	g_unk0x1005c640.m_unk0x710[1].m_unk0x04 = g_unk0x1005c640.m_unk0x710[1].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[2].m_unk0x04 = g_unk0x1005c640.m_unk0x710[2].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[3].m_unk0x04 = g_unk0x1005c640.m_unk0x710[3].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[4].m_unk0x04 = g_unk0x1005c640.m_unk0x710[4].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[5].m_unk0x04 = g_unk0x1005c640.m_unk0x710[5].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[6].m_unk0x04 = g_unk0x1005c640.m_unk0x710[6].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[7].m_unk0x04 = g_unk0x1005c640.m_unk0x710[7].m_unk0x00 * 2;
	g_unk0x1005c640.m_unk0x710[0].m_unk0x08 = g_unk0x1007c820[0].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[1].m_unk0x08 = g_unk0x1007c820[1].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[2].m_unk0x08 = g_unk0x1007c820[2].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[3].m_unk0x08 = g_unk0x1007c820[3].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[4].m_unk0x08 = g_unk0x1007c820[4].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[5].m_unk0x08 = g_unk0x1007c820[5].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[6].m_unk0x08 = g_unk0x1007c820[6].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[7].m_unk0x08 = g_unk0x1007c820[7].m_unk0x00;
	g_unk0x1005c640.m_unk0x710[0].m_unk0x0c = -1;
	g_unk0x1005c640.m_unk0x710[1].m_unk0x0c = g_unk0x1007c820[1].m_unk0x04;
	g_unk0x1005c640.m_unk0x710[2].m_unk0x0c = g_unk0x1007c820[2].m_unk0x04;
	g_unk0x1005c640.m_unk0x710[3].m_unk0x0c = g_unk0x1007c820[3].m_unk0x04;
	g_unk0x1005c640.m_unk0x710[4].m_unk0x0c = -1;
	g_unk0x1005c640.m_unk0x710[5].m_unk0x0c = -1;
	g_unk0x1005c640.m_unk0x710[6].m_unk0x0c = -1;
	g_unk0x1005c640.m_unk0x710[7].m_unk0x0c = -1;
	g_unk0x1005c640.m_unk0x314 = 0;

	g_unk0x1005c640.m_unk0x790 = 0;
	g_unk0x1005c640.m_unk0x794 = 0;
	g_unk0x1005c640.m_unk0x798 = 0;
	g_unk0x1005c640.m_unk0x79c = 0;
	g_unk0x1005c640.m_unk0x7a0 = 0;
	g_unk0x1005c640.m_unk0x7a4 = (g_unk0x1005c640.m_unk0x200 / 25 + 50) / 100;
	if (FUN_1000b6f4(5001, NULL, NULL)) {
		g_unk0x1005c640.m_unk0x270 = g_unk0x1005c640.m_unk0x7a4 * 100;
		g_unk0x1005c640.m_unk0x790 = 1;
	}

	FUN_10007b4d(5301);
	FUN_10007b4d(5351);
	g_unk0x1005c640.m_unk0x794 = FUN_10007b4d(5401);
	g_unk0x1005c640.m_unk0x798 = FUN_10007b4d(5451);
	FUN_10007b4d(5302);
	FUN_10007b4d(5352);
	g_unk0x1005c640.m_unk0x79c = FUN_10007b4d(5402);
	g_unk0x1005c640.m_unk0x7a0 = FUN_10007b4d(5452);
	FUN_1000be7a(4, 0, 5301);
	FUN_1000be7a(4, 1, 5351);
	if (g_unk0x1005c640.m_unk0x794) {
		FUN_1000be7a(4, 2, 5401);
	}
	if (g_unk0x1005c640.m_unk0x798) {
		FUN_1000be7a(4, 3, 5451);
	}
	FUN_1000be7a(5, 0, 5302);
	FUN_1000be7a(5, 1, 5352);
	if (g_unk0x1005c640.m_unk0x79c) {
		FUN_1000be7a(5, 2, 5402);
	}
	if (g_unk0x1005c640.m_unk0x7a0) {
		FUN_1000be7a(5, 3, 5452);
	}

	FUN_1000884c();
	FUN_100088ec();
	FUN_100089a8();
	FUN_10008989();

	g_unk0x1005c640.m_unk0x228 = 0;
	if (FUN_1000b6f4(6001, &i, &j)) {
		g_unk0x1005c640.m_unk0x318[i][j] = 0;
		if (FUN_1000b6f4(6001, NULL, NULL)) {
			g_unk0x1005c640.m_unk0x228 = 2;
		}
		else {
			g_unk0x1005c640.m_unk0x228 = 1;
		}
		g_unk0x1005c640.m_unk0x318[i][j] = 6001;
	}

	if (g_unk0x1005c640.m_unk0x228 == 0) {
		if (g_unk0x1005c640.m_unk0x204 > g_unk0x1005c640.m_unk0x200 && g_unk0x1005c640.m_unk0x22c >= 1000 &&
			g_unk0x1005c640.m_unk0x22c / 2 % 100 == 0) {
			g_unk0x1005c640.m_unk0x228 = 2;
		}
		else {
			g_unk0x1005c640.m_unk0x228 = 1;
		}
	}

	if (g_unk0x1005c640.m_unk0x228 == 2) {
		g_unk0x1005c640.m_unk0x22c = g_unk0x1005c640.m_unk0x22c / 2 - 500;
		FUN_10008989();
	}

	g_unk0x1005c640.m_unk0x230 = -1;
	FUN_10008786();
}

// Lists the variants of a mech: the standard ones from the project file in 1-99 (and the name
// of the first in 0), the user's from the mek directory in 100-199.
// FUNCTION: MW2SHELL 0x1000c8b8
void LoadMechBuildList(MechChar* p_prefix)
{
	MechS32 i;
	HANDLE findFile;
	WIN32_FIND_DATA findData;

	memset(g_unk0x10079d80, 0, sizeof(g_unk0x10079d80));
	sprintf(g_unk0x1007a800, "%s%02dstd", p_prefix, 0);
	strcpy(g_unk0x10079d80[0], g_unk0x1007a800);

	for (i = 1; i < 100; i++) {
		sprintf(g_unk0x1007a800, "%s%02dstd", p_prefix, i);
		if (g_projectArchive->FindResourceId(g_unk0x1007a800, 6) >= 0) {
			strcpy(g_unk0x10079d80[i], g_unk0x1007a800);
		}
	}

	sprintf(g_unk0x1007a800, "mek\\%s??usr.mek", p_prefix);
	findFile = FindFirstFile(g_unk0x1007a800, &findData);
	if (findFile == INVALID_HANDLE_VALUE) {
		return;
	}

	for (;;) {
		i = (findData.cAlternateFileName[3] - '0') * 10 + findData.cAlternateFileName[4] - '0' + 100;
		strncpy(g_unk0x10079d80[i], findData.cAlternateFileName, 8);
		g_unk0x10079d80[i][8] = '\0';
		if (!FindNextFile(findFile, &findData)) {
			break;
		}
	}

	FindClose(findFile);
}

// FUNCTION: MW2SHELL 0x1000ca74
void FUN_1000ca74()
{
	sprintf(g_unk0x10079d38, g_unk0x10061748, g_unk0x10061560[g_unk0x1006176c].m_unk0x00);
	FUN_10016cc0(0x10, 0x40000000, 0x40000000);
	FUN_10017460(0x10, g_unk0x10079d38, g_unk0x10079a98, g_unk0x10079a9c, 0x88, 0xe);
	LoadMechBuildList(g_unk0x10061560[g_unk0x1006176c].m_unk0x04);
	FUN_1000befe(g_unk0x10079d80[g_unk0x1007cc80]);
	g_unk0x1005c640.m_unk0x00[0] = '~';
	strcpy(&g_unk0x1005c640.m_unk0x00[1], g_unk0x10061560[g_unk0x1006176c].m_unk0x0c);
}

// FUNCTION: MW2SHELL 0x1000cb4b
void FUN_1000cb4b()
{
	FUN_10016cc0(0x10, 0x40000000, 0x40000000);
}

// FUNCTION: MW2SHELL 0x1000cb6f
void FUN_1000cb6f()
{
	FUN_10016d27(0x10);
}

// FUNCTION: MW2SHELL 0x1000cb89
void FUN_1000cb89()
{
	FUN_10016cc0(0x10, 0x20, 0x20);
}

// Plays the selected mech's name. Stack-slot permutation: audioData and audioSize.
// FUNCTION: MW2SHELL 0x1000cba7
void FUN_1000cba7()
{
	void* audioData;
	MechS32 audioSize;

	if (g_unk0x10061560[g_unk0x1006176c].m_unk0x14 < 0) {
		return;
	}

	g_pDatabaseMw2->GetDBItem(g_unk0x10061560[g_unk0x1006176c].m_unk0x14, &audioData, &audioSize);
	if (g_unk0x10061770 != NULL) {
		g_unk0x10061770->Stop();
		delete g_unk0x10061770;
	}

	g_unk0x10061770 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	g_unk0x10061770->SetVolume(40);
	g_unk0x10061770->Start();
}

// Comparison operand order: the original loads g_unk0x10061728 into eax and compares
// g_unk0x1006176c with it, the reverse of ours; flipping the source didn't change it.
// FUNCTION: MW2SHELL 0x1000cce5
void FUN_1000cce5()
{
	g_unk0x1006176c++;
	if (g_unk0x1006176c >= g_unk0x10061728) {
		g_unk0x1006176c = 0;
	}

	g_unk0x1007cc80 = 0;
	FUN_1000cba7();
	FUN_1000ca74();
}

// FUNCTION: MW2SHELL 0x1000cd2a
void FUN_1000cd2a()
{
	g_unk0x1006176c--;
	if (g_unk0x1006176c < 0) {
		g_unk0x1006176c = g_unk0x10061728 - 1;
	}

	g_unk0x1007cc80 = 0;
	FUN_1000cba7();
	FUN_1000ca74();
}

// FUNCTION: MW2SHELL 0x1000cd65
void FUN_1000cd65()
{
	for (g_unk0x1007cc80++; g_unk0x1007cc80 < 200; g_unk0x1007cc80++) {
		if (g_unk0x10079d80[g_unk0x1007cc80][0]) {
			break;
		}
	}

	if (g_unk0x1007cc80 >= 200) {
		g_unk0x1007cc80 = 0;
	}

	FUN_1000befe(g_unk0x10079d80[g_unk0x1007cc80]);
	g_unk0x1005c640.m_unk0x00[0] = '~';
	strcpy(&g_unk0x1005c640.m_unk0x00[1], g_unk0x10061560[g_unk0x1006176c].m_unk0x0c);
	FUN_10007ac8(g_unk0x1005de28);
	g_unk0x1005de28 = NULL;
	FUN_100079f8(g_unk0x1005de2c);
}

// FUNCTION: MW2SHELL 0x1000ce50
void FUN_1000ce50()
{
	if (g_unk0x1007cc80 == 0) {
		g_unk0x1007cc80 = 200;
	}

	for (g_unk0x1007cc80--; g_unk0x1007cc80 >= 0; g_unk0x1007cc80--) {
		if (g_unk0x10079d80[g_unk0x1007cc80][0]) {
			break;
		}
	}

	if (g_unk0x1007cc80 < 0) {
		g_unk0x1007cc80 = 0;
	}

	FUN_1000befe(g_unk0x10079d80[g_unk0x1007cc80]);
	g_unk0x1005c640.m_unk0x00[0] = '~';
	strcpy(&g_unk0x1005c640.m_unk0x00[1], g_unk0x10061560[g_unk0x1006176c].m_unk0x0c);
	FUN_10007ac8(g_unk0x1005de28);
	g_unk0x1005de28 = NULL;
	FUN_100079f8(g_unk0x1005de2c);
}

// Saves the variant under the first free user slot. Returns FALSE when the variant is invalid.
// FUNCTION: MW2SHELL 0x1000cf4c
MechS32 FUN_1000cf4c()
{
	MechS32 i;

	if (g_unk0x1005c640.m_unk0x4a0[0].m_unk0x00 != -1) {
		ShowDialog("Invalid 'Mech specification:|Unassigned criticals detected.#Ok", 0);
		return FALSE;
	}

	if (g_unk0x1005c640.m_unk0x204 > g_unk0x1005c640.m_unk0x200) {
		ShowDialog("Invalid 'Mech specification:|Chassis can not support|current mass.#Ok", 0);
		return FALSE;
	}

	for (i = 100; i < 200; i++) {
		if (!g_unk0x10079d80[i][0]) {
			break;
		}
	}

	if (i >= 200) {
		ShowDialog("Error: Too many mechs|of this variant to save.#Ok", 0);
		return TRUE;
	}

	sprintf(g_unk0x10079aa8, "%s%02dusr.mek", g_unk0x10061560[g_unk0x1006176c].m_unk0x04, i - 100);
	strcpy(g_unk0x10079d80[i], g_unk0x10079aa8);
	strncpy(g_unk0x10079d80[i], g_unk0x10079aa8, 8);
	g_unk0x10079d80[i][8] = '\0';
	if (!FUN_1000b771(g_unk0x10079aa8)) {
		ShowDialog("Error saving 'Mech.#Ok", 0);
		return TRUE;
	}

	g_unk0x1007cc80 = i;
	return TRUE;
}

// The mech bay's field tables: the bay's own (g_unk0x10060d20), the customize screen's
// (g_unk0x10060698) and the ones its components switch to on the right.
// A field's m_top: a packed row and offset below the previous field (see ScreenField).
#define MB_ROW(row, offset) ((MechS32) (0x80000000 | ((row) << 4) | (offset)))
#define MB_TAB(left, top, width, height, colors, draw, click, data, next)                                              \
	{left, top, width, height, 0, colors, NULL, draw, click, (void*) (data), next}
#define MB_END {-1, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL}

extern ScreenField g_unk0x1005de80[];
extern ScreenField g_unk0x1005e140[];
extern ScreenField g_unk0x1005e2f8[];
extern ScreenField g_unk0x1005e458[];
extern ScreenField g_unk0x1005e820[];
extern ScreenField g_unk0x1005efe0[];
extern ScreenField g_unk0x1005f248[];
extern ScreenField g_unk0x1005fc70[];
extern ScreenField g_unk0x10060698[];
extern ScreenField g_unk0x10060d20[];

// GLOBAL: MW2SHELL 0x1005de80
ScreenField g_unk0x1005de80[] = {
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "ENGINE", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Rating", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008dcc, NULL, &g_unk0x1005c640.m_unk0x20c, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Type", NULL),
	MB_TAB(544, MB_ROW(0, 0), 50, -1, g_unk0x1007ca60, FUN_10008e4f, FUN_1000a668, &g_unk0x1005c640.m_unk0x20c, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Manufactur", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008eaa, NULL, &g_unk0x1005c640.m_unk0x20c, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Mass", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x214, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Walking Speed", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008f14, NULL, &g_unk0x1005c640.m_unk0x278, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Running Speed", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008f14, NULL, &g_unk0x1005c640.m_unk0x27c, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a4f0, "FASTER", NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a5a3, "SLOWER", NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005e140
ScreenField g_unk0x1005e140[] = {
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "HEAT SINKS", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Count", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008fdd, NULL, &g_unk0x1005c640.m_unk0x228, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Type", NULL),
	MB_TAB(544, MB_ROW(0, 0), 50, -1, g_unk0x1007ca60, FUN_10009076, FUN_1000a8b0, &g_unk0x1005c640.m_unk0x228, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Mass", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x22c, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a84d, "ADD", NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a878, "DELETE", NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005e2f8
ScreenField g_unk0x1005e2f8[] = {
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "JUMP JETS", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Count", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x280, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Mass", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x238, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a7ae, "ADD", NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a803, "DELETE", NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005e458
ScreenField g_unk0x1005e458[] = {
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "INTERNAL STRUCTURE", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Type", NULL),
	MB_TAB(544, MB_ROW(0, 0), 50, -1, g_unk0x1007ca60, FUN_1000922c, FUN_1000aac2, &g_unk0x1005c640.m_unk0x240, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Mass", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x244, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Head", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[0], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Right Torso", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[1], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Center Torso", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[2], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Left Torso", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[3], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Right Arm", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[4], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Left Arm", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[5], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Right Leg", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[6], NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Left Leg", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x710[7], NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005e820
ScreenField g_unk0x1005e820[] = {
	MB_TAB(0, 0, -1, -1, g_unk0x1007cb60, FUN_10008bce, NULL, 0, NULL),
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "ARMOR", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Factor", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x258, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Allocated", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x25c, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Mass", NULL),
	MB_TAB(544, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x250, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Type", NULL),
	MB_TAB(544, MB_ROW(0, 0), 100, -1, g_unk0x1007ca60, FUN_10009284, FUN_1000a9b8, &g_unk0x1005c640.m_unk0x24c, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a955, "ADD", NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a980, "DELETE", NULL),
	MB_TAB(444, MB_ROW(3, 3), 100, -1, g_unk0x1007ca60, FUN_100094ba, FUN_1000af87, &g_unk0x1005c640.m_unk0x314, NULL),
	MB_TAB(554, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100092dc, NULL, 0, NULL),
	MB_TAB(584, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100092dc, NULL, 1, NULL),
	MB_TAB(551, 168, -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000b166, "", NULL),
	MB_TAB(581, MB_ROW(0, 0), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000b284, "", NULL),
	MB_TAB(551, 202, -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000b239, "", NULL),
	MB_TAB(581, MB_ROW(0, 0), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000b339, "", NULL),
	MB_TAB(224, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "ARMOR ALLOCATION", NULL),
	MB_TAB(224, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 0, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 0, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 1, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 1, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 2, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 2, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 3, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 3, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 4, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 4, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 5, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 5, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 6, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 6, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000943f, NULL, 7, NULL),
	MB_TAB(344, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100093a7, NULL, 7, NULL),
	MB_TAB(291, 223, 100, 34, g_unk0x1007cb60, NULL, FUN_1000afc9, 0, NULL),
	MB_TAB(262, 258, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 1, NULL),
	MB_TAB(301, 258, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 2, NULL),
	MB_TAB(342, 258, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 3, NULL),
	MB_TAB(226, 256, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 4, NULL),
	MB_TAB(378, 256, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 5, NULL),
	MB_TAB(255, 362, 60, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 6, NULL),
	MB_TAB(325, 362, 60, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 7, NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005efe0
ScreenField g_unk0x1005efe0[] = {
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "EQUIPMENT", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Yes", NULL),
	MB_TAB(474, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "CASE", NULL),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		100,
		-1,
		g_unk0x1007ca60,
		FUN_1000a1b0,
		FUN_1000b384,
		&g_unk0x1005c640.m_unk0x790,
		(ScreenField*) 5000
	),
	MB_TAB(474, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000a208, NULL, 5000, NULL),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		100,
		-1,
		g_unk0x1007ca60,
		FUN_1000a1b0,
		FUN_1000b384,
		&g_unk0x1005c640.m_unk0x794,
		(ScreenField*) 5401
	),
	MB_TAB(474, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000a208, NULL, 5401, NULL),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		100,
		-1,
		g_unk0x1007ca60,
		FUN_1000a1b0,
		FUN_1000b384,
		&g_unk0x1005c640.m_unk0x798,
		(ScreenField*) 5451
	),
	MB_TAB(474, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000a208, NULL, 5451, NULL),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		100,
		-1,
		g_unk0x1007ca60,
		FUN_1000a1b0,
		FUN_1000b384,
		&g_unk0x1005c640.m_unk0x79c,
		(ScreenField*) 5402
	),
	MB_TAB(474, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000a208, NULL, 5402, NULL),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		100,
		-1,
		g_unk0x1007ca60,
		FUN_1000a1b0,
		FUN_1000b384,
		&g_unk0x1005c640.m_unk0x7a0,
		(ScreenField*) 5452
	),
	MB_TAB(474, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000a208, NULL, 5452, NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005f248
ScreenField g_unk0x1005f248[] = {
	MB_TAB(224, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "WEAPONS AND AMMO", NULL),
	MB_TAB(
		224,
		MB_ROW(2, 2),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[0],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[1],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[2],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[3],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[4],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[5],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[6],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[7],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[8],
		NULL
	),
	MB_TAB(
		224,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[9],
		NULL
	),
	MB_TAB(224, MB_ROW(2, 2), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000abe8, "ADD WEAPON", NULL),
	MB_TAB(324, MB_ROW(0, 0), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000ad3f, "ADD AMMO", NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000acc4, "DELETE WEAPON", NULL),
	MB_TAB(324, MB_ROW(0, 0), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000adf9, "DELETE AMMO", NULL),
	MB_TAB(224, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "WEAPON INFO", NULL),
	MB_TAB(224, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Type", NULL),
	MB_TAB(264, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 0, NULL),
	MB_TAB(224, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Heat", NULL),
	MB_TAB(274, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 1, NULL),
	MB_TAB(324, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Mass", NULL),
	MB_TAB(374, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 7, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Damage", NULL),
	MB_TAB(274, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 2, NULL),
	MB_TAB(324, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Crit", NULL),
	MB_TAB(374, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 8, NULL),
	MB_TAB(224, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Range", NULL),
	MB_TAB(274, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 6, NULL),
	MB_TAB(324, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Ammo", NULL),
	MB_TAB(374, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100097a0, NULL, 9, NULL),
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "WEAPONS TABLE", NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 22, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 23, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 24, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 21, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 25, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 26, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 27, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 11, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 12, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 13, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 14, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 15, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 10, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 16, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 17, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 18, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 19, NULL),
	MB_TAB(444, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 6, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 5, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 4, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 9, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 8, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 7, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 3, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 2, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 1, NULL),
	MB_TAB(444, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_100096bd, FUN_1000af16, 0, NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x1005fc70
ScreenField g_unk0x1005fc70[] = {
	MB_TAB(0, 0, -1, -1, g_unk0x1007cb60, FUN_10008bce, NULL, 0, NULL),
	MB_TAB(224, 64, 100, -1, g_unk0x1007ca60, FUN_10009d50, FUN_1000af87, &g_unk0x1005c640.m_unk0x314, NULL),
	MB_TAB(224, MB_ROW(1, 5), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 0, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 1, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 2, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 3, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 4, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 5, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 6, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 7, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 8, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 9, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 10, NULL),
	MB_TAB(224, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009da9, FUN_1000b0dc, 11, NULL),
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "UNASSIGNED CRITICALS", NULL),
	MB_TAB(444, MB_ROW(2, 2), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 0, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 1, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 2, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 3, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 4, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 5, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 6, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 7, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 8, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 9, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 10, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 11, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 12, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 13, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 14, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 15, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 16, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 17, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 18, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 19, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 20, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 21, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 22, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 23, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 24, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 25, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 26, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 27, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 28, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 29, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 30, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 31, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 32, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_10009f92, FUN_1000afef, 33, NULL),
	MB_TAB(444, MB_ROW(1, 1), 100, -1, g_unk0x1007cb60, FUN_1000a161, FUN_1000b0c2, 34, NULL),
	MB_TAB(291, 223, 100, 34, g_unk0x1007cb60, NULL, FUN_1000afc9, 0, NULL),
	MB_TAB(262, 258, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 1, NULL),
	MB_TAB(301, 258, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 2, NULL),
	MB_TAB(342, 258, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 3, NULL),
	MB_TAB(226, 256, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 4, NULL),
	MB_TAB(378, 256, 35, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 5, NULL),
	MB_TAB(255, 362, 60, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 6, NULL),
	MB_TAB(325, 362, 60, 100, g_unk0x1007cb60, NULL, FUN_1000afc9, 7, NULL),
	MB_END,
};

// GLOBAL: MW2SHELL 0x10060698
ScreenField g_unk0x10060698[] = {
	MB_TAB(320, 4, -1, -1, g_unk0x1007cb60, FUN_100091dc, NULL, "~CUSTOMIZING", NULL),
	MB_TAB(320, 28, -1, -1, g_unk0x1007cb60, FUN_100091dc, NULL, g_unk0x1005c640.m_unk0x00, NULL),
	MB_TAB(20, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Variant:", NULL),
	MB_TAB(65, 64, 130, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a43b, g_unk0x1005c640.m_unk0x100, NULL),
	MB_TAB(20, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "COMPONENT", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "MASS", NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "NOTES", NULL),
	MB_TAB(20, MB_ROW(2, 2), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a2eb, "Engine", g_unk0x1005de80),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x214, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008dcc, NULL, &g_unk0x1005c640.m_unk0x20c, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Gyro", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x220, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Cockpit", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x224, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a2eb, "Heat Sinks", g_unk0x1005e140),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x22c, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008fdd, NULL, &g_unk0x1005c640.m_unk0x228, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a2eb, "Jump Jets", g_unk0x1005e2f8),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x238, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x280, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a2eb, "Internal", g_unk0x1005e458),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x244, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000922c, NULL, &g_unk0x1005c640.m_unk0x240, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a3b5, "Armor", g_unk0x1005e820),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x250, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10009284, NULL, &g_unk0x1005c640.m_unk0x24c, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a36d, "Weapons", g_unk0x1005f248),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x260, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a36d, "Ammo", g_unk0x1005f248),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x268, NULL),
	MB_TAB(20, MB_ROW(1, 1), 100, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a2eb, "Equipment", g_unk0x1005efe0),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x270, NULL),
	MB_TAB(20, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Used Mass", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008cb0, NULL, &g_unk0x1005c640.m_unk0x204, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Max Mass", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x200, NULL),
	MB_TAB(20, MB_ROW(2, 2), -1, -1, g_unk0x1007ca60, FUN_1000918c, FUN_1000a3f3, "Assign Criticals", g_unk0x1005fc70),
	MB_END,
};

// GLOBAL: MW2SHELL 0x10060d20
ScreenField g_unk0x10060d20[] = {
	MB_TAB(320, 4, -1, -1, g_unk0x1007cb60, FUN_100091dc, NULL, g_unk0x10079ad0, NULL),
	MB_TAB(320, 28, -1, -1, g_unk0x1007cb60, FUN_100091dc, NULL, g_unk0x1005c640.m_unk0x00, NULL),
	MB_TAB(20, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Variant:", NULL),
	MB_TAB(65, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, g_unk0x1005c640.m_unk0x100, NULL),
	MB_TAB(20, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "COMPONENT", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "MASS", NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "NOTES", NULL),
	MB_TAB(20, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Engine", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x214, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008dcc, NULL, &g_unk0x1005c640.m_unk0x20c, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Gyro", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x220, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Cockpit", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x224, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Heat Sinks", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x22c, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008fdd, NULL, &g_unk0x1005c640.m_unk0x228, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Jump Jets", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x238, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_100090ce, NULL, &g_unk0x1005c640.m_unk0x280, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Internal", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x244, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_1000922c, NULL, &g_unk0x1005c640.m_unk0x240, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Armor", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x250, NULL),
	MB_TAB(147, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10009284, NULL, &g_unk0x1005c640.m_unk0x24c, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Weapons", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x260, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Ammo", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x268, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Equipment", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x270, NULL),
	MB_TAB(20, MB_ROW(2, 2), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Used Mass", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008cb0, NULL, &g_unk0x1005c640.m_unk0x204, NULL),
	MB_TAB(20, MB_ROW(1, 1), -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "Max Mass", NULL),
	MB_TAB(98, MB_ROW(0, 0), -1, -1, g_unk0x1007cb60, FUN_10008c30, NULL, &g_unk0x1005c640.m_unk0x200, NULL),
	MB_TAB(444, 64, -1, -1, g_unk0x1007cb60, FUN_1000918c, NULL, "WEAPONS AND AMMO", NULL),
	MB_TAB(
		444,
		MB_ROW(2, 2),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[0],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[1],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[2],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[3],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[4],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[5],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[6],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[7],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[8],
		NULL
	),
	MB_TAB(
		444,
		MB_ROW(1, 1),
		130,
		-1,
		g_unk0x1007cb60,
		FUN_10009537,
		FUN_1000aeaa,
		&g_unk0x1005c640.m_unk0x2e8[9],
		NULL
	),
	MB_END,
};

#undef MB_ROW
#undef MB_TAB
#undef MB_END

void MechbayClickCallback(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar** p_scenario, MechS32 p_msg);

// Opens the mech bay: its sounds, color maps, menu and videos, the chassis and variant of the
// current star's selected mech, and its fields.
// Not 100%: the stack slots of data, i and size are permuted.
// FUNCTION: MW2SHELL 0x1000d0d4
void FUN_1000d0d4(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam)
{
	CustomStar* star;
	void* data;
	MechS32 i;
	MechChar* variant;
	MechS32 size;

	g_unk0x1007a7f8 = p_wParam;
	if (g_unk0x10061774 && p_wParam == 0x413) {
		g_unk0x10079aa0 = 1;
	}
	else {
		g_unk0x10079aa0 = 0;
	}
	g_unk0x10061774 = 0;
	g_unk0x10061770 = NULL;

	g_pDatabaseMw2->GetDBItem(0x50, &data, &size);
	g_unk0x1005de78 = new AudioSample(g_pAudioSubsystem, data, size);
	g_unk0x1005de78->SetVolume(0x28);
	p_database->GetDBItem(0x4b, &data, &size);
	g_unk0x1006177c = new AudioSample(g_pAudioSubsystem, data, size);
	g_unk0x1006177c->EnableLoop();

	g_unk0x1007cb60[0] = 0xff;
	g_unk0x1007ca60[0] = 0xff;
	g_unk0x1007c960[0] = 0xff;
	for (i = 1; i < 0x100; i++) {
		g_unk0x1007cb60[i] = g_unk0x1007ca60[i] = g_unk0x1007c960[i] = i;
	}
	g_unk0x1007cb60[1] = 6;
	g_unk0x1007ca60[1] = 1;
	g_unk0x1007c960[1] = 8;

	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006fff0[p_campaign].m_picture);
	g_unk0x10061778 = new ButtonMenu(
		g_pVideoDriver,
		g_unk0x1007120c,
		FALSE,
		g_unk0x1006fff0[p_campaign].m_buttons,
		g_unk0x1006fff0[p_campaign].m_count
	);
	g_unk0x10061778->FUN_10048d65(8);
	g_unk0x10061778->FUN_10048d65(9);
	if (g_unk0x10079aa0) {
		g_unk0x10061778->FUN_10048d65(0);
		g_unk0x10061778->FUN_10048d65(6);
		g_unk0x10061778->FUN_10048d65(7);
		g_unk0x10061778->FUN_10048d65(10);
	}
	else {
		if (g_unk0x1006176c >= 15) {
			g_unk0x10061778->FUN_10048d65(6);
		}
		if (g_unk0x1007cc80 < 100) {
			g_unk0x10061778->FUN_10048d65(10);
		}
	}
	FUN_10016f45();

	switch (p_campaign) {
	case 0:
		FUN_10017460(0, "awogrid", 0x8c, 0x136, 0x4a, 0);
		FUN_10017460(1, "wwomp1", 0xc, 0x34, 0x42, 0);
		FUN_10017460(10, "wwomp4ar", 0xd8, 0x34, 0x64, 0);
		FUN_10017460(11, "wwomp7cr", 0x1b4, 0x34, 0x64, 0);
		FUN_10017460(12, "wwomp4mp", 0x4b, 0xab, 0x21, 0);
		FUN_10017460(13, "wwomp5wa", 0xd8, 0x34, 0x64, 0);
		FUN_10017460(14, "wwomp1es", 0x1b4, 0x34, 0x42, 0);
		FUN_10017460(4, "wwobkg", 0xdb, 0x1ad, 2, 0);
		FUN_10017460(5, "wwostar", 0x197, 0x19a, 0x4a, 0);
		FUN_10017460(6, "wwocn", 0x122, 0x1b1, 0x64, 0);
		FUN_10017460(7, "wwocp", 0xf0, 0x1b3, 0x64, 0);
		FUN_10017460(8, "wwovn", 0x12c, 0x1ad, 0x64, 0);
		FUN_10017460(9, "wwovp", 0xdc, 0x1b0, 0x64, 0);
		g_unk0x10061748 = g_unk0x10061730;
		g_unk0x10079a98 = 0x146;
		g_unk0x10079a9c = 0x17f;
		g_unk0x1007c960[1] = 0x17;
		break;
	case 1:
		FUN_10017460(0, "ajfgrid", 0x6c, 0x132, 0x4a, 0);
		FUN_10017460(1, "wjfmp1", 0xc, 0x34, 0x42, 0);
		FUN_10017460(10, "wjfmp4ar", 0xd8, 0x34, 0x64, 0);
		FUN_10017460(11, "wjfmp7cr", 0x1b4, 0x34, 0x64, 0);
		FUN_10017460(12, "wjfmp4mp", 0x4b, 0xab, 0x21, 0);
		FUN_10017460(13, "wjfmp5wa", 0xd8, 0x34, 0x64, 0);
		FUN_10017460(14, "wjfmp1es", 0x1b4, 0x34, 0x42, 0);
		FUN_10017460(4, "wjfbkg", 0xd8, 0x1b2, 2, 0);
		FUN_10017460(5, "wjfstar", 0x171, 0x1a0, 8, 0);
		FUN_10017460(6, "wjfcn", 0x11e, 0x1b6, 0x64, 0);
		FUN_10017460(7, "wjfcp", 0xf2, 0x1b6, 0x64, 0);
		FUN_10017460(8, "wjfvn", 0x128, 0x1b2, 0x64, 0);
		FUN_10017460(9, "wjfvp", 0xdc, 0x1b5, 0x64, 0);
		g_unk0x10061748 = g_unk0x10061738;
		g_unk0x10079a98 = 0x146;
		g_unk0x10079a9c = 0x17f;
		g_unk0x1007c960[1] = 0x17;
		break;
	case 2:
		FUN_10017460(0, "aiagrid", 0x6c, 0x132, 0x4a, 0);
		FUN_10017460(1, "wiamp1", 0xc, 0x34, 0x42, 0);
		FUN_10017460(10, "wiamp4ar", 0xd8, 0x34, 0x64, 0);
		FUN_10017460(11, "wiamp7cr", 0x1b4, 0x34, 0x64, 0);
		FUN_10017460(12, "wiamp4mp", 0x4b, 0xab, 0x21, 0);
		FUN_10017460(13, "wiamp5wa", 0xd8, 0x34, 0x64, 0);
		FUN_10017460(14, "wiamp1es", 0x1b4, 0x34, 0x42, 0);
		FUN_10017460(4, "wiabkg1", 0xdb, 0x19e, 4, 0);
		FUN_10017460(5, "wiastar", 0x19d, 0x1a1, 0x24, 0);
		FUN_10017460(6, "wiacn", 0x109, 0x1ae, 0x24, 0);
		FUN_10017460(7, "wiacp", 0xee, 0x1b5, 0x24, 0);
		FUN_10017460(8, "wiavn", 0x12e, 0x1af, 0x24, 0);
		FUN_10017460(9, "wiavp", 0xdf, 0x1b5, 0x24, 0);
		g_unk0x10061748 = g_unk0x10061740;
		g_unk0x10079a98 = 0x136;
		g_unk0x10079a9c = 0x17f;
		g_unk0x1007c960[1] = 0xca;
		break;
	}

	if (!g_unk0x10079aa0) {
		FUN_10002de7(0, NULL, NULL);
	}

	if (p_campaign == 2) {
		g_unk0x10061728 = FUN_100381c2();
	}
	else {
		g_unk0x10061728 = 15;
	}

	g_unk0x1007cc80 = 0;
	g_unk0x1006176c = FUN_1000307c(-1);
	if (g_unk0x1006176c >= 0) {
		variant = FUN_10003013(-1);
		g_unk0x1007cc80 = (variant[3] - '0') * 10 + variant[4] - '0';
		if (_strnicmp(variant + 5, "std", 3)) {
			g_unk0x1007cc80 += 100;
		}
	}
	else {
		g_unk0x1006176c = 0;
	}
	if (g_unk0x1006176c >= g_unk0x10061728) {
		g_unk0x1006176c = 0;
	}

	g_pDatabaseMw2->GetDBItem(0x65, &data, &size);
	g_unk0x10079d18 = new AudioSample(g_pAudioSubsystem, data, size);
	g_unk0x10079d18->SetVolume(0x32);
	g_pDatabaseMw2->GetDBItem(0x66, &data, &size);
	g_unk0x10079ac8 = new AudioSample(g_pAudioSubsystem, data, size);
	g_unk0x10079ac8->SetVolume(0x32);

	star = FUN_1000312e(-1);
	sprintf(
		g_unk0x10079ad0,
		"~CALLSIGN: %s (%d.00 T MAX)",
		star->m_unk0x14[star->m_unk0x04].m_unk0x14,
		star->m_unk0x10
	);
	FUN_1000cba7();
	FUN_1000ca74();
	g_unk0x1005de2c = g_unk0x10060d20;
	FUN_100078cd(g_unk0x1005de2c);
	g_unk0x10061780 = p_wParam;
	FUN_100108e5(MechbayClickCallback);
	FUN_1001661b();
}

// The mech bay's frame: the fields' clicks and the menu (EXIT LAB, STAR CONFIG, the chassis and
// variant arrows, CUSTOMIZE, ACCEPT MECH, SAVE, ABORT and DELETE).
// Not 100%: the stack slots of the locals and the delete temporaries are permuted.
// FUNCTION: MW2SHELL 0x1000db35
void MechbayClickCallback(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg)
{
	MechS32 pressed;
	MechS32 i;
	MechS32 button;
	ScreenField* tab;

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_fQuickTips && !g_unk0x10061784 && !g_unk0x10061770->IsPlaying()) {
		DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x71), g_pWnd, (DLGPROC) FUN_1001067f, 0);
		g_unk0x10061784 = 1;
	}

	pressed = g_pMouseState->GetLeftPressed();
	if (pressed == 1) {
		tab = FUN_1000b5ed(g_unk0x1005de2c, g_pMouseState->m_x, g_pMouseState->m_y);
		if (tab && tab->m_unk0x20) {
			tab->m_unk0x20(tab);
		}
		else if (g_unk0x1005de28) {
			tab = FUN_1000b5ed(g_unk0x1005de28, g_pMouseState->m_x, g_pMouseState->m_y);
			if (tab && tab->m_unk0x20) {
				tab->m_unk0x20(tab);
				FUN_100079f8(g_unk0x1005de28);
				FUN_100079f8(g_unk0x1005de2c);
			}
		}
	}

	button = g_unk0x10061778->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
	if (*p_campaign == 2) {
		FUN_10016cc0(5, 0x20, 0x20);
	}
	else {
		FUN_10016cc0(5, 1, 1);
	}
	FUN_10016cc0(6, 0x20, 0x20);
	FUN_10016cc0(7, 0x20, 0x20);
	FUN_10016cc0(8, 0x20, 0x20);
	FUN_10016cc0(9, 0x20, 0x20);

	switch (button) {
	case 0:
		if (pressed != 1) {
			break;
		}
		p_msg = 0x411;
		break;
	case 1:
		if (*p_campaign == 2) {
			FUN_10016d27(5);
		}
		else {
			FUN_10016cc0(5, 1, 0);
		}
		if (pressed != 1) {
			break;
		}
		g_unk0x10079ac8->Start();
		if (g_unk0x10079aa0) {
			if (!FUN_10002de7(-1, g_unk0x10079d80[g_unk0x1007cc80], NULL)) {
				ShowDialog("'Mech exceeds|Keshik Defined Maximum Tonnage (KDMT)|for mission.#Ok", 0);
			}
			else {
				p_msg = 0x413;
			}
		}
		else {
			p_msg = 0x413;
		}
		break;
	case 2:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(8);
		}
		if (pressed != 1) {
			break;
		}
		FUN_1000cce5();
		FUN_10007ac8(g_unk0x1005de28);
		g_unk0x1005de28 = NULL;
		FUN_100079f8(g_unk0x1005de2c);
		if (g_unk0x1006176c >= 15) {
			g_unk0x10061778->FUN_10048d65(6);
		}
		else if (!g_unk0x10079aa0) {
			g_unk0x10061778->FUN_10048cc1(6);
		}
		g_unk0x10061778->FUN_10048d65(10);
		break;
	case 3:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(9);
		}
		if (pressed != 1) {
			break;
		}
		FUN_1000cd2a();
		FUN_10007ac8(g_unk0x1005de28);
		g_unk0x1005de28 = NULL;
		FUN_100079f8(g_unk0x1005de2c);
		if (g_unk0x1006176c >= 15) {
			g_unk0x10061778->FUN_10048d65(6);
		}
		else if (!g_unk0x10079aa0) {
			g_unk0x10061778->FUN_10048cc1(6);
		}
		g_unk0x10061778->FUN_10048d65(10);
		break;
	case 4:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(6);
		}
		if (pressed != 1) {
			break;
		}
		g_unk0x10079d18->Start();
		FUN_1000cd65();
		if (g_unk0x1007cc80 < 100) {
			g_unk0x10061778->FUN_10048d65(10);
		}
		else {
			g_unk0x10061778->FUN_10048cc1(10);
		}
		break;
	case 5:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(7);
		}
		if (pressed != 1) {
			break;
		}
		g_unk0x10079d18->Start();
		FUN_1000ce50();
		if (g_unk0x1007cc80 < 100) {
			g_unk0x10061778->FUN_10048d65(10);
		}
		else {
			g_unk0x10061778->FUN_10048cc1(10);
		}
		break;
	case 10:
		if (pressed != 1) {
			break;
		}
		if (ShowDialog("Delete this 'Mech?|Are you sure?#Yes|No", 1) == 1) {
			break;
		}
		sprintf(g_szTempBuffer, "mek\\%s.mek", g_unk0x10079d80[g_unk0x1007cc80]);
		remove(g_szTempBuffer);
		g_unk0x10079d80[g_unk0x1007cc80][0] = '\0';
		FUN_1000cd65();
		if (g_unk0x1007cc80 < 100) {
			g_unk0x10061778->FUN_10048d65(10);
		}
		else {
			g_unk0x10061778->FUN_10048cc1(10);
		}
		break;
	case 6:
		if (pressed != 1) {
			break;
		}
		for (i = 100; i < 200; i++) {
			if (!g_unk0x10079d80[i][0]) {
				break;
			}
		}
		if (i >= 200) {
			ShowDialog("Error: Too many mechs|of this variant to save.#Ok", 0);
			break;
		}
		sprintf(g_unk0x1005c640.m_unk0x100, "User Variant #%d", i - 99);
		FUN_10016cc0(0, 1, 1);
		FUN_1000cb4b();
		FUN_10007ac8(g_unk0x1005de2c);
		g_unk0x1005de2c = g_unk0x10060698;
		FUN_100078cd(g_unk0x1005de2c);
		FUN_10007ac8(g_unk0x1005de28);
		g_unk0x1005de28 = g_unk0x1005de80;
		FUN_100078cd(g_unk0x1005de28);
		g_unk0x10061778->FUN_10048d65(0);
		g_unk0x10061778->FUN_10048d65(1);
		g_unk0x10061778->FUN_10048d65(2);
		g_unk0x10061778->FUN_10048d65(3);
		g_unk0x10061778->FUN_10048d65(4);
		g_unk0x10061778->FUN_10048d65(5);
		g_unk0x10061778->FUN_10048d65(6);
		g_unk0x10061778->FUN_10048d65(7);
		g_unk0x10061778->FUN_10048d65(10);
		g_unk0x10061778->FUN_10048cc1(8);
		g_unk0x10061778->FUN_10048cc1(9);
		if (g_fQuickTips && !g_unk0x10061788) {
			g_pVideoDriver->DrawShell();
			DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x72), g_pWnd, (DLGPROC) FUN_1001067f, 0);
			g_unk0x10061788 = 1;
		}
		break;
	case 7:
		if (pressed != 1) {
			break;
		}
		if (!FUN_10002de7(-1, g_unk0x10079d80[g_unk0x1007cc80], NULL)) {
			ShowDialog("'Mech exceeds|Keshik Defined Maximum Tonnage (KDMT)|for mission.#Ok", 0);
		}
		else {
			p_msg = g_unk0x10061780;
		}
		break;
	case 8:
	case 9:
		if (pressed != 1) {
			break;
		}
		if (button == 8 && !FUN_1000cf4c()) {
			break;
		}
		FUN_10016cc0(10, 0x20, 0x20);
		FUN_10016cc0(11, 0x20, 0x20);
		FUN_10016cc0(12, 0x20, 0x20);
		FUN_10016cc0(13, 0x20, 0x20);
		FUN_10016d27(14);
		FUN_1000ca74();
		FUN_10007ac8(g_unk0x1005de28);
		g_unk0x1005de28 = NULL;
		FUN_10007ac8(g_unk0x1005de2c);
		g_unk0x1005de2c = g_unk0x10060d20;
		FUN_100078cd(g_unk0x1005de2c);
		FUN_10016cc0(0, 1, 0);
		FUN_1000cb6f();
		g_unk0x10061778->FUN_10048cc1(0);
		g_unk0x10061778->FUN_10048cc1(1);
		g_unk0x10061778->FUN_10048cc1(2);
		g_unk0x10061778->FUN_10048cc1(3);
		g_unk0x10061778->FUN_10048cc1(4);
		g_unk0x10061778->FUN_10048cc1(5);
		g_unk0x10061778->FUN_10048cc1(6);
		g_unk0x10061778->FUN_10048cc1(7);
		g_unk0x10061778->FUN_10048d65(8);
		g_unk0x10061778->FUN_10048d65(9);
		if (g_unk0x1007cc80 < 100) {
			g_unk0x10061778->FUN_10048d65(10);
		}
		else {
			g_unk0x10061778->FUN_10048cc1(10);
		}
		break;
	}

done:
	if (p_msg != 0x404) {
		FUN_10007ac8(g_unk0x1005de2c);
		FUN_10007ac8(g_unk0x1005de28);
		FUN_10016f45();
		if (g_unk0x1006177c) {
			delete g_unk0x1006177c;
		}
		if (g_unk0x10061770) {
			delete g_unk0x10061770;
		}
		if (g_unk0x1005de78) {
			delete g_unk0x1005de78;
		}
		g_unk0x1006177c = g_unk0x10061770 = g_unk0x1005de78 = NULL;
		g_unk0x10061784 = 0;
		g_unk0x10061788 = 0;
		g_unk0x1006178c = 1;
		delete g_unk0x10079ac8;
		delete g_unk0x10079d18;
		delete g_unk0x10061778;
		PostMessage(g_pWnd, p_msg, 0x40f, 0);
		FUN_100108fd(MechbayClickCallback);
	}
}
