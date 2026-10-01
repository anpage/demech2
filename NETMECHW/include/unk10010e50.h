#ifndef UNK10010E50_H
#define UNK10010E50_H

#include <dplay.h>
#include <windows.h>

// The functions and globals of unk10010e50.cpp that other units use.
DWORD WINAPI WatchPlayersThread(LPVOID p_unused);
BOOL FAR PASCAL AddMissingPlayer(DPID p_id, LPSTR p_friendlyName, LPSTR p_formalName, DWORD p_flags, LPVOID p_context);

#endif // UNK10010E50_H
