#include "cdcheck.h"
#include "debug.h"
#include "decomp.h"
#include "types.h"

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The launcher. It owns the game window and hands it back and forth between the shell
// (MW2SHELL.DLL) and the simulator (MW2.DLL), or runs NetMech's launcher (NETMECHW.DLL).
// Each DLL announces itself with a message to the window, which routes the window's
// messages to that DLL's window procedure.

// Messages the DLLs send to the game window
enum LauncherMessage {
	c_msgActivateLauncher = 0x41e,
	c_msgActivateSim = 0x41f,
	c_msgActivateShell = 0x420
};

// The DLL owning the game window
enum WindowOwner {
	c_processLauncher = 0,
	c_processSim = 1,
	c_processShell = 2
};

// Passed to NETMECHW.DLL's Launcher and on to SimMain (MW2's SimMain declares the same
// layout)
typedef struct NetLaunchInfo {
	void* m_directPlay;           // 0x00
	MechS32 m_localPlayerId;      // 0x04
	undefined4 m_unk0x08;         // 0x08
	undefined4 m_unk0x0c;         // 0x0c
	MechS32* m_playerIds;         // 0x10
	undefined4 m_unk0x14;         // 0x14
	undefined4 m_unk0x18;         // 0x18
	undefined4 m_unk0x1c;         // 0x1c
	undefined4 m_missionNameSize; // 0x20
	MechChar* m_missionName;      // 0x24
} NetLaunchInfo;

// mw2prm.cfg, written by the shell before it hands over to the simulator
typedef struct SimParameters {
	undefined m_unk0x00[0x118];        // 0x00
	MechChar m_cmdLine[0x218 - 0x118]; // 0x118
} SimParameters;

typedef int(__stdcall* SimMainProc)(HINSTANCE, undefined4, LPSTR, NetLaunchInfo*, undefined4, HWND);
typedef int(__stdcall* ShellMainProc)(HINSTANCE, HINSTANCE, LPSTR, int, HWND);
typedef int(__stdcall* LauncherProc)(NetLaunchInfo*);

WNDPROC g_shellWindowProc = NULL;
WNDPROC g_simWindowProc = NULL;
MechS32 g_currentProcess = c_processLauncher;
BOOL g_classRegistered = FALSE;

LRESULT CALLBACK MechWindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	switch (p_msg) {
	case c_msgActivateSim:
		g_currentProcess = c_processSim;
		break;
	case c_msgActivateShell:
		g_currentProcess = c_processShell;
		break;
	case c_msgActivateLauncher:
		g_currentProcess = c_processLauncher;
		break;
	}

	switch (g_currentProcess) {
	case c_processSim:
		return g_simWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	case c_processShell:
		return g_shellWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	default:
		return DefWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	}
}

// Creates the game window: a centered captioned window when the desktop is larger than
// the game's resolution, a full-screen popup otherwise.
HWND CreateGameWindow(HINSTANCE p_hInstance, MechS32 p_width, MechS32 p_height, BOOL p_isNetGame)
{
	RECT rect;
	DWORD style;
	HWND hWnd;
	WNDCLASS wndClass;
	MechS32 screenHeight;
	MechS32 screenWidth;

	style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	if (!g_classRegistered) {
		g_classRegistered = TRUE;
		wndClass.style = CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW;
		wndClass.lpfnWndProc = MechWindowProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = p_hInstance;
		wndClass.hIcon = LoadIcon(p_hInstance, MAKEINTRESOURCE(p_isNetGame ? 108 : 103));
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = "MECHWARRIOR 2";
		RegisterClass(&wndClass);
	}

	rect.left = 0;
	rect.top = 0;
	rect.right = p_width;
	rect.bottom = p_height;
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	if (p_width < screenWidth || p_height < screenHeight) {
		AdjustWindowRect(&rect, style, FALSE);
		rect.right -= rect.left;
		rect.bottom -= rect.top;
		rect.top = (screenHeight - rect.bottom) / 2;
		rect.left = (screenWidth - rect.right) / 2;
	}
	else {
		style = WS_POPUP;
	}

	hWnd = CreateWindowEx(
		0,
		"MECHWARRIOR 2",
		"MECHWARRIOR 2",
		style,
		rect.left,
		rect.top,
		rect.right,
		rect.bottom,
		NULL,
		NULL,
		p_hInstance,
		NULL
	);
	if (hWnd == NULL) {
		return NULL;
	}

	SetMenu(hWnd, NULL);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	return hWnd;
}

MechS32 StartSim(HWND p_hWnd, LPSTR p_cmdLine, NetLaunchInfo* p_netLaunch, undefined4 p_unk0x14)
{
	HMODULE module;
	MechS32 result;
	SimMainProc simMain;

	module = LoadLibrary("MW2.DLL");
	if (module == NULL) {
		return 1;
	}

	simMain = (SimMainProc) GetProcAddress(module, "SimMain");
	if (simMain == NULL) {
		return 1;
	}

	g_simWindowProc = (WNDPROC) GetProcAddress(module, "SimWindowProc");
	if (g_simWindowProc == NULL) {
		return 1;
	}

	DebugPrint("Entering Sim...\n");
	result = simMain(module, 0, p_cmdLine, p_netLaunch, p_unk0x14, p_hWnd);
	DebugPrint("Returned from Sim, retval=%d.\n", result);
	FreeLibrary(module);
	return result;
}

MechS32 StartShell(HWND p_hWnd, LPSTR p_cmdLine, int p_cmdShow)
{
	HMODULE module;
	MechS32 result;
	ShellMainProc shellMain;

	module = LoadLibrary("MW2SHELL.DLL");
	if (module == NULL) {
		return 1;
	}

	shellMain = (ShellMainProc) GetProcAddress(module, "ShellMain");
	if (shellMain == NULL) {
		return 1;
	}

	g_shellWindowProc = (WNDPROC) GetProcAddress(module, "ShellWindowProc");
	if (g_shellWindowProc == NULL) {
		return 1;
	}

	DebugPrint("Entering Shell...\n");
	result = shellMain(module, NULL, p_cmdLine, p_cmdShow, p_hWnd);
	DebugPrint("Returned from Shell, retval=%d.\n", result);
	FreeLibrary(module);
	return result;
}

BOOL CheckMech2Running(void)
{
	if (FindWindow("MECHWARRIOR 2", "MECHWARRIOR 2") != NULL) {
		return TRUE;
	}

	if (FindWindow("MECHWARRIOR 2", "MECHWARRIOR 2") != NULL) {
		return TRUE;
	}

	return FALSE;
}

// Offers to run SETUP.EXE from the game CD. Nothing calls it.
void RunSetup(LPCSTR p_message)
{
	MechChar setupPath[0x50] = "?:\\SETUP.EXE";

	if (MessageBox(NULL, p_message, "MECHWARRIOR 2", MB_ICONERROR | MB_YESNO) == IDYES) {
		setupPath[0] = CdCheck();
		if (setupPath[0] == '\0') {
			MessageBox(
				NULL,
				"You must insert the MECHWARRIOR 2 CD into your CD-ROM drive.",
				"MECHWARRIOR 2",
				MB_ICONERROR
			);
		}
		else {
			// Only returns on failure
			_spawnl(_P_OVERLAY, setupPath, setupPath, NULL, NULL);
			MessageBox(NULL, "Unable to execute MECHWARRIOR 2 setup program.", "MECHWARRIOR 2", MB_ICONERROR);
		}
	}
}

// The command line selects the mode: "net..." runs NetMech, an empty command line runs
// the shell (which hands over to the simulator through mw2prm.cfg), and anything else
// goes straight to the simulator.
int WINAPI WinMain(HINSTANCE p_hInstance, HINSTANCE p_hPrevInstance, LPSTR p_cmdLine, int p_cmdShow)
{
	HWND hWnd;
	HMODULE netMechModule;
	FILE* paramFile;
	NetLaunchInfo netLaunch;
	SimParameters params;
	MechChar missionName[0x40];
	MechS32 result;
	MechS32 launchResult;
	HMODULE netMechLibrary;
	LauncherProc launcher;

	netMechModule = NULL;
	result = 0;
	hWnd = NULL;

	if (GetDeviceCaps(GetDC(NULL), BITSPIXEL) < 8) {
		MessageBox(
			NULL,
			"Your Display Properties setting supports only 16 colors.  MECHWARRIOR 2 requires a color palette of "
			"256 colors.  Change your Display Properties using the Windows Control Panel, and run MECHWARRIOR 2 "
			"again.",
			"MECHWARRIOR 2",
			MB_ICONERROR
		);
		exit(1);
	}

	if (CheckMech2Running()) {
		MessageBox(NULL, "MECHWARRIOR 2 is already running.", "MECHWARRIOR 2", MB_ICONERROR);
		exit(1);
	}

	if (_strnicmp(p_cmdLine, "net", 3) == 0) {
		netMechLibrary = LoadLibrary("NETMECHW.DLL");
		if (netMechLibrary == NULL) {
			MessageBox(NULL, "NetMech has not been installed.", "MECHWARRIOR 2", MB_ICONERROR);
			return 1;
		}

		launcher = (LauncherProc) GetProcAddress(netMechLibrary, "Launcher");
		if (launcher == NULL) {
			MessageBox(NULL, "NetMech has not been installed properly.", "MECHWARRIOR 2", MB_ICONERROR);
			return 1;
		}

		netLaunch.m_unk0x0c = 0x1008;
		netLaunch.m_directPlay = NULL;
		netLaunch.m_localPlayerId = 0;
		netLaunch.m_unk0x08 = 0;
		netLaunch.m_missionName = missionName;
		netLaunch.m_missionNameSize = sizeof(missionName);
		netLaunch.m_playerIds = calloc(8, sizeof(MechS32));
		if (netLaunch.m_playerIds == NULL) {
			exit(1);
		}

		for (;;) {
			DebugPrint("Entering net DLL...\n");
			launchResult = launcher(&netLaunch);
			DebugPrint("Returned from net DLL.\n");
			if (launchResult == 0) {
				DebugPrint("Returned from net DLL.  Exiting...\n");
				break;
			}

			hWnd = CreateGameWindow(p_hInstance, 640, 480, TRUE);
			result = StartSim(hWnd, p_cmdLine, &netLaunch, 1);
			DestroyWindow(hWnd);
			hWnd = NULL;
			netLaunch.m_unk0x0c = 0x1006;
		}

		FreeLibrary(netMechLibrary);
	}
	else if (_strcmpi(p_cmdLine, "") == 0) {
		if (CdCheck() == '\0') {
			MessageBox(
				NULL,
				"You must insert the MECHWARRIOR 2 CD into your CD-ROM drive.",
				"MECHWARRIOR 2",
				MB_ICONERROR
			);
			exit(1);
		}

		// Unload NetMech if something left it loaded
		netMechModule = GetModuleHandle("NETMECHW.DLL");
		if (netMechModule != NULL) {
			FreeLibrary(netMechModule);
		}
		netMechModule = NULL;

		hWnd = CreateGameWindow(p_hInstance, 640, 480, FALSE);
		result = StartShell(hWnd, "intro", 1);
		for (;;) {
			// -1: a DLL is missing; low byte 0xff: quit. (The message's typos are the original's.)
			if (result == -1) {
				MessageBox(
					NULL,
					"MECHWARRIOR 2 is unable to locate necessary program components.  Please check the \"Start "
					"In:\" directory specified in you MECHWARRIROR 2 shortcut, or reinstall MECHWARRIOR 2.",
					"MECHWARRIOR 2",
					MB_ICONERROR
				);
				exit(1);
			}
			else if ((MechChar) result == -1) {
				break;
			}

			paramFile = fopen("mw2prm.cfg", "rb");
			if (paramFile == NULL) {
				MessageBox(NULL, "Error opening MechWarrior 2 parameter file.", "MECHWARRIOR 2", MB_ICONERROR);
				exit(1);
			}

			if (fread(&params, sizeof(params), 1, paramFile) != 1) {
				MessageBox(NULL, "Error reading MechWarrior 2 parameter file.", "MECHWARRIOR 2", MB_ICONERROR);
				exit(1);
			}

			fclose(paramFile);
			result = StartSim(hWnd, params.m_cmdLine, NULL, 1);
			if (result == -1) {
				MessageBox(
					NULL,
					"MECHWARRIOR 2 is unable to locate necessary program components.  Please check the \"Start "
					"In:\" directory specified in you MECHWARRIROR 2 shortcut, or reinstall MECHWARRIOR 2.",
					"MECHWARRIOR 2",
					MB_ICONERROR
				);
				exit(1);
			}
			else if ((MechChar) result == -1) {
				break;
			}

			result = StartShell(hWnd, "sim", 1);
		}
	}
	else {
		hWnd = CreateGameWindow(p_hInstance, 640, 480, FALSE);
		result = StartSim(hWnd, p_cmdLine, NULL, 1);
	}

	return result;
}
