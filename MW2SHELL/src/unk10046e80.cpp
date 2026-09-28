#include "brasslantern0x414.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "mousestate.h"
#include "silverreel0x18.h"
#include "types.h"
#include "videodriver.h"

#include <windows.h>

void operator delete(void*);

extern VideoDriver* g_pVideoDriver;
extern HollowReed0x110* g_unk0x100711f8;
extern MouseState* g_pMouseState;
extern BrassLantern0x414* g_unk0x10071210;
extern BrassLantern0x414* g_unk0x10071214;
extern BrassLantern0x414* g_unk0x10071218;
extern HMENU g_windowMenu;
extern MechS32 g_menuDialogOpen;
extern PaletteColor g_unk0x10071378[0x100];

extern void FUN_1001661b();
extern void FUN_100109a0(void (*p_callback)(MechS32));
extern void FUN_100109b8(void (*p_callback)(MechS32));

void FUN_10046fa8(MechS32 p_active);

// GLOBAL: MW2SHELL 0x10071db0
MechChar* g_unk0x10071db0[0x21f] = {
	"<ACTIVISION presents",
	NULL,
	"<MECHWARRIOR 2",
	NULL,
	"<based upon the BattleTech Universe created by",
	NULL,
	"<FASA CORPORATION",
	NULL,
	NULL,
	NULL,
	"<PRODUCTION",
	NULL,
	">Associate Producer",
	"Chacko Sonny",
	NULL,
	">Associate Producer",
	">MechWarrior 2 Add-Ons, NetMech, Ports and Localizations",
	"Tim Morten",
	NULL,
	">Executive Producer",
	"John Spinale",
	NULL,
	">Shell/Sim Programming By",
	">Non-Linear Arts, Inc.",
	"Flag",
	"John Lemberger",
	NULL,
	">Installer/Splash Screen Programming",
	"Bill DuSha",
	"George Collins",
	NULL,
	">Additional Writing By",
	"Sacha Howells",
	"Trey Watkins",
	NULL,
	">Tools/Drivers Licensed from",
	"John Lemberger",
	"John Miles",
	NULL,
	NULL,
	"<ART",
	NULL,
	">Additional Art",
	"Scott Goffman",
	"Danny Matson",
	"Chacko Sonny",
	NULL,
	NULL,
	"<AUDIO",
	NULL,
	">Sound Engineering And Design by",
	"Bill Black",
	"Michael Schwartz",
	NULL,
	">Additional Voice Over Talent",
	"John Keating",
	"Zachary Norman",
	NULL,
	NULL,
	"<PACKAGING AND PROMOTIONAL MATERIALS",
	NULL,
	">Documentation Manager",
	"Mike Rivera",
	NULL,
	">Manual Layout",
	"Cindy Whitlock",
	"Sylvia Orzel",
	NULL,
	">Package Design",
	"Jonathan Brown",
	"Ron Graening",
	NULL,
	">Additional Copy Writing",
	"Veronica Milito",
	NULL,
	NULL,
	"<QUALITY ASSURANCE",
	NULL,
	">Quality Assurance Director",
	"Jon Doellstedt",
	NULL,
	">Quality Assurance Manager",
	"Dave Arnspiger",
	NULL,
	">Customer Support Manager",
	"Jameson Wong",
	NULL,
	">Lead Testers",
	"John Cibulski",
	"Jason Wong",
	NULL,
	">Testers",
	"Keith Alexander",
	"David Avery",
	"Robert Berger",
	"Chris Campbell",
	"David Fielding",
	"Rue Hon",
	NULL,
	">Beta Testers",
	"Paul Beane",
	"Alan B. Clegg",
	"Ben Combee",
	"Rich DeFrancesco",
	"Albert Diaz",
	"Sven Doersam",
	"Rick Flower",
	"Richard Grochowski",
	"Jonathon Haas",
	"Michael Iguico",
	"Mark Kaelin",
	"Marc Lewis",
	"Ed Milovic",
	"James Noonan",
	"Gary Novosel",
	"Thieng Pang",
	"Dennis Schauman",
	"Bill Seiver",
	"Terry Tusher",
	"Stan VanDruff",
	"Ryan Wells",
	"Larry Wililamson",
	"Jun Ying",
	"Woojin Yu",
	"Jack Zarember",
	"Dave Zins",
	NULL,
	">Additional Testers",
	"Curtis Crockett",
	"Mark Harwood",
	"Tim Vanlaw",
	NULL,
	NULL,
	"<SPECIAL THANKS TO...",
	"John Lafleur",
	"Tom Dowd",
	"Jack Mamais",
	"Josh Resnick",
	"David Greenspan",
	"Dan Stanfill",
	"Ken Hullett",
	"Chad Findley",
	"Matt Candler",
	"Daiva Venckus",
	"Eric Zala",
	"Jack Burton",
	"J.J. Franzen",
	"Sean Kinnear",
	"Missy Chapman",
	"Mark Cooper",
	"Bob Pettit",
	"Sarah Hanlon",
	"Steve Willsey",
	"Alan Gershenfeld",
	"Howard Marks",
	"Fred Hummel",
	"Roger Weiss",
	"Mike Luedke",
	"Jason Robar",
	"Alex St. John",
	NULL,
	NULL,
	">All Righty Then!",
	NULL,
	NULL,
	"<ORIGINAL DOS VERSION CREDITS",
	NULL,
	NULL,
	">DIRECTOR",
	"John Spinale",
	NULL,
	">PRODUCER",
	"Josh Resnick",
	NULL,
	">LEAD GAME DESIGNER",
	"Sean Vesce",
	NULL,
	">GAME DESIGNER AND WRITER",
	"Zachary Norman",
	NULL,
	">ASSOCIATE GAME DESIGNERS",
	"Chad Findley",
	"Ken Hullett",
	"David White",
	NULL,
	">PRODUCTION COORDINATORS",
	"Lars Fuhrken-Batista",
	"Andrew Held",
	NULL,
	">PRODUCTION ASSISTANT",
	"Jeehun Hwang",
	NULL,
	NULL,
	"<PROGRAMMING",
	NULL,
	">SIMULATION PROGRAMMING",
	"John A. Clarke",
	"Michael H. Douglas",
	"Scott T. Etherton",
	"John Keating",
	"Tim Morten",
	"Bob Mortensen",
	"Eric Peterson",
	"Dan Stanfill",
	"Dave Zobel",
	NULL,
	">SHELL PROGRAMMING by REDLINE GAMES",
	"James E. Anhalt, III",
	"John C. Peck, Jr.",
	NULL,
	">ADDITIONAL PROGRAMMING",
	"David White",
	NULL,
	">PROGRAMMING INTERNS",
	"Brian Jennings",
	"Daniel Kamins",
	NULL,
	">TOOLS/DRIVERS LICENSED FROM",
	"John Lemberger",
	"John Miles",
	"John Ratcliff",
	NULL,
	">INSTALLER",
	"Greg Sanborn, Quicksilver Software",
	NULL,
	NULL,
	"<ART",
	NULL,
	">SIMULATION COLOR, LIGHTING, TEXTURES AND ANIMATIONS",
	"Danny Matson",
	NULL,
	">ADDITIONAL SIMULATION ART DIRECTION",
	"Danny Matson",
	NULL,
	">3D SIMULATION MODELS AND ANIMATIONS",
	"J.J. Franzen",
	NULL,
	">ADDITIONAL 3D SIMULATION GEOMETRY",
	"Sean Kinnear",
	NULL,
	">SHELL STILLS AND ANIMATIONS DIRECTION",
	"Jefferson Elliot",
	NULL,
	">SHELL STILLS AND ANIMATIONS",
	"Jack Burton",
	"Scott Goffman",
	"Jim Mitchell",
	NULL,
	">CLAN INTRO MOVIES",
	"Scott Goffman",
	"Jim Mitchell",
	NULL,
	">CLAN END MOVIES",
	"Scott Goffman",
	NULL,
	">INTRO MOVIE by DIGITAL DOMAIN",
	"EXECUTIVE PRODUCER              Chris McKibbon",
	"DIRECTOR OF ANIMATIONS          Darnell Williams",
	"CHARACTER ANIMATION             Craig Caton",
	"COMPOSITING SUPERVISOR          Nathan Wilson",
	"ADDITIONAL COMPOSITING          Steve Gray",
	"COLOR & LIGHTING CONSULTANT     Daniel Robichaud",
	"UNIT PRODUCTION MANAGER         Susan Thurmond",
	NULL,
	NULL,
	"<AUDIO",
	NULL,
	">MUSIC AND SOUND DIRECTOR",
	"Kelly Rogers",
	NULL,
	">ORIGINAL MUSICAL SCORE",
	"Gregory Alper",
	NULL,
	">ADDITIONAL MUSIC",
	"Jeehun Hwang",
	NULL,
	">MUSICAL SCORES FOR ALL MOVIE SEQUENCES",
	">AND TRIALS OF GRIEVANCE",
	"Jeehun Hwang",
	NULL,
	">MUSICAL SCORES FOR JADE FALCON CLAN",
	"Jeehun Hwang",
	NULL,
	">MUSICAL SCORES FOR WOLF CLAN",
	"Gregory Alper and Jeehun Hwang",
	NULL,
	">AUDIO ENGINEER",
	"Michael Schwartz",
	NULL,
	">MIDI ENGINEER",
	"Bill Black",
	NULL,
	">SOUND DESIGN",
	"SOUNDELUX MEDIA LABS",
	NULL,
	">SOUND EFFECTS SUPERVISION & SOUND DESIGN",
	"Scott Martin Gershen",
	"Michael Reagan",
	NULL,
	">SOUND EFFECTS MASTERING",
	"Teri Madsen",
	"Caron Weidner",
	NULL,
	">VOICE OVER TALENT",
	"Bryan Bowen",
	"Scott Martin Gershen",
	"John Keating",
	"Zachary Norman",
	"Michael Reagan",
	"Carol Ruggier",
	NULL,
	NULL,
	"<PACKAGING AND PROMOTIONAL MATERIALS",
	NULL,
	">CREATIVE DIRECTOR",
	"Ron Gould",
	NULL,
	">DOCUMENTATION MANAGER",
	"Mike Rivera",
	NULL,
	">GRAPHIC DESIGNER",
	"Jonathan Brown",
	NULL,
	">ADDITIONAL COVER ART",
	"Scott Goffman",
	"Danny Matson",
	NULL,
	">PACKAGE COPYWRITER",
	"Veronica Milito",
	NULL,
	">MANUAL DESIGNER",
	"Marcella Missirian",
	NULL,
	">MANUAL WRITER",
	"Veronica Milito",
	NULL,
	">MANUAL ILLUSTRATIONS",
	"Zachary Norman",
	NULL,
	">MECHWARRIOR2 WEB SITE COORDINATOR",
	"Sean Lally",
	NULL,
	">MECHWARRIOR2 WEB SITE",
	"http://www.activision.com",
	NULL,
	NULL,
	"<FASA",
	NULL,
	">BATTLETECH UNIVERSE CREATORS",
	"Jordan Weisman and L. Ross Babcock III",
	NULL,
	">ORIGINAL BATTLEMECH DESIGNS",
	"Dana Knutson",
	"Jim Nelson",
	"Steve Venters",
	NULL,
	">FALCON AND THE WOLF SCENARIOS",
	"Rodney Knox",
	"Bryan Nystul",
	NULL,
	">3D RENDERING ASSISTANCE",
	"Jamie Marshall",
	NULL,
	">BATTLETECH BACKGROUND MATERIAL",
	"The Adventure Architects",
	"Brent Carter",
	"Rob Cruz",
	"Evan Jameson",
	"Sam Lewis",
	"Bryan Nystul",
	"Michael Pellicciotti",
	"Boy Peterson",
	"Diane Piron-Gelman",
	"Sharon Turrier-Mulvhill",
	NULL,
	NULL,
	NULL,
	"<QUALITY ASSURANCE",
	NULL,
	">QUALITY ASSURANCE DIRECTOR",
	"Jon Doellstedt",
	NULL,
	">QUALITY ASSURANCE MANAGER",
	"Dave Arnspiger",
	NULL,
	">LEAD TESTERS",
	"John Lafleur",
	"Jack Mamais",
	NULL,
	">TEST TEAM",
	"Steven Bishop",
	"Tom Butler",
	"Matthew Candler",
	"Linus Chen",
	"Curtis Crockett",
	"Matthew Gibbs",
	"George Hoyt",
	"Nate Marston",
	"Elvis Murray",
	"Daiva Venckus",
	"Zack Wood",
	NULL,
	">ADDITIONAL TESTERS",
	"Keith Alexander",
	"Kurt Barker",
	"Rick Baumgartner",
	"Peter Blumel",
	"Willie Bolton",
	"Aaron Cammarata",
	"Judith Chlipala",
	"Ray Choi",
	"Tyron Chookolingo",
	"Ingrid de Beus",
	"Andre Emerson",
	"Sean Espinoza",
	"John Fair",
	"Greg Fulton",
	"Tony Grant",
	"Seth Grenald",
	"Daniel Kamins",
	"David King",
	"Danny Lee",
	"Jacque LaMaire",
	"Tim McMahon",
	"Michael O'Brien",
	"Brad Pickering",
	"Chris Strompolos",
	"Murali Tegulapalle",
	"Nadine Theuzillot",
	"David R. Tulo, Jr.",
	"Mark Volpe",
	"Matt Weinstein",
	"William Westwater",
	"Paul Wiley",
	NULL,
	">EXTERNAL BETA TESTERS",
	"Weslee Bilodeau",
	"Todd Bilsborrow",
	"Dave Bourque",
	"Charles Bowlby",
	"Bill \"Axor\" Brown",
	"Paul Cabana",
	"Alex Chan",
	"Gary & Dolly Cook",
	"Rich De Francesco Jr.",
	"Jesse Derks",
	"Mark Dittenber",
	"Trent Ditto",
	"Sven Doersam",
	"Pamela Dreizen",
	"Brian Eichler",
	"David Ferrell",
	"Carl Finley",
	"Robert George",
	"Richard Grochowski",
	"Steven Hawley",
	"Cezzanne Huq",
	"James Jackson",
	"Brian James",
	"Jody Johnson",
	"Mark Kaelin",
	"Lenore Kaye",
	"Bill Kirkman",
	"Shaun Klomp",
	"Stephen Lafleur",
	"Matthew Lewis",
	"Ed Milovic",
	"Ford Maxim",
	"Steven Mo",
	"Dayle Moore",
	"Mat O'Connor",
	"Scooter Oehlerking",
	"Anthony Pham",
	"Richard Powell",
	"Alan Precourt",
	"Karen Rapchak",
	"Mark Reindl",
	"Donald Rinker",
	"Jason Robinson",
	"John Roper",
	"Loring Rose",
	"Joseph Ruffolo",
	"Jorja Rule",
	"James Sablatura",
	"Rick Salvador",
	"Julie Smith",
	"Montgomery Spencer",
	"Ryan Tykwinski",
	"Mike Udovic",
	"Bernard Yee",
	NULL,
	NULL,
	"<SPECIAL THANKS TO...",
	"All the wives and girlfriends",
	"John Clarke and Tim Morten (for the Second Coming)",
	"Dave and RJ (for their percussion talents)",
	"Doug Bambridge",
	"John Bruther",
	"Chris Campbell",
	"Ray Choi",
	"Tyron Chookolingo",
	"Brad Crystal",
	"Roy the Wonder Boy",
	"Nathalie Deschatres",
	"Tom Dowd, Denny Thorley and Mort Weisman (FASA)",
	"John Fair",
	"Michael Fletcher",
	"Alan Gershenfeld",
	"Larry Goldberg",
	"Vinod Gupta",
	"Sarah Hanlon",
	"Snoopy the Hamster",
	"Trials of Grievance: Hobbes",
	"Eric Johnson",
	"Kerstine Johnson",
	"Brian Kelly",
	"Edward Kilham and Kalani Streicher (Ronin Entertainment)",
	"Scott Lahman",
	"Mitch Lasky",
	"Maryanne Lataif",
	"Alan Lazar",
	"Howard Marks",
	"Mom",
	"Brad Pickering",
	"Barry Plaga",
	"Tom Sloper",
	"Smacker Boy",
	"Ben Tenn",
	"Trey Watkins",
	"The Original Production Crew",
	"...and all the other folks at Activision who helped this game come to life",
	NULL,
	NULL,
	"<THE LEGAL STUFF",
	NULL,
	"Mechwarrior, BattleTech, BattleMech and 'Mech are registered trademarks of FASA CORPORATION.  Used with "
	"permission.",
	NULL,
	"Activision is a registered trademark of Activision, Inc. (c) 1995 Activision, Inc.  All rights reserved.",
	NULL,
	NULL,
	NULL,
	"<This is the burn that fixes everything....",
	NULL,
};

// GLOBAL: MW2SHELL 0x1007262c
SilverReel0x18* g_unk0x1007262c = NULL;

// GLOBAL: MW2SHELL 0x10074648
MechChar g_unk0x10074648[0x10] = "amwlogo1";

// GLOBAL: MW2SHELL 0x10094b80
MechS32 g_unk0x10094b80;

// GLOBAL: MW2SHELL 0x10094b88
undefined g_unk0x10094b88[0x100];

// GLOBAL: MW2SHELL 0x10094c88
MechS32 g_unk0x10094c88;

// FUNCTION: MW2SHELL 0x10046e80
void FUN_10046e80()
{
	MechS32 i;

	g_unk0x10094b80 = 0;
	g_pVideoDriver->GetPalette(g_unk0x10071378);
	g_unk0x100711f8->FUN_100440ed();
	g_pVideoDriver->LoadPalette(5);
	g_pVideoDriver->m_unk0x3a6 = 0;
	g_unk0x1007262c = new SilverReel0x18(g_unk0x10074648, 0x78, 4);

	g_unk0x10094b88[0] = 0xff;
	g_unk0x10094b88[1] = 0x10;
	for (i = 2; i < 0x100; i++) {
		g_unk0x10094b88[i] = (MechU8) i;
	}
	g_unk0x10094c88 = 0x1cc;
	FUN_100109a0(FUN_10046fa8);
}

// Scroll the credits, and restore the shell once dismissed by a key or mouse click.
// FUNCTION: MW2SHELL 0x10046fa8
void FUN_10046fa8(MechS32 p_active)
{
	MechU32 index;
	MechS32 top;
	MechS32 width;

	if (p_active) {
		g_pVideoDriver->FUN_100071ad(0, 0x7d, 0x280, 0x14f);
		top = g_unk0x10094b80 * 0x14 + g_unk0x10094c88;
		g_unk0x10094c88--;
		for (index = g_unk0x10094b80; index < 0x21f && top < 0x1cc; index++) {
			if (top >= 0x69) {
				if (g_unk0x10071db0[index] == NULL) {
					// A blank line.
				}
				else if (g_unk0x10071db0[index][0] == '<') {
					width = 0x140 - g_unk0x10071214->FUN_100053be(g_unk0x10071db0[index] + 1) / 2;
					g_pVideoDriver
						->FUN_100074d2(width, top, g_unk0x10071214->m_unk0x408, g_unk0x10071db0[index] + 1, NULL);
				}
				else if (g_unk0x10071db0[index][0] == '>') {
					width = 0x140 - g_unk0x10071218->FUN_100053be(g_unk0x10071db0[index] + 1) / 2;
					g_pVideoDriver
						->FUN_100074d2(width, top, g_unk0x10071218->m_unk0x408, g_unk0x10071db0[index] + 1, NULL);
				}
				else if (g_unk0x10071db0[index][0] == '~') {
					width = 0x140 - g_unk0x10071214->FUN_100053be(g_unk0x10071db0[index] + 1) / 2;
					g_pVideoDriver->FUN_100074d2(
						width,
						top,
						g_unk0x10071214->m_unk0x408,
						g_unk0x10071db0[index] + 1,
						g_unk0x10094b88
					);
				}
				else {
					width = 0x140 - g_unk0x10071210->FUN_100053be(g_unk0x10071db0[index]) / 2;
					g_pVideoDriver->FUN_100074d2(width, top, g_unk0x10071210->m_unk0x408, g_unk0x10071db0[index], NULL);
				}
			}
			else {
				g_unk0x10094b80 = index;
			}
			top += 0x14;
		}
		g_pVideoDriver->FUN_100071ad(0, 0x69, 0x280, 0x14);
		g_pVideoDriver->FUN_100071ad(0, 0x1cc, 0x280, 0x14);
		if (g_unk0x1007262c != NULL) {
			g_unk0x1007262c->FUN_1001630b();
		}
	}
	if (!p_active || g_pMouseState->GetRightPressed() == 1 || g_pMouseState->GetLeftPressed() == 1 ||
		g_unk0x100711f8->FUN_10044189() != 0) {
		FUN_100109b8(FUN_10046fa8);
		EnableMenuItem(g_windowMenu, 0x9c92, MF_ENABLED);
		g_menuDialogOpen = FALSE;
		if (g_unk0x1007262c != NULL) {
			delete g_unk0x1007262c;
		}
		g_unk0x1007262c = NULL;
		g_pVideoDriver->m_unk0x3a6 = -1;
		g_pVideoDriver->FUN_100071ad(0, 0, 0x280, 0x1e0);
		FUN_1001661b();
		g_pVideoDriver->SetPalette(g_unk0x10071378, TRUE);
		if (p_active) {
			g_pVideoDriver->DrawShell();
			g_pVideoDriver->FUN_100071ad(0, 0, 0x280, 0x1e0);
			FUN_1001661b();
		}
	}
}
