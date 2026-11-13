#ifndef DPLAY_H
#define DPLAY_H

// DPLAY.DLL, DirectPlay 1 as DirectX 2 shipped it, which NetMech's lobby (NETMECHW.DLL) and the
// simulator's network play talk through (dplay.def): our own declarations of what the game uses,
// not the SDK's header. Names, layouts, vtable slot order and values are DirectPlay's.
// IDirectPlay declares every slot, so that each method sits at its offset; the C units call
// through lpVtbl, the C++ ones call the methods. clang-format is off for the interface (see
// ddraw.h).
#include <objbase.h>
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// DirectPlay 1 packs its structures (all of the ones below are aligned anyway).
#pragma pack(push, 1)

	typedef struct IDirectPlay* LPDIRECTPLAY;

	// A player's or group's ID within a session; messages addressed to player 0 are DirectPlay's
	// own (DPSYS_*).
	typedef DWORD DPID;
	typedef DPID* LPDPID;

	// The capabilities of the service provider or a player's connection. 0x1c bytes.
	typedef struct _DPCAPS {
		DWORD dwSize;          // 0x00
		DWORD dwFlags;         // 0x04
		DWORD dwMaxBufferSize; // 0x08
		DWORD dwMaxQueueSize;  // 0x0c
		DWORD dwMaxPlayers;    // 0x10
		DWORD dwHundredBaud;   // 0x14: the line's speed in hundreds of baud
		DWORD dwLatency;       // 0x18
	} DPCAPS;
	typedef DPCAPS* LPDPCAPS;

#define DPLONGNAMELEN 52
#define DPSHORTNAMELEN 20
#define DPSESSIONNAMELEN 32
#define DPPASSWORDLEN 16
#define DPUSERRESERVED 16

	// A session: what IDirectPlay::Open opens or creates, and what EnumSessions reports.
	// 0x7c bytes.
	typedef struct {
		DWORD dwSize;                         // 0x00
		GUID guidSession;                     // 0x04: the application's GUID
		DWORD dwSession;                      // 0x14: the session's ID
		DWORD dwMaxPlayers;                   // 0x18
		DWORD dwCurrentPlayers;               // 0x1c
		DWORD dwFlags;                        // 0x20: DPOPEN_*
		char szSessionName[DPSESSIONNAMELEN]; // 0x24
		char szUserField[DPUSERRESERVED];     // 0x44
		DWORD dwReserved1;                    // 0x54
		char szPassword[DPPASSWORDLEN];       // 0x58
		DWORD dwReserved2;                    // 0x68
		DWORD dwUser1;                        // 0x6c
		DWORD dwUser2;                        // 0x70
		DWORD dwUser3;                        // 0x74
		DWORD dwUser4;                        // 0x78
	} DPSESSIONDESC;
	typedef DPSESSIONDESC* LPDPSESSIONDESC;

	// DirectPlayEnumerate's callback, called once per service provider.
	typedef BOOL(PASCAL* LPDPENUMDPCALLBACK)(
		LPGUID p_guid,
		LPSTR p_name,
		DWORD p_majorVersion,
		DWORD p_minorVersion,
		LPVOID p_context
	);

	// EnumSessions' callback, called once per session and with DPESC_TIMEDOUT (and no session)
	// when the timeout expires; returning TRUE then waits another timeout.
	typedef BOOL(PASCAL* LPDPENUMSESSIONSCALLBACK)(
		LPDPSESSIONDESC p_desc,
		LPVOID p_context,
		LPDWORD p_timeout,
		DWORD p_flags
	);

	HRESULT WINAPI DirectPlayCreate(LPGUID p_guid, LPDIRECTPLAY* p_directPlay, IUnknown* p_unkOuter);
	HRESULT WINAPI DirectPlayEnumerate(LPDPENUMDPCALLBACK p_callback, LPVOID p_context);

	// The player and group enumerations' callback, called once per player.
	typedef BOOL(PASCAL* LPDPENUMPLAYERSCALLBACK)(
		DPID p_id,
		LPSTR p_friendlyName,
		LPSTR p_formalName,
		DWORD p_flags,
		LPVOID p_context
	);

	// The DirectPlay object: one session, its players and groups, and their messages.
	// clang-format off
#undef INTERFACE
#define INTERFACE IDirectPlay
	DECLARE_INTERFACE_(IDirectPlay, IUnknown)
	{
		STDMETHOD(QueryInterface)(THIS_ REFIID p_iid, LPVOID * p_object) PURE; // vtable+0x00
		STDMETHOD_(ULONG, AddRef)(THIS) PURE;                                  // vtable+0x04
		STDMETHOD_(ULONG, Release)(THIS) PURE;                                 // vtable+0x08
		STDMETHOD(AddPlayerToGroup)(THIS_ DPID p_group, DPID p_player) PURE;   // vtable+0x0c
		STDMETHOD(Close)(THIS) PURE;                                           // vtable+0x10
		STDMETHOD(CreatePlayer)(THIS_ LPDPID p_id, LPSTR p_friendlyName, LPSTR p_formalName, LPHANDLE p_event)
			PURE;                                                                                 // vtable+0x14
		STDMETHOD(CreateGroup)(THIS_ LPDPID p_id, LPSTR p_friendlyName, LPSTR p_formalName) PURE; // vtable+0x18
		STDMETHOD(DeletePlayerFromGroup)(THIS_ DPID p_group, DPID p_player) PURE;                 // vtable+0x1c
		STDMETHOD(DestroyPlayer)(THIS_ DPID p_id) PURE;                                           // vtable+0x20
		STDMETHOD(DestroyGroup)(THIS_ DPID p_id) PURE;                                            // vtable+0x24
		STDMETHOD(EnableNewPlayers)(THIS_ BOOL p_enable) PURE;                                    // vtable+0x28
		STDMETHOD(EnumGroupPlayers)(
			THIS_ DPID p_group,
			LPDPENUMPLAYERSCALLBACK p_callback,
			LPVOID p_context,
			DWORD p_flags
		) PURE; // vtable+0x2c
		STDMETHOD(EnumGroups)(
			THIS_ DWORD p_session,
			LPDPENUMPLAYERSCALLBACK p_callback,
			LPVOID p_context,
			DWORD p_flags
		) PURE; // vtable+0x30
		STDMETHOD(EnumPlayers)(
			THIS_ DWORD p_session,
			LPDPENUMPLAYERSCALLBACK p_callback,
			LPVOID p_context,
			DWORD p_flags
		) PURE; // vtable+0x34
		STDMETHOD(EnumSessions)(
			THIS_ LPDPSESSIONDESC p_desc,
			DWORD p_timeout,
			LPDPENUMSESSIONSCALLBACK p_callback,
			LPVOID p_context,
			DWORD p_flags
		) PURE;                                                            // vtable+0x38
		STDMETHOD(GetCaps)(THIS_ LPDPCAPS p_caps) PURE;                    // vtable+0x3c
		STDMETHOD(GetMessageCount)(THIS_ DPID p_id, LPDWORD p_count) PURE; // vtable+0x40
		STDMETHOD(GetPlayerCaps)(THIS_ DPID p_id, LPDPCAPS p_caps) PURE;   // vtable+0x44
		STDMETHOD(GetPlayerName)(
			THIS_ DPID p_id,
			LPSTR p_friendlyName,
			LPDWORD p_friendlyNameSize,
			LPSTR p_formalName,
			LPDWORD p_formalNameSize
		) PURE;                                             // vtable+0x48
		STDMETHOD(Initialize)(THIS_ LPGUID p_guid) PURE;    // vtable+0x4c
		STDMETHOD(Open)(THIS_ LPDPSESSIONDESC p_desc) PURE; // vtable+0x50
		STDMETHOD(Receive)(THIS_ LPDPID p_from, LPDPID p_to, DWORD p_flags, LPVOID p_buffer, LPDWORD p_size)
			PURE;                                                                                         // vtable+0x54
		STDMETHOD(SaveSession)(THIS_ LPSTR p_name) PURE;                                                  // vtable+0x58
		STDMETHOD(Send)(THIS_ DPID p_from, DPID p_to, DWORD p_flags, LPVOID p_buffer, DWORD p_size) PURE; // vtable+0x5c
		STDMETHOD(SetPlayerName)(THIS_ DPID p_id, LPSTR p_friendlyName, LPSTR p_formalName) PURE;         // vtable+0x60
	};
	// clang-format on

	// DP_OK, or an error of facility 0x877 (MAKE_HRESULT(1, 0x877, code)).
#define DP_OK 0
#define DPERR_NOMESSAGES ((HRESULT) 0x887700beL) // code 190
#define DPERR_BUSY ((HRESULT) 0x8877010eL)       // code 270
#define DPERR_USERCANCEL ((HRESULT) 0x88770118L) // code 280

	// DPSESSIONDESC::dwFlags for Open.
#define DPOPEN_OPENSESSION 0x00000001
#define DPOPEN_CREATESESSION 0x00000002

	// Send's flags.
#define DPSEND_GUARANTEE 0x00000001
#define DPSEND_TRYONCE 0x00000004

	// Receive's flags.
#define DPRECEIVE_ALL 0x00000001

	// EnumSessions' flags.
#define DPENUMSESSIONS_AVAILABLE 0x00000001
#define DPENUMSESSIONS_PREVIOUS 0x00000004

	// EnumPlayers' flags.
#define DPENUMPLAYERS_ALL 0x00000000
#define DPENUMPLAYERS_REMOTE 0x00000010
#define DPENUMPLAYERS_GROUP 0x00000020
#define DPENUMPLAYERS_SESSION 0x00000080

	// The EnumSessions callback's flag.
#define DPESC_TIMEDOUT 0x00000001

	// The type (dwType) of a system message, the messages addressed to player 0.
#define DPSYS_ADDPLAYER 0x0003    // DPMSG_ADDPLAYER
#define DPSYS_DELETEPLAYER 0x0005 // DPMSG_DELETEPLAYER
#define DPSYS_ADDPLAYERTOGROUP 0x0007
#define DPSYS_INVITE 0x000e
#define DPSYS_DELETEGROUP 0x0020
#define DPSYS_DELETEPLAYERFROMGRP 0x0021
#define DPSYS_CONNECT 0x484b

	// A player or group joined the session. 0x58 bytes.
	typedef struct {
		DWORD dwType;                     // 0x00: DPSYS_ADDPLAYER
		DWORD dwPlayerType;               // 0x04
		DPID dpId;                        // 0x08
		char szLongName[DPLONGNAMELEN];   // 0x0c
		char szShortName[DPSHORTNAMELEN]; // 0x40
		DWORD dwCurrentPlayers;           // 0x54
	} DPMSG_ADDPLAYER;

	// A player or group left the session. 0x08 bytes.
	typedef struct {
		DWORD dwType; // 0x00: DPSYS_DELETEPLAYER
		DPID dpId;    // 0x04
	} DPMSG_DELETEPLAYER;

	// The start every system message shares.
	typedef struct {
		DWORD dwType; // 0x00: DPSYS_*
	} DPMSG_GENERIC;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // DPLAY_H
