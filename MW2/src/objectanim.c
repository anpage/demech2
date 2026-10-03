/* Hand-written assembly: FUN_10048c50, FUN_10048d46, FUN_10048ebe and FUN_10048faf are C
   functions with __asm bodies, and FUN_10049155 has __asm blocks. Their portable C (PORTABLE_C)
   is tested against the assembly by tests/asmequiv: it replaces each whole function, whose C
   wraps where standard C overflows. */
#include "objectanim.h"

#include "ambientsound.h"
#include "callbacks.h"
#include "classtable.h"
#include "clock.h"
#include "compat.h"
#include "config.h"
#include "decomp.h"
#include "depthsort.h"
#include "face.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "fixedtrig.h"
#include "geocache.h"
#include "object.h"
#include "path.h"
#include "players.h"
#include "polydraw.h"
#include "poolsizes.h"
#include "portable.h"
#include "projectedvertex.h"
#include "queuedpolygon.h"
#include "ramp.h"
#include "recordstacks.h"
#include "reel.h"
#include "reelevent.h"
#include "resource.h"
#include "resourcename.h"
#include "shape.h"
#include "simmain.h"
#include "soundfx.h"
#include "staticmem.h"
#include "types.h"
#include "vertex.h"
#include "view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#pragma warning(disable : 4102) /* labels only __asm blocks jump to */

/* The __asm blocks of FUN_10048c50, FUN_10048ebe, FUN_10048faf and FUN_10049155 jump to C labels,
   which newer compilers reject: their reference build (REFERENCE_ASM) compiles those functions'
   portable C too. */
#if defined(PORTABLE_C) || !defined(_MSC_VER) || _MSC_VER >= 1100
#define PORTABLE_C_LABELS
#endif

// The state of FUN_1004748c's callback: the faces of a shape cycle through up to sixteen colors.
// SIZE 0x50
typedef struct JadeCycle0x50 {
	Shape** m_shape;      // 0x00 — the star's shape (FUN_1001f873)
	Shape* m_model;       // 0x04
	MechS32 m_faceCount;  // 0x08 — -1 until counted
	MechS32 m_colorCount; // 0x0c
	MechS32 m_colors[16]; // 0x10
} JadeCycle0x50;

// The state of FUN_1004771e's callback: an object turning at constant rates.
// SIZE 0x1c
typedef struct JadeSpin0x1c {
	Shape** m_shape;       // 0x00
	SceneObject* m_object; // 0x04
	MechS32 m_rateX;       // 0x08 — 16.16 per period
	MechS32 m_rateY;       // 0x0c
	MechS32 m_rateZ;       // 0x10
	MechS32 m_period;      // 0x14 — clock ticks
	MechS32 m_lastClock;   // 0x18
} JadeSpin0x1c;

// The state of FUN_100479ec's callback: an object circling while it turns.
// SIZE 0x1c
typedef struct JadeOrbit0x1c {
	Shape** m_shape;       // 0x00
	SceneObject* m_object; // 0x04
	MechS32 m_angle;       // 0x08 — 16.16 degrees
	MechS32 m_turnRate;    // 0x0c
	MechS32 m_speed;       // 0x10
	MechS32 m_enabled;     // 0x14
	MechS32 m_lastClock;   // 0x18
} JadeOrbit0x1c;

// The state of FUN_10046750's callback: an object moving through an animation (Reel)
// in step with its player's animation state (m_unk0x80-0x94).
// SIZE 0x2c
typedef struct JadeMotion0x2c {
	Reel* m_reel;          // 0x00
	Reel* m_initial;       // 0x04
	SceneObject* m_object; // 0x08
	Player* m_player;      // 0x0c
	MechS32 m_rate;        // 0x10 — clock ticks per frame
	MechS32 m_timer;       // 0x14 — until the next frame
	MechS32 m_amount;      // 0x18 — left to move in this frame
	MechS32 m_lastClock;   // 0x1c
	MechS32 m_enabled;     // 0x20
	MechS32 m_frame;       // 0x24 — -1 before the first
	MechU32 m_flags;       // 0x28 — 1: drives the player's animation state
} JadeMotion0x2c;

// The state of FUN_10047f60's callback: a star's object following a path, eased by the ramps.
// SIZE 0x88
typedef struct JadePath0x88 {
	Shape** m_shape;       // 0x00 — the star's shape (FUN_1001f873)
	SceneObject* m_object; // 0x04
	Path* m_path;          // 0x08
	MechS32 m_startClock;  // 0x0c
	MechS32 m_duration;    // 0x10 — the points' times added up
	MechS32 m_rotate;      // 0x14 — turns along the path
	MechS32 m_mode;        // 0x18 — at the end: 0 "loop" runs on, 1 "repeat" restarts, 2 stops
	Ramp m_x;              // 0x1c
	Ramp m_y;              // 0x2c
	Ramp m_z;              // 0x3c
	WrappedRamp m_pitch;   // 0x4c
	WrappedRamp m_heading; // 0x60
	WrappedRamp m_roll;    // 0x74
} JadePath0x88;

// An animation file LoadAnimFile has loaded: its id and the base of its animation numbers.
// SIZE 0x8
typedef struct MossLedger0x8 {
	MechS32 m_id;   // 0x00
	MechS32 m_base; // 0x04
} MossLedger0x8;

DECOMP_SIZE_ASSERT(AmbientSound, 0x1e)
DECOMP_SIZE_ASSERT(PathPoint, 0x1c)
DECOMP_SIZE_ASSERT(Path, 0x744)
DECOMP_SIZE_ASSERT(JadeCycle0x50, 0x50)
DECOMP_SIZE_ASSERT(JadeSpin0x1c, 0x1c)
DECOMP_SIZE_ASSERT(JadeOrbit0x1c, 0x1c)
DECOMP_SIZE_ASSERT(MossLedger0x8, 0x8)
DECOMP_SIZE_ASSERT(JadeMotion0x2c, 0x2c)
DECOMP_SIZE_ASSERT(JadePath0x88, 0x88)
DECOMP_SIZE_ASSERT(ReelEvent, 0x8)

// Why FUN_10046750 last failed: 1 no player, 2 disabled, 3 no object, 4-7 missing state.
// GLOBAL: MW2 0x100a6d64
MechS32 g_unk0x100a6d64 = 0;

// The number of entries in g_paths.
// GLOBAL: MW2 0x100a6d68
MechS32 g_pathCount = 0;

// GLOBAL: MW2 0x100a6d6c
MechS32 g_unk0x100a6d6c = 0;

// The base of the animation numbers of the file being loaded.
// GLOBAL: MW2 0x100a6d70
MechS32 g_unk0x100a6d70 = 0;

// The number of entries in g_unk0x101097e0.
// GLOBAL: MW2 0x100a6d74
MechS32 g_unk0x100a6d74 = 0;

// Set to shade from the origin rather than the light (FUN_10048faf).
// GLOBAL: MW2 0x1010b530
MechS32 g_unk0x1010b530;

// The outcodes of the polygon being built: any vertex's (or) and every vertex's (and).
// GLOBAL: MW2 0x1010b53c
MechU8 g_unk0x1010b53c;

// GLOBAL: MW2 0x1010b5b8
MechU8 g_unk0x1010b5b8;

// The projected vertices of the polygon being built, and their count.
// GLOBAL: MW2 0x1010b550
ProjectedVertex* g_unk0x1010b550[20];

// GLOBAL: MW2 0x1010b5b0
MechS32 g_unk0x1010b5b0;

// Where FUN_10049155 copies the next polygon's vertex pointers in the draw buffer.
// GLOBAL: MW2 0x1010b534
MechU8* g_unk0x1010b534;

// FUN_10049155's counts: faces it took (5bc), faces past the back-face test (538), vertices it
// projected (5b4) and polygons it queued (5a8).

// GLOBAL: MW2 0x1010b5bc
MechS32 g_unk0x1010b5bc;

// GLOBAL: MW2 0x1010b538
MechS32 g_unk0x1010b538;

// GLOBAL: MW2 0x1010b5b4
MechS32 g_unk0x1010b5b4;

// GLOBAL: MW2 0x1010b5a8
MechS32 g_unk0x1010b5a8;

// GLOBAL: MW2 0x100ea8e0
Path g_paths[0x40];

// The animations of the loaded animation files, by number.
// GLOBAL: MW2 0x101079e0
Reel* g_reels[0x780];

// GLOBAL: MW2 0x101097e0
MossLedger0x8 g_unk0x101097e0[60];

// A timed callback (TimedCallbackFn) moving a thing's object through an animation. Its data is
// "<thing or class id>;<rate>,<flags>,<animation>", made for the player being created
// (g_unk0x100a8638). Each frame moves or turns the object by the frame's amount, spread over
// the rate; the frame events jump to other frames by the player's animation state (m_unk0x84,
// the frame it reached, and m_unk0x88, the one it wants).
// Stack-slot permutation of the locals. The original adds i before scaling frame in the
// m_values[i] reads (index order).
// FUNCTION: MW2 0x10046750
MechS32 FUN_10046750(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechS32 next;
	MechS32 elapsed;
	MechS32 y;
	MechChar* token;
	MechS32 z;
	MechS32 jump;
	JadeMotion0x2c* motion;
	MechS32 thing;
	MechS32 back;
	MechS32 enabled;
	MechS32 found;
	MechS32 amount;
	MechS32 turnX;
	MechS32 step;
	MechS32 target;
	MechS32 turnY;
	MechS32 value;
	MechS32 number;
	MechS32 turnZ;
	MechS32 frame;
	Shape* shape;
	MechS32 rate;
	MechS32 id;
	void** slot;
	MechS32 x;
	MechS32 match;
	MechS32 i;

	rate = 0;
	enabled = 0;
	number = 0;
	jump = FALSE;
	found = FALSE;
	thing = -1;
	switch (p_event) {
	case 0:
		motion = StaticPoolAlloc(sizeof(JadeMotion0x2c), g_staticPoolTags[4]);
		if (!motion) {
			return 0;
		}

		if (!g_unk0x100a8638) {
			g_unk0x100a6d64 = 1;
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, motion);
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = motion;
		motion->m_object = NULL;
		motion->m_reel = NULL;
		motion->m_enabled = 0;
		motion->m_rate = 0;
		motion->m_flags = 0;
		motion->m_player = g_unk0x100a8638;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%ld,%d,%d", &rate, &enabled, &number);
			number += g_unk0x100a6d70;
			if (number > g_unk0x100a6d6c) {
				g_unk0x100a6d6c = number;
			}

			if (enabled) {
				motion->m_enabled = 1;
				if (enabled > 0x7f) {
					motion->m_flags |= 1;
				}
			}
			else {
				motion->m_enabled = 0;
				g_unk0x100a6d64 = 2;
			}

			motion->m_rate = rate;
			motion->m_reel = g_reels[number];
			motion->m_initial = g_reels[number];
			motion->m_frame = -1;
			motion->m_timer = 0;
			motion->m_amount = 0;
			motion->m_lastClock = p_clock;
			motion->m_player->m_unk0x94 = rate;
			motion->m_player->m_unk0x84 = -1;
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		thing = FindThingIdxById(id);
		if (thing != -1) {
			motion->m_object = FUN_1001d980(thing);
		}
		else {
			shape = FindClassById(id);
			if (shape) {
				motion->m_object = GetShapeObject(shape);
			}
		}

		if (!motion->m_object) {
			g_unk0x100a6d64 = 3;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		motion = *slot;
		if (!motion) {
			return 0;
		}

		if (!motion->m_object) {
			return 0;
		}

		if (!motion->m_player) {
			g_unk0x100a6d64 = 5;
			return 0;
		}

		if (!motion->m_enabled) {
			g_unk0x100a6d64 = 4;
			return 0;
		}

		elapsed = p_clock - motion->m_lastClock;
		motion->m_lastClock = p_clock;
		if (motion->m_player->m_unk0x80 & 0x10) {
			motion->m_reel = motion->m_initial;
			motion->m_frame = -1;
			motion->m_timer = 0;
			motion->m_amount = 0;
			motion->m_lastClock = p_clock;
			if (motion->m_flags & 1) {
				motion->m_player->m_unk0x94 = motion->m_rate;
				motion->m_player->m_unk0x80 &= ~0x11;
				motion->m_player->m_unk0x84 = -1;
			}
		}
		else if (motion->m_player->m_unk0x80 & 1) {
			if (!motion->m_rate) {
				g_unk0x100a6d64 = 6;
				return 0;
			}

			if (!motion->m_player->m_unk0x94) {
				g_unk0x100a6d64 = 7;
				return 0;
			}

			if (motion->m_timer <= 0) {
				amount = motion->m_amount;
			}
			else {
				amount = motion->m_amount * elapsed / motion->m_timer;
			}

			if (motion->m_amount < 0) {
				if (motion->m_amount > amount) {
					amount = motion->m_amount;
				}
			}
			else if (motion->m_amount < amount) {
				amount = motion->m_amount;
			}

			if (motion->m_reel->m_kind >= 3) {
				turnX = turnY = turnZ = 0;
				switch (motion->m_reel->m_kind) {
				case 3:
					turnX = amount;
					break;
				case 4:
					turnY = amount;
					break;
				case 5:
					turnZ = amount;
					break;
				}

				FUN_1000184b(motion->m_object, turnX, turnY, turnZ, 0);
			}
			else {
				x = y = z = 0;
				switch (motion->m_reel->m_kind) {
				case 0:
					x = amount;
					break;
				case 1:
					y = amount;
					break;
				case 2:
					z = amount;
					break;
				}

				FUN_10001667(motion->m_object, x, y, z);
			}

			FUN_10001cf8(motion->m_object);
			motion->m_amount -= amount;
			motion->m_timer -= elapsed;
			if (motion->m_frame == -1) {
				motion->m_frame = 0;
				motion->m_timer = 0;
				motion->m_player->m_unk0x84 = 0;
			}

			if (motion->m_timer <= 0) {
				frame = motion->m_frame;
				motion->m_frame++;
				motion->m_timer += motion->m_player->m_unk0x94;
				if (-motion->m_player->m_unk0x94 > motion->m_timer) {
					motion->m_timer = -motion->m_player->m_unk0x94;
				}

				if (motion->m_reel->m_events[frame].m_flags & 0x40) {
					target = motion->m_reel->m_events[frame].m_values[1];
					if (motion->m_player->m_unk0x88 == target) {
						jump = TRUE;
					}
					else {
						jump = FALSE;
					}
				}

				if (motion->m_reel->m_events[frame].m_flags & 0x80) {
					target = motion->m_reel->m_events[frame].m_values[1];
					if (motion->m_player->m_unk0x88 != -1 &&
						motion->m_player->m_unk0x84 != motion->m_player->m_unk0x88) {
						jump |= TRUE;
					}
					else {
					}
				}

				if (motion->m_reel->m_events[frame].m_flags & 0x100) {
					target = motion->m_reel->m_events[frame].m_values[2];
					if (motion->m_player->m_unk0x88 == -1) {
						jump |= TRUE;
					}
					else {
					}
				}

				if (jump) {
					jump = FALSE;
					if (motion->m_player->m_unk0x84 < target) {
						step = 1;
					}
					else {
						step = -1;
					}

					found = FALSE;
					next = frame;
					while (!found) {
						next += step;
						if (next >= motion->m_reel->m_frameCount) {
							next = -1;
						}

						if (next == -1) {
							found = TRUE;
						}
						else if (motion->m_reel->m_events[next].m_flags & 1) {
							value = motion->m_reel->m_events[next].m_values[0];
							if (value == target) {
								found = TRUE;
							}
						}
					}

					if (next != -1) {
						found = FALSE;
						while (!found) {
							if (next >= motion->m_reel->m_frameCount) {
								next = -1;
								found = TRUE;
							}
							else {
								if (motion->m_reel->m_events[next].m_flags & 0x200) {
									value = motion->m_reel->m_events[next].m_values[3];
									if (motion->m_player->m_unk0x84 == value) {
										found = TRUE;
									}
								}

								next++;
							}
						}
					}

					if (next != -1) {
						motion->m_frame = next;
						jump = TRUE;
					}
				}

				if (!jump && (motion->m_reel->m_events[frame].m_flags & 0xc)) {
					match = TRUE;
					if (motion->m_reel->m_events[frame].m_flags & 4) {
						match = FALSE;
						for (i = 0; i < 4; i++) {
							if (motion->m_reel->m_events[frame].m_values[i] == -1) {
								break;
							}

							if (motion->m_reel->m_events[frame].m_values[i] == (MechS8) motion->m_player->m_unk0x88) {
								match = TRUE;
							}
							else {
								match = FALSE;
							}

							if (match) {
								break;
							}
						}
					}

					if (match) {
						back = frame - 1;
						while (!(motion->m_reel->m_events[back].m_flags & 2) && back >= 0) {
							back--;
						}

						motion->m_frame = back;
					}
				}

				if ((!jump && (motion->m_reel->m_events[frame].m_flags & 0x400)) ||
					motion->m_frame >= motion->m_reel->m_frameCount) {
					motion->m_timer = 0;
					motion->m_frame = -1;
				}

				if (motion->m_frame != -1) {
					motion->m_amount += motion->m_reel->m_amounts[motion->m_frame];
				}

				if (motion->m_flags & 1) {
					if (jump) {
						motion->m_player->m_unk0x84 = target;
					}

					motion->m_player->m_unk0x80 &= ~2;
					motion->m_player->m_unk0x80 &= ~8;
					if (motion->m_frame != -1) {
						motion->m_player->m_unk0x94 = FUN_100472fe(motion->m_player->m_unk0x8c, motion->m_rate);
						if (motion->m_reel->m_events[motion->m_frame].m_flags & 0x10) {
							motion->m_player->m_unk0x80 |= 2;
						}

						if (motion->m_reel->m_events[motion->m_frame].m_flags & 0x800) {
							motion->m_player->m_unk0x80 |= 8;
						}
					}
					else if (motion->m_player->m_unk0x88 == -1) {
						motion->m_player->m_unk0x80 &= ~1;
						motion->m_player->m_unk0x84 = -1;
					}
				}
			}
		}
		else if (motion->m_flags & 1) {
			if (motion->m_player->m_unk0x88 != -1 || motion->m_player->m_unk0x84 != -1) {
				motion->m_player->m_unk0x80 |= 1;
				motion->m_player->m_unk0x94 = FUN_100472fe(motion->m_player->m_unk0x8c, motion->m_rate);
			}
		}
		break;
	default:
		break;
	}

	return 1;
}

// Scales p_value by mode p_mode: 1 by 1.5, 3 by 0.75, any other mode leaves it.
// FUNCTION: MW2 0x100472fe
MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value)
{
	MechS32 result;

	switch (p_mode) {
	case 1:
		result = (p_value >> 1) + p_value;
		break;
	case 2:
		result = p_value;
		break;
	case 3:
		result = p_value - (p_value >> 2);
		break;
	default:
		result = p_value;
		break;
	}

	return result;
}

// Loads the animation file p_ref unless it has been, and sets the base of its animation numbers
// (g_unk0x100a6d70). Returns -1 if it was already loaded.
// FUNCTION: MW2 0x10047380
MechS32 LoadAnimFile(ResourceRef* p_ref)
{
	MechS32 result;
	MechS32 i;
	MechS32 id;

	result = 0;
	id = p_ref->m_id;
	for (i = 0; i < g_unk0x100a6d74 && g_unk0x101097e0[i].m_id != id; i++) {
	}

	if (i < g_unk0x100a6d74) {
		g_unk0x100a6d70 = g_unk0x101097e0[i].m_base;
		result = -1;
	}
	else if (g_unk0x100a6d74 < 60) {
		if (g_unk0x100a6d6c > 0) {
			g_unk0x100a6d70 = g_unk0x100a6d6c + 1;
		}

		result = FUN_100708f4(p_ref);
		g_unk0x101097e0[g_unk0x100a6d74].m_base = g_unk0x100a6d70;
		g_unk0x101097e0[g_unk0x100a6d74].m_id = id;
		g_unk0x100a6d74++;
	}

	return result;
}

// FUNCTION: MW2 0x10047462
MechS32 FUN_10047462(void)
{
	return g_unk0x100a6d70;
}

// FUNCTION: MW2 0x10047477
MechS32 FUN_10047477(void)
{
	return 0x2c;
}

// A timed callback (TimedCallbackFn) cycling the face colors of a star's shape. Its data is
// "<star id>;<color>,<color>,...": up to sixteen colors, each a palette row.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004748c
MechS32 FUN_1004748c(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	JadeCycle0x50* cycle;
	MechS32 vertexCount;
	MechS32 offset;
	MechS32 i;
	MechS32 star;
	MechS32 id;
	void** slot;

	if (!p_period) {
		p_period = 1;
	}

	switch (p_event) {
	case 0:
		cycle = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadeCycle0x50));
		if (!cycle) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = cycle;
		cycle->m_colorCount = 0;
		cycle->m_model = NULL;
		cycle->m_faceCount = -1;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			for (token = strtok(token, ","); token; token = strtok(NULL, ",")) {
				if (cycle->m_colorCount < 16) {
					cycle->m_colors[cycle->m_colorCount] = strtoul(token, NULL, 0) << 4;
					cycle->m_colorCount++;
				}
				else {
					break;
				}
			}
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			cycle->m_shape = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		cycle = *slot;
		if (!cycle) {
			return 0;
		}
		else if (!cycle->m_shape) {
			return 0;
		}

		if (!*cycle->m_shape) {
			return 0;
		}

		cycle->m_model = *cycle->m_shape;
		if (cycle->m_faceCount == -1) {
			GetModelCounts(cycle->m_model, &vertexCount, &cycle->m_faceCount);
		}

		offset = p_clock / p_period % cycle->m_colorCount;
		for (i = 0; i < cycle->m_faceCount; i++) {
			FUN_1003b696(cycle->m_model, i, cycle->m_colors[(offset + i) % cycle->m_colorCount]);
		}
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) turning a star's object. Its data is
// "<star id>;<x>,<y>,<z>,<period>": the turns in degrees per period of seconds.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004771e
MechS32 FUN_1004771e(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechS32 dz;
	MechChar* token;
	JadeSpin0x1c* spin;
	MechFloat x;
	MechFloat y;
	MechFloat z;
	MechFloat period;
	MechS32 star;
	MechS32 t;
	MechS32 id;
	MechS32 dx;
	void** slot;
	MechS32 dy;

	switch (p_event) {
	case 0:
		spin = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadeSpin0x1c));
		if (!spin) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = spin;
		spin->m_rateX = spin->m_rateY = spin->m_rateZ = 0;
		spin->m_object = NULL;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%f,%f,%f,%f", &x, &y, &z, &period);
			spin->m_rateX = x * 65536.0 + 0.5;
			spin->m_rateY = y * 65536.0 + 0.5;
			spin->m_rateZ = z * 65536.0 + 0.5;
			spin->m_period = period * 181.0f;
			if (!spin->m_period) {
				spin->m_period = 181;
			}

			spin->m_lastClock = p_clock;
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			spin->m_shape = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		spin = *slot;
		if (!spin) {
			return 0;
		}
		else if (!spin->m_shape) {
			return 0;
		}

		if (!*spin->m_shape) {
			return 0;
		}

		spin->m_object = GetShapeObject(*spin->m_shape);
		t = ((p_clock - spin->m_lastClock) << 16) / spin->m_period;
		dx = FixedMul16(spin->m_rateX, t);
		dy = FixedMul16(spin->m_rateY, t);
		dz = FixedMul16(spin->m_rateZ, t);
		spin->m_lastClock = p_clock;
		FUN_1000184b(spin->m_object, dx, dy, dz, 0);
		FUN_10001cf8(spin->m_object);
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) moving a star's object around a circle. Its data is
// "<star id>;<radius>,<period>,<enabled>,<unused>".
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100479ec
MechS32 FUN_100479ec(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	MechS32 z;
	MechS32 cosine;
	JadeOrbit0x1c* orbit;
	MechS32 t;
	MechS32 step;
	MechS32 star;
	MechS32 sine;
	MechS32 enabled;
	MechFloat radius;
	MechS32 unused;
	MechFloat period;
	MechS32 id;
	void** slot;
	MechS32 x;

	switch (p_event) {
	case 0:
		orbit = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadeOrbit0x1c));
		if (!orbit) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = orbit;
		orbit->m_enabled = 0;
		orbit->m_object = NULL;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%f,%f,%d,%d", &radius, &period, &enabled, &unused);
			orbit->m_turnRate = 360.0 / period * 65536.0 + 0.5;
			orbit->m_speed = radius * 6.283185307179586 / period * 65536.0 + 0.5;
			orbit->m_enabled = enabled;
			orbit->m_angle = 0;
		}

		orbit->m_lastClock = p_clock;
		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			orbit->m_shape = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		orbit = *slot;
		if (!orbit) {
			return 0;
		}
		else if (!orbit->m_shape || !orbit->m_enabled) {
			return 0;
		}

		if (!*orbit->m_shape) {
			return 0;
		}

		orbit->m_object = GetShapeObject(*orbit->m_shape);
		cosine = FUN_1006973a(orbit->m_angle);
		sine = FUN_100696c0(orbit->m_angle);
		t = FixedDiv16(p_clock - orbit->m_lastClock, 181);
		orbit->m_lastClock = p_clock;
		step = FixedMul16(orbit->m_speed, t) >> 16;
		x = FixedMul16(cosine, step) >> 13;
		z = FixedMul16(-sine, step) >> 13;
		FUN_10001667(orbit->m_object, x, 0, z);
		FUN_10001cf8(orbit->m_object);
		step = FixedMul16(orbit->m_turnRate, t);
		FUN_1000184b(orbit->m_object, 0, step, 0, 0);
		FUN_10001cf8(orbit->m_object);
		orbit->m_angle += step;
		orbit->m_angle %= 0x1680000;
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) looping a sound on a star's object. Its data is
// "<star id>;<range>,<sound name>,<unk0x14>".
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10047d10
MechS32 FUN_10047d10(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	AmbientSound* sound;
	MechS32 star;
	MechS32 id;
	void** slot;
	MechChar name[100];

	switch (p_event) {
	case 0:
		sound = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(AmbientSound));
		if (!sound) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = sound;
		sound->m_slot = -1;
		sound->m_data = NULL;
		sound->m_skip = 1;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%d,%[^,],%d", &sound->m_range, name, &sound->m_unk0x14);
			sound->m_range *= 100;
		}

		sound->m_id = FindResourceIdByName(0xb, name);
		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			sound->m_unk0x0c = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		sound = *slot;
		if (!sound) {
			return 0;
		}
		else if (!sound->m_unk0x0c || !sound->m_unk0x14) {
			StopAmbientSound(sound);
			return 0;
		}

		if (!*sound->m_unk0x0c) {
			StopAmbientSound(sound);
			return 0;
		}

		sound->m_obj = GetShapeObject(*sound->m_unk0x0c);
		UpdateAmbientSound(sound);
		break;
	case 2:
		slot = FUN_1007d51f(FUN_1007d2e0());
		sound = *slot;
		if (!sound) {
			return 1;
		}

		StopAmbientSound(sound);
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn).
// A timed callback (TimedCallbackFn) moving a star's object along a path. Its data is
// "<star id>;<mode>,<rotate>,<path name>": mode "loop", "repeat" or else stop at the end, and
// "rotate" to turn the object along the path. Event -1 restarts it.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10047f60
MechS32 FUN_10047f60(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar rotate[64];
	MechChar* token;
	JadePath0x88* follower;
	MechS32 total;
	MechChar name[64];
	MechS32 i;
	MechChar mode[64];
	MechS32 star;
	MechS32 id;
	void** slot;
	PathPoint* first;
	PathPoint* origin;
	MechS32 elapsed;
	Path* path;
	MechS32 start;
	MechS32 end;
	PathPoint* point;
	MechS32 t;
	MechS32 delta;
	PathPoint* next;
	MechS32 angle;

	switch (p_event) {
	case -1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		follower = *slot;
		if (!follower) {
			return 0;
		}

		follower->m_startClock = p_clock;
		if (follower->m_path) {
			first = follower->m_path->m_points;
			StartRamp(&follower->m_x, first->m_x, first->m_x, 0.3);
			StartRamp(&follower->m_y, first->m_y, first->m_y, 0.3);
			StartRamp(&follower->m_z, first->m_z, first->m_z, 0.3);
			StartWrappedRamp(&follower->m_pitch, 0, 0, 0.3, 0x1680000);
			StartWrappedRamp(&follower->m_heading, 0, 0, 0.3, 0x1680000);
			StartWrappedRamp(&follower->m_roll, 0, 0, 0.3, 0x1680000);
			total = 0;
			for (i = 0; i < follower->m_path->m_count; i++) {
				total += follower->m_path->m_points[i].m_unk0x18;
			}

			follower->m_duration = total;
			id = atoi(p_data);
			id = MapResourceId(id);
			id = atoi(p_data);
			id = MapResourceId(id);
			star = FindStarIdxById(id);
			if (star != -1) {
				follower->m_shape = FUN_1001f873(star);
			}
			else {
				return 0;
			}
		}
		break;
	case 0:
		follower = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadePath0x88));
		if (!follower) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = follower;
		follower->m_object = NULL;
		follower->m_path = NULL;
		follower->m_startClock = p_clock;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%[^,],%[^,],%[^,]", mode, rotate, name);
			if (_strcmpi(mode, "loop") == 0) {
				follower->m_mode = 0;
			}
			else if (_strcmpi(mode, "repeat") == 0) {
				follower->m_mode = 1;
			}
			else {
				follower->m_mode = 2;
			}

			if (_strcmpi(rotate, "rotate") == 0) {
				follower->m_rotate = 1;
			}
			else {
				follower->m_rotate = 0;
			}

			for (i = 0; i < g_pathCount; i++) {
				if (_strcmpi(name, g_paths[i].m_name) == 0) {
					follower->m_path = &g_paths[i];
					break;
				}
			}
		}

		if (follower->m_path) {
			origin = follower->m_path->m_points;
			StartRamp(&follower->m_x, origin->m_x, origin->m_x, 0.3);
			StartRamp(&follower->m_y, origin->m_y, origin->m_y, 0.3);
			StartRamp(&follower->m_z, origin->m_z, origin->m_z, 0.3);
			StartWrappedRamp(&follower->m_pitch, 0, 0, 0.3, 0x1680000);
			StartWrappedRamp(&follower->m_heading, 0, 0, 0.3, 0x1680000);
			StartWrappedRamp(&follower->m_roll, 0, 0, 0.3, 0x1680000);
			total = 0;
			for (i = 0; i < follower->m_path->m_count; i++) {
				total += follower->m_path->m_points[i].m_unk0x18;
			}

			follower->m_duration = total;
			id = atoi(p_data);
			id = MapResourceId(id);
			id = atoi(p_data);
			id = MapResourceId(id);
			star = FindStarIdxById(id);
			if (star != -1) {
				follower->m_shape = FUN_1001f873(star);
			}
			else {
				return 0;
			}
			break;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		follower = *slot;
		if (!follower) {
			return 0;
		}
		else if (!follower->m_shape || !follower->m_path) {
			return 0;
		}

		if (!*follower->m_shape) {
			return 0;
		}

		follower->m_object = GetShapeObject(*follower->m_shape);
		if (!follower->m_object) {
			return 0;
		}

		path = follower->m_path;
		if (path->m_count <= 1) {
			return 0;
		}

		elapsed = p_clock - follower->m_startClock;
		end = 0;
		start = 0;
		for (i = 0; i < path->m_count; i++) {
			start = end;
			end += path->m_points[i].m_unk0x18;
			if (end >= elapsed) {
				break;
			}
		}

		if (i >= path->m_count - 1) {
			if (follower->m_mode == 2) {
				return 0;
			}
			else if (follower->m_mode == 1) {
				i = 0;
				elapsed = 0;
				follower->m_startClock = p_clock;
				start = 0;
				end = path->m_points[0].m_unk0x18;
				StartRamp(&follower->m_x, path->m_points[0].m_x, path->m_points[0].m_x, 0.3);
				StartRamp(&follower->m_y, path->m_points[0].m_y, path->m_points[0].m_y, 0.3);
				StartRamp(&follower->m_z, path->m_points[0].m_z, path->m_points[0].m_z, 0.3);
				StartWrappedRamp(&follower->m_pitch, 0, 0, 0.3, 0x1680000);
				StartWrappedRamp(&follower->m_heading, 0, 0, 0.3, 0x1680000);
				StartWrappedRamp(&follower->m_roll, 0, 0, 0.3, 0x1680000);
			}
			else if (path->m_count == i) {
				i = 0;
				elapsed -= follower->m_duration;
				follower->m_startClock = p_clock - elapsed;
				start = 0;
				end = path->m_points[0].m_unk0x18;
			}
		}

		point = &path->m_points[i];
		if (path->m_count - 1 == i) {
			next = path->m_points;
		}
		else {
			next = &path->m_points[i + 1];
		}

		t = FixedDiv16(elapsed - start, point->m_unk0x18);
		follower->m_x.m_target = point->m_x + FixedMul16(t, next->m_x - point->m_x);
		follower->m_y.m_target = point->m_y + FixedMul16(t, next->m_y - point->m_y);
		follower->m_z.m_target = point->m_z + FixedMul16(t, next->m_z - point->m_z);
		SetObjPosition(
			follower->m_object,
			UpdateRamp(&follower->m_x),
			UpdateRamp(&follower->m_y),
			UpdateRamp(&follower->m_z)
		);
		angle = point->m_unk0x10;
		if (follower->m_rotate) {
			angle += FUN_100698de(next->m_x - point->m_x, next->m_z - point->m_z);
		}

		delta = angle - follower->m_heading.m_value;
		while (delta > 0xb40000) {
			delta -= 0x1680000;
		}

		while (delta < -0xb40000) {
			delta += 0x1680000;
		}

		follower->m_heading.m_target = angle;
		follower->m_heading.m_value = angle - delta;
		angle = point->m_unk0x0c;
		if (follower->m_rotate) {
			angle -= FUN_1006975b((next->m_y - point->m_y) << 13);
		}

		delta = angle - follower->m_pitch.m_value;
		while (delta > 0xb40000) {
			delta -= 0x1680000;
		}

		while (delta < -0xb40000) {
			delta += 0x1680000;
		}

		follower->m_pitch.m_target = angle;
		follower->m_pitch.m_value = angle - delta;
		angle = point->m_unk0x14;
		delta = angle - follower->m_roll.m_value;
		while (delta > 0xb40000) {
			delta -= 0x1680000;
		}

		while (delta < -0xb40000) {
			delta += 0x1680000;
		}

		follower->m_roll.m_target = angle;
		follower->m_roll.m_value = angle - delta;
		SetObjRotation(
			follower->m_object,
			UpdateWrappedRamp(&follower->m_pitch),
			UpdateWrappedRamp(&follower->m_heading),
			UpdateWrappedRamp(&follower->m_roll),
			0
		);
		FUN_10001cf8(follower->m_object);
		break;
	default:
		break;
	}

	return 1;
}

#ifdef PORTABLE_C_LABELS
// The 64-bit product of two 32-bit values, as imul leaves it in edx:eax. Sums of products wrap,
// like the add/adc chains.
static MechU64 Product(MechS32 p_a, MechS32 p_b)
{
	return (MechU64) ((MechS64) p_a * p_b);
}

// sub: wraps.
static MechS32 Difference(MechS32 p_a, MechS32 p_b)
{
	return PortableS32((MechU32) p_a - (MechU32) p_b);
}

// add: wraps.
static MechS32 Sum(MechS32 p_a, MechS32 p_b)
{
	return PortableS32((MechU32) p_a + (MechU32) p_b);
}

// The view transform's row (p_a, p_b, p_c) times the vertex's offset from the eyepoint
// (g_unk0x100ea8b8, g_unk0x100ea8b4, g_unk0x100ea8bc), shifted right by 27 and rounded.
static MechS32 ViewRow(MechS32 p_a, MechS32 p_b, MechS32 p_c, Vertex* p_vertex)
{
	MechU64 sum = Product(p_a, Difference(p_vertex->m_unk0x0c, g_unk0x100ea8b8)) +
				  Product(p_b, Difference(p_vertex->m_unk0x10, g_unk0x100ea8b4)) +
				  Product(p_c, Difference(p_vertex->m_unk0x14, g_unk0x100ea8bc));

	return PortableS32(PortableShrdRound(sum, 27));
}

// The call FUN_10049155 makes through g_renderSettings.m_drawFace, which has no prototype: the hook
// is FUN_10036230 in the 3D view, and the map view's FUN_1003f0e7 takes three of the arguments.
typedef MechS32 (*DrawFaceHook)(Face* p_face, Vertex* p_vertices, MechS32 p_flags, MechS32 p_depth);

// A screen offset: p_value shifted left by p_shift (modulo 32, like the shld's count), divided by
// the depth, and rounded to a quarter.
static MechS32 ProjectAxis(MechS32 p_value, MechS32 p_shift, MechS32 p_depth)
{
	MechS64 scaled = (MechS64) p_value * ((MechS64) 1 << ((MechU32) p_shift & 31));

	return PortableSar32(Sum(PortableIdiv(scaled, p_depth), 2), 2);
}
#endif

#ifdef PORTABLE_C
// The edge value at the near plane: p_from plus the share p_toPlane / p_span of the way to p_to.
static MechS32 Interpolate(MechS32 p_from, MechS32 p_to, MechS32 p_toPlane, MechS32 p_span)
{
	return Sum(PortableIdiv((MechS64) Difference(p_from, p_to) * p_toPlane, p_span), p_to);
}
#endif

// Returns the vertex's projected copy (m_unk0x24), transforming it into view space
// (g_unk0x100ea864's rows, from the eyepoint g_unk0x100ea8b4) the first time. The transform is an
// __asm block.
// Stack-slot permutation of the locals the __asm blocks name.
// FUNCTION: MW2 0x10048c50
ProjectedVertex* FUN_10048c50(Vertex* p_vertex)
{
#ifdef PORTABLE_C_LABELS
	ProjectedVertex* result = p_vertex->m_unk0x24;
	MechS32 x;
	MechS32 y;

	if (result) {
		return result;
	}

	result = FUN_1007d248();
	x = ViewRow(g_unk0x100ea864, g_unk0x100ea868, g_unk0x100ea86c, p_vertex);
	y = ViewRow(g_unk0x100ea870, g_unk0x100ea874, g_unk0x100ea878, p_vertex);
	p_vertex->m_unk0x24 = result;
	result->m_x = x;
	result->m_y = y;
	result->m_z = PortableS32(p_vertex->m_unk0x20);
	result->m_u = PortableS32(p_vertex->m_unk0x18 << 16);
	result->m_v = PortableS32(p_vertex->m_unk0x1c << 16);
	return result;
#else
	MechS32 u;
	MechS32 v;
	ProjectedVertex* result;
	MechS32 deltaX;
	MechS32 deltaY;
	MechS32 deltaZ;

	__asm {
		mov ebx, p_vertex
		mov eax, dword ptr [ebx + 0x24]
		mov result, eax
		or eax, eax
		je jmp_10048c72
	}

	return result;

jmp_10048c72:
	result = FUN_1007d248();
	__asm {
		mov ebx, p_vertex
		mov eax, dword ptr [ebx + 0xc]
		sub eax, dword ptr [g_unk0x100ea8b8]
		mov deltaX, eax
		mov eax, dword ptr [ebx + 0x10]
		sub eax, dword ptr [g_unk0x100ea8b4]
		mov deltaY, eax
		mov eax, dword ptr [ebx + 0x14]
		sub eax, dword ptr [g_unk0x100ea8bc]
		mov deltaZ, eax
		mov eax, dword ptr [ebx + 0x18]
		mov u, eax
		mov eax, dword ptr [ebx + 0x1c]
		mov v, eax
		mov eax, dword ptr [g_unk0x100ea864]
		mov edx, deltaX
		imul edx
		mov esi, eax
		mov edi, edx
		mov eax, dword ptr [g_unk0x100ea868]
		mov edx, deltaY
		imul edx
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea86c]
		mov edx, deltaZ
		imul edx
		add esi, eax
		adc edi, edx
		shrd esi, edi, 0x1b
		adc esi, 0
		mov ecx, esi
		mov eax, dword ptr [g_unk0x100ea870]
		mov edx, deltaX
		imul edx
		mov esi, eax
		mov edi, edx
		mov eax, dword ptr [g_unk0x100ea874]
		mov edx, deltaY
		imul edx
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea878]
		mov edx, deltaZ
		imul edx
		add esi, eax
		adc edi, edx
		shrd esi, edi, 0x1b
		adc esi, 0
		mov eax, esi
		mov esi, result
		mov dword ptr [ebx + 0x24], esi
		mov esi, dword ptr [ebx + 0x20]
		mov ebx, result
		mov dword ptr [ebx], ecx
		mov dword ptr [ebx + 4], eax
		mov dword ptr [ebx + 8], esi
		mov eax, u
		shl eax, 0x10
		mov dword ptr [ebx + 0x14], eax
		mov eax, v
		shl eax, 0x10
		mov dword ptr [ebx + 0x18], eax
	}

	return result;
#endif
}

// Returns a new projected vertex where the edge from p_a to p_b crosses the near plane
// (g_unk0x100ea820), its position and texture coordinates interpolated. The body is an __asm
// block.
// Stack-slot permutation of the locals the __asm block names.
// FUNCTION: MW2 0x10048d46
ProjectedVertex* FUN_10048d46(Vertex* p_a, Vertex* p_b)
{
#ifdef PORTABLE_C
	ProjectedVertex* a;
	ProjectedVertex* b;
	ProjectedVertex* result;
	MechS32 x0;
	MechS32 y0;
	MechS32 z0;
	MechS32 u0;
	MechS32 v0;
	MechS32 x1;
	MechS32 y1;
	MechS32 z1;
	MechS32 u1;
	MechS32 v1;
	MechS32 toPlane;
	MechS32 span;

	a = p_a->m_unk0x24;
	if (!a) {
		a = FUN_10048c50(p_a);
	}

	b = p_b->m_unk0x24;
	if (!b) {
		b = FUN_10048c50(p_b);
	}

	result = FUN_1007d248();
	x0 = a->m_x;
	y0 = a->m_y;
	u0 = a->m_u;
	v0 = a->m_v;
	z0 = a->m_z;
	x1 = b->m_x;
	y1 = b->m_y;
	u1 = b->m_u;
	v1 = b->m_v;
	z1 = b->m_z;

	/* From the nearer end; the depths' difference wraps. */
	if (z1 <= z0) {
		span = Difference(z0, z1);
		if (span) {
			toPlane = Difference(g_unk0x100ea820, z1);
			x0 = Interpolate(x0, x1, toPlane, span);
			y0 = Interpolate(y0, y1, toPlane, span);
			u0 = Interpolate(u0, u1, toPlane, span);
			v0 = Interpolate(v0, v1, toPlane, span);
		}
	}
	else {
		span = Difference(z1, z0);
		if (span) {
			toPlane = Difference(g_unk0x100ea820, z0);
			x0 = Interpolate(x1, x0, toPlane, span);
			y0 = Interpolate(y1, y0, toPlane, span);
			u0 = Interpolate(u1, u0, toPlane, span);
			v0 = Interpolate(v1, v0, toPlane, span);
		}
	}

	result->m_x = x0;
	result->m_y = y0;
	result->m_z = g_unk0x100ea820;
	result->m_u = u0;
	result->m_v = v0;
	return result;
#else
	MechS32 z0;
	MechS32 z1;
	MechS32 u0;
	MechS32 u1;
	ProjectedVertex* a;
	ProjectedVertex* b;
	MechS32 v0;
	ProjectedVertex* result;
	MechS32 v1;
	MechS32 x0;
	MechS32 x1;
	MechS32 y0;
	MechS32 y1;

	__asm {
		mov ebx, p_a
		mov eax, dword ptr [ebx + 0x24]
		mov a, eax
		or eax, eax
		jne jmp_10048d6f
		mov eax, p_a
		push eax
		call FUN_10048c50
		add esp, 4
		mov a, eax
jmp_10048d6f:
		mov ebx, p_b
		mov eax, dword ptr [ebx + 0x24]
		mov b, eax
		or eax, eax
		jne jmp_10048d8f
		mov eax, p_b
		push eax
		call FUN_10048c50
		add esp, 4
		mov b, eax
jmp_10048d8f:
		call FUN_1007d248
		mov result, eax
		mov ebx, a
		mov eax, dword ptr [ebx]
		mov x0, eax
		mov eax, dword ptr [ebx + 4]
		mov y0, eax
		mov eax, dword ptr [ebx + 0x14]
		mov u0, eax
		mov eax, dword ptr [ebx + 0x18]
		mov v0, eax
		mov eax, dword ptr [ebx + 8]
		mov z0, eax
		mov ebx, b
		mov eax, dword ptr [ebx]
		mov x1, eax
		mov eax, dword ptr [ebx + 4]
		mov y1, eax
		mov eax, dword ptr [ebx + 0x14]
		mov u1, eax
		mov eax, dword ptr [ebx + 0x18]
		mov v1, eax
		mov eax, dword ptr [ebx + 8]
		mov z1, eax
		cmp eax, z0
		jg jmp_10048e3a
		mov ecx, z0
		sub ecx, z1
		je jmp_10048e35
		mov edi, dword ptr [g_unk0x100ea820]
		sub edi, z1
		mov eax, x0
		sub eax, x1
		imul edi
		idiv ecx
		add eax, x1
		mov x0, eax
		mov eax, y0
		sub eax, y1
		imul edi
		idiv ecx
		add eax, y1
		mov y0, eax
		mov eax, u0
		sub eax, u1
		imul edi
		idiv ecx
		add eax, u1
		mov u0, eax
		mov eax, v0
		sub eax, v1
		imul edi
		idiv ecx
		add eax, v1
		mov v0, eax
jmp_10048e35:
		jmp jmp_10048e8f
jmp_10048e3a:
		mov ecx, z1
		sub ecx, z0
		je jmp_10048e8f
		mov edi, dword ptr [g_unk0x100ea820]
		sub edi, z0
		mov eax, x1
		sub eax, x0
		imul edi
		idiv ecx
		add eax, x0
		mov x0, eax
		mov eax, y1
		sub eax, y0
		imul edi
		idiv ecx
		add eax, y0
		mov y0, eax
		mov eax, u1
		sub eax, u0
		imul edi
		idiv ecx
		add eax, u0
		mov u0, eax
		mov eax, v1
		sub eax, v0
		imul edi
		idiv ecx
		add eax, v0
		mov v0, eax
jmp_10048e8f:
		mov ebx, result
		mov eax, x0
		mov dword ptr [ebx], eax
		mov eax, y0
		mov dword ptr [ebx + 4], eax
		mov eax, dword ptr [g_unk0x100ea820]
		mov dword ptr [ebx + 8], eax
		mov eax, u0
		mov dword ptr [ebx + 0x14], eax
		mov eax, v0
		mov dword ptr [ebx + 0x18], eax
	}

	return result;
#endif
}

// Projects a view-space vertex onto the screen once per frame (m_unk0x1d), with its clip
// outcodes (m_unk0x1c: 1 left, 2 right, 4 top, 8 bottom), accumulates the outcodes of the
// polygon being built and adds the vertex to its list (up to 20). The body is an __asm block.
// FUNCTION: MW2 0x10048ebe
ProjectedVertex* FUN_10048ebe(ProjectedVertex* p_vertex)
{
#ifdef PORTABLE_C_LABELS
	MechS32 screen;
	MechU8 outcode;

	if (!p_vertex->m_projected) {
		screen = Sum(ProjectAxis(p_vertex->m_x, g_unk0x100ea824, p_vertex->m_z), g_unk0x100ea834);
		p_vertex->m_screenX = screen;
		outcode = 0;
		if (screen > g_unk0x100ea84c) {
			outcode |= 2;
		}
		if (screen < g_unk0x100ea830) {
			outcode |= 1;
		}

		screen = Difference(g_unk0x100ea858, ProjectAxis(p_vertex->m_y, g_unk0x100ea828, p_vertex->m_z));
		p_vertex->m_screenY = screen;
		if (screen > g_unk0x100ea840) {
			outcode |= 8;
		}
		if (screen < g_unk0x100ea850) {
			outcode |= 4;
		}

		p_vertex->m_outcode = outcode;
		p_vertex->m_projected = 1;
	}

	g_unk0x1010b53c |= p_vertex->m_outcode;
	g_unk0x1010b5b8 &= p_vertex->m_outcode;
	if (g_unk0x1010b5b0 >= 20) {
		g_unk0x1010b5ac = 0;
	}
	else {
		g_unk0x1010b550[g_unk0x1010b5b0++] = p_vertex;
	}

	return p_vertex;
#else
	__asm {
		mov ebx, p_vertex
		test byte ptr [ebx + 0x1d], 0xff
		je jmp_10048ed6
		jmp jmp_10048f61
jmp_10048ed6:
		xor ecx, ecx
		mov cl, byte ptr [g_unk0x100ea824]
		mov esi, dword ptr [ebx + 8]
		mov eax, dword ptr [ebx]
		cdq
		shld edx, eax, cl
		shl eax, cl
		idiv esi
		add eax, 2
		sar eax, 2
		add eax, dword ptr [g_unk0x100ea834]
		mov dword ptr [ebx + 0xc], eax
		xor ch, ch
		cmp eax, dword ptr [g_unk0x100ea84c]
		jle jmp_10048f0b
		or ch, 2
jmp_10048f0b:
		cmp eax, dword ptr [g_unk0x100ea830]
		jge jmp_10048f1a
		or ch, 1
jmp_10048f1a:
		mov cl, byte ptr [g_unk0x100ea828]
		mov eax, dword ptr [ebx + 4]
		cdq
		shld edx, eax, cl
		shl eax, cl
		idiv esi
		add eax, 2
		sar eax, 2
		neg eax
		add eax, dword ptr [g_unk0x100ea858]
		mov dword ptr [ebx + 0x10], eax
		cmp eax, dword ptr [g_unk0x100ea840]
		jle jmp_10048f4b
		or ch, 8
jmp_10048f4b:
		cmp eax, dword ptr [g_unk0x100ea850]
		jge jmp_10048f5a
		or ch, 4
jmp_10048f5a:
		mov byte ptr [ebx + 0x1c], ch
		mov byte ptr [ebx + 0x1d], 1
jmp_10048f61:
		mov al, byte ptr [ebx + 0x1c]
		or byte ptr [g_unk0x1010b53c], al
		and byte ptr [g_unk0x1010b5b8], al
		cmp dword ptr [g_unk0x1010b5b0], 0x14
		jl jmp_10048f8c
		mov dword ptr [g_unk0x1010b5ac], 0
		jmp jmp_10048fa2
jmp_10048f8c:
		mov eax, p_vertex
		mov ecx, dword ptr [g_unk0x1010b5b0]
		mov dword ptr [g_unk0x1010b550 + ecx*4], eax
		inc dword ptr [g_unk0x1010b5b0]
	}

	jmp_10048fa2 : return p_vertex;
#endif
}

// Returns the shade (0x7f: full) of p_face from the angle between its normal and the direction
// from its first vertex to the light (g_unk0x100ea8c0, or the origin with g_unk0x1010b530). The
// shading is an __asm block.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10048faf
MechS32 FUN_10048faf(Face* p_face, Vertex* p_vertices)
{
#ifdef PORTABLE_C_LABELS
	Vertex* vertex = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04]];
	MechS32 x = vertex->m_unk0x0c;
	MechS32 y = vertex->m_unk0x10;
	MechS32 z = vertex->m_unk0x14;
	MechU32 magnitudeX;
	MechU32 magnitudeY;
	MechU32 magnitudeZ;
	MechU32 bits;
	MechU32 squares;
	MechU64 dot;
	MechS32 scale;

	if (g_unk0x1010b530) {
		x = y = z = 0;
	}

	/* The direction to the light, with each component's magnitude. A component is negated when
	   the light's coordinate is the smaller one, which leaves a difference that wrapped as it is. */
	dot = Product(Difference(g_unk0x100ea8c4, x), p_face->m_normal[0]) +
		  Product(Difference(g_unk0x100ea8c8, y), p_face->m_normal[1]);
	magnitudeX = (MechU32) Difference(g_unk0x100ea8c4, x);
	if (g_unk0x100ea8c4 < x) {
		magnitudeX = 0 - magnitudeX;
	}

	magnitudeY = (MechU32) Difference(g_unk0x100ea8c8, y);
	if (g_unk0x100ea8c8 < y) {
		magnitudeY = 0 - magnitudeY;
	}

	magnitudeZ = (MechU32) Difference(g_unk0x100ea8c0, z);
	if (g_unk0x100ea8c0 < z) {
		magnitudeZ = 0 - magnitudeZ;
	}

	bits = magnitudeX | magnitudeY | magnitudeZ;
	if (!bits) {
		return 0x7f;
	}

	/* The dot product in 64 bits, shifted right by 16, and the magnitudes scaled to put the
	   highest bit of any at bit 7, the dot product with them. */
	dot += Product(Difference(g_unk0x100ea8c0, z), p_face->m_normal[2]);
	dot = (MechU64) PortableSar64(PortableS64(dot), 16);
	scale = PortableBsr(bits) - 7;
	if (scale > 0) {
		magnitudeX >>= scale;
		magnitudeY >>= scale;
		magnitudeZ >>= scale;
		dot = (MechU64) PortableSar64(PortableS64(dot), scale);
	}
	else if (scale < 0) {
		magnitudeX <<= -scale;
		magnitudeY <<= -scale;
		magnitudeZ <<= -scale;
		dot <<= -scale;
	}

	/* Divided by the length from the square root table, of the low bytes' squares. */
	squares = (magnitudeX & 0xff) * (magnitudeX & 0xff) + (magnitudeY & 0xff) * (magnitudeY & 0xff) +
			  (magnitudeZ & 0xff) * (magnitudeZ & 0xff);
	return PortableS16((MechU16) PortableIdiv(PortableS64(dot), g_sqrtTable[squares >> 8]));
#else
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS16 shade;
	MechS32 nx;
	MechS32 ny;
	MechS32 nz;
	Vertex* vertex;

	vertex = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04]];
	nx = p_face->m_normal[0];
	ny = p_face->m_normal[1];
	nz = p_face->m_normal[2];
	x = vertex->m_unk0x0c;
	y = vertex->m_unk0x10;
	z = vertex->m_unk0x14;
	if (g_unk0x1010b530) {
		x = y = z = 0;
	}

	__asm {
		mov eax, dword ptr [g_unk0x100ea8c4]
		sub eax, x
		mov ecx, eax
		jge jmp_1004903e
		neg ecx
jmp_1004903e:
		mov x, ecx
		mov ebx, ecx
		imul nx
		mov edi, edx
		mov esi, eax
		mov eax, dword ptr [g_unk0x100ea8c8]
		sub eax, y
		mov ecx, eax
		jge jmp_1004905c
		neg ecx
jmp_1004905c:
		mov y, ecx
		or ebx, ecx
		imul ny
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea8c0]
		sub eax, z
		mov ecx, eax
		jge jmp_1004907a
		neg ecx
jmp_1004907a:
		mov z, ecx
		or ebx, ecx
		jne jmp_10049092
		mov ax, 0x7f
		mov shade, ax
		jmp jmp_10049147
jmp_10049092:
		imul nz
		add esi, eax
		adc edi, edx
		shrd esi, edi, 0x10
		sar edi, 0x10
		xor ecx, ecx
		test ebx, 0xff000000
		je jmp_100490b7
		add cx, 0x10
		jmp jmp_100490c7
jmp_100490b7:
		test ebx, 0xffff0000
		je jmp_100490c9
		add cx, 8
jmp_100490c7:
		shr ebx, cl
jmp_100490c9:
		bsr ax, bx
		add cx, ax
		sub cx, 7
		je jmp_10049103
		jl jmp_100490f3
		shr x, cl
		shr y, cl
		shr z, cl
		shrd esi, edi, cl
		sar edi, cl
		jmp jmp_10049103
jmp_100490f3:
		neg cl
		shl x, cl
		shl y, cl
		shl z, cl
		shld edi, esi, cl
		shl esi, cl
jmp_10049103:
		mov al, byte ptr x
		mul al
		mov bx, ax
		xor dx, dx
		mov al, byte ptr y
		mul al
		add bx, ax
		adc dx, 0
		mov al, byte ptr z
		mul al
		add bx, ax
		adc dx, 0
		shrd bx, dx, 7
		and ebx, 0xfffe
		add ebx, dword ptr [g_sqrtTable]
		mov ax, word ptr [ebx]
		cwde
		mov ebx, eax
		mov edx, edi
		mov eax, esi
		idiv ebx
		mov shade, ax
	}

	jmp_10049147 : return shade;
#endif
}

// Queues a face of a model for drawing, unless it faces away: projects the vertices it hasn't
// yet, clips it to the near plane through the filter hooks (g_renderSettings) and adds the
// polygon to the list being built (g_unk0x1010b5c4) with its depth, by g_unk0x1010b5c8's rule:
// the average (2), nearest (4) or farthest of its vertices' depths. The back-face test, the
// projection, the clipping walk and the depth rules are __asm blocks; the first keeps the normal's
// z in depth.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10049155
void FUN_10049155(Face* p_face, Vertex* p_vertices)
{
#ifdef PORTABLE_C_LABELS
	Vertex* vertex;
	Vertex* first;
	Vertex* previous;
	QueuedPolygon* poly;
	MechU8* index;
	MechU16 count;
	MechU8 andCodes;
	MechU8 clipped;
	MechU8 firstClipped;
	MechU8 previousClipped;
	MechU64 dot;
	MechS64 last;
	MechS32 depth;
	MechS32 i;

	g_unk0x1010b5bc++;

	/* Faces of three vertices or more face away when the eyepoint's offset from the first one
	   has a dot product with the normal that isn't negative. The flags (jl) test the exact sum
	   of the first two products, which wraps at 64 bits, and the third. */
	if (p_face->m_unk0x02 >= 3) {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04]];
		dot = Product(Difference(vertex->m_unk0x0c, g_unk0x100ea8b8), p_face->m_normal[0]) +
			  Product(Difference(vertex->m_unk0x10, g_unk0x100ea8b4), p_face->m_normal[1]);
		last = (MechS64) Difference(vertex->m_unk0x14, g_unk0x100ea8bc) * p_face->m_normal[2];
		if (PortableS64(dot) >= -last) {
			return;
		}
	}

	/* The depths of the vertices not yet transformed this frame, with their near (1) and far (2)
	   clip codes. The counts are 16-bit: 0 runs 0x10000 times. */
	g_unk0x1010b538++;
	andCodes = 3;
	count = p_face->m_unk0x02;
	index = (MechU8*) p_face + p_face->m_unk0x04;
	do {
		vertex = &p_vertices[*index];
		if (!(vertex->m_unk0x28 & 4)) {
			MechU8 codes = 0;

			g_unk0x1010b5b4++;
			depth = ViewRow(g_unk0x100ea87c, g_unk0x100ea880, g_unk0x100ea884, vertex);
			vertex->m_unk0x20 = (MechU32) depth;
			vertex->m_unk0x28 |= 4;
			if (depth < g_unk0x100ea820) {
				codes |= 1;
			}
			if (depth > g_unk0x100ea82c) {
				codes |= 2;
			}

			vertex->m_unk0x28 = (MechU8) ((vertex->m_unk0x28 & 0xfc) | codes);
		}

		andCodes &= vertex->m_unk0x28;
		index++;
	} while (--count);

	if (andCodes == 1 || andCodes == 2) {
		return;
	}

	/* The clipping walk: each vertex in front of the near plane, and each crossing of it. */
	g_unk0x1010b53c = 0;
	g_unk0x1010b5b8 = 0xf;
	g_unk0x1010b5b0 = 0;
	index = (MechU8*) p_face + p_face->m_unk0x04;
	count = p_face->m_unk0x02;
	first = previous = &p_vertices[*index];
	firstClipped = previousClipped = first->m_unk0x28 & 1;
	if (!firstClipped) {
		g_renderSettings.m_projectVertex(FUN_10048c50(first));
	}

	while (--count) {
		index++;
		vertex = &p_vertices[*index];
		clipped = vertex->m_unk0x28 & 1;
		if (clipped != previousClipped) {
			g_renderSettings.m_projectVertex(FUN_10048d46(previous, vertex));
		}

		previous = vertex;
		previousClipped = clipped;
		if (!clipped) {
			g_renderSettings.m_projectVertex(FUN_10048c50(previous));
		}
	}

	if (previousClipped != firstClipped) {
		g_renderSettings.m_projectVertex(FUN_10048d46(previous, first));
	}

	if ((g_unk0x1010b5b0 < 3 && p_face->m_unk0x02 > 2) || g_unk0x1010b5b8) {
		return;
	}

	poly = (QueuedPolygon*) FUN_1007d296();
	poly->m_face = p_face;
	g_unk0x1010b534 = g_unk0x100c2698;
	poly->m_count = (MechS16) g_unk0x1010b5b0;

	/* The loops run their count (loop) times: the polygon has a point, as the projection hook
	   has added one whenever it cleared g_unk0x1010b5b8. */
	PORTABLE_ASSERT(g_unk0x1010b5b0 > 0);
	if (g_unk0x1010b5c8 & 2) {
		MechU32 sum = 0;

		for (i = 0; i < g_unk0x1010b5b0; i++) {
			sum += (MechU32) g_unk0x1010b550[i]->m_z;
		}

		/* The sum is unsigned (xor edx, edx), and the count a word. */
		depth = PortableIdiv(sum, (MechU16) g_unk0x1010b5b0);
	}
	else if (g_unk0x1010b5c8 & 4) {
		depth = 0x7fffffff;
		for (i = 0; i < g_unk0x1010b5b0; i++) {
			if (depth > g_unk0x1010b550[i]->m_z) {
				depth = g_unk0x1010b550[i]->m_z;
			}
		}
	}
	else {
		depth = -0x7fffffff;
		for (i = 0; i < g_unk0x1010b5b0; i++) {
			if (depth < g_unk0x1010b550[i]->m_z) {
				depth = g_unk0x1010b550[i]->m_z;
			}
		}
	}

	if (g_unk0x1010b5c8 & 1) {
		depth |= 0x40000000;
	}

	poly->m_depth = depth;
	memcpy(g_unk0x1010b534, g_unk0x1010b550, g_unk0x1010b5b0 * sizeof(g_unk0x1010b550[0]));
	g_unk0x1010b534 += g_unk0x1010b5b0 * sizeof(g_unk0x1010b550[0]);
	g_unk0x100c2698 = g_unk0x1010b534;
	poly->m_flags =
		(MechU16) ((DrawFaceHook) g_renderSettings.m_drawFace)(p_face, p_vertices, p_face->m_unk0x00, depth);
	if (g_unk0x100a54b0 < g_unk0x100c1a68) {
		g_unk0x1010b5a8++;
		if (g_unk0x1010b5b0 > 1) {
			g_unk0x100a54b4++;
		}

		g_unk0x1010b5c4[g_unk0x100a54b0].m_poly = poly;
		g_unk0x1010b5c4[g_unk0x100a54b0].m_depth = depth;
		g_unk0x100a54b0++;
	}
	else {
		g_unk0x1010b5ac = 0;
	}
#else
	MechS32 stride;
	Vertex* vertex;
	MechS32 normalY;
	MechS32 depth;
	MechS8 andCodes;
	MechU16 count;
	MechS8 orCodes;
	MechU8* index;
	QueuedPolygon* poly;
	MechS8 clipped;
	Vertex* first;
	Vertex* previous;
	MechS8 firstClipped;
	MechU8* cursor;
	MechS8 previousClipped;
	ProjectedVertex** points;

	stride = sizeof(Vertex);
	orCodes = 0;
	andCodes = 3;
	g_unk0x1010b5bc++;
	if (p_face->m_unk0x02 < 3) {
		goto project;
	}

	// clang-format off
	__asm {
		mov ebx, p_face
		mov edx, dword ptr [ebx + 0x14]
		mov eax, dword ptr [ebx + 0x18]
		mov normalY, eax
		mov eax, dword ptr [ebx + 0x1c]
		mov depth, eax
		add ebx, dword ptr [ebx + 4]
		xor eax, eax
		mov al, byte ptr [ebx]
		mov ebx, stride
		mul bl
		mov ebx, p_vertices
		add ebx, eax
		mov eax, dword ptr [ebx + 0xc]
		sub eax, dword ptr [g_unk0x100ea8b8]
		imul edx
		mov esi, eax
		mov edi, edx
		mov eax, dword ptr [ebx + 0x10]
		sub eax, dword ptr [g_unk0x100ea8b4]
		imul normalY
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [ebx + 0x14]
		sub eax, dword ptr [g_unk0x100ea8bc]
		imul depth
		add esi, eax
		adc edi, edx
		jl project
		jmp done
	}

project:
	__asm {
		inc dword ptr [g_unk0x1010b538]
		mov ebx, p_face
		mov ax, word ptr [ebx + 2]
		mov count, ax
		add ebx, dword ptr [ebx + 4]
		mov index, ebx
jmp_100491fe:
		mov esi, index
		xor eax, eax
		mov al, byte ptr [esi]
		mov ebx, stride
		mul bl
		mov esi, p_vertices
		add esi, eax
		test byte ptr [esi + 0x28], 4
		jne jmp_1004928f
		inc dword ptr [g_unk0x1010b5b4]
		mov eax, dword ptr [g_unk0x100ea87c]
		mov edx, dword ptr [esi + 0xc]
		sub edx, dword ptr [g_unk0x100ea8b8]
		imul edx
		mov ecx, eax
		mov edi, edx
		mov eax, dword ptr [g_unk0x100ea880]
		mov edx, dword ptr [esi + 0x10]
		sub edx, dword ptr [g_unk0x100ea8b4]
		imul edx
		add ecx, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea884]
		mov edx, dword ptr [esi + 0x14]
		sub edx, dword ptr [g_unk0x100ea8bc]
		imul edx
		add ecx, eax
		adc edi, edx
		shrd ecx, edi, 0x1b
		adc ecx, 0
		mov dword ptr [esi + 0x20], ecx
		or byte ptr [esi + 0x28], 4
		xor ax, ax
		cmp ecx, dword ptr [g_unk0x100ea820]
		jge jmp_1004927a
		or al, 1
jmp_1004927a:
		cmp ecx, dword ptr [g_unk0x100ea82c]
		jle jmp_10049288
		or al, 2
jmp_10049288:
		and byte ptr [esi + 0x28], 0xfc
		or byte ptr [esi + 0x28], al
jmp_1004928f:
		mov al, byte ptr [esi + 0x28]
		or orCodes, al
		and andCodes, al
		inc index
		dec count
		je projected
		_emit 0xe9 /* jmp jmp_100491fe */
		_emit 0x54
		_emit 0xff
		_emit 0xff
		_emit 0xff
	}

projected:
	if (andCodes == 1 || andCodes == 2) {
		return;
	}

	g_unk0x1010b53c = 0;
	g_unk0x1010b5b8 = 0xf;
	g_unk0x1010b5b0 = 0;
	__asm {
		mov ebx, p_face
		mov eax, ebx
		add eax, dword ptr [ebx + 4]
		mov cursor, eax
		mov ax, word ptr [ebx + 2]
		mov count, ax
		mov ebx, cursor
		xor eax, eax
		mov al, byte ptr [ebx]
		mov ebx, stride
		mul bl
		mov ebx, p_vertices
		add ebx, eax
		mov eax, ebx
		mov first, eax
		mov previous, eax
		mov al, byte ptr [ebx + 0x28]
		and al, 1
		mov firstClipped, al
		mov previousClipped, al
		jne jmp_10049334
		mov eax, first
		push eax
		call FUN_10048c50
		add esp, 4
		push eax
		call dword ptr [g_renderSettings + 0x5c]
		add esp, 4
jmp_10049334:
		dec count
		je closed
		inc cursor
		mov ebx, cursor
		xor eax, eax
		mov al, byte ptr [ebx]
		mov ebx, stride
		mul bl
		mov ebx, p_vertices
		add ebx, eax
		mov vertex, ebx
		mov al, byte ptr [ebx + 0x28]
		and al, 1
		mov clipped, al
		cmp al, previousClipped
		je jmp_10049380
		mov eax, vertex
		push eax
		mov eax, previous
		push eax
		call FUN_10048d46
		add esp, 8
		push eax
		call dword ptr [g_renderSettings + 0x5c]
		add esp, 4
jmp_10049380:
		mov ebx, cursor
		xor eax, eax
		mov al, byte ptr [ebx]
		mov ebx, stride
		mul bl
		mov ebx, p_vertices
		add ebx, eax
		mov previous, ebx
		mov al, clipped
		mov previousClipped, al
		or al, al
		jne jmp_100493b8
		mov eax, previous
		push eax
		call FUN_10048c50
		add esp, 4
		push eax
		call dword ptr [g_renderSettings + 0x5c]
		add esp, 4
jmp_100493b8:
		_emit 0xe9 /* jmp jmp_10049334 */
		_emit 0x77
		_emit 0xff
		_emit 0xff
		_emit 0xff
	}

closed:
		// clang-format on
		if (previousClipped != firstClipped)
	{
		g_renderSettings.m_projectVertex(FUN_10048d46(previous, first));
	}

	if ((g_unk0x1010b5b0 < 3 && p_face->m_unk0x02 > 2) || g_unk0x1010b5b8) {
		return;
	}

	points = g_unk0x1010b550;
	poly = (QueuedPolygon*) FUN_1007d296();
	poly->m_face = p_face;
	g_unk0x1010b534 = g_unk0x100c2698;
	poly->m_count = g_unk0x1010b5b0;
	if (g_unk0x1010b5c8 & 2) {
		__asm {
			mov ebx, poly
			mov ecx, dword ptr [g_unk0x1010b5b0]
			mov esi, points
			xor eax, eax
jmp_10049462:
			mov ebx, dword ptr [esi]
			add eax, dword ptr [ebx + 8]
			add esi, 4
			loop jmp_10049462
			xor edx, edx
			xor esi, esi
			mov si, word ptr [g_unk0x1010b5b0]
			idiv esi
			mov depth, eax
		}
	}
	else if (g_unk0x1010b5c8 & 4) {
		__asm {
			mov ecx, dword ptr [g_unk0x1010b5b0]
			mov esi, points
			mov eax, 0x7fffffff
jmp_1004949c:
			mov ebx, dword ptr [esi]
			cmp eax, dword ptr [ebx + 8]
			jle jmp_100494aa
			mov eax, dword ptr [ebx + 8]
jmp_100494aa:
			add esi, 4
			loop jmp_1004949c
			mov depth, eax
		}
	}
	else {
		__asm {
			mov ecx, dword ptr [g_unk0x1010b5b0]
			mov esi, points
			mov eax, 0x80000001
jmp_100494c5:
			mov ebx, dword ptr [esi]
			cmp eax, dword ptr [ebx + 8]
			jge jmp_100494d3
			mov eax, dword ptr [ebx + 8]
jmp_100494d3:
			add esi, 4
			loop jmp_100494c5
			mov depth, eax
		}
	}

	if (g_unk0x1010b5c8 & 1) {
		depth |= 0x40000000;
	}

	poly->m_depth = depth;
	__asm {
		mov edi, dword ptr [g_unk0x1010b534]
		mov ecx, dword ptr [g_unk0x1010b5b0]
		mov esi, points
		rep movsd
		mov dword ptr [g_unk0x1010b534], edi
		mov points, esi
	}

	g_unk0x100c2698 = g_unk0x1010b534;
	poly->m_flags = g_renderSettings.m_drawFace(p_face, p_vertices, p_face->m_unk0x00, depth);
	if (g_unk0x100a54b0 < g_unk0x100c1a68) {
		g_unk0x1010b5a8++;
		if (g_unk0x1010b5b0 > 1) {
			g_unk0x100a54b4++;
		}

		g_unk0x1010b5c4[g_unk0x100a54b0].m_poly = poly;
		g_unk0x1010b5c4[g_unk0x100a54b0].m_depth = depth;
		g_unk0x100a54b0++;
	}
	else {
		g_unk0x1010b5ac = 0;
	}

done:;
#endif
}
