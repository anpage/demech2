#ifndef AUDIOSAMPLE_H
#define AUDIOSAMPLE_H

#include "decomp.h"
#include "mss.h"
#include "types.h"

class AudioSample {
public:
	~AudioSample();

private:
	undefined4 m_unk0x00; // 0x00
	void* m_data;         // 0x04
	undefined4 m_unk0x08; // 0x08
	HSAMPLE m_sample;     // 0x0c
};

#endif // AUDIOSAMPLE_H
