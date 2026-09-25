#ifndef COPPERLEDGER0X04_H
#define COPPERLEDGER0X04_H

#include "collection.h"
#include "decomp.h"
#include "types.h"

class EmberGlyph0x3e;

// SIZE 0x04
class CopperLedger0x04 {
public:
	CopperLedger0x04();
	~CopperLedger0x04();

	void FUN_1003e171(EmberGlyph0x3e* p_item);
	void FUN_1003e19b(EmberGlyph0x3e* p_item);
	void FUN_1003e1e6(MechU8 p_delete);
	void FUN_1003e286();

private:
	Collection* m_items; // 0x00
};

#endif // COPPERLEDGER0X04_H
