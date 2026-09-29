#include "config.h"

#include "decomp.h"
#include "loadres.h"
#include "render.h"
#include "simmain.h"
#include "soundconfig.h"
#include "staticmem.h"
#include "types.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(DifficultyCfg, 0x17)

enum FilePermission {
	c_permissionWrite = 0x80 // _S_IWRITE (sys/stat.h)
};

// The game directory (the MECHWARRIOR environment variable).
// GLOBAL: MW2 0x100ae400
MechChar g_gameDir[256] = {0};

// The path BuildGamePath returns.
// GLOBAL: MW2 0x100bef58
MechChar g_gamePath[0x50];

// Writes the screen as a picture: a 0x20-byte header from TABL resource 15, the palette, then
// the main pixel buffer. Returns whether it could.
// Stack-slot permutation: header, file and pixels.
// FUNCTION: MW2 0x10071026
MechS32 FUN_10071026(MechChar* p_path, void* p_palette)
{
	void* header;
	MechS32 file;
	undefined* pixels;

	header = NULL;
	file = open(p_path, _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file == -1) {
		return FALSE;
	}

	header = FUN_1001a19f(g_unk0x100a8740, 15, g_unk0x100a8698, 1);
	if (header == NULL) {
		close(file);
		return FALSE;
	}

	write(file, header, 0x20);
	write(file, p_palette, 0x300);
	pixels = g_mainPixelBuffer.m_pixels;
	write(file, pixels, g_screenPixelCount);
	close(file);
	FUN_1001a163(15, g_unk0x100a8698);
	return TRUE;
}

// Reads a whole file into memory from the heap, or from a static pool when p_poolTag is given.
// Returns the open file, or -1 (logging the path to symlog.txt when it can't be opened).
// Stack-slot permutation: log and file.
// FUNCTION: MW2 0x10071108
MechS32 LoadFile(MechChar* p_path, MechS32* p_size, void** p_data, MechU32* p_poolTag)
{
	FILE* log;
	MechS32 file;

	file = open(p_path, _O_BINARY);
	if (file == -1) {
		log = fopen("symlog.txt", "a");
		if (log) {
			fprintf(log, "Couldn't load ID=%s\n", p_path);
		}
		fclose(log);
		return -1;
	}

	*p_size = filelength(file);
	if (p_poolTag == NULL) {
		*p_data = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, *p_size);
	}
	else {
		*p_data = StaticPoolAlloc(*p_size, *p_poolTag);
	}

	if (*p_data == NULL) {
		close(file);
		return -1;
	}

	if (read(file, *p_data, *p_size) != *p_size) {
		if (p_poolTag == NULL) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, *p_data);
		}
		close(file);
		return -1;
	}

	return file;
}

// Reads a game file into memory. Returns the file (closed), or -1.
// Stack-slot permutation: file and data.
// FUNCTION: MW2 0x10071251
MechS32 FUN_10071251(MechChar* p_name, void** p_data)
{
	MechS32 size;
	MechS32 file;
	void* data;

	*p_data = NULL;
	file = LoadFile(BuildGamePath(p_name), &size, &data, NULL);
	if (file != -1) {
		close(file);
		*p_data = data;
	}

	return file;
}

// FUNCTION: MW2 0x100712b0
MechS32 FUN_100712b0(MechChar* p_name, void* p_data)
{
	MechS32 file;
	MechS32 result;

	file = open(BuildGamePath(p_name), _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file != -1) {
		write(file, p_data, 0xd6);
		close(file);
		result = 0;
	}
	else {
		result = -1;
	}

	return result;
}

// Reads the difficulty settings into a new block, then overrides some of them in network games.
// Returns 1, or -1 if there is no block.
// Stack-slot permutation: file and data.
// FUNCTION: MW2 0x10071326
MechS32 LoadDifficultyCfg(MechChar* p_name, DifficultyCfg** p_cfg)
{
	MechS32 size;
	MechS32 file;
	void* data;

	*p_cfg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(DifficultyCfg));
	file = LoadFile(BuildGamePath(p_name), &size, &data, NULL);
	if (file != -1) {
		memcpy(*p_cfg, data, size);
	}
	else if (*p_cfg == NULL) {
		return -1;
	}

	close(file);
	if (g_isNetworkGame) {
		(*p_cfg)->m_unk0x05 = 2;
		(*p_cfg)->m_unk0x01 = 0;
	}
	else {
		(*p_cfg)->m_unk0x0f = 0;
		(*p_cfg)->m_unk0x0b = 0;
		(*p_cfg)->m_unk0x13 = 0;
		(*p_cfg)->m_unk0x09 = 1;
	}

	if (g_isNetworkGame > 1) {
		(*p_cfg)->m_heatTracking = 1;
		(*p_cfg)->m_unk0x00 = 0;
		(*p_cfg)->m_splashDamage = 1;
		(*p_cfg)->m_unk0x03 = 1;
	}

	return 1;
}

// FUNCTION: MW2 0x10071440
MechS32 SaveDifficultyCfg(MechChar* p_name, DifficultyCfg* p_cfg)
{
	MechS32 file;
	MechS32 result;

	file = open(BuildGamePath(p_name), _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file != -1) {
		write(file, p_cfg, sizeof(DifficultyCfg));
		close(file);
		result = 0;
	}
	else {
		result = -1;
	}

	return result;
}

// Reads the sound settings, or allocates cleared ones. Returns 1, or -1 if it couldn't read them.
// Stack-slot permutation: file and data.
// FUNCTION: MW2 0x100714b3
MechS32 LoadSndCfg(MechChar* p_name, SoundConfig** p_cfg)
{
	MechS32 size;
	MechS32 file;
	void* data;

	file = LoadFile(BuildGamePath(p_name), &size, &data, NULL);
	if (file != -1) {
		*p_cfg = data;
	}
	else {
		*p_cfg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(SoundConfig));
		return -1;
	}

	close(file);
	return 1;
}

// FUNCTION: MW2 0x1007152f
MechS32 SaveSndCfg(MechChar* p_name, SoundConfig* p_cfg)
{
	MechS32 file;
	MechS32 result;

	file = open(BuildGamePath(p_name), _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file != -1) {
		write(file, p_cfg, sizeof(SoundConfig));
		close(file);
		result = 0;
	}
	else {
		result = -1;
	}

	return result;
}

// Returns the path of a game file: in g_gameDir unless the name has a directory already.
// FUNCTION: MW2 0x1007162a
MechChar* BuildGamePath(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_gamePath[i] = 0;
	}

	if (g_gameDir[0] && !strchr(p_name, '\\') && !strchr(p_name, '/')) {
		sprintf(g_gamePath, "%s\\%s", g_gameDir, p_name);
	}
	else {
		strcpy(g_gamePath, p_name);
	}

	return g_gamePath;
}

// Returns the path of a file in g_gameDir.
// FUNCTION: MW2 0x100716ec
MechChar* FUN_100716ec(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_gamePath[i] = 0;
	}

	if (g_gameDir[0]) {
		sprintf(g_gamePath, "%s\\%s", g_gameDir, p_name);
	}
	else {
		strcpy(g_gamePath, p_name);
	}

	return g_gamePath;
}
