#include "config.h"

#include "ammobin.h"
#include "anim2d.h"
#include "approxlen.h"
#include "bargauges.h"
#include "cockpit.h"
#include "cockpitframe.h"
#include "cockpitpanel.h"
#include "damagepanel.h"
#include "decomp.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixedfloat.h"
#include "gamekeys.h"
#include "hud.h"
#include "loadres.h"
#include "mech.h"
#include "mechdamage.h"
#include "mechviewpanel.h"
#include "midi.h"
#include "mw2prj.h"
#include "network.h"
#include "objectanim.h"
#include "palette.h"
#include "players.h"
#include "point.h"
#include "poolsizes.h"
#include "random.h"
#include "recttransition.h"
#include "reel.h"
#include "render.h"
#include "resource.h"
#include "resourcename.h"
#include "resourceref.h"
#include "screenscale.h"
#include "screenshot.h"
#include "simmain.h"
#include "soundconfig.h"
#include "soundfx.h"
#include "speech.h"
#include "staticmem.h"
#include "statuspanels.h"
#include "targeting.h"
#include "targetpanel.h"
#include "types.h"
#include "weapondata.h"
#include "weaponpanel.h"
#include "weapons.h"
#include "weaponslot.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(DifficultyCfg, 0x17)
DECOMP_SIZE_ASSERT(Reel, 0x14)
DECOMP_SIZE_ASSERT(CockpitFrame, 0x8)

enum FilePermission {
	c_permissionWrite = 0x80 // _S_IWRITE (sys/stat.h)
};

// The cockpit panels: 1.1 has them here, the Matrox edition at the end of staticmem.c's object.
#ifndef MW2_MATROX
#include "cockpitpanels.c"
#endif

// The game directory (the MECHWARRIOR environment variable).
// GLOBAL: MW2 0x100ae400
// GLOBAL: MW2MATROX 0x100a3f00
MechChar g_gameDir[256] = {0};

// The number of the next screenshot SaveScreenshot saves.
// GLOBAL: MW2 0x100ae500
// GLOBAL: MW2MATROX 0x100a4000
MechS32 g_screenshotCount = 0;

// The path BuildGamePath returns.
// GLOBAL: MW2 0x100bef58
// GLOBAL: MW2MATROX 0x100c1d20
MechChar g_gamePath[0x50];

// The three values of the HUD layout (LoadHudFile).
// GLOBAL: MW2 0x10109c30
// GLOBAL: MW2MATROX 0x1015e8b0
MechS32 g_hudLayoutValues[3];

// Loads seven values from resource p_ref.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100707c0
MechS32 LoadMgdFile(
	ResourceRef* p_ref,
	MechS32* p_height,
	MechS32* p_cockpitHeight,
	MechS32* p_unk0x0c,
	MechS32* p_unk0x10,
	MechS32* p_unk0x14,
	MechS32* p_maxTorsoTwist,
	MechS32* p_radius
)
{
	MechS32 size;
	MechS32* data;
	MechS32* cursor;
	FILE* file;

	data = LoadResourceByRef(
		p_ref,
		g_resourceTypeTags[c_resTagMgeo],
		g_resourceTypeExtensions[c_resExtMgi],
		5,
		&size,
		NULL
	);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_resourceTypeTags[c_resTagMgeo]);
			fclose(file);
		}

		return FALSE;
	}

	cursor = data;
	*p_height = *cursor;
	cursor++;
	*p_cockpitHeight = *cursor;
	cursor++;
	*p_unk0x0c = *cursor;
	cursor++;
	*p_unk0x10 = *cursor;
	cursor++;
	*p_unk0x14 = *cursor;
	cursor++;
	*p_maxTorsoTwist = *cursor;
	cursor++;
	*p_radius = *cursor;
	if (p_ref->m_id == -1) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		UnlockCachedResource(p_ref->m_id, g_resourceTypeTags[c_resTagMgeo]);
	}

	return TRUE;
}

// Loads the animation file p_ref: up to 32 animations, numbered from the current base
// (GetAnimBase), into g_reels.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100708f4
// FUNCTION: MW2MATROX 0x1000ae34
MechS32 LoadReels(ResourceRef* p_ref)
{
	MechS32 ids[32];
	MechS32 size;
	MechS32 unk0x08;
	MechU8* frames;
	MechS32 base;
	MechS32 offset;
	MechS32 count;
	MechS32 frameCount;
	MechS32 i;
	MechU8* data;
	MechS32 stride;
	MechU8* end;
	MechS32 index;
	FILE* file;
#ifdef MW2_MATROX
	MechS32 j;
	MechS32* amount;
#endif

	offset = 0;
	stride = sizeof(MechS32);
	data = LoadResourceByRef(
		p_ref,
		g_resourceTypeTags[c_resTagAnim],
		g_resourceTypeExtensions[c_resExt3di],
		2,
		&size,
		&g_staticPoolTags[6]
	);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_resourceTypeTags[c_resTagAnim]);
		}

		fclose(file);
		return FALSE;
	}

	count = *(MechS32*) data;
	frameCount = *(MechS32*) (data + 4);
	offset = stride * 2;
	base = GetAnimBase();
	for (i = 0; i < count; i++) {
		index = *(MechS32*) (data + offset);
		offset += stride;
		unk0x08 = *(MechS32*) (data + offset);
		offset += stride;
		if (index >= 0x20) {
			return FALSE;
		}

		index += base;
		if (index >= 0x780) {
			return FALSE;
		}

		frames = data + offset;
		offset += frameCount * sizeof(MechS32);
		g_reels[index] = StaticPoolAlloc(sizeof(Reel), g_staticPoolTags[5]);
		if (!g_reels[index]) {
			return FALSE;
		}
#ifdef MW2_MATROX

		// The Matrox edition's reels hold floats: plain values below kind 3, 16.16 from it.
		if (frameCount) {
			for (j = 0; j < frameCount; j++) {
				amount = (MechS32*) frames + j;
				if (unk0x08 < 3) {
					((MechFloat*) frames)[j] = *amount;
				}
				else {
					((MechFloat*) frames)[j] = *amount * (1.0f / 65536.0f);
				}
			}
		}
#endif

		g_reels[index]->m_amounts = (MechS32*) frames;
		g_reels[index]->m_kind = unk0x08;
		g_reels[index]->m_frameCount = frameCount;
		if (p_ref->m_id == -1) {
			g_reels[index]->m_byName = 1;
		}
		else {
			g_reels[index]->m_byName = 0;
		}

		ids[i] = index;
	}

	end = data + offset;
	for (i = 0; i < count; i++) {
		g_reels[ids[i]]->m_events = (ReelEvent*) end;
	}

	return TRUE;
}

// Loads the cockpit layout resource p_ref: the gauge positions (g_hudGaugePositions), three values
// (g_hudLayoutValues) and fifteen rectangles in percent of the screen (g_outlinePartRects), any of
// them out of range cleared.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10070bda
// FUNCTION: MW2MATROX 0x1000b1c6
MechS32 LoadHudFile(ResourceRef* p_ref)
{
	MechS32 size;
	MechS32 value;
	MechS32 i;
	MechS32* data;
	MechS32* cursor;
	MechS32 right;
	MechS32 bottom;
	MechS32 left;
	MechS32 top;
	FILE* file;

	data = LoadResourceByRef(
		p_ref,
		g_resourceTypeTags[c_resTagHud],
		g_resourceTypeExtensions[c_resExtHdi],
		4,
		&size,
		NULL
	);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_resourceTypeTags[c_resTagHud]);
		}

		fclose(file);
		return FALSE;
	}

	cursor = data;
	for (i = 0; i < 5; i++) {
		g_hudGaugePositions[i].m_x = *cursor;
		cursor++;
		g_hudGaugePositions[i].m_y = *cursor;
		cursor++;
	}

	for (i = 0; i < 3; i++) {
		value = *cursor;
		cursor++;
		g_hudLayoutValues[i] = value;
	}

	for (i = 0; i < 15; i++) {
		left = *cursor;
		cursor++;
		top = *cursor;
		cursor++;
		right = *cursor;
		cursor++;
		bottom = *cursor;
		cursor++;
		if (left < 0 || left > 100 || top < 0 || top > 100 || right < 0 || right > 100 || bottom < 0 || bottom > 100) {
			left = top = right = bottom = 0;
		}

		g_outlinePartRects[i].m_x0 = left;
		g_outlinePartRects[i].m_y0 = top;
		g_outlinePartRects[i].m_x1 = right;
		g_outlinePartRects[i].m_y1 = bottom;
	}

	if (p_ref->m_id == -1) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		UnlockCachedResource(p_ref->m_id, g_resourceTypeTags[c_resTagHud]);
	}

	return TRUE;
}

// Loads the cockpit layout resource p_ref: five rectangles into p_gauges, fifteen into p_panels
// and a point.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10070e22
// FUNCTION: MW2MATROX 0x1000b40e
MechS32 LoadCptFile(ResourceRef* p_ref, PANE* p_gauges, PANE* p_panels, Point* p_point)
{
	MechS32 size;
	PANE* target;
	MechS32 i;
	CockpitFrame* frame;
	void* data;
	MechS16* value;
	FILE* file;

	if (!p_gauges || !p_panels || !p_point) {
		return FALSE;
	}

	data = LoadResourceByRef(
		p_ref,
		g_resourceTypeTags[c_resTagCpit],
		g_resourceTypeExtensions[c_resExtCpi],
		3,
		&size,
		NULL
	);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_resourceTypeTags[c_resTagCpit]);
		}

		fclose(file);
		return FALSE;
	}

	target = p_gauges;
	frame = data;
	for (i = 0; i < 5; i++) {
		target->m_x0 = frame->m_x;
		target->m_y0 = frame->m_y;
		target->m_x1 = frame->m_x + frame->m_width - 1;
		target->m_y1 = frame->m_y + frame->m_height - 1;
		frame++;
		target++;
	}

	target = p_panels;
	for (i = 0; i < 15; i++) {
		target->m_x0 = frame->m_x;
		target->m_y0 = frame->m_y;
		target->m_x1 = frame->m_x + frame->m_width - 1;
		target->m_y1 = frame->m_y + frame->m_height - 1;
		frame++;
		target++;
	}

	value = (MechS16*) frame;
	p_point->m_x = *value;
	value++;
	p_point->m_y = *value;
	value++;
	if (p_ref->m_id == -1) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		UnlockCachedResource(p_ref->m_id, g_resourceTypeTags[c_resTagCpit]);
	}

	return TRUE;
}

// Writes the screen as a picture: a 0x20-byte header from TABL resource 15, the palette, then
// the main pixel buffer. Returns whether it could.
// Stack-slot permutation: header, file and pixels.
// FUNCTION: MW2 0x10071026
// FUNCTION: MW2MATROX 0x1000b612
MechS32 WriteScreenPicture(MechChar* p_path, void* p_palette)
{
	void* header;
	MechS32 file;
	undefined* pixels;

	header = NULL;
	file = open(p_path, _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file == -1) {
		return FALSE;
	}

	header = LoadCachedResource(g_mw2PrjHandle, 15, g_resourceTypeTags[c_resTagTable], 1);
	if (header == NULL) {
		close(file);
		return FALSE;
	}

	write(file, header, 0x20);
	write(file, p_palette, 0x300);
	pixels = g_mainPixelBuffer.m_buffer;
	write(file, pixels, g_screenPixelCount);
	close(file);
	UnlockCachedResource(15, g_resourceTypeTags[c_resTagTable]);
	return TRUE;
}

// Reads a whole file into memory from the heap, or from a static pool when p_poolTag is given.
// Returns the open file, or -1 (logging the path to symlog.txt when it can't be opened).
// Stack-slot permutation: log and file.
// FUNCTION: MW2 0x10071108
// FUNCTION: MW2MATROX 0x1000b6f4
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
// FUNCTION: MW2MATROX 0x1000b83d
MechS32 ReadGameFile(MechChar* p_name, void** p_data)
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
// FUNCTION: MW2MATROX 0x1000b89c
MechS32 WriteCareerRecordFile(MechChar* p_name, void* p_data)
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
// FUNCTION: MW2MATROX 0x1000b912
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
		(*p_cfg)->m_enemySkill = 2;
		(*p_cfg)->m_invulnerable = 0;
	}
	else {
		(*p_cfg)->m_gravity = 0;
		(*p_cfg)->m_timeOfDay = 0;
		(*p_cfg)->m_temperature = 0;
		(*p_cfg)->m_radar = 1;
	}

	if (g_isNetworkGame > 1) {
		(*p_cfg)->m_heatTracking = 1;
		(*p_cfg)->m_unlimitedAmmo = 0;
		(*p_cfg)->m_splashDamage = 1;
		(*p_cfg)->m_collisionDamage = 1;
	}

	return 1;
}

// FUNCTION: MW2 0x10071440
// FUNCTION: MW2MATROX 0x1000ba2c
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
// FUNCTION: MW2MATROX 0x1000ba9f
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
// FUNCTION: MW2MATROX 0x1000bb1b
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

// Saves the screen as the next of mw2NNNN.gif, up to 1000 of them.
// FUNCTION: MW2 0x100715a2
// FUNCTION: MW2MATROX 0x1000bb8e
void SaveScreenshot(void)
{
	MechS32 count;
	PANE target;
	MechChar name[16];
#ifdef MW2_MATROX
	FILE* file;
	MechU16* pixel;
	MechU32 n;
	MechU32 rgb;
	MechU16 width;
	MechU16 height;
#endif

	target.m_window = &g_mainPixelBuffer;
	target.m_x0 = 0;
	target.m_y0 = 0;
	target.m_x1 = g_screenWidthMinus1;
	target.m_y1 = g_screenHeightMinus1;
	if (g_screenshotCount < 1000) {
		count = g_screenshotCount++;
#ifdef MW2_MATROX
		// The Matrox edition saves the 16-bit screen as raw 24-bit pixels, after a header of two words
		// it never sets.
		sprintf(name, "mw2%04d.888", count);
		file = fopen(name, "wb");
		if (!file) {
			return;
		}

		fwrite(&width, 2, 1, file);
		fwrite(&height, 2, 1, file);
		pixel = (MechU16*) g_mainPixelBuffer.m_buffer;
		for (n = (g_screenHeightMinus1 + 1) * (g_screenWidthMinus1 + 1); n > 0; n--) {
			rgb = ((*pixel >> 10) & 0x1f) * 8;
			rgb |= ((*pixel >> 5) & 0x1f) << 11;
			rgb |= (*pixel & 0x1f) << 19;
			fwrite(&rgb, 3, 1, file);
			pixel++;
		}

		fclose(file);
#else
		sprintf(name, "mw2%04d.gif", count);
		ScreenshotBegin(name);
		ScreenshotWritePalette();
		ScreenshotWriteImage(&target);
		ScreenshotEnd();
#endif
	}
}

// Returns the path of a game file: in g_gameDir unless the name has a directory already.
// FUNCTION: MW2 0x1007162a
// FUNCTION: MW2MATROX 0x1000bcd2
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
// FUNCTION: MW2MATROX 0x1000bd94
MechChar* BuildGameDirPath(MechChar* p_name)
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
