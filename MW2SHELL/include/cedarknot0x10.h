#ifndef CEDARKNOT0X10_H
#define CEDARKNOT0X10_H

#include "decomp.h"
#include "types.h"

// SIZE 0x10
class CedarKnot0x10 {
public:
	CedarKnot0x10(const char* p_name);
	~CedarKnot0x10();

private:
	MechS32 m_unk0x00;    // 0x00
	undefined4 m_unk0x04; // 0x04
	undefined4 m_unk0x08; // 0x08
	undefined4 m_unk0x0c; // 0x0c
};

#endif // CEDARKNOT0X10_H
