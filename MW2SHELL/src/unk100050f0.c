#include "decomp.h"
#include "types.h"

#include <string.h>

// Resource names: each resource type has a TABL resource listing its entries' names
// (stored negated) and ids.

enum ResourceTag {
	c_tagTable = 9 // "TABL"
};

typedef struct ResourceEntry {
	char m_name[10]; // 0x00
	MechS16 m_id;    // 0x0a
} ResourceEntry;

typedef struct ResourceTable {
	undefined m_unk0x00[0x08];  // 0x00
	MechS16 m_count;            // 0x08
	undefined2 m_unk0x0a;       // 0x0a
	ResourceEntry m_entries[1]; // 0x0c
} ResourceTable;

void FUN_10013c79(MechS32 p_id, char* p_type);
void* FUN_10013cb5(MechS32 p_unk0x00, MechS32 p_id, char* p_type, MechS32 p_unk0x0c);

extern char* g_unk0x1006a9f8[26];
extern MechS32 g_unk0x1006aac4;

// Stack-slot permutation: every local, name included, takes a different [ebp-N] slot. The
// original side also can't name g_unk0x1006a9f8 until it is annotated (see unk1003bfb0.c).
// FUNCTION: MW2SHELL 0x100050f0
MechS32 FindResourceIdByName(MechS32 p_type, char* p_name)
{
	MechS32 j;
	ResourceTable* table;
	MechS32 id;
	ResourceTable* data;
	ResourceEntry* entry;
	MechS32 i;
	MechS32 c;
	char name[12];

	id = -1;
	data = FUN_10013cb5(g_unk0x1006aac4, p_type, g_unk0x1006a9f8[c_tagTable], 1);
	if (data != NULL) {
		table = data;
		entry = data->m_entries;
		for (j = 0; j < 10; j++) {
			c = entry->m_name[j];
			c = 0x100 - c;
			name[j] = c;
		}

		i = 0;
		while (i < table->m_count && _strcmpi(name, p_name)) {
			entry++;
			i++;
			for (j = 0; j < 10; j++) {
				c = entry->m_name[j];
				c = 0x100 - c;
				name[j] = c;
			}
		}

		if (!_strcmpi(name, p_name)) {
			id = entry->m_id;
		}

		FUN_10013c79(p_type, g_unk0x1006a9f8[c_tagTable]);
	}

	return id;
}

// Stack-slot permutation: j, table, data, entry, result, i and c. The original side also
// can't name g_unk0x1006a9f8 until it is annotated (see unk1003bfb0.c).
// FUNCTION: MW2SHELL 0x10005232
char* FUN_10005232(MechS32 p_type, MechS32 p_id, char* p_buffer)
{
	MechS32 j;
	ResourceTable* table;
	ResourceTable* data;
	ResourceEntry* entry;
	char* result;
	MechS32 i;
	MechS32 c;

	result = NULL;
	if (p_buffer != NULL) {
		data = FUN_10013cb5(g_unk0x1006aac4, p_type, g_unk0x1006a9f8[c_tagTable], 1);
		if (data != NULL) {
			table = data;
			entry = data->m_entries;
			i = 0;
			while (i < table->m_count && entry->m_id != p_id) {
				entry++;
				i++;
			}

			if (entry->m_id == p_id) {
				result = p_buffer;
				for (j = 0; j < 9; j++) {
					c = entry->m_name[j];
					c = 0x100 - c;
					result[j] = c;
				}
				result[8] = '\0';
			}

			FUN_10013c79(p_type, g_unk0x1006a9f8[c_tagTable]);
		}
	}

	return result;
}
