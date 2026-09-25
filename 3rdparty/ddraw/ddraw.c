/* Symbol source for the DDRAW import library (see ddraw.def). Only the __stdcall decoration
   matters; this object is never linked into a game DLL. */

long __stdcall DirectDrawCreate(void* p_guid, void* p_directDraw, void* p_unkOuter)
{
	return 0;
}
