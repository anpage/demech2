/*
 * Build-time helper: writes an icon group of a PE image as an .ico file, or a bitmap
 * resource as a .bmp file.
 *
 *   extract_resource icon <image.dll> <group-id> <out.ico>
 *   extract_resource bitmap <image.dll> <id> <out.bmp>
 *
 * The game's icons and pictures are not in the repository; the .rc files take them from
 * the original binaries through this tool. Plain C89 and stdio, so the project's own
 * toolchain (VC++ 4.1 included) builds it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RT_BITMAP_ID 2
#define RT_ICON_ID 3
#define RT_GROUP_ICON_ID 14

static unsigned char* g_image;
static unsigned long g_imageSize;

static unsigned long ReadU16(unsigned long p_offset)
{
	return g_image[p_offset] | ((unsigned long) g_image[p_offset + 1] << 8);
}

static unsigned long ReadU32(unsigned long p_offset)
{
	return ReadU16(p_offset) | (ReadU16(p_offset + 2) << 16);
}

static void WriteU16(FILE* p_file, unsigned long p_value)
{
	fputc((int) (p_value & 0xff), p_file);
	fputc((int) ((p_value >> 8) & 0xff), p_file);
}

static void WriteU32(FILE* p_file, unsigned long p_value)
{
	WriteU16(p_file, p_value & 0xffff);
	WriteU16(p_file, p_value >> 16);
}

static void Fail(const char* p_message)
{
	fprintf(stderr, "extract_resource: %s\n", p_message);
	exit(1);
}

static void Check(unsigned long p_offset, unsigned long p_size)
{
	if (p_offset > g_imageSize || p_size > g_imageSize - p_offset) {
		Fail("truncated or malformed image");
	}
}

/* Maps an RVA to a file offset through the section table. */
static unsigned long RvaToOffset(unsigned long p_rva)
{
	unsigned long pe = ReadU32(0x3c);
	unsigned long sections = ReadU16(pe + 6);
	unsigned long section = pe + 24 + ReadU16(pe + 20);
	unsigned long i;

	for (i = 0; i < sections; i++, section += 40) {
		unsigned long address;
		unsigned long size;

		Check(section, 40);
		address = ReadU32(section + 12);
		size = ReadU32(section + 16);
		if (p_rva >= address && p_rva < address + size) {
			return ReadU32(section + 20) + (p_rva - address);
		}
	}

	Fail("RVA outside every section");
	return 0;
}

/* Returns the offset (relative to the resource root) of the entry with ID p_id in the
 * directory at p_directory, or 0 if there is none. */
static unsigned long FindEntry(unsigned long p_root, unsigned long p_directory, unsigned long p_id)
{
	unsigned long named;
	unsigned long ids;
	unsigned long entry;
	unsigned long i;

	Check(p_root + p_directory, 16);
	named = ReadU16(p_root + p_directory + 12);
	ids = ReadU16(p_root + p_directory + 14);
	entry = p_root + p_directory + 16 + named * 8;

	for (i = 0; i < ids; i++, entry += 8) {
		Check(entry, 8);
		if (ReadU32(entry) == p_id) {
			return ReadU32(entry + 4);
		}
	}

	return 0;
}

/* Finds the data of resource p_type/p_id (first language); returns its file offset. */
static unsigned long LookupResource(
	unsigned long p_root,
	unsigned long p_type,
	unsigned long p_id,
	unsigned long* p_size
)
{
	unsigned long entry = FindEntry(p_root, 0, p_type);
	unsigned long data;

	if (!(entry & 0x80000000)) {
		Fail("resource type not found");
	}
	entry = FindEntry(p_root, entry & 0x7fffffff, p_id);
	if (!(entry & 0x80000000)) {
		Fail("resource not found");
	}

	/* First language */
	Check(p_root + (entry & 0x7fffffff), 24);
	entry = ReadU32(p_root + (entry & 0x7fffffff) + 16 + 4);
	if (entry & 0x80000000) {
		Fail("malformed resource directory");
	}

	Check(p_root + entry, 16);
	data = RvaToOffset(ReadU32(p_root + entry));
	*p_size = ReadU32(p_root + entry + 4);
	Check(data, *p_size);

	return data;
}

/* ICONDIR, then one ICONDIRENTRY per image: the group's GRPICONDIRENTRY with the
 * image's file offset in place of its resource ID. */
static void WriteIcon(FILE* p_file, unsigned long p_root, unsigned long p_id)
{
	unsigned long groupSize;
	unsigned long group = LookupResource(p_root, RT_GROUP_ICON_ID, p_id, &groupSize);
	unsigned long count;
	unsigned long offset;
	unsigned long i;

	if (groupSize < 6) {
		Fail("malformed icon group");
	}
	count = ReadU16(group + 4);
	if (groupSize < 6 + count * 14) {
		Fail("malformed icon group");
	}

	WriteU16(p_file, 0);
	WriteU16(p_file, 1);
	WriteU16(p_file, count);
	offset = 6 + count * 16;
	for (i = 0; i < count; i++) {
		unsigned long entry = group + 6 + i * 14;

		fwrite(g_image + entry, 1, 12, p_file);
		WriteU32(p_file, offset);
		offset += ReadU32(entry + 8);
	}

	for (i = 0; i < count; i++) {
		unsigned long entry = group + 6 + i * 14;
		unsigned long size;
		unsigned long data = LookupResource(p_root, RT_ICON_ID, ReadU16(entry + 12), &size);

		if (size != ReadU32(entry + 8)) {
			Fail("icon size differs from its group entry");
		}
		fwrite(g_image + data, 1, size, p_file);
	}
}

/* BITMAPFILEHEADER, then the resource: a BITMAPINFOHEADER, its color table and the
 * pixels, which is what RC strips from a .bmp. */
static void WriteBitmap(FILE* p_file, unsigned long p_root, unsigned long p_id)
{
	unsigned long size;
	unsigned long data = LookupResource(p_root, RT_BITMAP_ID, p_id, &size);
	unsigned long headerSize;
	unsigned long bitCount;
	unsigned long colors;

	if (size < 40) {
		Fail("malformed bitmap");
	}
	headerSize = ReadU32(data);
	bitCount = ReadU16(data + 14);
	colors = ReadU32(data + 32);
	if (headerSize != 40 || bitCount > 8) {
		Fail("unsupported bitmap format (only 1-8 bpp BITMAPINFOHEADER bitmaps)");
	}
	if (colors == 0) {
		colors = 1UL << bitCount;
	}

	fputc('B', p_file);
	fputc('M', p_file);
	WriteU32(p_file, 14 + size);
	WriteU32(p_file, 0);
	WriteU32(p_file, 14 + headerSize + colors * 4);
	fwrite(g_image + data, 1, size, p_file);
}

int main(int p_argc, char** p_argv)
{
	FILE* file;
	unsigned long pe;
	unsigned long root;
	int icon;

	if (p_argc != 5 || (strcmp(p_argv[1], "icon") != 0 && strcmp(p_argv[1], "bitmap") != 0)) {
		fprintf(stderr, "usage: extract_resource icon <image.dll> <group-id> <out.ico>\n");
		fprintf(stderr, "       extract_resource bitmap <image.dll> <id> <out.bmp>\n");
		return 2;
	}
	icon = strcmp(p_argv[1], "icon") == 0;

	file = fopen(p_argv[2], "rb");
	if (!file) {
		Fail("cannot open image");
	}
	fseek(file, 0, SEEK_END);
	g_imageSize = (unsigned long) ftell(file);
	fseek(file, 0, SEEK_SET);
	g_image = (unsigned char*) malloc(g_imageSize);
	if (!g_image || fread(g_image, 1, g_imageSize, file) != g_imageSize) {
		Fail("cannot read image");
	}
	fclose(file);

	Check(0x3c, 4);
	pe = ReadU32(0x3c);
	Check(pe, 24 + 96 + 3 * 8);
	if (ReadU32(pe) != 0x4550 || ReadU16(pe + 24) != 0x10b) {
		Fail("not a PE32 image");
	}
	root = RvaToOffset(ReadU32(pe + 24 + 96 + 2 * 8));

	file = fopen(p_argv[4], "wb");
	if (!file) {
		Fail("cannot create output");
	}
	if (icon) {
		WriteIcon(file, root, strtoul(p_argv[3], NULL, 0));
	}
	else {
		WriteBitmap(file, root, strtoul(p_argv[3], NULL, 0));
	}

	if (fclose(file) != 0) {
		Fail("cannot write output");
	}

	return 0;
}
