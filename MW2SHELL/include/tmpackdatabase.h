#ifndef TMPACKDATABASE_H
#define TMPACKDATABASE_H

#include "decomp.h"
#include "types.h"

// SIZE 0x8c
class TMPackDataBase {
public:
	TMPackDataBase(char* p_name);
	~TMPackDataBase();

	void GetDBItem(MechS32 p_id, void** p_data, MechS32* p_size);

private:
	undefined m_unk0x00[0x8c]; // 0x00
};

#endif // TMPACKDATABASE_H
