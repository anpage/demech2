#ifndef TEXTGLYPHLIST_H
#define TEXTGLYPHLIST_H

#include "collection.h"
#include "decomp.h"
#include "types.h"

class TextGlyph;

// SIZE 0x04
class TextGlyphList {
public:
	TextGlyphList();
	~TextGlyphList();

	void FUN_1003e171(TextGlyph* p_item);
	void FUN_1003e19b(TextGlyph* p_item);
	void FUN_1003e1e6(MechU8 p_delete);
	void FUN_1003e286();

private:
	Collection* m_items; // 0x00
};

#endif // TEXTGLYPHLIST_H
