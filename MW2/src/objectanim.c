/* The faces' projection, with its hand-written assembly, is in faceprojection.c, which this file
   includes in 1.1's build. */
#define FIXEDTRIG_FLOAT_SINE /* the accelerated editions' float sine, where they have it (fixedtrig.h) */
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

// The state of ColorCycleTask's callback: the faces of a shape cycle through up to sixteen colors.
// SIZE 0x50
typedef struct ColorCycle {
	Shape** m_shape;      // 0x00 — the star's shape (GetStaticShapeSlot)
	Shape* m_model;       // 0x04
	MechS32 m_faceCount;  // 0x08 — -1 until counted
	MechS32 m_colorCount; // 0x0c
	MechS32 m_colors[16]; // 0x10
} ColorCycle;

// The state of SpinTask's callback: an object turning at constant rates.
// SIZE 0x1c
typedef struct ObjectSpin {
	Shape** m_shape;       // 0x00
	SceneObject* m_object; // 0x04
	MechScalar m_rateX;    // 0x08 — 16.16 per period
	MechScalar m_rateY;    // 0x0c
	MechScalar m_rateZ;    // 0x10
	MechScalar m_period;   // 0x14 — clock ticks
	MechS32 m_lastClock;   // 0x18
} ObjectSpin;

// The state of OrbitTask's callback: an object circling while it turns.
// SIZE 0x1c
typedef struct ObjectOrbit {
	Shape** m_shape;       // 0x00
	SceneObject* m_object; // 0x04
	MechScalar m_angle;    // 0x08 — 16.16 degrees
	MechScalar m_turnRate; // 0x0c
	MechScalar m_speed;    // 0x10
	MechS32 m_enabled;     // 0x14
	MechS32 m_lastClock;   // 0x18
} ObjectOrbit;

// The state of ReelMotionTask's callback: an object moving through an animation (Reel)
// in step with its player's animation state (m_animFlags to m_animRate).
// SIZE 0x2c
typedef struct ReelMotion {
	Reel* m_reel;          // 0x00
	Reel* m_initial;       // 0x04
	SceneObject* m_object; // 0x08
	Player* m_player;      // 0x0c
	MechS32 m_rate;        // 0x10 — clock ticks per frame
	MechS32 m_timer;       // 0x14 — until the next frame
	MechScalar m_amount;   // 0x18 — left to move in this frame
	MechS32 m_lastClock;   // 0x1c
	MechS32 m_enabled;     // 0x20
	MechS32 m_frame;       // 0x24 — -1 before the first
	MechU32 m_flags;       // 0x28 — 1: drives the player's animation state
} ReelMotion;

// The state of PathTask's callback: a star's object following a path, eased by the ramps.
// SIZE 0x88
typedef struct PathFollower {
	Shape** m_shape;       // 0x00 — the star's shape (GetStaticShapeSlot)
	SceneObject* m_object; // 0x04
	Path* m_path;          // 0x08
	MechS32 m_startClock;  // 0x0c
	MechS32 m_duration;    // 0x10 — the points' times added up
	MechS32 m_rotate;      // 0x14 — turns along the path
	MechS32 m_mode;        // 0x18 — at the end: 0 "loop" runs on, 1 "repeat" restarts, 2 stops
	ScalarRamp m_x;        // 0x1c
	ScalarRamp m_y;        // 0x2c
	ScalarRamp m_z;        // 0x3c
	WrappedRamp m_pitch;   // 0x4c
	WrappedRamp m_heading; // 0x60
	WrappedRamp m_roll;    // 0x74
} PathFollower;

// An animation file LoadAnimFile has loaded: its id and the base of its animation numbers.
// SIZE 0x8
typedef struct AnimFile {
	MechS32 m_id;   // 0x00
	MechS32 m_base; // 0x04
} AnimFile;

DECOMP_SIZE_ASSERT(AmbientSound, 0x1e)
DECOMP_SIZE_ASSERT(PathPoint, 0x1c)
DECOMP_SIZE_ASSERT(Path, 0x744)
DECOMP_SIZE_ASSERT(ColorCycle, 0x50)
DECOMP_SIZE_ASSERT(ObjectSpin, 0x1c)
DECOMP_SIZE_ASSERT(ObjectOrbit, 0x1c)
DECOMP_SIZE_ASSERT(AnimFile, 0x8)
DECOMP_SIZE_ASSERT(ReelMotion, 0x2c)
DECOMP_SIZE_ASSERT(PathFollower, 0x88)
DECOMP_SIZE_ASSERT(ReelEvent, 0x8)

// Why ReelMotionTask last failed: 1 no player, 2 disabled, 3 no object, 4-7 missing state.
// GLOBAL: MW2 0x100a6d64
// GLOBAL: MW2GBL 0x1009b038
// GLOBAL: MW2MATROX 0x100ae2b4
// GLOBAL: MW23DFX 0x1024b328
// GLOBAL: MW2ATI 0x1017f6a8
// GLOBAL: MW2S3 0x10060f34
MechS32 g_reelMotionError = 0;

// The number of entries in g_paths.
// GLOBAL: MW2 0x100a6d68
// GLOBAL: MW2GBL 0x1009b03c
// GLOBAL: MW2PREMERCS 0x100b0c7c
// GLOBAL: MW2MATROX 0x100ae2b8
// GLOBAL: MW23DFX 0x1024b32c
// GLOBAL: MW2ATI 0x1017f6ac
// GLOBAL: MW2S3 0x10060f38
// GLOBAL: MW2POWERVR 0x101aa420
MechS32 g_pathCount = 0;

// GLOBAL: MW2 0x100a6d6c
// GLOBAL: MW2GBL 0x1009b040
// GLOBAL: MW2PREMERCS 0x100b0c80
// GLOBAL: MW2MATROX 0x100ae2bc
// GLOBAL: MW23DFX 0x1024b330
// GLOBAL: MW2ATI 0x1017f6b0
// GLOBAL: MW2S3 0x10060f3c
// GLOBAL: MW2POWERVR 0x101aa424
MechS32 g_maxAnimNumber = 0;

// The base of the animation numbers of the file being loaded.
// GLOBAL: MW2 0x100a6d70
// GLOBAL: MW2GBL 0x1009b044
// GLOBAL: MW2PREMERCS 0x100b0c84
// GLOBAL: MW2MATROX 0x100ae2c0
// GLOBAL: MW23DFX 0x1024b334
// GLOBAL: MW2ATI 0x1017f6b4
// GLOBAL: MW2S3 0x10060f40
// GLOBAL: MW2POWERVR 0x101aa428
MechS32 g_animBase = 0;

// The number of entries in g_animFiles.
// GLOBAL: MW2 0x100a6d74
// GLOBAL: MW2GBL 0x1009b048
// GLOBAL: MW2PREMERCS 0x100b0c88
// GLOBAL: MW2MATROX 0x100ae2c4
// GLOBAL: MW23DFX 0x1024b338
// GLOBAL: MW2ATI 0x1017f6b8
// GLOBAL: MW2S3 0x10060f44
// GLOBAL: MW2POWERVR 0x101aa42c
MechS32 g_animFileCount = 0;

// Set to shade from the origin rather than the light (GetFaceShade).
// GLOBAL: MW2 0x1010b530
// GLOBAL: MW2GBL 0x100e99e0
// GLOBAL: MW2PREMERCS 0x1017ba00
// GLOBAL: MW2MATROX 0x101d6908
// GLOBAL: MW23DFX 0x101b0e58
// GLOBAL: MW2ATI 0x100e4e48
// GLOBAL: MW2S3 0x100b1cf0
// GLOBAL: MW2POWERVR 0x1010c360
MechS32 g_directionalLight;

// The outcodes of the polygon being built: any vertex's (or) and every vertex's (and).
// GLOBAL: MW2 0x1010b53c
// GLOBAL: MW2GBL 0x100e99ec
// GLOBAL: MW2PREMERCS 0x1017ba0c
// GLOBAL: MW2MATROX 0x101d6900
// GLOBAL: MW23DFX 0x101b0e50
// GLOBAL: MW2ATI 0x100e4e40
// GLOBAL: MW2S3 0x100b1cfc
// GLOBAL: MW2POWERVR 0x1010c35c
MechU8 g_polygonOrCodes;

// GLOBAL: MW2 0x1010b5b8
// GLOBAL: MW2GBL 0x100e9a68
// GLOBAL: MW2PREMERCS 0x1017ba88
// GLOBAL: MW2MATROX 0x101d6974
// GLOBAL: MW23DFX 0x101b0ed4
// GLOBAL: MW2ATI 0x100e4eb4
// GLOBAL: MW2S3 0x100b1d78
// GLOBAL: MW2POWERVR 0x1010c3cc
MechU8 g_polygonAndCodes;

// The projected vertices of the polygon being built, and their count.
// GLOBAL: MW2 0x1010b550
// GLOBAL: MW2GBL 0x100e9a00
// GLOBAL: MW2PREMERCS 0x1017ba20
// GLOBAL: MW2MATROX 0x101d6910
// GLOBAL: MW23DFX 0x101b0e70
// GLOBAL: MW2ATI 0x100e4e50
// GLOBAL: MW2S3 0x100b1d10
// GLOBAL: MW2POWERVR 0x1010c370
ProjectedVertex* g_polygonPoints[20];

// GLOBAL: MW2 0x1010b5b0
// GLOBAL: MW2GBL 0x100e9a60
// GLOBAL: MW2PREMERCS 0x1017ba80
// GLOBAL: MW2MATROX 0x101d6978
// GLOBAL: MW23DFX 0x101b0ed8
// GLOBAL: MW2ATI 0x100e4eb8
// GLOBAL: MW2S3 0x100b1d70
// GLOBAL: MW2POWERVR 0x1010c3d0
MechS32 g_polygonPointCount;

// Where QueueFace copies the next polygon's vertex pointers in the draw buffer.
// GLOBAL: MW2 0x1010b534
// GLOBAL: MW2GBL 0x100e99e4
// GLOBAL: MW2MATROX 0x101d698c
// GLOBAL: MW23DFX 0x101b0eec
// GLOBAL: MW2S3 0x100b1cf4
MechU8* g_polygonPointCursor;

// QueueFace's counts: faces it took (5bc), faces past the back-face test (538), vertices it
// projected (5b4) and polygons it queued (5a8).

// GLOBAL: MW2 0x1010b5bc
// GLOBAL: MW2GBL 0x100e9a6c
// GLOBAL: MW2MATROX 0x101d6904
// GLOBAL: MW23DFX 0x101b0e54
MechS32 g_facesTried;

// GLOBAL: MW2 0x1010b538
// GLOBAL: MW2GBL 0x100e99e8
// GLOBAL: MW2MATROX 0x101d6988
// GLOBAL: MW23DFX 0x101b0ee8
MechS32 g_facesFrontFacing;

// GLOBAL: MW2 0x1010b5b4
// GLOBAL: MW2GBL 0x100e9a64
// GLOBAL: MW2MATROX 0x101d6984
// GLOBAL: MW23DFX 0x101b0ee4
MechS32 g_verticesTransformed;

// GLOBAL: MW2 0x1010b5a8
// GLOBAL: MW2GBL 0x100e9a58
// GLOBAL: MW2MATROX 0x101d68fc
// GLOBAL: MW23DFX 0x101b0e4c
// GLOBAL: MW2S3 0x100847cc
MechS32 g_polygonsQueued;

// GLOBAL: MW2 0x100ea8e0
// GLOBAL: MW2GBL 0x1015e210
// GLOBAL: MW2PREMERCS 0x101b76b0
// GLOBAL: MW2MATROX 0x101315a0
// GLOBAL: MW23DFX 0x101e8e20
// GLOBAL: MW2ATI 0x1011cae0
// GLOBAL: MW2S3 0x10162fe0
// GLOBAL: MW2POWERVR 0x101477a0
Path g_paths[0x40];

// The animations of the loaded animation files, by number.
// GLOBAL: MW2 0x101079e0
// GLOBAL: MW2GBL 0x1017b310
// GLOBAL: MW2PREMERCS 0x101d47b0
// GLOBAL: MW2MATROX 0x1014e880
// GLOBAL: MW23DFX 0x10206100
// GLOBAL: MW2ATI 0x10139dc0
// GLOBAL: MW2S3 0x101800e0
// GLOBAL: MW2POWERVR 0x10164a80
Reel* g_reels[0x780];

// GLOBAL: MW2 0x101097e0
// GLOBAL: MW2GBL 0x1017d110
// GLOBAL: MW2PREMERCS 0x101d65b0
// GLOBAL: MW2MATROX 0x1014e6a0
// GLOBAL: MW2ATI 0x10139be0
// GLOBAL: MW2S3 0x10181ee0
// GLOBAL: MW2POWERVR 0x101648a0
AnimFile g_animFiles[60];

// A timed callback (TimedCallbackFn) moving a thing's object through an animation. Its data is
// "<thing or class id>;<rate>,<flags>,<animation>", made for the player being created
// (g_lastPlayer). Each frame moves or turns the object by the frame's amount, spread over
// the rate; the frame events jump to other frames by the player's animation state (m_motionState,
// the frame it reached, and m_nextMotionState, the one it wants).
// FUNCTION: MW2 0x10046750
// FUNCTION: MW2GBL 0x100011c0
// FUNCTION: MW2MATROX 0x1006ece0
// FUNCTION: MW23DFX 0x1000dec0
// FUNCTION: MW2ATI 0x1000e5b0
// FUNCTION: MW2S3 0x10003360
// FUNCTION: MW2POWERVR 0x1000d1e0
MechS32 ReelMotionTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechS32 next;
	MechS32 elapsed;
	MechScalar y;
	MechChar* token;
	MechScalar z;
	MechS32 jump;
	ReelMotion* motion;
	MechS32 thing;
	MechS32 back;
	MechS32 enabled;
	MechScalar turnX;
	MechScalar amount;
	MechS32 found;
	MechScalar turnY;
	MechS32 target;
	MechS32 step;
	MechS32 value;
	MechS32 i;
	MechS32 frame;
	MechScalar turnZ;
	MechScalar x;
	void** slot;
	MechS32 id;
	MechS32 rate;
	Shape* shape;
	MechS32 match;
	MechS32 number;

	rate = 0;
	enabled = 0;
	number = 0;
	jump = FALSE;
	found = FALSE;
	thing = -1;
	switch (p_event) {
	case 0:
		motion = StaticPoolAlloc(sizeof(ReelMotion), g_staticPoolTags[4]);
		if (!motion) {
			return 0;
		}

		if (!g_lastPlayer) {
			g_reelMotionError = 1;
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, motion);
			return 0;
		}

		slot = GetCallbackData(GetCurrentCallback());
		*slot = motion;
		motion->m_object = NULL;
		motion->m_reel = NULL;
		motion->m_enabled = 0;
		motion->m_rate = 0;
		motion->m_flags = 0;
		motion->m_player = g_lastPlayer;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%ld,%d,%d", &rate, &enabled, &number);
			number += g_animBase;
			if (number > g_maxAnimNumber) {
				g_maxAnimNumber = number;
			}

			if (enabled) {
				motion->m_enabled = 1;
				if (enabled > 0x7f) {
					motion->m_flags |= 1;
				}
			}
			else {
				motion->m_enabled = 0;
				g_reelMotionError = 2;
			}

			motion->m_rate = rate;
			motion->m_reel = g_reels[number];
			motion->m_initial = g_reels[number];
			motion->m_frame = -1;
			motion->m_timer = 0;
			motion->m_amount = 0;
			motion->m_lastClock = p_clock;
			motion->m_player->m_animRate = rate;
			motion->m_player->m_motionState = -1;
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		thing = FindThingIdxById(id);
		if (thing != -1) {
			motion->m_object = GetClassObject(thing);
		}
		else {
			shape = FindClassById(id);
			if (shape) {
				motion->m_object = GetShapeObject(shape);
			}
		}

		if (!motion->m_object) {
			g_reelMotionError = 3;
		}
		break;
	case 1:
		slot = GetCallbackData(GetCurrentCallback());
		motion = *slot;
		if (!motion) {
			return 0;
		}

		if (!motion->m_object) {
			return 0;
		}

		if (!motion->m_player) {
			g_reelMotionError = 5;
			return 0;
		}

		if (!motion->m_enabled) {
			g_reelMotionError = 4;
			return 0;
		}

		elapsed = p_clock - motion->m_lastClock;
		motion->m_lastClock = p_clock;
		if (motion->m_player->m_animFlags & 0x10) {
			motion->m_reel = motion->m_initial;
			motion->m_frame = -1;
			motion->m_timer = 0;
			motion->m_amount = 0;
			motion->m_lastClock = p_clock;
			if (motion->m_flags & 1) {
				motion->m_player->m_animRate = motion->m_rate;
				motion->m_player->m_animFlags &= ~0x11;
				motion->m_player->m_motionState = -1;
			}
		}
		else if (motion->m_player->m_animFlags & 1) {
			if (!motion->m_rate) {
				g_reelMotionError = 6;
				return 0;
			}

			if (!motion->m_player->m_animRate) {
				g_reelMotionError = 7;
				return 0;
			}

			if (motion->m_timer <= 0) {
				amount = motion->m_amount;
			}
			else {
				amount = (motion->m_amount * elapsed) / motion->m_timer;
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

				RotateObj(motion->m_object, turnX, turnY, turnZ, 0);
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

				MoveObj(motion->m_object, x, y, z);
			}

			UpdateObj(motion->m_object);
			motion->m_amount -= amount;
			motion->m_timer -= elapsed;
			if (motion->m_frame == -1) {
				motion->m_frame = 0;
				motion->m_timer = 0;
				motion->m_player->m_motionState = 0;
			}

			if (motion->m_timer <= 0) {
				frame = motion->m_frame;
				motion->m_frame++;
				motion->m_timer += motion->m_player->m_animRate;
				if (-motion->m_player->m_animRate > motion->m_timer) {
					motion->m_timer = -motion->m_player->m_animRate;
				}

				if (motion->m_reel->m_events[frame].m_flags & 0x40) {
					target = motion->m_reel->m_events[frame].m_values[1];
					if (motion->m_player->m_nextMotionState == target) {
						jump = TRUE;
					}
					else {
						jump = FALSE;
					}
				}

				if (motion->m_reel->m_events[frame].m_flags & 0x80) {
					target = motion->m_reel->m_events[frame].m_values[1];
					if (motion->m_player->m_nextMotionState != -1 &&
						motion->m_player->m_motionState != motion->m_player->m_nextMotionState) {
						jump |= TRUE;
					}
					else {
					}
				}

				if (motion->m_reel->m_events[frame].m_flags & 0x100) {
					target = motion->m_reel->m_events[frame].m_values[2];
#ifdef MW2_CONDITIONAL_EXPRESSIONS
					jump |= motion->m_player->m_nextMotionState == -1;
#else
					if (motion->m_player->m_nextMotionState == -1) {
						jump |= TRUE;
					}
					else {
					}
#endif
				}

				if (jump) {
					jump = FALSE;
					if (motion->m_player->m_motionState < target) {
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
									if (motion->m_player->m_motionState == value) {
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

							if (motion->m_reel->m_events[frame].m_values[i] ==
								(MechS8) motion->m_player->m_nextMotionState) {
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
						motion->m_player->m_motionState = target;
					}

					motion->m_player->m_animFlags &= ~2;
					motion->m_player->m_animFlags &= ~8;
					if (motion->m_frame != -1) {
						motion->m_player->m_animRate =
							ScaleBySpeedLevel(motion->m_player->m_speedLevel, motion->m_rate);
						if (motion->m_reel->m_events[motion->m_frame].m_flags & 0x10) {
							motion->m_player->m_animFlags |= 2;
						}

						if (motion->m_reel->m_events[motion->m_frame].m_flags & 0x800) {
							motion->m_player->m_animFlags |= 8;
						}
					}
					else if (motion->m_player->m_nextMotionState == -1) {
						motion->m_player->m_animFlags &= ~1;
						motion->m_player->m_motionState = -1;
					}
				}
			}
		}
		else if (motion->m_flags & 1) {
			if (motion->m_player->m_nextMotionState != -1 || motion->m_player->m_motionState != -1) {
				motion->m_player->m_animFlags |= 1;
				motion->m_player->m_animRate = ScaleBySpeedLevel(motion->m_player->m_speedLevel, motion->m_rate);
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
// FUNCTION: MW2GBL 0x10001d74
// FUNCTION: MW2PREMERCS 0x100310e5
// FUNCTION: MW2MATROX 0x1006f8d3
// FUNCTION: MW23DFX 0x1000e600
// FUNCTION: MW2ATI 0x1000ecf0
// FUNCTION: MW2S3 0x10003a90
// FUNCTION: MW2POWERVR 0x1000d920
MechS32 ScaleBySpeedLevel(MechS32 p_mode, MechS32 p_value)
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
// (g_animBase). Returns -1 if it was already loaded.
// FUNCTION: MW2 0x10047380
// FUNCTION: MW2GBL 0x10001df6
// FUNCTION: MW2PREMERCS 0x10031167
// FUNCTION: MW2MATROX 0x1006f955
// FUNCTION: MW23DFX 0x1000e630
// FUNCTION: MW2ATI 0x1000ed20
// FUNCTION: MW2S3 0x10003ac0
// FUNCTION: MW2POWERVR 0x1000d950
MechS32 LoadAnimFile(ResourceRef* p_ref)
{
	MechS32 result;
	MechS32 i;
	MechS32 id;

	result = 0;
	id = p_ref->m_id;
	for (i = 0; i < g_animFileCount && g_animFiles[i].m_id != id; i++) {
	}

	if (i < g_animFileCount) {
		g_animBase = g_animFiles[i].m_base;
		result = -1;
	}
	else if (g_animFileCount < 60) {
		if (g_maxAnimNumber > 0) {
			g_animBase = g_maxAnimNumber + 1;
		}

		result = LoadReels(p_ref);
		g_animFiles[g_animFileCount].m_base = g_animBase;
		g_animFiles[g_animFileCount].m_id = id;
		g_animFileCount++;
	}

	return result;
}

// FUNCTION: MW2 0x10047462
// FUNCTION: MW2GBL 0x10001ed8
// FUNCTION: MW2PREMERCS 0x10031249
// FUNCTION: MW2MATROX 0x1006fa37
// FUNCTION: MW23DFX 0x1000e6c0
// FUNCTION: MW2ATI 0x1000edb0
// FUNCTION: MW2S3 0x10003b50
// FUNCTION: MW2POWERVR 0x1000d9e0
MechS32 GetAnimBase(void)
{
	return g_animBase;
}

// FUNCTION: MW2 0x10047477
// FUNCTION: MW2GBL 0x10001eed
// FUNCTION: MW2PREMERCS 0x1003125e
// FUNCTION: MW2MATROX 0x1006fa4c
// FUNCTION: MW23DFX 0x1000e6d0
MechS32 GetReelMotionSize(void)
{
	return 0x2c;
}

// A timed callback (TimedCallbackFn) cycling the face colors of a star's shape. Its data is
// "<star id>;<color>,<color>,...": up to sixteen colors, each a palette row.
// FUNCTION: MW2 0x1004748c
// FUNCTION: MW2GBL 0x10001f02
// FUNCTION: MW2PREMERCS 0x10031273
// FUNCTION: MW2MATROX 0x1006fa61
// FUNCTION: MW23DFX 0x1000e6e0
// FUNCTION: MW2ATI 0x1000edc0
// FUNCTION: MW2S3 0x10003b60
// FUNCTION: MW2POWERVR 0x1000d9f0
MechS32 ColorCycleTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	ColorCycle* cycle;
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
		cycle = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(ColorCycle));
		if (!cycle) {
			return 0;
		}

		slot = GetCallbackData(GetCurrentCallback());
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
			cycle->m_shape = GetStaticShapeSlot(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = GetCallbackData(GetCurrentCallback());
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
			SetFaceColor(cycle->m_model, i, cycle->m_colors[(offset + i) % cycle->m_colorCount]);
		}
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) turning a star's object. Its data is
// "<star id>;<x>,<y>,<z>,<period>": the turns in degrees per period of seconds.
// MW2MATROX: the rates and the period are floats, the rates in degrees.
// FUNCTION: MW2 0x1004771e
// FUNCTION: MW2GBL 0x10002194
// FUNCTION: MW2PREMERCS 0x10031505
// FUNCTION: MW2MATROX 0x1006fcf3
// FUNCTION: MW23DFX 0x1000e880
// FUNCTION: MW2ATI 0x1000ef60
// FUNCTION: MW2S3 0x10003d00
// FUNCTION: MW2POWERVR 0x1000db90
MechS32 SpinTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechScalar dz;
	MechChar* token;
	ObjectSpin* spin;
	MechFloat x;
	MechFloat y;
	MechFloat z;
	MechFloat period;
	MechS32 star;
	MechScalar t;
	MechS32 id;
	MechScalar dx;
	void** slot;
	MechScalar dy;

	switch (p_event) {
	case 0:
		spin = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(ObjectSpin));
		if (!spin) {
			return 0;
		}

		slot = GetCallbackData(GetCurrentCallback());
		*slot = spin;
		spin->m_rateX = spin->m_rateY = spin->m_rateZ = 0;
		spin->m_object = NULL;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%f,%f,%f,%f", &x, &y, &z, &period);
#ifdef MW2_FLOAT_SCALARS
			spin->m_rateX = x;
			spin->m_rateY = y;
			spin->m_rateZ = z;
#else
			spin->m_rateX = x * 65536.0 + 0.5;
			spin->m_rateY = y * 65536.0 + 0.5;
			spin->m_rateZ = z * 65536.0 + 0.5;
#endif
			spin->m_period = period * 181.0f;
			if (!FIXED_IS_NONZERO(spin->m_period)) {
				spin->m_period = 181;
			}

			spin->m_lastClock = p_clock;
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			spin->m_shape = GetStaticShapeSlot(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = GetCallbackData(GetCurrentCallback());
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
#ifdef MW2_FLOAT_SCALARS
		t = (p_clock - spin->m_lastClock) / spin->m_period;
		dx = t * spin->m_rateX;
		dy = spin->m_rateY * t;
		dz = spin->m_rateZ * t;
#else
		t = ((p_clock - spin->m_lastClock) << 16) / spin->m_period;
		dx = FixedMul16(spin->m_rateX, t);
		dy = FixedMul16(spin->m_rateY, t);
		dz = FixedMul16(spin->m_rateZ, t);
#endif
		spin->m_lastClock = p_clock;
		RotateObj(spin->m_object, dx, dy, dz, 0);
		UpdateObj(spin->m_object);
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) moving a star's object around a circle. Its data is
// "<star id>;<radius>,<period>,<enabled>,<unused>".
// MW2MATROX: the angle, rates and steps are floats, the angle in degrees.
// FUNCTION: MW2 0x100479ec
// FUNCTION: MW2GBL 0x10002462
// FUNCTION: MW2PREMERCS 0x100317d3
// FUNCTION: MW2MATROX 0x1006ff89
// FUNCTION: MW23DFX 0x1000ea50
// FUNCTION: MW2ATI 0x1000f130
// FUNCTION: MW2S3 0x10003ec0
// FUNCTION: MW2POWERVR 0x1000dd60
MechS32 OrbitTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	MechScalar z;
	MechScalar cosine;
	ObjectOrbit* orbit;
	MechScalar t;
	MechScalar step;
	MechS32 star;
	MechScalar sine;
	MechS32 enabled;
	MechFloat radius;
	MechS32 unused;
	MechFloat period;
	MechS32 id;
	void** slot;
	MechScalar x;

	switch (p_event) {
	case 0:
		orbit = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(ObjectOrbit));
		if (!orbit) {
			return 0;
		}

		slot = GetCallbackData(GetCurrentCallback());
		*slot = orbit;
		orbit->m_enabled = 0;
		orbit->m_object = NULL;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%f,%f,%d,%d", &radius, &period, &enabled, &unused);
#ifdef MW2_FLOAT_SCALARS
			orbit->m_turnRate = 360.0 / period;
			orbit->m_speed = (radius * 6.283185307179586) / period;
#else
			orbit->m_turnRate = 360.0 / period * 65536.0 + 0.5;
			orbit->m_speed = radius * 6.283185307179586 / period * 65536.0 + 0.5;
#endif
			orbit->m_enabled = enabled;
			orbit->m_angle = 0;
		}

		orbit->m_lastClock = p_clock;
		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			orbit->m_shape = GetStaticShapeSlot(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = GetCallbackData(GetCurrentCallback());
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
		cosine = FixedCos(orbit->m_angle);
		sine = FixedSin(orbit->m_angle);
#ifdef MW2_FLOAT_SCALARS
		t = (p_clock - orbit->m_lastClock) / 181.0f;
		orbit->m_lastClock = p_clock;
		step = t * orbit->m_speed;
		x = step * cosine;
		z = -(sine * step);
#else
		t = FixedDiv16(p_clock - orbit->m_lastClock, 181);
		orbit->m_lastClock = p_clock;
		step = FixedMul16(orbit->m_speed, t) >> 16;
		x = FixedMul16(cosine, step) >> 13;
		z = FixedMul16(-sine, step) >> 13;
#endif
		MoveObj(orbit->m_object, x, 0, z);
		UpdateObj(orbit->m_object);
#ifdef MW2_FLOAT_SCALARS
		step = orbit->m_turnRate * t;
#else
		step = FixedMul16(orbit->m_turnRate, t);
#endif
		RotateObj(orbit->m_object, 0, step, 0, 0);
		UpdateObj(orbit->m_object);
		orbit->m_angle += step;
#ifdef MW2_FLOAT_SCALARS
		orbit->m_angle = FIXED_MOD360(orbit->m_angle);
#else
		orbit->m_angle %= 0x1680000;
#endif
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) looping a sound on a star's object. Its data is
// "<star id>;<range>,<sound name>,<enabled>".
// FUNCTION: MW2 0x10047d10
// FUNCTION: MW2GBL 0x10002786
// FUNCTION: MW2PREMERCS 0x10031af7
// FUNCTION: MW2MATROX 0x100702a2
// FUNCTION: MW23DFX 0x1000ec90
// FUNCTION: MW2ATI 0x1000f370
// FUNCTION: MW2S3 0x100040d0
// FUNCTION: MW2POWERVR 0x1000dfa0
MechS32 AmbientSoundTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
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

		slot = GetCallbackData(GetCurrentCallback());
		*slot = sound;
		sound->m_slot = -1;
		sound->m_data = NULL;
		sound->m_skip = 1;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%d,%[^,],%d", &sound->m_range, name, &sound->m_enabled);
			sound->m_range *= 100;
		}

		sound->m_id = FindResourceIdByName(0xb, name);
		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			sound->m_shape = GetStaticShapeSlot(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = GetCallbackData(GetCurrentCallback());
		sound = *slot;
		if (!sound) {
			return 0;
		}
		else if (!sound->m_shape || !sound->m_enabled) {
			StopAmbientSound(sound);
			return 0;
		}

		if (!*sound->m_shape) {
			StopAmbientSound(sound);
			return 0;
		}

		sound->m_obj = GetShapeObject(*sound->m_shape);
		UpdateAmbientSound(sound);
		break;
	case 2:
		slot = GetCallbackData(GetCurrentCallback());
		sound = *slot;
		if (!sound) {
			return 1;
		}

		StopAmbientSound(sound);
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) moving a star's object along a path. Its data is
// "<star id>;<mode>,<rotate>,<path name>": mode "loop", "repeat" or else stop at the end, and
// "rotate" to turn the object along the path. Event -1 restarts it.
// FUNCTION: MW2 0x10047f60
// FUNCTION: MW2GBL 0x100029d6
// FUNCTION: MW2MATROX 0x100704f2
// FUNCTION: MW23DFX 0x1000ee10
// FUNCTION: MW2ATI 0x1000f4f0
// FUNCTION: MW2S3 0x10004250
// FUNCTION: MW2POWERVR 0x1000e120
MechS32 PathTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar rotate[64];
	MechChar* token;
	PathFollower* follower;
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
	MechScalar angle;
	PathPoint* next;
	MechScalar delta;
	MechScalar t;
	PathPoint* point;
	MechS32 end;

	switch (p_event) {
	case -1:
		slot = GetCallbackData(GetCurrentCallback());
		follower = *slot;
		if (!follower) {
			return 0;
		}

		follower->m_startClock = p_clock;
		if (follower->m_path) {
			first = follower->m_path->m_points;
			StartScalarRamp(&follower->m_x, first->m_x, first->m_x, 0.3);
			StartScalarRamp(&follower->m_y, first->m_y, first->m_y, 0.3);
			StartScalarRamp(&follower->m_z, first->m_z, first->m_z, 0.3);
			StartWrappedRamp(&follower->m_pitch, 0, 0, 0.3, FIXED_CONST(360));
			StartWrappedRamp(&follower->m_heading, 0, 0, 0.3, FIXED_CONST(360));
			StartWrappedRamp(&follower->m_roll, 0, 0, 0.3, FIXED_CONST(360));
			total = 0;
			for (i = 0; i < follower->m_path->m_count; i++) {
				total += follower->m_path->m_points[i].m_duration;
			}

			follower->m_duration = total;
			id = atoi(p_data);
			id = MapResourceId(id);
			id = atoi(p_data);
			id = MapResourceId(id);
			star = FindStarIdxById(id);
			if (star != -1) {
				follower->m_shape = GetStaticShapeSlot(star);
			}
			else {
				return 0;
			}
		}
		break;
	case 0:
		follower = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(PathFollower));
		if (!follower) {
			return 0;
		}

		slot = GetCallbackData(GetCurrentCallback());
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

			follower->m_rotate = _strcmpi(rotate, "rotate") == 0;

			for (i = 0; i < g_pathCount; i++) {
				if (_strcmpi(name, g_paths[i].m_name) == 0) {
					follower->m_path = &g_paths[i];
					break;
				}
			}
		}

		if (follower->m_path) {
			origin = follower->m_path->m_points;
			StartScalarRamp(&follower->m_x, origin->m_x, origin->m_x, 0.3);
			StartScalarRamp(&follower->m_y, origin->m_y, origin->m_y, 0.3);
			StartScalarRamp(&follower->m_z, origin->m_z, origin->m_z, 0.3);
			StartWrappedRamp(&follower->m_pitch, 0, 0, 0.3, FIXED_CONST(360));
			StartWrappedRamp(&follower->m_heading, 0, 0, 0.3, FIXED_CONST(360));
			StartWrappedRamp(&follower->m_roll, 0, 0, 0.3, FIXED_CONST(360));
			total = 0;
			for (i = 0; i < follower->m_path->m_count; i++) {
				total += follower->m_path->m_points[i].m_duration;
			}

			follower->m_duration = total;
			id = atoi(p_data);
			id = MapResourceId(id);
			id = atoi(p_data);
			id = MapResourceId(id);
			star = FindStarIdxById(id);
			if (star != -1) {
				follower->m_shape = GetStaticShapeSlot(star);
			}
			else {
				return 0;
			}
			break;
		}
		break;
	case 1:
		slot = GetCallbackData(GetCurrentCallback());
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
			end += path->m_points[i].m_duration;
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
				end = path->m_points[0].m_duration;
				StartScalarRamp(&follower->m_x, path->m_points[0].m_x, path->m_points[0].m_x, 0.3);
				StartScalarRamp(&follower->m_y, path->m_points[0].m_y, path->m_points[0].m_y, 0.3);
				StartScalarRamp(&follower->m_z, path->m_points[0].m_z, path->m_points[0].m_z, 0.3);
				StartWrappedRamp(&follower->m_pitch, 0, 0, 0.3, FIXED_CONST(360));
				StartWrappedRamp(&follower->m_heading, 0, 0, 0.3, FIXED_CONST(360));
				StartWrappedRamp(&follower->m_roll, 0, 0, 0.3, FIXED_CONST(360));
			}
			else if (path->m_count == i) {
				i = 0;
				elapsed -= follower->m_duration;
				follower->m_startClock = p_clock - elapsed;
				start = 0;
				end = path->m_points[0].m_duration;
			}
		}

		point = &path->m_points[i];
		if (path->m_count - 1 == i) {
			next = path->m_points;
		}
		else {
			next = &path->m_points[i + 1];
		}

#ifdef MW2_FLOAT_SCALARS
		t = (MechFloat) (elapsed - start) / point->m_duration;
		follower->m_x.m_target = (next->m_x - point->m_x) * t + point->m_x;
		follower->m_y.m_target = (next->m_y - point->m_y) * t + point->m_y;
		follower->m_z.m_target = (next->m_z - point->m_z) * t + point->m_z;
#else
		t = FixedDiv16(elapsed - start, point->m_duration);
		follower->m_x.m_target = point->m_x + FixedMul16(t, next->m_x - point->m_x);
		follower->m_y.m_target = point->m_y + FixedMul16(t, next->m_y - point->m_y);
		follower->m_z.m_target = point->m_z + FixedMul16(t, next->m_z - point->m_z);
#endif
		SetObjPosition(
			follower->m_object,
			UpdateScalarRamp(&follower->m_x),
			UpdateScalarRamp(&follower->m_y),
			UpdateScalarRamp(&follower->m_z)
		);
		angle = point->m_heading;
		if (follower->m_rotate) {
			angle += FixedAtan2(next->m_x - point->m_x, next->m_z - point->m_z);
		}

		delta = angle - follower->m_heading.m_value;
		while (delta > FIXED_CONST(180)) {
			delta -= FIXED_CONST(360);
		}

		while (delta < -FIXED_CONST(180)) {
			delta += FIXED_CONST(360);
		}

		follower->m_heading.m_target = angle;
		follower->m_heading.m_value = angle - delta;
		angle = point->m_pitch;
		if (follower->m_rotate) {
#ifdef MW2_FLOAT_SCALARS
			angle -= FixedAsin((next->m_y - point->m_y) / 65536.0f);
#else
			angle -= FixedAsin(FIXED_SHL(next->m_y - point->m_y, 13));
#endif
		}

		delta = angle - follower->m_pitch.m_value;
		while (delta > FIXED_CONST(180)) {
			delta -= FIXED_CONST(360);
		}

		while (delta < -FIXED_CONST(180)) {
			delta += FIXED_CONST(360);
		}

		follower->m_pitch.m_target = angle;
		follower->m_pitch.m_value = angle - delta;
		angle = point->m_roll;
		delta = angle - follower->m_roll.m_value;
		while (delta > FIXED_CONST(180)) {
			delta -= FIXED_CONST(360);
		}

		while (delta < -FIXED_CONST(180)) {
			delta += FIXED_CONST(360);
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
		UpdateObj(follower->m_object);
		break;
	default:
		break;
	}

	return 1;
}

// The faces' projection: 1.1 has it here, the Matrox edition in an object of its own, after
// mechcollision.c's, in floats (accel/queueface.c), and GBL in an object of its own at that place
// (and so does the early Mercenaries build, at its own place).
#if !defined(MW2_A3D) && !defined(MW2_GBL) && !defined(MW2_PREMERCS)
#include "faceprojection.c"
#endif
