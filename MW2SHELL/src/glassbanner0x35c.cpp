#include "glassbanner0x35c.h"

#include "audiosample.h"
#include "shellmain.h"
#include "videodriver.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(GlassBanner0x35c, 0x35c)

// The blit routines live in the blit unit, a C translation unit (hand-written assembly).
extern "C"
{
	MechS32 FUN_10032f84(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top);
	void FUN_10034f18(
		PixelView* p_unk0x00,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		PixelView* p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18
	);
	MechS32 FUN_10037504(void* p_data, MechS32 p_index);
}

// The only diffs are operand order: m_width * m_height and m_width + m_left load m_width
// first in the original, and swapping them in the source doesn't change the output.
// FUNCTION: MW2SHELL 0x10045d60
GlassBanner0x35c::GlassBanner0x35c(
	undefined* p_data,
	MechS32 p_size,
	VideoDriver* p_videoDriver,
	MechS32 p_left,
	MechS32 p_top,
	AudioSample* p_sample
)
{
	MechS32 size;

	m_data = p_data;
	m_size = p_size;
	m_videoDriver = p_videoDriver;
	m_sample = p_sample;
	m_videoDriver->FUN_10006a99(&m_width, &m_height, m_data, m_size, 2);

	size = FUN_10037504(p_data, 0);
	m_width = (size >> 16) + 2;
	m_height = (size & 0xffff) + 2;
	m_left = p_left;
	m_top = p_top;
	m_left = (640 - m_width) / 2;
	m_top = (480 - m_height) / 2;

	m_saved.m_maxX = m_width - 1;
	m_saved.m_maxY = m_height - 1;
	m_saved.m_pixels = (undefined*) HeapAlloc(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_width * m_height);

	m_savedView.m_left = 0;
	m_savedView.m_top = 0;
	m_savedView.m_right = m_saved.m_maxX;
	m_savedView.m_bottom = m_saved.m_maxY;
	m_savedView.m_buffer = &m_saved;

	m_screenView.m_left = m_left;
	m_screenView.m_top = m_top;
	m_screenView.m_right = m_width + m_left - 1;
	m_screenView.m_bottom = m_height + m_top - 1;
	m_screenView.m_buffer = &m_videoDriver->m_backBuffer;

	FUN_10034f18(&m_screenView, 0, 0, &m_savedView, 0, 0, -1);
}

// FUNCTION: MW2SHELL 0x10045f19
GlassBanner0x35c::~GlassBanner0x35c()
{
	Hide();
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_data);

	if (m_sample) {
		m_sample->Stop();
	}
}

// FUNCTION: MW2SHELL 0x10045f69
void GlassBanner0x35c::Show()
{
	if (m_sample && !m_sample->IsPlaying()) {
		m_sample->Start();
	}

	FUN_10032f84(&m_videoDriver->m_backView, (undefined4) m_data, 0, 319, 239);
	m_videoDriver->FUN_100071ad(m_left, m_top, m_width, m_height);
	m_videoDriver->FUN_10007763(0);
	m_videoDriver->FUN_10007763(1);
}

// FUNCTION: MW2SHELL 0x1004601c
void GlassBanner0x35c::Hide()
{
	FUN_10034f18(&m_savedView, 0, 0, &m_screenView, 0, 0, -1);
	m_videoDriver->FUN_100071ad(m_left, m_top, m_width, m_height);
	m_videoDriver->FUN_10007763(0);
	m_videoDriver->FUN_10007763(1);
}
