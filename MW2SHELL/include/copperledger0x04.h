#ifndef COPPERLEDGER0X04_H
#define COPPERLEDGER0X04_H

#include "decomp.h"
#include "types.h"

// SIZE 0x04
class CopperLedger0x04 {
public:
	CopperLedger0x04();
	~CopperLedger0x04();

	void FUN_1003e171(undefined4 p_item);
	void FUN_1003e19b(undefined4 p_item);
	void FUN_1003e1e6(MechS8 p_unk0x00);
	void FUN_1003e286();

private:
	undefined4 m_unk0x00; // 0x00
};

#endif // COPPERLEDGER0X04_H
