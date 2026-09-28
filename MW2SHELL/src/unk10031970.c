#include "decomp.h"
#include "types.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Resource file helpers: loading a file whole, and prefixing names with the base directory.

// Optional base directory for resource names.
// GLOBAL: MW2SHELL 0x10067708
MechChar g_unk0x10067708[0x100] = {0};

// Shared scratch buffer returned by the path helpers (overwritten on each call).
// GLOBAL: MW2SHELL 0x1008ff58
static MechChar g_unk0x1008ff58[0x50];

// Read a whole file into memory, allocated here unless p_preallocated; returns the open handle,
// or -1 on failure.
// FUNCTION: MW2SHELL 0x10031970
MechS32 FUN_10031970(MechChar* p_name, MechS32* p_size, void** p_data, MechS32 p_preallocated)
{
	MechS32 handle;
	FILE* log;

	handle = _open(p_name, _O_BINARY);
	if (handle == -1) {
		log = fopen("symlog.txt", "a");
		if (log != NULL) {
			fprintf(log, "Couldn't load ID=%s\n", p_name);
		}
		fclose(log);
		return -1;
	}

	*p_size = _filelength(handle);
	if (!p_preallocated) {
		*p_data = malloc(*p_size);
	}
	if (*p_data == NULL) {
		_close(handle);
		return -1;
	}

	if (_read(handle, *p_data, *p_size) != *p_size) {
		if (!p_preallocated) {
			free(*p_data);
		}
		_close(handle);
		return -1;
	}

	return handle;
}

// Prefix a bare name with the base directory; leave paths containing a separator unchanged.
// FUNCTION: MW2SHELL 0x10031a8f
MechChar* FUN_10031a8f(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_unk0x1008ff58[i] = '\0';
	}

	if (g_unk0x10067708[0] != '\0' && strchr(p_name, '\\') == NULL && strchr(p_name, '/') == NULL) {
		sprintf(g_unk0x1008ff58, "%s\\%s", g_unk0x10067708, p_name);
	}
	else {
		strcpy(g_unk0x1008ff58, p_name);
	}

	return g_unk0x1008ff58;
}

// Prefix a name with the base directory without checking for path separators.
// FUNCTION: MW2SHELL 0x10031b51
MechChar* FUN_10031b51(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_unk0x1008ff58[i] = '\0';
	}

	if (g_unk0x10067708[0] != '\0') {
		sprintf(g_unk0x1008ff58, "%s\\%s", g_unk0x10067708, p_name);
	}
	else {
		strcpy(g_unk0x1008ff58, p_name);
	}

	return g_unk0x1008ff58;
}
