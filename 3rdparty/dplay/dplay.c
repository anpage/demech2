/* Symbol source for the DPLAY import library (see dplay.def). Only the __stdcall decorations
   matter; this object is never linked into a game DLL. */

long __stdcall DirectPlayCreate(void* p_guid, void* p_directPlay, void* p_unkOuter)
{
	return 0;
}

long __stdcall DirectPlayEnumerate(void* p_callback, void* p_context)
{
	return 0;
}
