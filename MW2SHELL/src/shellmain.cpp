#include "shellmain.h"

#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "cedarknot0x10.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "midisequence.h"
#include "mousestate.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// The debug print and debug output units are C translation units.
extern "C" void DebugPrint(const MechChar* p_format, ...);
extern "C" void FUN_100178cc(MechS32 p_mode);

void FUN_1001661b();

// GLOBAL: MW2SHELL 0x10062978
void (*g_pShellCallback)(
	TMPackDataBase* p_database,
	MechS32* p_campaign,
	MechU8* p_pilotChosen,
	char** p_scenario,
	MechS32 p_msg
) = NULL;

// GLOBAL: MW2SHELL 0x1006297c
void (*g_unk0x1006297c)(MechS32 p_unk0x00) = NULL;

// GLOBAL: MW2SHELL 0x10062980
MidiSequence* g_midiBackgroundMusic = NULL;

// GLOBAL: MW2SHELL 0x10062984
MechS32 g_fCursorHidden = 0;

// GLOBAL: MW2SHELL 0x10062ffc
MechS32 g_nWindowMode = 0;

// GLOBAL: MW2SHELL 0x1006a9c0
char g_unk0x1006a9c0[0x10] = "MECHWARRIOR 2";

// GLOBAL: MW2SHELL 0x1006a9d0
MechS32 g_fWindowActive = 1;

// GLOBAL: MW2SHELL 0x1006a9dc
MechU32 g_fQuickTips = 1;

// GLOBAL: MW2SHELL 0x1006a9e0
MechS32 g_unk0x1006a9e0 = 1;

// GLOBAL: MW2SHELL 0x1006a9e4
MechS32 g_menuVisible = 0;

// GLOBAL: MW2SHELL 0x1006a9e8
MechS32 g_fHelpRegistered = 0;

// GLOBAL: MW2SHELL 0x1006a9ec
MechS32 g_menuDialogOpen = 0;

// GLOBAL: MW2SHELL 0x1006a9f0
MechS32 g_unk0x1006a9f0 = 0;

// GLOBAL: MW2SHELL 0x1006a9f4
HANDLE g_hPrimaryHeap = NULL;

// Tab stops for the "\T" text escape, in pixels from the left edge.
// GLOBAL: MW2SHELL 0x1006e150
MechS32 g_unk0x1006e150[19] =
	{0, 36, 72, 108, 144, 180, 216, 252, 288, 324, 360, 396, 432, 468, 504, 540, 576, 612, 640};

// GLOBAL: MW2SHELL 0x1006e19c
char* g_unk0x1006e19c = "DATABASE.MW2";

// GLOBAL: MW2SHELL 0x100711f8
HollowReed0x110* g_unk0x100711f8 = NULL;

// GLOBAL: MW2SHELL 0x100711fc
AudioSubsystem* g_pAudioSubsystem = NULL;

// GLOBAL: MW2SHELL 0x10071200
void* g_unk0x10071200 = NULL;

// GLOBAL: MW2SHELL 0x10071204
MouseState* g_pMouseState = NULL;

// GLOBAL: MW2SHELL 0x10071208
VideoDriver* g_pVideoDriver = NULL;

// GLOBAL: MW2SHELL 0x1007120c
BrassLantern0x414* g_unk0x1007120c = NULL;

// GLOBAL: MW2SHELL 0x10071210
BrassLantern0x414* g_unk0x10071210 = NULL;

// GLOBAL: MW2SHELL 0x10071214
BrassLantern0x414* g_unk0x10071214 = NULL;

// GLOBAL: MW2SHELL 0x10071218
BrassLantern0x414* g_unk0x10071218 = NULL;

// GLOBAL: MW2SHELL 0x1007121c
BrassLantern0x414* g_unk0x1007121c = NULL;

// GLOBAL: MW2SHELL 0x10071220
BrassLantern0x414* g_unk0x10071220 = NULL;

// GLOBAL: MW2SHELL 0x10071224
BrassLantern0x414* g_unk0x10071224 = NULL;

// GLOBAL: MW2SHELL 0x10071228
BrassLantern0x414* g_unk0x10071228 = NULL;

// GLOBAL: MW2SHELL 0x1007122c
TMPackDataBase* g_pDatabaseMw2 = NULL;

// GLOBAL: MW2SHELL 0x10071230
CedarKnot0x10* g_unk0x10071230 = NULL;

// GLOBAL: MW2SHELL 0x10071234
MechS32 g_fAudio = 1;

// GLOBAL: MW2SHELL 0x10071238
MechS32 g_fDigitalAudio = 1;

// GLOBAL: MW2SHELL 0x1007123c
MechS32 g_unk0x1007123c = 1;

// GLOBAL: MW2SHELL 0x10071240
MechS32 g_unk0x10071240 = 1;

// GLOBAL: MW2SHELL 0x1007124c
MechU8 g_fDrawFmv = 0;

// GLOBAL: MW2SHELL 0x10071250
MechChar g_szDataDrivePath[4] = "A:\\";

// GLOBAL: MW2SHELL 0x1007cc84
char* g_pScenario;

// GLOBAL: MW2SHELL 0x1007cc88
MechS32 g_nSelectedCampaign;

// GLOBAL: MW2SHELL 0x1007cc8c
MechU8 g_fPilotChosen;

// GLOBAL: MW2SHELL 0x100965d8
MechS32 g_unk0x100965d8;

// GLOBAL: MW2SHELL 0x100965dc
MechS32 g_unk0x100965dc;

// GLOBAL: MW2SHELL 0x100965e0
HINSTANCE g_pModule;

// The C debug print unit (debugprint.c) shows its message boxes over the shell window.
extern "C" HWND g_pWnd;

// GLOBAL: MW2SHELL 0x100965ec
HWND g_pWnd;

// GLOBAL: MW2SHELL 0x100965f0
HMENU g_windowMenu;

void DrawMissionDebrief(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario);
void FUN_10003d3a(TMPackDataBase* p_database, MechS32 p_campaign);
extern "C" void HandleKeyboardMessages(UINT p_msg, WPARAM p_wParam, LPARAM p_lParam);
void FUN_1000d0d4(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam);
void PlayMidiSong(UINT p_msg, MechS32 p_campaign);
void ParseCommandLineFlags(char* p_cmdLine);
void FUN_1001023c(HMENU p_menu);
void FUN_10010320(HMENU p_menu);
MechS32 LoadSettingsFromRegistry(MechU32* p_quickTips, MechS32* p_unk0x04, MechS32* p_unk0x08);
MechS32 FUN_1001053e(MechS32 p_quickTips, MechS32 p_unk0x04, MechS32 p_unk0x08);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
BOOL CALLBACK FUN_10010724(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam);
void FUN_1001093e();
void ClearRegisteredMenuFunction();
void DrawClanHall(TMPackDataBase* p_database, MechS32 p_campaign, MechU8 p_pilotChosen, WPARAM p_wParam);
void FUN_10015008(TMPackDataBase* p_database, MechS32 p_campaign, MechU8* p_pilotChosen, char** p_scenario);
MechS32 FUN_10015f58(const char* p_name, MechS32 p_msg, MechS32 p_wParam);
MechS32 PlayFullscreenVideo(const char* p_name, MechS32 p_unk0x04, MechS32 p_unk0x08);
MechS32 FUN_10016be7();
void FUN_10016c1d();
void FUN_10016c3e();
void FUN_10016d90(MechS32 p_unk0x00);
void FUN_10016f45();
void FUN_10029010(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam);
void LoadPilotRoster();
char CdCheck();
void FUN_100382e6(TMPackDataBase* p_database, char** p_scenario, WPARAM p_wParam);
void FUN_10039b50(BOOL p_fromSim, MechS32* p_campaign, MechU8* p_pilotChosen, char** p_scenario);
void WriteSimHandoff(UINT p_msg, MechS32 p_campaign, MechU8 p_pilotChosen, const char* p_scenario);
void FUN_10039e72(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario, WPARAM p_wParam);
void FUN_1003c7e0(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario, WPARAM p_wParam);
void FUN_1003dc10(TMPackDataBase* p_database, MechS32* p_campaign);
void FUN_1003e3c9();
void OpenCockpitControls();
void FUN_10043979();
void FUN_10043a72();
MechS32 ShowDialog(const char* p_text, MechS32 p_unk0x04);
void FUN_10046200(TMPackDataBase* p_database, char* p_scenario, MechS32 p_campaign);
void FUN_10046e80();
void FUN_10047370();

// Matches except for the stack slots of helpFile, msg and mouseY (a consistent permutation) and
// the operand order of the WM_PALETTECHANGED comparison (the original loads p_hWnd first; moving,
// renaming or re-declaring g_pWnd and renaming p_hWnd don't flip it).
// FUNCTION: MW2SHELL 0x1000e670
extern "C" LRESULT CALLBACK ShellWindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	char helpFile[32];
	UINT msg;
	MechS32 mouseY;
	PAINTSTRUCT paint;

	if (p_msg >= WM_KEYFIRST && p_msg <= WM_KEYLAST) {
		HandleKeyboardMessages(p_msg, p_wParam, p_lParam);
		return 0;
	}

	switch (p_msg) {
	case WM_ACTIVATEAPP:
		g_fWindowActive = p_wParam;
		if (g_fWindowActive) {
			if (FUN_10016be7()) {
				FUN_10016c3e();
			}

			if (g_midiBackgroundMusic) {
				g_midiBackgroundMusic->Start();
			}

			SetFocus(g_pWnd);
		}
		else {
			if (FUN_10016be7()) {
				FUN_10016c1d();
			}

			if (g_midiBackgroundMusic) {
				g_midiBackgroundMusic->Stop();
			}
		}
		return 0;
	case WM_PALETTECHANGED:
		if (g_pWnd == p_hWnd) {
			return 0;
		}
	case WM_QUERYNEWPALETTE:
		if (g_pVideoDriver) {
			g_pVideoDriver->UpdatePalette();
			if (g_fDrawFmv) {
				g_pVideoDriver->ExpandRectBySize(0, 0, 320, 200);
				g_pVideoDriver->DrawFmv();
			}
			else {
				g_pVideoDriver->ExpandRectBySize(0, 0, 640, 480);
				g_pVideoDriver->DrawShell();
			}
			return 1;
		}
		else {
			return 0;
		}
	case WM_PAINT:
		BeginPaint(p_hWnd, &paint);
		if (g_pVideoDriver) {
			if (g_fDrawFmv) {
				g_pVideoDriver->ExpandRectBySize(0, 0, 320, 200);
				g_pVideoDriver->DrawFmv();
			}
			else {
				g_pVideoDriver
					->ExpandRect(paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right, paint.rcPaint.bottom);
				g_pVideoDriver->DrawShell();
			}
		}
		EndPaint(p_hWnd, &paint);
		return 0;
	case WM_NCMOUSEMOVE:
		if (g_fCursorHidden) {
			while (ShowCursor(TRUE) < 0)
				;
			g_fCursorHidden = FALSE;
		}
		return 0;
	case WM_MOUSEMOVE:
		if (FUN_10016be7() && !g_fCursorHidden) {
			while (ShowCursor(FALSE) >= 0)
				;
			g_fCursorHidden = TRUE;
		}

		mouseY = ((MechU32) p_lParam >> 16) & 0xffff;
		if (g_menuVisible && g_nWindowMode == 1 && mouseY > 2) {
			SetMenu(p_hWnd, NULL);
			g_menuVisible = FALSE;
		}
		else if (g_nWindowMode == 1 && !g_menuVisible && !FUN_10016be7() && GetSystemMetrics(SM_CYMENU) >= mouseY) {
			SetMenu(p_hWnd, g_windowMenu);
			g_menuVisible = TRUE;
			g_pVideoDriver->ExpandRect(0, 0, 640, 480);
			g_pVideoDriver->DrawShell();
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_COMMAND:
		if (FUN_10016be7()) {
			FUN_10016d90(0);
		}

		switch (LOWORD(p_wParam)) {
		case 0x9c41:
			ClearRegisteredMenuFunction();
			if (g_pShellCallback) {
				g_pShellCallback(g_pDatabaseMw2, &g_nSelectedCampaign, &g_fPilotChosen, &g_pScenario, 0x40e);
			}
			else {
				PostMessage(g_pWnd, 0x40e, 0x40e, 0);
			}
			break;
		case 0x9c42:
			ClearRegisteredMenuFunction();
			EnableMenuItem(g_windowMenu, 0x9c42, MF_GRAYED);
			FUN_1003e3c9();
			g_menuDialogOpen = TRUE;
			break;
		case 0x9c72:
			CheckMenuItem(g_windowMenu, p_wParam, ~GetMenuState(g_windowMenu, p_wParam, MF_BYCOMMAND) & MF_CHECKED);
			g_fQuickTips = 1 - g_fQuickTips;
			break;
		case 0x9c43:
			if (ShowDialog("Embrace cowardice?#Yes|No", 1) == 0) {
				ClearRegisteredMenuFunction();
				g_pVideoDriver->ActivateFramebuffer();
				memset(g_pVideoDriver->m_backBuffer.m_pixels, 0, 640 * 480);
				if (g_pShellCallback) {
					g_pShellCallback(g_pDatabaseMw2, &g_nSelectedCampaign, &g_fPilotChosen, &g_pScenario, 0x402);
				}
				else {
					PostMessage(g_pWnd, 0x402, 0, 0);
				}
				g_menuDialogOpen = TRUE;
			}
			break;
		case 0x9c94:
			ClearRegisteredMenuFunction();
			EnableMenuItem(g_windowMenu, 0x9c94, MF_GRAYED);
			FUN_10043a72();
			g_menuDialogOpen = TRUE;
			break;
		case 0x9c4b:
			ClearRegisteredMenuFunction();
			EnableMenuItem(g_windowMenu, 0x9c4b, MF_GRAYED);
			OpenCockpitControls();
			g_menuDialogOpen = TRUE;
			break;
		case 0x9c96:
			DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x8a), g_pWnd, (DLGPROC) FUN_10010724, 0);
			break;
		case 0x9c92:
			ClearRegisteredMenuFunction();
			EnableMenuItem(g_windowMenu, 0x9c92, MF_GRAYED);
			FUN_10046e80();
			g_menuDialogOpen = TRUE;
			break;
		case 0x9c4c:
			sprintf(helpFile, "%s", "mw2help.hlp");
			if (GetFileAttributes(helpFile) == -1) {
				sprintf(helpFile, "%s%s", g_szDataDrivePath, "mw2help.hlp");
			}

			if (WinHelp(g_pWnd, helpFile, HELP_CONTENTS, 0)) {
				g_fHelpRegistered = TRUE;
			}
			break;
		case 0x9c95:
			sprintf(helpFile, "%s", "tech.hlp");
			if (GetFileAttributes(helpFile) == -1) {
				sprintf(helpFile, "%s%s", g_szDataDrivePath, "tech.hlp");
			}

			if (WinHelp(g_pWnd, helpFile, HELP_CONTENTS, 0)) {
				g_fHelpRegistered = TRUE;
			}
			break;
		}
		return 0;
	case 0x40e:
		g_fPilotChosen = FALSE;
		FUN_1003dc10(g_pDatabaseMw2, &g_nSelectedCampaign);
		if (g_fCursorHidden) {
			while (ShowCursor(TRUE) < 0)
				;
			g_fCursorHidden = FALSE;
		}
		break;
	case 0x40d:
		g_nSelectedCampaign = 2;
		FUN_100382e6(g_pDatabaseMw2, &g_pScenario, p_wParam);
		break;
	case 0x407:
		if (!g_fPilotChosen) {
			switch (g_nSelectedCampaign) {
			case 0:
				FUN_10015f58("aworgstr", 0x412, 0x407);
				break;
			case 1:
				FUN_10015f58("ajfrgstr", 0x412, 0x407);
				break;
			}
			return 0;
		}
		else {
			if (g_fCursorHidden) {
				while (ShowCursor(TRUE) < 0)
					;
				g_fCursorHidden = FALSE;
			}

			DrawClanHall(g_pDatabaseMw2, g_nSelectedCampaign, g_fPilotChosen, p_wParam);
		}
		break;
	case 0x412:
		if (g_fCursorHidden) {
			while (ShowCursor(TRUE) < 0)
				;
			g_fCursorHidden = FALSE;
		}

		FUN_10015008(g_pDatabaseMw2, g_nSelectedCampaign, &g_fPilotChosen, &g_pScenario);
		break;
	case 0x40b:
		FUN_10029010(g_pDatabaseMw2, g_nSelectedCampaign, p_wParam);
		break;
	case 0x414:
		FUN_1003c7e0(g_pDatabaseMw2, g_nSelectedCampaign, &g_pScenario, p_wParam);
		break;
	case 0x411:
		if (g_nSelectedCampaign == 2) {
			FUN_100382e6(g_pDatabaseMw2, &g_pScenario, p_wParam);
		}
		else {
			FUN_10039e72(g_pDatabaseMw2, g_nSelectedCampaign, &g_pScenario, p_wParam);
		}
		break;
	case 0x40f:
		FUN_1000d0d4(g_pDatabaseMw2, g_nSelectedCampaign, p_wParam);
		break;
	case 0x413:
		FUN_10003d3a(g_pDatabaseMw2, g_nSelectedCampaign);
		break;
	case 0x406:
		FUN_10046200(g_pDatabaseMw2, g_pScenario, g_nSelectedCampaign);
		break;
	case 0x410:
		if (p_wParam == 0x406) {
			msg = 0x409;
		}
		else {
			msg = p_wParam;
		}

		if (g_unk0x10071240) {
			WriteSimHandoff(msg, g_nSelectedCampaign, g_fPilotChosen, g_pScenario);
			PostQuitMessage(0x401);
			return 0;
		}
		else {
			PostMessage(p_hWnd, msg, 0x410, 0);
		}
		break;
	case 0x409:
		DrawMissionDebrief(g_pDatabaseMw2, g_nSelectedCampaign, &g_pScenario);
		break;
	case 0x415:
		while (ShowCursor(FALSE) >= 0)
			;
		g_fCursorHidden = TRUE;

		switch (g_nSelectedCampaign) {
		case 0:
			if (!PlayFullscreenVideo("mwoland", 0x407, 0x415)) {
				PlayFullscreenVideo("mjfland", 0x407, 0x415);
			}
			break;
		case 1:
			if (!PlayFullscreenVideo("mjfland", 0x407, 0x415)) {
				PlayFullscreenVideo("mwoland", 0x407, 0x415);
			}
			break;
		}
		break;
	case 0x416:
		while (ShowCursor(FALSE) >= 0)
			;
		g_fCursorHidden = TRUE;

		switch (g_nSelectedCampaign) {
		case 0:
			if (!PlayFullscreenVideo("mend", 0x407, 0x416)) {
				PlayFullscreenVideo("mend2", 0x407, 0x416);
			}
			break;
		case 1:
			if (!PlayFullscreenVideo("mend2", 0x407, 0x416)) {
				PlayFullscreenVideo("mend", 0x407, 0x416);
			}
			break;
		}
		break;
	case 0x402:
		WriteSimHandoff(0x402, g_nSelectedCampaign, g_fPilotChosen, "exittos");
		PostQuitMessage(0x402);
		return 0;
	default:
		return DefWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	}

	if (g_fAudio) {
		PlayMidiSong(p_msg, g_nSelectedCampaign);
	}

	return 0;
}

// Matches except for the stack slots of itemData, unk0x14 and itemSize (a consistent
// permutation that VC++ 4.1 doesn't reproduce from this source).
// FUNCTION: MW2SHELL 0x1000f35e
extern "C" int __stdcall ShellMain(
	HINSTANCE p_hInstance,
	HINSTANCE p_hPrevInstance,
	char* p_cmdLine,
	int p_cmdShow,
	HWND p_hWnd
)
{
	void* itemData = NULL;
	MechS32 unk0x14 = 0x402;
	MechS32 itemSize;
	MSG msg;
	BOOL fromSim = FALSE;

	g_hPrimaryHeap = HeapCreate(HEAP_NO_SERIALIZE, 1000000, 0);
	if (g_hPrimaryHeap == NULL) {
		MessageBox(NULL, "Insufficient memory available.", g_unk0x1006a9c0, MB_ICONEXCLAMATION);
		return 0xff;
	}

	g_pModule = p_hInstance;
	g_pWnd = p_hWnd;

	if (*p_cmdLine == '\0') {
		g_fDigitalAudio = 0;
		g_fAudio = 0;
		g_unk0x1007123c = 0;
		g_unk0x10071240 = 0;
	}
	else if (strstr(p_cmdLine, "sim") != NULL) {
		fromSim = TRUE;
	}

	ParseCommandLineFlags(p_cmdLine);
	g_szDataDrivePath[0] = CdCheck();
	if (*p_cmdLine == '\0') {
		g_szDataDrivePath[0] = '\0';
	}

	FUN_10047370();

	g_pDatabaseMw2 = new TMPackDataBase(g_unk0x1006e19c);
	g_pAudioSubsystem = new AudioSubsystem();
	g_unk0x100965dc = 640;
	g_unk0x100965d8 = 480;
	g_pVideoDriver = new VideoDriver();

	g_windowMenu = LoadMenu(g_pModule, MAKEINTRESOURCE(0x68));
	if (g_nWindowMode != 1) {
		SetMenu(g_pWnd, g_windowMenu);
	}

	LoadSettingsFromRegistry(&g_fQuickTips, &g_unk0x1006a9e0, &g_unk0x1006a9f0);
	if (!fromSim) {
		PlayFullscreenVideo("mintro", 0x40e, 0x40e);
	}

	g_pDatabaseMw2->GetDBItem(0x1a, &itemData, &itemSize);
	g_unk0x10071210 = new BrassLantern0x414(itemData, g_pVideoDriver);
	g_pDatabaseMw2->GetDBItem(0x1b, &itemData, &itemSize);
	g_unk0x10071214 = new BrassLantern0x414(itemData, g_pVideoDriver);
	g_pDatabaseMw2->GetDBItem(0x1c, &itemData, &itemSize);
	g_unk0x10071218 = new BrassLantern0x414(itemData, g_pVideoDriver);
	g_pDatabaseMw2->GetDBItem(0x1e, &itemData, &itemSize);
	g_unk0x1007121c = new BrassLantern0x414(itemData, g_pVideoDriver);
	g_pDatabaseMw2->GetDBItem(0x1f, &itemData, &itemSize);
	g_unk0x10071220 = new BrassLantern0x414(itemData, g_pVideoDriver);
	g_unk0x1007120c = g_unk0x10071210;
	g_pDatabaseMw2->GetDBItem(0x20, &itemData, &itemSize);
	g_unk0x10071228 = new BrassLantern0x414(itemData, g_pVideoDriver);
	g_unk0x10071224 = g_unk0x10071228;
	g_unk0x10071210 = g_unk0x10071228;
	g_unk0x1007120c = g_unk0x10071210;

	g_pDatabaseMw2->GetDBItem(0x19, &g_unk0x10071200, &itemSize);
	g_pMouseState = new MouseState(g_pVideoDriver, g_unk0x1007120c, g_unk0x10071200);
	g_unk0x100711f8 = new HollowReed0x110();
	g_unk0x10071230 = new CedarKnot0x10("MW2.PRJ");

	LoadPilotRoster();
	FUN_10043979();
	FUN_10039b50(fromSim, &g_nSelectedCampaign, &g_fPilotChosen, &g_pScenario);
	ClipCursor(NULL);

	if (fromSim) {
		FUN_1001023c(g_windowMenu);
		while (ShowCursor(TRUE) < 0)
			;
		g_fCursorHidden = FALSE;
	}
	else {
		while (ShowCursor(FALSE) >= 0)
			;
		g_fCursorHidden = TRUE;
	}

	SendMessage(g_pWnd, 0x420, 0, 0);

	for (;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (g_fWindowActive) {
			g_pMouseState->ReadMouseState();
			FUN_1001093e();
			if (g_fDrawFmv) {
				g_pVideoDriver->DrawFmv();
			}
			else {
				g_pVideoDriver->DrawShell();
			}
		}
	}

	SendMessage(g_pWnd, 0x41e, 0, 0);
	SetMenu(g_pWnd, NULL);
	FUN_1001053e(g_fQuickTips, g_unk0x1006a9e0, g_unk0x1006a9f0);
	g_pVideoDriver->ActivateFramebuffer();

	if (g_unk0x1006297c) {
		g_unk0x1006297c(0);
	}

	if (g_pShellCallback) {
		g_pShellCallback(g_pDatabaseMw2, &g_nSelectedCampaign, &g_fPilotChosen, &g_pScenario, 0x402);
	}

	FUN_10016f45();

	delete g_unk0x10071230;
	delete g_pDatabaseMw2;
	delete g_pMouseState;
	delete g_unk0x100711f8;
	delete g_pVideoDriver;
	delete g_pAudioSubsystem;

	if (g_fHelpRegistered) {
		WinHelp(g_pWnd, NULL, HELP_QUIT, 0);
	}

	HeapDestroy(g_hPrimaryHeap);
	g_hPrimaryHeap = NULL;

	if (msg.wParam == 0x401) {
		return 3;
	}
	else {
		return 0xff;
	}
}

// STUB: MW2SHELL 0x1000fe86
void PlayMidiSong(UINT p_msg, MechS32 p_campaign)
{
	STUB(0x1000fe86);
}

// FUNCTION: MW2SHELL 0x10010137
void ParseCommandLineFlags(char* p_cmdLine)
{
	char* token;

	token = strtok(p_cmdLine, " ");
	while (token != NULL) {
		if (token[0] == '-') {
			switch (toupper(token[1])) {
			case 'X':
				if (token[2] == '=') {
					switch (toupper(token[3])) {
					case 'F':
						FUN_100178cc(4);
						break;
					case 'S':
						FUN_100178cc(2);
						break;
					case 'M':
						FUN_100178cc(1);
						break;
					}
				}
				break;
			}
		}
		token = strtok(NULL, " ");
	}
}

// FUNCTION: MW2SHELL 0x1001023c
void FUN_1001023c(HMENU p_menu)
{
	MechS32 result;

	result = EnableMenuItem(p_menu, 1, 0x400);
	result = EnableMenuItem(p_menu, 0x9c41, 0);
	result = EnableMenuItem(p_menu, 0x9c42, 0);
	result = EnableMenuItem(p_menu, 0x9c72, 0);
	CheckMenuItem(p_menu, 0x9c72, g_fQuickTips ? MF_CHECKED : MF_UNCHECKED);
	result = EnableMenuItem(p_menu, 0x9c94, 0);
	result = EnableMenuItem(p_menu, 0x9c4b, 0);
	result = EnableMenuItem(p_menu, 0x9c96, 0);
	result = EnableMenuItem(p_menu, 0x9c92, 0);
	result = DrawMenuBar(g_pWnd);
}

// FUNCTION: MW2SHELL 0x10010320
void FUN_10010320(HMENU p_menu)
{
	MechS32 result;

	result = EnableMenuItem(p_menu, 1, 0x401);
	result = EnableMenuItem(p_menu, 0x9c41, 1);
	result = EnableMenuItem(p_menu, 0x9c42, 1);
	result = EnableMenuItem(p_menu, 0x9c72, 1);
	result = EnableMenuItem(p_menu, 0x9c94, 1);
	result = EnableMenuItem(p_menu, 0x9c4b, 1);
	result = EnableMenuItem(p_menu, 0x9c96, 1);
	result = EnableMenuItem(p_menu, 0x9c92, 1);
	result = DrawMenuBar(g_pWnd);
}

// Stack-slot permutation: value and result swap [ebp-N] slots with the original.
// FUNCTION: MW2SHELL 0x100103e2
MechS32 LoadSettingsFromRegistry(MechU32* p_quickTips, MechS32* p_unk0x04, MechS32* p_unk0x08)
{
	DWORD size;
	MechS32 value;
	MechS32 result;
	HKEY key;
	DWORD type;

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Activision\\MechWarrior 2\\1.0", 0, KEY_QUERY_VALUE, &key);
	if (result != 0) {
		DebugPrint("Could not open registry MechWarrior2 key: %d\n", result);
		return 0;
	}

	size = 4;
	type = 4;
	result = RegQueryValueEx(key, "QuickTips", NULL, &type, (LPBYTE) &value, &size);
	if (result == 0) {
		*p_quickTips = value;
	}
	else {
		*p_quickTips = 1;
	}

	size = 4;
	type = 4;
	result = RegQueryValueEx(key, "ShowDialog", NULL, &type, (LPBYTE) &value, &size);
	if (result == 0) {
		*p_unk0x04 = value;
	}
	else {
		*p_unk0x04 = 1;
	}

	size = 4;
	type = 4;
	result = RegQueryValueEx(key, "LittleMovies", NULL, &type, (LPBYTE) &value, &size);
	if (result == 0 && value != -1) {
		*p_unk0x08 = value;
	}
	else {
		*p_unk0x08 = 1;
	}

	RegCloseKey(key);
	return (MechU8) (result == 0);
}

// Stack-slot permutation: value and result swap [ebp-N] slots with the original.
// FUNCTION: MW2SHELL 0x1001053e
MechS32 FUN_1001053e(MechS32 p_quickTips, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	MechS32 result;
	MechS32 value;
	HKEY key;

	result = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		"Software\\Activision\\MechWarrior 2\\1.0",
		0,
		NULL,
		0,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		(LPDWORD) &value
	);
	if (result != 0) {
		DebugPrint("Could not open registry MechWarrior2 key: %d\n", result);
		return 0;
	}

	value = p_quickTips;
	result = RegSetValueEx(key, "QuickTips", 0, REG_DWORD, (const BYTE*) &value, 4);
	if (result != 0) {
		DebugPrint("Could not save quickTips registry MechWarrior2 key: %d\n", result);
		return 0;
	}

	value = p_unk0x04;
	result = RegSetValueEx(key, "ShowDialog", 0, REG_DWORD, (const BYTE*) &value, 4);
	if (result != 0) {
		DebugPrint("Could not save showDialog registry MechWarrior2 key: %d\n", result);
		return 0;
	}

	value = p_unk0x08;
	result = RegSetValueEx(key, "LittleMovies", 0, REG_DWORD, (const BYTE*) &value, 4);
	if (result != 0) {
		DebugPrint("Could not save littleMovies registry MechWarrior2 key: %d\n", result);
		return 0;
	}

	RegCloseKey(key);
	return (MechU8) (result == 0);
}

// FUNCTION: MW2SHELL 0x10010724
BOOL CALLBACK FUN_10010724(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	UINT command;

	switch (p_msg) {
	case WM_INITDIALOG:
		if (g_unk0x1006a9f0 != 0) {
			CheckDlgButton(p_hWnd, 0x3e8, 1);
		}
		else {
			CheckDlgButton(p_hWnd, 0x3e9, 1);
		}
		SetFocus(GetDlgItem(p_hWnd, 1));
		return 0;
	case WM_COMMAND:
		command = LOWORD(p_wParam);
		switch (command) {
		case 0x3e8:
			if (IsDlgButtonChecked(p_hWnd, 0x3e8) == 1) {
				CheckDlgButton(p_hWnd, 0x3e9, 0);
			}
			else {
				CheckDlgButton(p_hWnd, 0x3e9, 1);
			}
			break;
		case 0x3e9:
			if (IsDlgButtonChecked(p_hWnd, 0x3e9) == 1) {
				CheckDlgButton(p_hWnd, 0x3e8, 0);
			}
			else {
				CheckDlgButton(p_hWnd, 0x3e8, 1);
			}
			break;
		case 1:
			if (IsDlgButtonChecked(p_hWnd, 0x3e8) == 1) {
				g_unk0x1006a9f0 = 1;
			}
			else {
				g_unk0x1006a9f0 = 0;
			}
			// fall through to EndDialog
		case 2:
			EndDialog(p_hWnd, 0);
			break;
		}
		return 1;
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x100108e5
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32))
{
	g_pShellCallback = p_callback;
}

// FUNCTION: MW2SHELL 0x1001093e
void FUN_1001093e()
{
	if (g_unk0x1006297c != NULL) {
		g_unk0x1006297c(1);
	}
	else {
		if (g_pShellCallback != NULL) {
			g_pShellCallback(g_pDatabaseMw2, &g_nSelectedCampaign, &g_fPilotChosen, &g_pScenario, 0x404);
			FUN_1001661b();
		}
	}
}

// FUNCTION: MW2SHELL 0x100109f9
void ClearRegisteredMenuFunction()
{
	if (g_unk0x1006297c != NULL) {
		g_unk0x1006297c(0);
	}
}
