#include "archivereader.h"

#include "audiosample.h"
#include "buttonmenu.h"
#include "collection.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "mainmenubutton.h"
#include "menudata.h"
#include "menuscreen.h"
#include "mousestate.h"
#include "page.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "stringutil.h"
#include "textglyph.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk1003bf90.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(ArchiveReader, 0x1b9)
DECOMP_SIZE_ASSERT(ArchiveReader::Topic, 0x06)

// GLOBAL: MW2SHELL 0x100665f8
ArchiveReader* g_unk0x100665f8 = NULL;

// The message to post when the archive is left.
// GLOBAL: MW2SHELL 0x100665fc
WPARAM g_unk0x100665fc = 0;

// Plays while an entry loads.
// GLOBAL: MW2SHELL 0x10066600
AudioSample* g_unk0x10066600 = NULL;

// The text of the entry being loaded.
// GLOBAL: MW2SHELL 0x1007ce10
MechChar g_unk0x1007ce10[0x10000];

// GLOBAL: MW2SHELL 0x1008ce10
MechChar g_unk0x1008ce10[0x200];

// GLOBAL: MW2SHELL 0x1008d010
MechChar g_unk0x1008d010[0x200];

// The link buttons of PrevPage and NextPage.
// GLOBAL: MW2SHELL 0x1008d210
MainMenuButton g_unk0x1008d210;

// GLOBAL: MW2SHELL 0x1008d230
MainMenuButton g_unk0x1008d230;

// GLOBAL: MW2SHELL 0x1008d250
MechChar g_unk0x1008d250[0x100];

void FUN_10029181(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32 p_msg);

// Opens the clan hall archive of a campaign.
// FUNCTION: MW2SHELL 0x10029010
void FUN_10029010(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam)
{
	void* audioData;
	MechS32 audioSize;

	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006fe70[p_campaign].m_picture);
	p_database->GetDBItem(103, &audioData, &audioSize);
	g_unk0x10066600 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	g_unk0x10066600->SetVolume(0x32);
	g_unk0x10066600->Start();

	g_unk0x100665f8 = new ArchiveReader(
		g_unk0x1006e1a0[p_campaign],
		g_unk0x10071224,
		1,
		TRUE,
		NULL,
		NULL,
		g_unk0x1006fe70[p_campaign].m_buttons,
		1
	);
	g_unk0x100665fc = p_wParam;
	FUN_100108e5(FUN_10029181);
}

// FUNCTION: MW2SHELL 0x10029181
void FUN_10029181(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32 p_msg)
{
	if (p_msg == 0x404) {
		p_msg = g_unk0x100665f8->Run();
	}

	if (p_msg != 0x40b) {
		delete g_unk0x100665f8;
		g_unk0x100665f8 = NULL;
		delete g_unk0x10066600;
		g_unk0x10066600 = NULL;

		if (p_msg == 0x402) {
			PostMessage(g_pWnd, 0x402, 0x40b, 0);
		}
		else if (p_msg == 0x403 || p_msg == 0x405) {
			PostMessage(g_pWnd, g_unk0x100665fc, 0x40b, 0);
		}
		else {
			PostMessage(g_pWnd, p_msg, 0x40b, 0);
		}
		FUN_100108fd(FUN_10029181);
	}
}

// FUNCTION: MW2SHELL 0x100292c2
void ArchiveReader::AddTopic(MechS16 p_entry, MechS32 p_index)
{
	Topic* topic = NULL;

	topic = (Topic*) HeapAlloc(g_hPrimaryHeap, HEAP_NO_SERIALIZE, sizeof(Topic));
	topic->m_id = p_index + c_buttonTopic;
	topic->m_entry = p_entry;
	ExpandCollection(m_topics, topic);
}

// FUNCTION: MW2SHELL 0x1002931d
void ArchiveReader::FUN_1002931d()
{
	g_pVideoDriver->FUN_100077b4(FALSE);
}

// FUNCTION: MW2SHELL 0x10029340
void ArchiveReader::FirstPage()
{
	Page* page = NULL;

	if (m_pages->m_count == 1) {
		m_menu->FUN_10048aec(c_buttonNextPage);
	}

	if (m_page > 0 && m_page < m_pages->m_count) {
		page = (Page*) CollectionGet(m_pages, m_page);
		page->FUN_10045ab0();
		if (m_database) {
			m_menu->FUN_100488ed(c_buttonTopic);
		}
	}

	m_page = 0;
	PrevPage();
}

// Stack-slot permutation: page, unused, link and button.
// FUNCTION: MW2SHELL 0x100293fc
void ArchiveReader::PrevPage()
{
	Page* page = NULL;
	undefined4 unused = 0;
	Page::Link* link = NULL;
	MainMenuButton* button = &g_unk0x1008d210;
	MechS32 i;

	button->m_textPos.x = 0;
	button->m_textPos.y = 0;
	button->m_text = FUN_10030900("");

	if (m_pages->m_count == 1) {
		m_menu->FUN_10048aec(c_buttonNextPage);
	}

	if (m_page > 0 && m_page < m_pages->m_count) {
		page = (Page*) CollectionGet(m_pages, m_page);
		page->FUN_10045ab0();
		if (m_database) {
			m_menu->FUN_100488ed(c_buttonTopic);
		}
	}

	m_page--;
	if (m_page < 0) {
		m_page = 0;
	}

	if (m_page == 0) {
		m_menu->FUN_10048aec(c_buttonPrevPage);
		if (m_titleGlyph) {
			delete m_titleGlyph;
		}
		m_titleGlyph = g_unk0x10071214->FUN_1000544e(0x140, 0x32, m_title, NULL);
	}

	if (m_page < m_pages->m_count && m_pages->m_count > 1) {
		m_menu->FUN_10048aec(c_buttonNextPage);
		m_menu->FUN_10048b95(m_buttons[c_buttonNextPage], c_buttonNextPage, m_menu->m_drawRect);
	}

	if (m_pages->m_count > 0) {
		m_currentPage = (Page*) CollectionGet(m_pages, m_page);
		for (i = 0; i < m_currentPage->m_links->m_count; i++) {
			link = (Page::Link*) CollectionGet(m_currentPage->m_links, i);
			button->m_left = link->m_left;
			button->m_top = link->m_top;
			button->m_right = link->m_right;
			button->m_bottom = link->m_bottom;
			m_menu->FUN_10048b95(g_unk0x1008d210, link->m_id + c_buttonTopic, m_menu->m_drawRect);
		}
		m_currentPage->FUN_1004596f();
	}
}

// Stack-slot permutation: page, button, i and link.
// FUNCTION: MW2SHELL 0x100296df
void ArchiveReader::NextPage()
{
	Page* page = NULL;
	MainMenuButton* button = &g_unk0x1008d230;
	MechS32 i;
	Page::Link* link;

	button->m_textPos.x = 0;
	button->m_textPos.y = 0;
	button->m_text = FUN_10030900("");

	if (m_page >= 0 && m_page < m_pages->m_count - 1) {
		page = (Page*) CollectionGet(m_pages, m_page);
		page->FUN_10045ab0();
		if (m_database) {
			m_menu->FUN_100488ed(c_buttonTopic);
		}
	}

	m_page++;
	if (m_page >= m_pages->m_count) {
		m_page = m_pages->m_count - 1;
	}

	if (m_page == m_pages->m_count - 1) {
		m_menu->FUN_10048aec(c_buttonNextPage);
	}

	if (m_page > 0) {
		m_menu->FUN_10048aec(c_buttonPrevPage);
		m_menu->FUN_10048b95(m_buttons[c_buttonPrevPage], c_buttonPrevPage, m_menu->m_drawRect);
	}

	m_currentPage = (Page*) CollectionGet(m_pages, m_page);
	for (i = 0; i < m_currentPage->m_links->m_count; i++) {
		link = (Page::Link*) CollectionGet(m_currentPage->m_links, i);
		button->m_left = link->m_left;
		button->m_top = link->m_top;
		button->m_right = link->m_right;
		button->m_bottom = link->m_bottom;
		m_menu->FUN_10048b95(g_unk0x1008d230, link->m_id + c_buttonTopic, m_menu->m_drawRect);
	}
	m_currentPage->FUN_1004596f();
}

// Follows a topic link: opens its entry in a child reader.
// Stack-slot permutation: topic and unused.
// FUNCTION: MW2SHELL 0x1002991e
void ArchiveReader::OpenTopic(MechS32 p_id)
{
	Page* page = NULL;
	Topic* topic = NULL;
	undefined4 unused = 0;
	MechS32 i;

	page = (Page*) CollectionGet(m_pages, m_page);
	page->FUN_10045ab0();

	for (i = 0; i < m_topics->m_count; i++) {
		topic = (Topic*) CollectionGet(m_topics, i);
		if (topic->m_id == p_id) {
			if (m_titleGlyph) {
				delete m_titleGlyph;
				m_titleGlyph = NULL;
			}

			m_child = new ArchiveReader(
				m_title,
				g_unk0x10071224,
				topic->m_entry,
				FALSE,
				m_database,
				NULL,
				m_buttons,
				m_count
			);
			break;
		}
	}

	if (m_topics->m_count == i) {
		m_child = NULL;
	}
}

// Returns 0x40b while the reader stays open.
// FUNCTION: MW2SHELL 0x10029ade
MechS32 ArchiveReader::Run()
{
	MechS32 result;
	MechS32 button;

	if (m_child) {
		result = m_child->Run();
		if (result != 0x40b) {
			delete m_child;
			m_child = NULL;
		}

		return result == 0x403 ? 0x40b : result;
	}

	if (m_currentPage) {
		m_currentPage->FUN_10045a2b();
	}

	if (!m_titleGlyph) {
		m_titleGlyph = g_unk0x10071214->FUN_1000544e(0x140, 0x32, m_title, NULL);
	}

	button = m_menu->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
	if (g_keyboardInput->m_key) {
		switch (g_keyboardInput->m_key) {
		case 0xc4:
			button = c_buttonPrevPage;
			g_pMouseState->PressButton(0);
			break;
		case 0xc5:
			button = c_buttonNextPage;
			g_pMouseState->PressButton(0);
			break;
		case 0xc2:
			button = c_buttonBack;
			g_pMouseState->PressButton(0);
			break;
		default:
			break;
		}
	}

	switch (button) {
	case c_buttonExit:
		if (g_pMouseState->GetLeftPressed() == 1) {
			return 0x405;
		}
		break;
	case c_buttonNextPage:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		NextPage();
		break;
	case c_buttonPrevPage:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		PrevPage();
		break;
	case c_buttonBack:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		if (m_page == 0) {
			return 0x403;
		}
		else {
			FirstPage();
		}
		break;
	default:
		if (m_database) {
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			if (button != -1) {
				OpenTopic(button);
			}
		}
		break;
	}

	return 0x40b;
}

// Reads an archive entry: a title command, then text, picture and topic commands up to the end
// command. The text is laid out into as many pages as it needs.
// Stack-slot permutation: most of the locals.
// FUNCTION: MW2SHELL 0x10029db5
void ArchiveReader::Load(MechS32 p_entry)
{
	Page* page;
	void* data;
	undefined4 unused;
	MechU8 soundStarted;
	MechS32 topics;
	MechS32 length;
	MechS32 offset;
	MechS16 command;
	MechS32 result;
	MechU8 picture;
	MechU8 unk0x2c;
	MechS16 topicEntry;
	MechS32 size;
	MechChar* text;
	MechChar* rest;
	MechS32 i;
	MechU32 wordLength;

	page = NULL;
	data = NULL;
	unused = 0;
	topics = 0;
	soundStarted = FALSE;
	topics = 0;
	length = 0;
	offset = 0;

	result = m_database->FUN_100483c8(p_entry, offset, &command, 2);
	offset += 2;
	if (result) {
		delete g_pVideoDriver;
		fprintf(stderr, "Could not access Archive DB entry\n");
		fflush(stderr);
		exit(1);
	}

	if (command != 0x100) {
		delete g_pVideoDriver;
		fprintf(stderr, "Cmd Title not found\n");
		fprintf(stderr, "Found: %d\n", command);
		fflush(stderr);
		exit(1);
	}

	result = m_database->FUN_10048501(p_entry, offset, g_unk0x1008ce10);
	offset += (MechS32) strlen(g_unk0x1008ce10) + 1;
	strcpy(m_title, "~");
	strcat(m_title, g_unk0x1008ce10);

	command = -1;
	while (command != -0x100) {
		g_pMouseState->ReadMouseState();
		result = m_database->FUN_100483c8(p_entry, offset, &command, 2);
		offset += 2;

		switch (command) {
		case 0x200:
			strcpy(g_unk0x1007ce10, "");
			text = g_unk0x1007ce10;
			do {
				result = m_database->FUN_1004843e(p_entry, offset, g_unk0x1008d250);
				g_pMouseState->ReadMouseState();
				if (result != 2) {
					wordLength = strlen(g_unk0x1008d250);
					length += wordLength;
					offset += wordLength + 1;
					g_unk0x1008d250[wordLength] = ' ';
					g_unk0x1008d250[wordLength + 1] = '\0';
					if (!soundStarted && length >= 0x800) {
						g_unk0x10066600->Start();
						soundStarted = TRUE;
					}
					strcpy(text, g_unk0x1008d250);
					text += wordLength;
				}
			} while (result != 2);
			offset++;

			m_colors[0] = 0xff;
			m_colors[1] = 5;
			for (i = 2; i < 0x100; i++) {
				m_colors[i] = i;
			}

			rest = NULL;
			do {
				if (page == NULL || rest != NULL) {
					page = new Page(m_font, g_pVideoDriver, m_colors, 0x58, 0x46, 0x1d2, 0xde);
					ExpandCollection(m_pages, page);
				}

				g_pMouseState->ReadMouseState();
				if (rest != NULL) {
					rest = page->Layout(rest);
				}
				else {
					rest = page->Layout(g_unk0x1007ce10);
				}
			} while (rest != NULL);
			page = NULL;
			break;
		case 0x400:
			result = m_database->FUN_100483c8(p_entry, offset, &topicEntry, 2);
			offset += 2;
			result = m_database->FUN_10048501(p_entry, offset, g_unk0x1008d010);
			offset += (MechS32) strlen(g_unk0x1008d010) + 1;
			AddTopic(topicEntry, topics++);
			break;
		case 0x300:
			result = m_database->FUN_100483c8(p_entry, offset, &picture, 1);
			offset++;
			result = m_database->FUN_100483c8(p_entry, offset, &unk0x2c, 1);
			offset++;
			result = m_database->GetDBItem(picture, &data, &size);
			if (page == NULL) {
				page = new Page(m_font, g_pVideoDriver, m_colors, 0x58, 0x46, 0x1d2, 0xde);
				ExpandCollection(m_pages, page);
			}
			page->FUN_10045b38((undefined*) data, size);
			break;
		case -0x100:
			break;
		default:
			delete g_pVideoDriver;
			fprintf(stderr, "Unknown Archive Command Code\n");
			fflush(stderr);
			exit(1);
			break;
		}
	}
}

// FUNCTION: MW2SHELL 0x1002a490
ArchiveReader::ArchiveReader(
	MechChar* p_name,
	Font* p_font,
	MechS32 p_entry,
	MechU8 p_ownsDatabase,
	TMPackDataBase* p_database,
	Collection* p_pages,
	MainMenuButton* p_buttons,
	MechS32 p_count
)
{
	MechS32 i;

	m_font = p_font;
	m_titleGlyph = NULL;
	m_currentPage = NULL;
	m_topics = NULL;
	m_titleGlyph = NULL;
	m_ownsDatabase = p_ownsDatabase;
	m_page = -1;
	strcpy(m_title, p_name);
	m_buttons = p_buttons;
	m_count = p_count;
	m_child = NULL;
	CreateCollection(&m_pages, 10, NULL, 4, NULL);
	CreateCollection(&m_topics, 3, NULL, 4, NULL);
	FUN_1002931d();

	m_menu = new ButtonMenu(g_pVideoDriver, m_font, 0, m_buttons, p_count);
	m_menu->FUN_10048b95(p_buttons[c_buttonBack], c_buttonBack, m_menu->m_drawRect);

	if (m_ownsDatabase == TRUE) {
		m_database = new TMPackDataBase(m_title);
		if (m_database == NULL) {
			delete g_pVideoDriver;
			fprintf(stderr, "Could not open archive database\n");
			fflush(stderr);
			exit(1);
		}
	}
	else {
		m_database = p_database;
	}

	if (p_pages == NULL) {
		Load(p_entry);
	}
	else {
		for (i = 0; i < p_pages->m_count; i++) {
			ExpandCollection(m_pages, CollectionGet(p_pages, i));
		}
		m_page = 0;
	}

	PrevPage();
}

// FUNCTION: MW2SHELL 0x1002a7b0
ArchiveReader::~ArchiveReader()
{
	MechS32 i;
	Page* page;

	if (m_child) {
		delete m_child;
	}

	for (i = 0; i < m_pages->m_count; i++) {
		page = (Page*) CollectionGet(m_pages, i);
		page->FUN_10045ab0();
		if (m_database) {
			delete page;
		}
	}

	delete m_menu;
	if (m_ownsDatabase == TRUE && m_database) {
		delete m_database;
	}

	if (m_titleGlyph) {
		delete m_titleGlyph;
	}
}
