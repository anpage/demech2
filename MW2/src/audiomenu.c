/* The in-mission menu's audio page: the music, sound effects and voice volumes. A data-only
   object: its data follows weaponpanel.c's. */
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
// GLOBAL: MW2GBL 0x100ad8c8
// GLOBAL: MW2MATROX 0x100a5318
// GLOBAL: MW23DFX 0x102651c0
// GLOBAL: MW2ATI 0x101994b8
// GLOBAL: MW2S3 0x10065540
// GLOBAL: MW2POWERVR 0x101c4830
MechChar g_audioItem[] = "Audio Ctrl";

// GLOBAL: MW2 0x100a5298
// GLOBAL: MW2GBL 0x100ad8d8
// GLOBAL: MW2MATROX 0x100a5328
// GLOBAL: MW23DFX 0x102651d0
// GLOBAL: MW2ATI 0x101994c8
// GLOBAL: MW2S3 0x10065550
// GLOBAL: MW2POWERVR 0x101c4840
MechChar g_audioTitle[] = "SET AUDIO VOLUME";

// GLOBAL: MW2 0x100a52b0
// GLOBAL: MW2GBL 0x100ad8f0
// GLOBAL: MW2MATROX 0x100a5340
MechChar g_bettyMessageItem[] = "Betty Message";

// GLOBAL: MW2 0x100a52c0
// GLOBAL: MW2GBL 0x100ad900
// GLOBAL: MW2MATROX 0x100a5350
// GLOBAL: MW23DFX 0x102651f8
// GLOBAL: MW2ATI 0x101994f0
// GLOBAL: MW2S3 0x10065578
// GLOBAL: MW2POWERVR 0x101c4868
MechChar g_soundEffectsItem[] = "Sound Effects";

// GLOBAL: MW2 0x100a52d0
// GLOBAL: MW2GBL 0x100ad910
// GLOBAL: MW2MATROX 0x100a5360
// GLOBAL: MW23DFX 0x10265208
// GLOBAL: MW2ATI 0x10199500
// GLOBAL: MW2S3 0x10065588
// GLOBAL: MW2POWERVR 0x101c4878
MechChar g_voiceItem[] = "Voice";

// GLOBAL: MW2 0x100a52d8
// GLOBAL: MW2GBL 0x100ad918
// GLOBAL: MW2MATROX 0x100a5368
// GLOBAL: MW23DFX 0x10265210
// GLOBAL: MW2ATI 0x10199508
// GLOBAL: MW2S3 0x10065590
// GLOBAL: MW2POWERVR 0x101c4880
MechChar g_musicItem[] = "Music";

// GLOBAL: MW2 0x100a52e0
// GLOBAL: MW2GBL 0x100ad920
// GLOBAL: MW2MATROX 0x100a5370
// GLOBAL: MW23DFX 0x10265218
// GLOBAL: MW2ATI 0x10199510
// GLOBAL: MW2S3 0x10065598
// GLOBAL: MW2POWERVR 0x101c4888
MenuControl g_soundEffectsControl =
	{2, 0, g_sliderShapes, 0, NULL, GetSoundSetting, PreviewSoundSetting, SetSoundSetting, RestoreSoundSetting};

// GLOBAL: MW2 0x100a5308
// GLOBAL: MW2GBL 0x100ad948
// GLOBAL: MW2MATROX 0x100a5398
// GLOBAL: MW23DFX 0x10265240
// GLOBAL: MW2ATI 0x10199538
// GLOBAL: MW2S3 0x100655c0
// GLOBAL: MW2POWERVR 0x101c48b0
MenuControl g_voiceControl =
	{2, 0, g_sliderShapes, 1, NULL, GetSoundSetting, PreviewSoundSetting, SetSoundSetting, RestoreSoundSetting};

// GLOBAL: MW2 0x100a5330
// GLOBAL: MW2GBL 0x100ad970
// GLOBAL: MW2MATROX 0x100a53c0
// GLOBAL: MW23DFX 0x10265268
// GLOBAL: MW2ATI 0x10199560
// GLOBAL: MW2S3 0x100655e8
// GLOBAL: MW2POWERVR 0x101c48d8
MenuControl g_musicControl =
	{2, 0, g_sliderShapes, 2, NULL, GetSoundSetting, PreviewSoundSetting, SetSoundSetting, RestoreSoundSetting};

// GLOBAL: MW2 0x100a5358
// GLOBAL: MW2GBL 0x100ad998
// GLOBAL: MW2MATROX 0x100a53e8
// GLOBAL: MW23DFX 0x10265290
// GLOBAL: MW2ATI 0x10199588
// GLOBAL: MW2S3 0x10065610
// GLOBAL: MW2POWERVR 0x101c4900
MenuPage g_audioPage = {
	0,
	g_audioTitle,
	0,
	4,
	0,
	NULL,
	{{1, g_musicItem, RunMenuSlider, &g_musicControl, NULL},
	 {1, g_soundEffectsItem, RunMenuSlider, &g_soundEffectsControl, NULL},
	 {1, g_voiceItem, RunMenuSlider, &g_voiceControl, NULL},
	 {2, g_acceptText, NULL, NULL, NULL}}
};
