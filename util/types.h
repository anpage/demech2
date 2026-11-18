#ifndef TYPES_H
#define TYPES_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* Project type vocabulary. Typedefs are codegen-neutral; the underlying type
   (signed/unsigned, char/int, float/double) is what affects /Od codegen, so the
   width and signedness are explicit here. */

typedef signed char MechS8;
typedef unsigned char MechU8;
typedef short MechS16;
typedef unsigned short MechU16;
typedef int MechS32;
typedef unsigned int MechU32;
typedef float MechFloat;
typedef double MechDouble;
typedef char MechChar;

/* The integers the original keeps in pointers (a screen field's item index, a file offset) and
   the pointers it keeps in 32-bit integers (a Miles sample's user data) go through a
   pointer-sized integer, so that the conversions are defined where pointers are wider than 32
   bits. Macros, not typedefs: a typedef here is a symbol in every unit, and 4.1's operand order
   and stack slots react to the symbol count. */
#if defined(_MSC_VER) && _MSC_VER < 1200
#define MECH_INTPTR int
#else
#include <stdint.h>
#define MECH_INTPTR intptr_t
#endif

#define MECH_PTR_TO_S32(p_pointer) ((MechS32) (MECH_INTPTR) (p_pointer))
#define MECH_S32_TO_PTR(p_value) ((void*) (MECH_INTPTR) (p_value))

/* The simulator's editions and their features. The build defines an edition's identity: nothing
   for 1.1, MW2_ACCEL for the accelerated (3D-card) editions, which share the Matrox edition's
   changes to 1.1, and with it MW2_MATROX (the Matrox Mystique edition) or MW2_3DFX (the 3Dfx
   edition). The code tests the features, named for what they do; an edition's own back end, and
   a computation one edition's source writes another way with the same behaviour, test the
   identity. Macros cost no unit anything here (types.h is already included), where a header of
   their own would move code: its first inclusion counts as a symbol on 4.0 and 4.1. Features
   no binary separates (the ones both accelerated editions have) are attributed by what the code
   does. */
#if defined(MW2_MATROX) && defined(MW2_3DFX)
#error "MW2_MATROX and MW2_3DFX are different editions"
#endif
#if (defined(MW2_MATROX) || defined(MW2_3DFX)) && !defined(MW2_ACCEL)
#error "an accelerated edition defines MW2_ACCEL"
#endif

#ifdef MW2_ACCEL
/* Representation: the simulation's 16.16 fixed-point values are floats (MechScalar, FIXED_*) */
#define MW2_FLOAT_SCALARS

/* The A3D renderer interface and its records: queued polygons, projected vertices, depth lists */
#define MW2_A3D
/* Per-vertex shading for the renderer: WTB normals and colours, shaded projected vertices */
#define MW2_VERTEX_SHADING
/* The 2D drawing goes to the 16-bit port of VFXA */
#define MW2_VFX16
/* VFX as released (unaligned fills and runs, word-wise character copies), where 1.1 modified it; the
   assembly gets it from cmake/editions.cmake */
#define MW2_VFX_STOCK
/* VFXA's VFX_lookaside_write, which sets one lookaside entry (cmake/editions.cmake) */
#define MW2_VFX_LOOKASIDE_WRITE
/* A larger draw buffer and polygon limit */
#define MW2_LARGER_POLYGON_BUFFERS
/* StartPalettes blends through the display back end, without 1.1's pixel dissolve */
#define MW2_NO_DISSOLVE
/* DispDibBegin always fails */
#define MW2_DISPLAYDIB_DISABLED
/* InitRefreshMode tries a mode without checking it's available, and marks it */
#define MW2_REFRESH_MODE_RETRY
/* Screenshots are the screen's raw 24-bit pixels (MW2NNNN.888), not GIFs */
#define MW2_RAW_SCREENSHOTS

/* The graphics page, in place of 1.1's monitor brightness page */
#define MW2_GRAPHICS_MENU
/* The sky, the ground and the renderer's settings are read from the registry */
#define MW2_REGISTRY_SETTINGS
/* The draw mode and its parameter start at 0 (1.1: -1 and 1) */
#define MW2_DEFAULT_DRAW_MODE

/* The HUD art: its resolution chosen before the 512x384 override, and the shapes' numbering */
#define MW2_HUD_ART
/* HUD shape resource ids 3 higher (the pause banner, the target panel, the nav icons) */
#define MW2_HUD_SHAPE_IDS
/* The menu sliders' shapes are 3 higher */
#define MW2_SLIDER_SHAPES
/* The main menu's background is shape 0x118, not 0x112 */
#define MW2_MENU_BACKGROUND_0X118
/* The resource archive numbers the preloaded cels differently */
#define MW2_PRJ_CEL_IDS
/* The maps' colours are other (brighter) palette entries */
#define MW2_BRIGHT_MAP_COLORS
/* The cockpit layout keeps only its point, and SetRes scales no gauge pane or point */
#define MW2_COCKPIT_POINT_ONLY
/* LoadCockpitLayout doesn't map the gauge indices to functions */
#define MW2_UNRESOLVED_GAUGE_INDICES
/* The debug overlays' text is colour 0x52, not 0x4f */
#define MW2_DEBUG_OVERLAY_COLOR
/* No eye position overlay */
#define MW2_NO_EYE_POSITION_OVERLAY
/* No debug drawing of the shapes' bounding spheres */
#define MW2_NO_BOUNDING_SPHERES
/* Turning infrared off fades back to the base palette */
#define MW2_INFRARED_OFF_FADE
/* The palette's fade back takes the duration as its step count */
#define MW2_FADE_BACK_DURATION
/* FadeToTimeOfDayPhase fades only while no phase is set */
#define MW2_TIME_OF_DAY_FADE_ONCE

/* Bit 15 of a face's id is part of its kind, not a flag for MapFaceId's remap */
#define MW2_FACE_KIND_BIT15
/* LoadShapeRecord decodes negated shape names */
#define MW2_WTB_NEGATED_NAMES
/* AddModel allocates at least one face */
#define MW2_MODEL_MIN_ONE_FACE
/* The grid object's anchor and cell keep only x and z */
#define MW2_GRID_CELL_XZ
/* The eyepoint has no projection offset */
#define MW2_NO_PROJECTION_OFFSET
/* The vertical frustum limit takes its sign from the side distance */
#define MW2_FRUSTUM_HEIGHT_SIGN_FROM_SIDE
/* TestRayBox doesn't record whether the ray entered behind */
#define MW2_NO_RAY_BOX_ENTRY_BEHIND
/* DestroyObjShape clears the aimed shape when it destroys it */
#define MW2_AIMED_SHAPE_RESET

/* The bounce off buildings isn't negated, and is halved either way */
#define MW2_BUILDING_BOUNCE
/* Damage from below scales by twice the mass ratio, not four times */
#define MW2_COLLISION_DAMAGE_X2
/* Radius (fire) damage to the head is half, not a quarter */
#define MW2_RADIUS_HEAD_DAMAGE_HALF
/* StartEngineNote returns at once */
#define MW2_NO_ENGINE_NOTE

/* IsTeamNearTarget tests only the team's leader */
#define MW2_LEADER_NEAR_TARGET
/* The local star starts at 0, and the player's formation goes to it */
#define MW2_LOCAL_STAR_FROM_ZERO
/* UpdateObjective takes star N as player N in network games */
#define MW2_NETWORK_STAR_PLAYERS
/* Restarting a star mission keeps its objectives' targets' listed flags */
#define MW2_RESTART_KEEPS_LISTED

/* JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX */
#define MW2_JOYSTICK_OPEN_POSEX
/* JoystickPoll neither clears its JOYINFOEX nor sets its size */
#define MW2_JOYINFOEX_UNCLEARED
/* UpdateInputs compares an axis' rate pointer with its position's address */
#define MW2_AXIS_RATE_POINTER_TEST
/* Key code 0x40d no longer toggles full screen */
#define MW2_NO_FULLSCREEN_KEY
/* The "lemmy" cheat */
#define MW2_LEMMY_CHEAT
/* 1.1's "dorcs" cheat is gone */
#define MW2_NO_DORCS_CHEAT
/* Debug output both accelerated editions add */
#define MW2_EXTRA_DEBUG_PRINTS
#endif

#ifdef MW2_MATROX
/* The 16-bit pixels and the palette: RGB565 */
#define MW2_PIXEL_FORMAT 565
/* Key colour fills and 0xfffe wipes for the renderer's overlay, ClipRectToPane */
#define MW2_KEY_COLOR
/* VFXREND's polygon renderer reads its table through CS and leaves ES as it is (cmake/editions.cmake) */
#define MW2_VFXREND_CS_SEGMENT
/* LOD.PAR's LOD_SCALE scales the detail scale */
#define MW2_LOD_PAR
/* The "Filter" registry setting, passed to the renderer */
#define MW2_TEXTURE_FILTER
/* A depth entry is 0xc bytes, a dword ahead of the polygon */
#define MW2_DEPTH_ENTRY_TAG
/* 640x400: SimMain sets it instead of SetGameResolution, and the art follows */
#define MW2_RES_640X400
/* LoadClassLevel records the player's detail level itself */
#define MW2_DETAIL_LEVEL_IN_LOADER
/* LoadClassEntryShape returns early when the entry is loaded at the level */
#define MW2_SKIP_LOADED_LEVEL
/* A shot's heat and damage are applied after RecordAttack */
#define MW2_SHOT_DAMAGE_AFTER_RECORD
/* Both power keys do nothing in power state 7, tested first (1.1: key 0x3e only) */
#define MW2_POWER_STATE_7_KEYS
/* JoystickPoll sums the axes between polls */
#define MW2_JOYSTICK_AXIS_SUMS
/* ToggleFullScreen records the current mode as the fastest */
#define MW2_FULLSCREEN_SAVES_MODE
/* HandleMessages returns when its mouse-skipping PeekMessage finds nothing */
#define MW2_MOUSE_PEEK_RETURNS
/* SupAnimTimerCallback is __stdcall and takes the timer's argument */
#define MW2_STDCALL_TIMER_CALLBACK
/* The debug log's messages end in newlines, with prints of its own */
#define MW2_DEBUG_LOG_TEXT
#endif

#ifdef MW2_3DFX
/* The 16-bit pixels and the palette: RGB555 */
#define MW2_PIXEL_FORMAT 555
/* The "Res800x600" setting: 800x600, the menus scaled, animation 0 tiled */
#define MW2_RES_800X600
/* ChooseArtResolution takes the nearest of the three sizes */
#define MW2_ART_RESOLUTION_NEAREST
/* The sky and ground textures' tiling defaults: 4 x 0.8, and 20000 */
#define MW2_SKY_TILE_DEFAULTS
/* DrawSkyAndGround sets the sky and ground palette entries */
#define MW2_SKY_GROUND_COLORS
/* Message boxes' backgrounds are shape 0x4f, not 0x4c */
#define MW2_MESSAGE_BACKGROUND_SHAPES
/* The wireframe outline clears flag bits 12, 14 and 15 */
#define MW2_OUTLINE_CLEARS_FLAGS
/* The torso pan's range is -90 to 90 */
#define MW2_NARROW_TORSO_PAN
/* The throttle's range is 0 to 1024 */
#define MW2_NO_REVERSE_THROTTLE
/* No hit feedback for the local player while its flag 2 is set */
#define MW2_HIT_FEEDBACK_FLAG_CHECK
/* FindSession and StopExternalIO return 0 where the others fall off the end */
#define MW2_NET_RETURN_ZERO
/* NetSend and NetReceive log nothing */
#define MW2_NO_NET_DEBUG_PRINTS
/* Key 0x3e asks for power down in power state 7 too, where 1.1 ignores it */
#define MW2_NO_STATE_7_POWER_TEST
/* The map layouts hold their gauge functions from the start, not indices in g_cockpitGauges */
#define MW2_LAYOUT_GAUGE_FUNCTIONS
/* Artefact: the edition's headers define FixedMul16, FixedDiv16, MulDiv64, HashName and DebugBreakpoint
   __inline (its /Ox expands them in every caller), and MemCopy and MemSet are C */
#define MW2_INLINE_HELPERS
/* The debug log's own formats (the sky's colours on one line, the armor as %f) */
#define MW2_DEBUG_LOG_FORMATS
#endif

/* Boolean typedefs (MechBool / MechBool8 ...) are added when a match proves
   the width and signedness. */

#endif /* TYPES_H */
