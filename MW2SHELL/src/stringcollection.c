#include "collection.h"
#include "decomp.h"
#include "stringutil.h"
#include "types.h"

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String collections: Collections of heap-copied strings, ordered by strcmp when sorted.

MechU8 FUN_10005a21(Collection* p_collection, MechChar* p_string, MechChar** p_found);
int FUN_10005df8(const void* p_a, const void* p_b);

// FUNCTION: MW2SHELL 0x10005970
Collection* FUN_10005970(MechS32 p_flags)
{
	Collection* collection;
	MechS32 result;

	result = CreateCollection(&collection, 10, free, (MechU8) p_flags, FUN_10005df8);
	if (result != 0) {
		return NULL;
	}

	return collection;
}

// FUNCTION: MW2SHELL 0x100059b6
MechU8 FUN_100059b6(Collection* p_collection, MechChar* p_string)
{
	MechChar* copy;

	if (p_string == NULL) {
		return TRUE;
	}

	if (FUN_10005a21(p_collection, p_string, NULL) == TRUE) {
		return FALSE;
	}

	copy = FUN_10030900(p_string);
	ExpandCollection(p_collection, copy);
	return TRUE;
}

// FUNCTION: MW2SHELL 0x10005a21
MechU8 FUN_10005a21(Collection* p_collection, MechChar* p_string, MechChar** p_found)
{
	MechS32 i;
	MechChar** entry;
	MechChar* item;

	if (p_collection->m_flags & c_collectionSorted) {
		FUN_1003c59e(p_collection);
		entry =
			bsearch(&p_string, p_collection->m_items, p_collection->m_count, sizeof(void*), p_collection->m_compare);
		if (entry == NULL) {
			return FALSE;
		}

		item = *entry;
		if (item != NULL) {
			if (p_found != NULL) {
				*p_found = item;
			}

			return TRUE;
		}
	}
	else {
		for (i = 0; i < p_collection->m_count; i++) {
			item = CollectionGet(p_collection, i);
			if (!strcmp(item, p_string)) {
				if (p_found != NULL) {
					*p_found = item;
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

// Moves every string of p_source into p_collection, freeing the ones it already holds.
// FUNCTION: MW2SHELL 0x10005b4f
void FUN_10005b4f(Collection* p_collection, Collection* p_source)
{
	MechS32 i;
	MechChar* item;

	for (i = 0; i < p_source->m_count; i++) {
		item = CollectionGet(p_source, i);
		p_source->m_items[i] = NULL;
		if (!FUN_100059b6(p_collection, item)) {
			p_source->m_destroyItem(item);
			p_source->m_items[i] = NULL;
		}
	}

	ClearCollection(p_source);
}

// FUNCTION: MW2SHELL 0x10005be8
void FUN_10005be8(Collection* p_collection, MechChar* p_string)
{
	FUN_1003c3ba(p_collection, p_string, TRUE);
}

// FUNCTION: MW2SHELL 0x10005c05
void FUN_10005c05(Collection* p_collection, MechChar* p_string)
{
	MechChar* copy;

	copy = FUN_10030900(p_string);
	ExpandCollection(p_collection, copy);
}

// FUNCTION: MW2SHELL 0x10005c32
MechU8 FUN_10005c32(Collection* p_a, MechS32 p_indexA, Collection* p_b, MechS32 p_indexB)
{
	MechChar* a;
	MechChar* b;

	a = CollectionGet(p_a, p_indexA);
	b = CollectionGet(p_b, p_indexB);
	if (!strcmp(a, b)) {
		return TRUE;
	}

	return FALSE;
}

// FUNCTION: MW2SHELL 0x10005cba
void FUN_10005cba(MechChar* p_title, Collection* p_collection)
{
	MechS32 i;

	fprintf(stdout, "%s\n", p_title);
	fflush(stdout);
	for (i = 0; i < p_collection->m_count; i++) {
		fprintf(stdout, "%d, %s\n", i, CollectionGet(p_collection, i));
		fflush(stdout);
	}

	fprintf(stdout, "\n");
	fflush(stdout);
}

// Stack-slot permutation: copy, i, collection and item.
// FUNCTION: MW2SHELL 0x10005d79
Collection* FUN_10005d79(Collection* p_source)
{
	MechChar* copy;
	MechS32 i;
	Collection* collection;
	MechChar* item;

	collection = FUN_10005970(p_source->m_flags);
	for (i = 0; i < p_source->m_count; i++) {
		item = CollectionGet(p_source, i);
		copy = FUN_10030900(item);
		ExpandCollection(collection, copy);
	}

	return collection;
}

// Stack-slot permutation: a, b, entryA and entryB.
// FUNCTION: MW2SHELL 0x10005df8
int FUN_10005df8(const void* p_a, const void* p_b)
{
	MechChar* a;
	MechChar* b;
	MechChar** entryA;
	MechChar** entryB;

	entryA = (MechChar**) p_a;
	entryB = (MechChar**) p_b;
	a = *entryA;
	b = *entryB;
	return strcmp(a, b);
}
