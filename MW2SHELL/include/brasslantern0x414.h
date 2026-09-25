#ifndef BRASSLANTERN0X414_H
#define BRASSLANTERN0X414_H

#include "decomp.h"
#include "types.h"

class VideoDriver;

// SIZE 0x414
class BrassLantern0x414 {
public:
	BrassLantern0x414(void* p_data, VideoDriver* p_videoDriver);

private:
	undefined m_unk0x00[0x414]; // 0x00
};

#endif // BRASSLANTERN0X414_H
