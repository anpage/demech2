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
   changes to 1.1, and with it MW2_MATROX (the Matrox Mystique edition), MW2_3DFX (the 3Dfx
   edition), MW2_ATI (the ATI 3D Rage edition), MW2_S3 (the S3 ViRGE edition) or MW2_POWERVR (the
   PowerVR edition), MW2_GBL alone for Ghost Bear's Legacy's simulator, 1.1's code a month
   later, and MW2_PREMERCS alone for an unused early build of Mercenaries' simulator, Ghost Bear's
   Legacy's code three months earlier. The code tests the features, named for what they do; an edition's own
   back end, and a computation one edition's source writes another way with the same behaviour, test the identity.
   Macros cost no unit anything here (types.h is already included), where a header of their own would move code: its
   first inclusion counts as a symbol on 4.0 and 4.1. Features no binary separates (the ones both accelerated editions
   have) are attributed by what the code does. */
#if (defined(MW2_MATROX) && defined(MW2_3DFX)) || (defined(MW2_MATROX) && defined(MW2_ATI)) ||                         \
	(defined(MW2_MATROX) && defined(MW2_S3)) || (defined(MW2_MATROX) && defined(MW2_POWERVR)) ||                       \
	(defined(MW2_3DFX) && defined(MW2_ATI)) || (defined(MW2_3DFX) && defined(MW2_S3)) ||                               \
	(defined(MW2_3DFX) && defined(MW2_POWERVR)) || (defined(MW2_ATI) && defined(MW2_S3)) ||                            \
	(defined(MW2_ATI) && defined(MW2_POWERVR)) || (defined(MW2_S3) && defined(MW2_POWERVR))
#error "MW2_MATROX, MW2_3DFX, MW2_ATI, MW2_S3 and MW2_POWERVR are different editions"
#endif
#if (defined(MW2_MATROX) || defined(MW2_3DFX) || defined(MW2_ATI) || defined(MW2_S3) || defined(MW2_POWERVR)) &&       \
	!defined(MW2_ACCEL)
#error "an accelerated edition defines MW2_ACCEL"
#endif
#if (defined(MW2_GBL) || defined(MW2_PREMERCS)) && defined(MW2_ACCEL)
#error "MW2_GBL and MW2_PREMERCS are no accelerated editions"
#endif
#if defined(MW2_GBL) && defined(MW2_PREMERCS)
#error "MW2_GBL and MW2_PREMERCS are different editions"
#endif
/* GBL's build and the early Mercenaries build have no /Oi (they call memcpy, memset, strcmp and the
   math functions where 1.1 expands them), but expand abs and fabs as 1.1 does. How it did that isn't known (a pragma
   like this one or other headers); this reproduces it. VC++ 4.1 takes the pragma only for a function declared ahead of
   it, hence the CRT's prototypes. */
#if defined(MW2_GBL) || defined(MW2_PREMERCS)
int __cdecl abs(int);
double __cdecl fabs(double);
#pragma intrinsic(abs, fabs)
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
/* JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX */
#define MW2_JOYSTICK_OPEN_POSEX
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
/* ChooseArtResolution takes art resolution 1 for a 512x384 window */
#define MW2_ART_RESOLUTION_512X384
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
/* QueueFace copies the face's normal into locals for its backface test, summing the z term first, keeps
   the z in depth, sums the average depth in dot, counts the depth loops down and or-s the vertices' clip
   codes into a signed byte */
#define MW2_QUEUEFACE_REUSED_LOCALS
/* ProjectVertex projects y after it tests x against the view's sides */
#define MW2_PROJECT_Y_AFTER_X
/* InitSlopeTables computes each slope once, for its cosine and its sine */
#define MW2_SLOPE_ONCE
/* HorizonYAtX keeps the horizon's y a float, converting it to an integer only for the return */
#define MW2_HORIZON_FLOAT_Y
/* SimMain reads the sky and ground settings (LoadSkyGroundSettings) just before FirstClock */
#define MW2_SKY_SETTINGS_BEFORE_CLOCK
/* SimMain reads the render settings (FUN_1008fad0) just before FirstClock */
#define MW2_RENDER_SETTINGS_BEFORE_CLOCK
/* SimMain releases the DC it tests for StartSupAnim */
#define MW2_SUPANIM_DC_RELEASED
/* SimMain releases the DC it tests the palette through */
#define MW2_PALETTE_DC_RELEASED
/* JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX */
#define MW2_JOYSTICK_OPEN_POSEX
/* The 16-bit pixels and the palette: RGB555 */
#define MW2_PIXEL_FORMAT 555
/* The "Res800x600" setting: 800x600, the menus scaled, animation 0 tiled */
#define MW2_RES_800X600
/* ChooseArtResolution takes the nearest of the three sizes */
#define MW2_ART_RESOLUTION_NEAREST
/* The sky and ground textures' tiling defaults: 4 x 0.8, and 20000 */
#define MW2_SKY_TILE_DEFAULTS
/* LoadSkyGroundSettings sets the defaults when skygnd.par has no line for the mission too */
#define MW2_SKY_DEFAULTS_AT_END
/* DrawSkyAndGround sets the sky and ground palette entries */
#define MW2_SKY_GROUND_COLORS
/* DrawSkyAndGround goes through the horizon's cases when it draws neither the sky nor the ground */
#define MW2_SKY_GROUND_ALL_CASES
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
/* Artefact: the edition's source declares some of UpdateMech's, LateUpdateMech's, DodgeShot's and the HUD
   brackets' locals in the blocks that use them, where VC++ 2.2 and 4.1 let them share the frame (1.1's and
   the Matrox edition's declare them at the top) */
#define MW2_BLOCK_SCOPED_LOCALS
/* UpdateMech's air drag divides by the length once, then scales both axes by it */
#define MW2_DRAG_DIVIDED_ONCE
/* Artefact: LateUpdateMech's torso recentring records the heading without testing isLocal again (it
   holds there) */
#define MW2_RECENTER_NO_RETEST
/* Artefact: DodgeShot picks the side before it divides the heading */
#define MW2_DODGE_SIDE_FIRST
/* Artefact: MoveMechWithCollisions sums the bounce's dot product in three statements and negates the
   speed before it quarters it (2.2 and 4.1 then keep each term's doubling and fold the sign) */
#define MW2_BOUNCE_DOT_STATEMENTS
/* Artefact: the edition's source reads an AI target id's two bytes themselves
   (missionobjective.h's OBJECTIVE_TARGET_KIND), where 1.1's and the Matrox edition's shift the word */
#define MW2_OBJECTIVE_TARGET_BYTES
/* The debug log prints the sky's and the ground's colours on one line */
#define MW2_DEBUG_LOG_FORMATS
/* ProjectPolygon clips to the near plane itself, as 1.1's QueueFace does, not through the renderer */
#define MW2_PROJECT_NEAR_CLIP
/* The sky and ground drawers clip the polygon to the pane themselves: their callers draw it even
   when ProjectPolygon leaves no points */
#define MW2_SKY_GROUND_CLIPPED_HELPERS
/* The polygon drawers clip to the pane's edges themselves (FUN_10067b90), and the outline is closed */
#define MW2_CLIP_TO_PANE_EDGES
/* The textures are the animations' frames as they are, 16-bit texels (FUN_10051dfe), and their
   average colors are computed from them */
#define MW2_FRAME_TEXELS
/* Artefact: RestoreBrightness passes the brightness to PreviewBrightness through its store */
#define MW2_BRIGHTNESS_STORE_CHAINED
/* Artefact: the edition's source writes some flags and choices as expressions (`m_up = angle <= 180`,
   `x = c ? a : b`) where 1.1's and the Matrox edition's /Od code shows if and else (GetRectNeedleAt,
   HandleDebugKey, InitializeManeuvers' skill flags, ReelMotionTask's jump flag) */
#define MW2_CONDITIONAL_EXPRESSIONS
/* HideCacheInfo blanks its line without sprintf's two stray arguments */
#define MW2_HIDE_CACHE_INFO_NO_ARGS
/* Artefact: LoadCachedResource formats both of its messages in one buffer */
#define MW2_ONE_RESOURCE_MESSAGE
/* Artefact: RunAIWeapons scales the torso tilt by -0xf00, where the others negate the product */
#define MW2_TILT_NEGATIVE_FACTOR
/* LoadMechConfig logs the armor and the jump thrust as %f */
#define MW2_MEK_LOG_FLOATS
/* Artefact: UpdateShot assigns the ground's height inside its comparison */
#define MW2_GROUND_HEIGHT_IN_TEST
#endif

#ifdef MW2_ATI
/* DrawSkyAndGround goes through the horizon's cases when it draws neither the sky nor the ground */
#define MW2_SKY_GROUND_ALL_CASES
/* DrawSkyAndGround sets the sky and ground palette entries */
#define MW2_SKY_GROUND_COLORS
/* QueueFace copies the face's normal into locals for its backface test, summing the z term first, keeps
   the z in depth, sums the average depth in dot, counts the depth loops down and or-s the vertices' clip
   codes into a signed byte */
#define MW2_QUEUEFACE_REUSED_LOCALS
/* ProjectVertex projects y after it tests x against the view's sides */
#define MW2_PROJECT_Y_AFTER_X
/* The wireframe outline clears flag bits 12, 14 and 15 */
#define MW2_OUTLINE_CLEARS_FLAGS
/* InitSlopeTables computes each slope once, for its cosine and its sine */
#define MW2_SLOPE_ONCE
/* HorizonYAtX keeps the horizon's y a float, converting it to an integer only for the return */
#define MW2_HORIZON_FLOAT_Y
/* VFX3D's VFX_flat_polygon fills its spans with the destination aligned, as 1.1's modified VFX does,
   where its other routines are VFX as released (the assembly gets it from cmake/editions.cmake) */
#define MW2_VFX3D_ALIGNED_FILL
/* JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX */
#define MW2_JOYSTICK_OPEN_POSEX
/* The 16-bit pixels and the palette: RGB555 */
#define MW2_PIXEL_FORMAT 555
/* ProjectPolygon clips to the near plane itself, as 1.1's QueueFace does, not through the renderer */
#define MW2_PROJECT_NEAR_CLIP
/* The sky and ground drawers clip the polygon to the pane themselves: their callers draw it even
   when ProjectPolygon leaves no points */
#define MW2_SKY_GROUND_CLIPPED_HELPERS
/* The "Filter" registry setting, passed to the renderer */
#define MW2_TEXTURE_FILTER
/* The graphics page lists the texture filter, third */
#define MW2_TEXTURE_FILTER_ITEM
/* Message boxes' backgrounds are shape 0x4f, not 0x4c */
#define MW2_MESSAGE_BACKGROUND_SHAPES
/* The torso pan's range is -90 to 90 */
#define MW2_NARROW_TORSO_PAN
/* The throttle's range is 0 to 1024 */
#define MW2_NO_REVERSE_THROTTLE
/* The map layouts hold their gauge functions from the start, not indices in g_cockpitGauges */
#define MW2_LAYOUT_GAUGE_FUNCTIONS
/* Artefact: the edition's headers define FixedMul16, FixedDiv16, MulDiv64, HashName and DebugBreakpoint
   __inline (its /Ox expands them in every caller), and MemCopy and MemSet are C */
#define MW2_INLINE_HELPERS
/* Artefact: the edition's source declares some of UpdateMech's, LateUpdateMech's, DodgeShot's and the HUD
   brackets' locals in the blocks that use them, where VC++ 2.2 and 4.1 let them share the frame (1.1's and
   the Matrox edition's declare them at the top) */
#define MW2_BLOCK_SCOPED_LOCALS
/* Artefact: LateUpdateMech's torso recentring records the heading without testing isLocal again (it
   holds there) */
#define MW2_RECENTER_NO_RETEST
/* UpdateMech's jump jets add their thrust to accelX before the drag (the later editions add the drag
   and the thrust first, as for accelZ) */
#define MW2_JUMP_JET_THRUST_FIRST
/* Artefact: DodgeShot picks the side before it divides the heading */
#define MW2_DODGE_SIDE_FIRST
/* Artefact: MoveMechWithCollisions sums the bounce's dot product in three statements and negates the
   speed before it quarters it (2.2 and 4.1 then keep each term's doubling and fold the sign) */
#define MW2_BOUNCE_DOT_STATEMENTS
/* Artefact: the edition's source reads an AI target id's two bytes themselves
   (missionobjective.h's OBJECTIVE_TARGET_KIND), where 1.1's and the Matrox edition's shift the word */
#define MW2_OBJECTIVE_TARGET_BYTES
/* The polygon drawers clip to the pane's edges themselves (FUN_10067b90), and the outline is closed */
#define MW2_CLIP_TO_PANE_EDGES
/* The textures are the animations' frames as they are, 16-bit texels (FUN_10051dfe), and their
   average colors are computed from them */
#define MW2_FRAME_TEXELS
/* VESA480.DLL is 512x384 when DirectDraw has the mode in 16 bits (FUN_100059b0), 640x480 otherwise */
#define MW2_RES_512X384
/* ChooseArtResolution takes art resolution 1 for a 512x384 window */
#define MW2_ART_RESOLUTION_512X384
/* Key 0x3e asks for power down in power state 7 too, where 1.1 ignores it */
#define MW2_NO_STATE_7_POWER_TEST
/* DdrawPaletteFade computes the fade's steps into g_paletteColors without setting DirectDraw's
   palette */
#define MW2_FADE_STEPS_ONLY
/* Artefact: RestoreBrightness passes the brightness to PreviewBrightness through its store */
#define MW2_BRIGHTNESS_STORE_CHAINED
/* The DirectDraw back end's palette writes record the colors and compute the entries, but set no
   DirectDraw palette (the display is 16-bit) */
#define MW2_DDRAW_PALETTE_UNSET
/* DdrawInit sets a 16-bit display mode */
#define MW2_DDRAW_MODE_16BIT
/* DdrawInit clears the frame with DdrawFill(0, 0, width, height), not to width - 1 and height - 1 */
#define MW2_DDRAW_FILL_SIZE
/* Artefact: DdrawInit's DDCAPS is DirectX 1's, 0xac bytes (the edition was built with DirectX 1's headers) */
#define MW2_DDRAW_CAPS_DX1
/* DdrawCreateSurfaces sets no DDSD_CAPS in its surface descriptions */
#define MW2_DDRAW_NO_CAPS_FLAG
/* DdrawCreateSurfaces makes the primary surface of the larger modes only the second way: with one back
   buffer when it flips, without video memory in refresh mode 3, and not at all in mode 2 */
#define MW2_DDRAW_FALLBACK_SURFACES_ONLY
/* Artefact: the edition's source writes some flags and choices as expressions (`m_up = angle <= 180`,
   `x = c ? a : b`) where 1.1's and the Matrox edition's /Od code shows if and else (GetRectNeedleAt,
   HandleDebugKey, InitializeManeuvers' skill flags, ReelMotionTask's jump flag) */
#define MW2_CONDITIONAL_EXPRESSIONS
/* HideCacheInfo blanks its line without sprintf's two stray arguments */
#define MW2_HIDE_CACHE_INFO_NO_ARGS
/* FindSession and StopExternalIO return 0 where the others fall off the end */
#define MW2_NET_RETURN_ZERO
/* NetSend and NetReceive log nothing */
#define MW2_NO_NET_DEBUG_PRINTS
/* Artefact: LoadCachedResource formats both of its messages in one buffer */
#define MW2_ONE_RESOURCE_MESSAGE
/* Artefact: RunAIWeapons scales the torso tilt by -0xf00, where the others negate the product */
#define MW2_TILT_NEGATIVE_FACTOR
/* No hit feedback for the local player while its flag 2 is set */
#define MW2_HIT_FEEDBACK_FLAG_CHECK
/* LoadMechConfig logs the armor and the jump thrust as %f */
#define MW2_MEK_LOG_FLOATS
/* The debug log prints the sky's and the ground's colours on one line */
#define MW2_DEBUG_LOG_FORMATS
/* Artefact: UpdateShot assigns the ground's height inside its comparison */
#define MW2_GROUND_HEIGHT_IN_TEST
#endif

#ifdef MW2_S3
/* QueueFace copies the face's normal into locals for its backface test, summing the z term first, keeps
   the z in depth, sums the average depth in dot, counts the depth loops down and or-s the vertices' clip
   codes into a signed byte */
#define MW2_QUEUEFACE_REUSED_LOCALS
/* QueueFace takes the range of the face's texture coordinates, and the polygon's flags and outcodes from it */
#define MW2_QUEUEFACE_TEXTURE_RANGE
/* ProjectPolygon leaves no points, at once, when every vertex is nearer than the near plane */
#define MW2_PROJECT_ALL_NEAR_EMPTY
/* ProjectVertex projects y after it tests x against the view's sides */
#define MW2_PROJECT_Y_AFTER_X
/* InitSlopeTables computes each slope once, for its cosine and its sine */
#define MW2_SLOPE_ONCE
/* HorizonYAtX keeps the horizon's y a float, converting it to an integer only for the return */
#define MW2_HORIZON_FLOAT_Y
/* The S3 edition starts with the ATI edition's features but its back end's, until its binary
   settles each one. */
/* JoystickOpenDevice's probe sets its JOYINFOEX's size without clearing it */
#define MW2_JOYSTICK_PROBE_SIZE_ONLY
/* SimMain reads the sky and ground settings (LoadSkyGroundSettings) just before FirstClock */
#define MW2_SKY_SETTINGS_BEFORE_CLOCK
/* SimMain reads the render settings (FUN_1008fad0) once the refresh mode is set */
#define MW2_RENDER_SETTINGS_AFTER_MODES
/* DrawTriangle, DrawQuad and DrawPentagon draw through the palette polygon (FUN_1000d960), clipped to
   the pane's four edges, not g_renderSettings' drawer */
#define MW2_SHAPES_PALETTE_POLYGON
/* SimMain releases the DC it tests for StartSupAnim */
#define MW2_SUPANIM_DC_RELEASED
/* The 16-bit pixels and the palette: RGB555 */
#define MW2_PIXEL_FORMAT 555
/* ProjectPolygon clips to the near plane itself, as 1.1's QueueFace does, not through the renderer */
#define MW2_PROJECT_NEAR_CLIP
/* The sky and ground drawers clip the polygon to the pane themselves: their callers draw it even
   when ProjectPolygon leaves no points */
#define MW2_SKY_GROUND_CLIPPED_HELPERS
/* Message boxes' backgrounds are shape 0x4f, not 0x4c */
#define MW2_MESSAGE_BACKGROUND_SHAPES
/* The torso pan's range is -90 to 90 */
#define MW2_NARROW_TORSO_PAN
/* The throttle's range is 0 to 1024 */
#define MW2_NO_REVERSE_THROTTLE
/* The map layouts hold their gauge functions from the start, not indices in g_cockpitGauges */
#define MW2_LAYOUT_GAUGE_FUNCTIONS
/* Artefact: the edition's headers define FixedMul16, FixedDiv16, MulDiv64, HashName and DebugBreakpoint
   __inline (its /Ox expands them in every caller), and MemCopy and MemSet are C */
#define MW2_INLINE_HELPERS
/* Artefact: the edition's source declares some of UpdateMech's, LateUpdateMech's, DodgeShot's and the HUD
   brackets' locals in the blocks that use them, where VC++ 2.2 and 4.1 let them share the frame (1.1's and
   the Matrox edition's declare them at the top) */
#define MW2_BLOCK_SCOPED_LOCALS
/* UpdateMech's air drag divides by the length once, then scales both axes by it */
#define MW2_DRAG_DIVIDED_ONCE
/* Artefact: DodgeShot picks the side before it divides the heading */
#define MW2_DODGE_SIDE_FIRST
/* Artefact: MoveMechWithCollisions sums the bounce's dot product in three statements and negates the
   speed before it quarters it (2.2 and 4.1 then keep each term's doubling and fold the sign) */
#define MW2_BOUNCE_DOT_STATEMENTS
/* Artefact: the edition's source reads an AI target id's two bytes themselves
   (missionobjective.h's OBJECTIVE_TARGET_KIND), where 1.1's and the Matrox edition's shift the word */
#define MW2_OBJECTIVE_TARGET_BYTES
/* The polygon drawers clip to the pane's edges themselves (FUN_10067b90), and the outline is closed */
#define MW2_CLIP_TO_PANE_EDGES
/* The textures are the animations' frames as they are, 16-bit texels (FUN_10051dfe), and their
   average colors are computed from them */
#define MW2_FRAME_TEXELS
/* The "Filter" registry setting, passed to the renderer */
#define MW2_TEXTURE_FILTER
/* The graphics page lists the texture filter, third */
#define MW2_TEXTURE_FILTER_ITEM
/* LoadClassEntryShape returns early when the entry is loaded at the level, in the test of a released
   entry (||) */
#define MW2_SKIP_LOADED_LEVEL_WITH_RELEASED
/* Artefact: ApplyCollisionDamage's damage times the masses' ratio isn't grouped (the edition divides the
   masses first) */
#define MW2_COLLISION_DAMAGE_UNGROUPED
/* The grid object is never shown: SetGridObject clears its set and shown flags once it has hidden it,
   and ShowGridObject only hides it */
#define MW2_GRID_OBJECT_HIDDEN
/* VFX3D and VFXREND, VFX's polygon fillers and renderer, aren't linked: the A3D renderer draws every
   polygon, and FirstRender has no code block to make writable */
#define MW2_NO_VFXREND
/* ChooseArtResolution takes the first size whose width reaches the window's, and records the last
   before it that fits inside the window */
#define MW2_ART_RESOLUTION_FIT
/* DdrawPaletteFade computes the fade's steps into g_paletteColors without setting DirectDraw's
   palette */
#define MW2_FADE_STEPS_ONLY
/* Artefact: RestoreBrightness passes the brightness to PreviewBrightness through its store */
#define MW2_BRIGHTNESS_STORE_CHAINED
/* The DirectDraw back end's palette writes record the colors and compute the entries, but set no
   DirectDraw palette (the display is 16-bit) */
#define MW2_DDRAW_PALETTE_UNSET
/* DdrawBlit's BltFast doesn't wait for the blitter (DDBLTFAST_NOCOLORKEY, not DDBLTFAST_WAIT) */
#define MW2_BLIT_NO_WAIT
/* DdrawInit sets a 16-bit display mode */
#define MW2_DDRAW_MODE_16BIT
/* DdrawInit clears the frame with DdrawFill(0, 0, width, height), not to width - 1 and height - 1 */
#define MW2_DDRAW_FILL_SIZE
/* ResetTicks restores ebx and ecx in the order it saved them (ticks.asm), where the others swap them */
#define MW2_RESET_TICKS_SAVE_ORDER
/* Artefact: the edition's source writes some flags and choices as expressions (`m_up = angle <= 180`,
   `x = c ? a : b`) where 1.1's and the Matrox edition's /Od code shows if and else (GetRectNeedleAt,
   HandleDebugKey, InitializeManeuvers' skill flags, ReelMotionTask's jump flag) */
#define MW2_CONDITIONAL_EXPRESSIONS
/* HideCacheInfo blanks its line without sprintf's two stray arguments */
#define MW2_HIDE_CACHE_INFO_NO_ARGS
/* An animation frame's average color is its texels' mean components times 8, less 12 (FUN_100519b0) */
#define MW2_DARKER_AVERAGE_COLORS
/* The satellite view clips its polygons and bands to all four pane edges (SatelliteDrawPolygon) */
#define MW2_SATELLITE_CLIP_ALL_EDGES
/* The wireframe outline clears flag bits 12, 14 and 15 */
#define MW2_OUTLINE_CLEARS_FLAGS
/* DrawQueuedPolygon flushes the renderer (FUN_10069591) between a wireframe polygon's fill and its
   outline */
#define MW2_OUTLINE_FLUSH
/* DrawQueuedPolygon passes 0xff for a filled polygon's edges, not 0xf */
#define MW2_QUEUED_POLYGON_EDGES_0XFF
/* DrawScenePolygon's lit polygons (0x1000) set only bit 3 of their color's shade (| 8), not all four
   (| 0xf): the middle of the color's ramp, which the light then scales */
#define MW2_LIT_POLYGON_SHADE_BIT3
/* FindSession and StopExternalIO return 0 where the others fall off the end */
#define MW2_NET_RETURN_ZERO
/* NetSend and NetReceive log nothing */
#define MW2_NO_NET_DEBUG_PRINTS
/* Artefact: LoadCachedResource formats both of its messages in one buffer */
#define MW2_ONE_RESOURCE_MESSAGE
/* Artefact: RunAIWeapons scales the torso tilt by -0xf00, where the others negate the product */
#define MW2_TILT_NEGATIVE_FACTOR
/* No hit feedback for the local player while its flag 2 is set */
#define MW2_HIT_FEEDBACK_FLAG_CHECK
/* LoadMechConfig logs the armor and the jump thrust as %f */
#define MW2_MEK_LOG_FLOATS
/* A weapon recycles at once while bit 1 of the unlimited ammo setting is set (the "flag" cheat's 3) */
#define MW2_CHEAT_RECYCLE
/* A held weapon is released at once while bit 1 of the unlimited ammo setting is set */
#define MW2_CHEAT_RECYCLE_HELD
/* Artefact: UpdateShot assigns the ground's height inside its comparison */
#define MW2_GROUND_HEIGHT_IN_TEST
/* FirstEnvironment starts the day at a phase's time for phases 1 to 3 only (0 keeps the time it has) */
#define MW2_TIME_OF_DAY_PHASES_1_TO_3
/* SimMain fills the frame with the ground's colour lit by the renderer's light (g_unk0x10184700) once
   the go state has bit 2, textured ground or not */
#define MW2_LIT_GROUND_FILL
/* SimMain releases the DC it tests the palette through */
#define MW2_PALETTE_DC_RELEASED
/* SimMain doesn't call A3D_shutdown after its last Blit */
#define MW2_NO_SIMMAIN_A3D_SHUTDOWN
/* The "flag" cheat ("Flag's Invincible!"): invulnerable, unlimited ammo, no heat, jets and their fuel */
#define MW2_FLAG_CHEAT
/* Key 0x3d is key 0x3e's case: neither asks for power down in power state 7 */
#define MW2_POWER_KEYS_ONE_CASE
#endif

#ifdef MW2_POWERVR
/* InitSlopeTables computes each slope once, for its cosine and its sine */
#define MW2_SLOPE_ONCE
/* The PowerVR edition starts with the ATI edition's features but its back end's, until its binary
   settles each one. */
/* The 16-bit pixels and the palette: RGB555 */
#define MW2_PIXEL_FORMAT 555
/* ProjectPolygon clips to the near plane itself, as 1.1's QueueFace does, not through the renderer */
#define MW2_PROJECT_NEAR_CLIP
/* The sky and ground drawers clip the polygon to the pane themselves: their callers draw it even
   when ProjectPolygon leaves no points */
#define MW2_SKY_GROUND_CLIPPED_HELPERS
/* VFX3D's VFX_flat_polygon fills its spans with the destination aligned, as 1.1's modified VFX does,
   where its other routines are VFX as released (the assembly gets it from cmake/editions.cmake) */
#define MW2_VFX3D_ALIGNED_FILL
/* Message boxes' backgrounds are shape 0x4f, not 0x4c */
#define MW2_MESSAGE_BACKGROUND_SHAPES
/* The torso pan's range is -90 to 90 */
#define MW2_NARROW_TORSO_PAN
/* The throttle's range is 0 to 1024 */
#define MW2_NO_REVERSE_THROTTLE
/* The map layouts hold their gauge functions from the start, not indices in g_cockpitGauges */
#define MW2_LAYOUT_GAUGE_FUNCTIONS
/* Artefact: the edition's headers define FixedMul16, FixedDiv16, MulDiv64, HashName and DebugBreakpoint
   __inline (its /Ox expands them in every caller), and MemCopy and MemSet are C */
#define MW2_INLINE_HELPERS
/* Artefact: the edition's source declares some of UpdateMech's, LateUpdateMech's, DodgeShot's and the HUD
   brackets' locals in the blocks that use them, where VC++ 2.2 and 4.1 let them share the frame (1.1's and
   the Matrox edition's declare them at the top) */
#define MW2_BLOCK_SCOPED_LOCALS
/* UpdateMech's air drag divides by the length once, then scales both axes by it */
#define MW2_DRAG_DIVIDED_ONCE
/* Artefact: DodgeShot picks the side before it divides the heading */
#define MW2_DODGE_SIDE_FIRST
/* Artefact: MoveMechWithCollisions sums the bounce's dot product in three statements and negates the
   speed before it quarters it (2.2 and 4.1 then keep each term's doubling and fold the sign) */
#define MW2_BOUNCE_DOT_STATEMENTS
/* Artefact: the edition's source reads an AI target id's two bytes themselves
   (missionobjective.h's OBJECTIVE_TARGET_KIND), where 1.1's and the Matrox edition's shift the word */
#define MW2_OBJECTIVE_TARGET_BYTES
/* The polygon drawers clip to the pane's edges themselves (FUN_10067b90), and the outline is closed */
#define MW2_CLIP_TO_PANE_EDGES
/* The textures are the animations' frames as they are, 16-bit texels (FUN_10051dfe), and their
   average colors are computed from them */
#define MW2_FRAME_TEXELS
/* The game starts with no draw mode (-1, as in 1.1), though without 1.1's fallback to the others */
#define MW2_NO_INITIAL_DRAW_MODE
/* ChooseArtResolution takes art resolution 1 for a 512x384 window */
#define MW2_ART_RESOLUTION_512X384
/* The time of day's phase is a function of the time (FUN_10006f10), which also gives SimMain the
   palette of the current one (FUN_10006f30) */
#define MW2_TIME_OF_DAY_LOOKUP
/* A palette fade sets the renderer's light to the fade's tint over 32 (FUN_100607d0), white when it
   has none */
#define MW2_FADE_LIGHT_COLOUR
/* FirstResource frees the MW2.PRJ path it duplicated, on the game's heap */
#define MW2_PRJ_PATH_FREED
/* The HUD's target marker keeps where it drew (statics): each frame draws the one kept from the
   frame before, then moves it, drawing the new one only when none was kept */
#define MW2_HUD_MARKER_LAG
/* The target panel writes the target's range on its bottom line, not on the line after its name */
#define MW2_TARGET_RANGE_AT_BOTTOM
/* GetMatrixAngles' test for a steep pitch is inverted (the arcsine unless the pitch's sine is both above
   0.1 and below -0.1), so the pitch always comes from the arcsine */
#define MW2_MATRIX_PITCH_ASIN_ONLY
/* The "chris" and "flag" cheats */
#define MW2_CHRIS_FLAG_CHEATS
/* Cheat codes that tune the renderer: the fog's density, the ground's and the sky's tiles and the sky's
   scrolling speed */
#define MW2_TUNING_CHEATS
/* The face tests of the shape collision (TestPointUnderFloor, TestPointAboveFaces, TestRayFaces)
   transform the shape's model themselves when it is older than the shape's transform: the renderer
   (SGL) doesn't transform the models the depth sort did */
#define MW2_COLLISION_TRANSFORMS_MODELS
/* Key 0x3d is key 0x3e's case: neither asks for power down in power state 7 */
#define MW2_POWER_KEYS_ONE_CASE
/* Key 0x5b takes no screenshot */
#define MW2_NO_SCREENSHOT_KEY
/* Key 0xa7 goes to RunGameKey's default case, where the others switch the wireframe view on and off
   with it */
#define MW2_NO_WIREFRAME_KEY
/* The target panel renders no view of its target (nor loads its detail level): it only wipes the pane,
   unless the cockpit's power state is 2, and outlines it */
#define MW2_RENDERER_TARGET_VIEW
/* SpawnEffectEx leaves an effect's object unrotated (it takes the rotation all the same), and moves one
   that hit a thing toward the eye by its shape's radius */
#define MW2_UNROTATED_EFFECTS
/* Artefact: RestoreBrightness passes the brightness to PreviewBrightness through its store */
#define MW2_BRIGHTNESS_STORE_CHAINED
/* The DirectDraw back end's palette writes record the colors and compute the entries, but set no
   DirectDraw palette (the display is 16-bit) */
#define MW2_DDRAW_PALETTE_UNSET
/* ApplyPaletteResource writes the lookaside table's pixels with red shifted by 10, as for RGB565 */
#define MW2_LOOKASIDE_RED_SHIFT_10
/* The renderer takes no palette: ApplyPaletteResource doesn't pass it to A3D_SetPalette */
#define MW2_A3D_NO_PALETTE
/* ResetTicks restores ebx and ecx in the order it saved them (ticks.asm), where the others swap them */
#define MW2_RESET_TICKS_SAVE_ORDER
/* HashName rotates the whole 32-bit hash after each character, not its low word */
#define MW2_HASH_NAME_ROL32
/* Artefact: the edition's source writes some flags and choices as expressions (`m_up = angle <= 180`,
   `x = c ? a : b`) where 1.1's and the Matrox edition's /Od code shows if and else (GetRectNeedleAt,
   HandleDebugKey, InitializeManeuvers' skill flags, ReelMotionTask's jump flag) */
#define MW2_CONDITIONAL_EXPRESSIONS
/* HideCacheInfo blanks its line without sprintf's two stray arguments */
#define MW2_HIDE_CACHE_INFO_NO_ARGS
/* The damaged map view flickers its gauge without drawing the view's damage animation
   (DrawDamagedMapView) */
#define MW2_NO_MAP_DAMAGE_ANIM
/* FindSession and StopExternalIO return 0 where the others fall off the end */
#define MW2_NET_RETURN_ZERO
/* NetSend and NetReceive log nothing */
#define MW2_NO_NET_DEBUG_PRINTS
/* Artefact: LoadCachedResource formats both of its messages in one buffer */
#define MW2_ONE_RESOURCE_MESSAGE
/* Artefact: RunAIWeapons scales the torso tilt by -0xf00, where the others negate the product */
#define MW2_TILT_NEGATIVE_FACTOR
/* No hit feedback for the local player while its flag 2 is set */
#define MW2_HIT_FEEDBACK_FLAG_CHECK
/* LoadMechConfig logs the armor and the jump thrust as %f */
#define MW2_MEK_LOG_FLOATS
/* A weapon recycles at once while bit 1 of the unlimited ammo setting is set (the "flag" cheat's 3) */
#define MW2_CHEAT_RECYCLE
/* HandleMessages stops skipping mouse messages when its PeekMessage finds nothing, and handles the
   last one it had */
#define MW2_MOUSE_PEEK_BREAKS
#endif

#ifdef MW2_GBL
/* AmsLaunch tests the weapon's ammunition bin before it takes a shot from it */
#define MW2_AMS_CHECKS_BIN
/* Mechs take heat from outside (Mech::m_externalHeat, which the radius damage adds to and
   CalculateHeat adds to the frame's heat) */
#define MW2_EXTERNAL_HEAT
/* Mechs have a second word of flags (Mech::m_unk0x112, whose bit 0 UpdateShot sets for a NARC hit) */
#define MW2_MECH_SECOND_FLAGS
/* Mechs have 14 weapon slots, not 10 (WEAPON_SLOT_COUNT) */
#define MW2_FOURTEEN_WEAPON_SLOTS
/* An AI player whose gunnery is above 6 takes its skills from a table of pilots, with two more
   values and a flag the AI's fire decisions test (Player::m_unk0x160, m_unk0x1b0, m_unk0x1b4) */
#define MW2_PILOT_TABLE
/* The maneuver tables are data, five of them the pilot table's, and each player has its table and a
   range (Player::m_maneuverTable, m_unk0x1b0: InitializeManeuvers) */
#define MW2_STATIC_MANEUVER_TABLES
/* Representation: a player's AI flags are an int, not a short (PlayerAi::m_flags) */
#define MW2_AI_FLAGS_INT
/* The local star starts at 0, and the player's formation goes to it */
#define MW2_LOCAL_STAR_FROM_ZERO
/* The CHAT_MODE game key, action 66: the later actions are one higher (GAME_KEY_ACTION) */
#define MW2_CHAT_KEY
/* The TOGGLE_AMS game key, action 169 */
#define MW2_AMS_KEY
/* The message boxes and the refresh mode errors name Ghost Bear's Legacy */
#define MW2_GHOST_BEAR_TITLES
/* The game's CD is the drive with GBL_HERC.DRV */
#define MW2_GBL_CD_DRIVER
/* Representation: GetEngagementAIFlags returns an int, not a short */
#define MW2_ENGAGEMENT_FLAGS_INT
/* VFX as released (unaligned fills and runs, word-wise character copies), where 1.1 modified it; the
   assembly gets it from cmake/editions.cmake */
#define MW2_VFX_STOCK
/* VFXA's VFX_lookaside_write, which sets one lookaside entry (cmake/editions.cmake) */
#define MW2_VFX_LOOKASIDE_WRITE
/* VFX3D's VFX_flat_polygon fills its spans with the destination aligned, as 1.1's modified VFX does,
   where its other routines are VFX as released (the assembly gets it from cmake/editions.cmake) */
#define MW2_VFX3D_ALIGNED_FILL
/* A recentering torso snaps to the legs within 1 degree, not 4 (TORSO_SNAP_RANGE) */
#define MW2_TORSO_SNAP_ONE_DEGREE
/* Weapons work through their definitions: each has a launch function (SpawnShot for all but the
   AMS), a function called each frame, flags (the launch sounds, an animation) and an animation, and
   there are 35 of them; shots and effects carry the weapon's flags and a word more, and there are 255
   of each and 36 effect types, with four more words each (WeaponDef, Shot, Effect, EffectInfo) */
#define MW2_WEAPON_FUNCTIONS
/* PlayPositionalSound takes the sample's flags (PlayPositionalSoundEx; the old entry point passes
   0x32), and plays nothing beyond a cutoff distance, g_unk0x100ba608 hundred units, when that is set */
#define MW2_POSITIONAL_SOUND_FLAGS
/* A game thing's destruction counts for the local team whoever's shot destroyed it, and forgets the
   shooter (KillGameThing) */
#define MW2_THING_KILLS_FORGET_SHOOTER
/* Artefact: weapons.c, ai.c, maneuvers.c and gridobject.c call the C runtime's abs, which GBL's
   other units expand (MW2_GBL's pragma above), and bandpoly.c, AssignStarTarget and
   ChooseArtResolution its labs */
#define MW2_ABS_CALLS
/* Artefact: GetBearingAndRange compares with the C runtime's labs */
#define MW2_BEARING_LABS
/* A shot destroys a game thing whose shape is of kind 0xb0 at once, and every destroyed thing leaves
   effect 0x20b (DamageGameThing) */
#define MW2_BUILDINGS_FALL_AT_ONCE
/* A destroyed mech blows the critical slot of its first ammunition bin, a bin blows whatever its
   weapon's ammunition, and its pilot ejects when auto-eject is on or the mech isn't the local
   player's (DestroyMech, DestroyCriticalSlot) */
#define MW2_DESTROY_BLOWS_FIRST_BIN
/* EjectPlayer tells the local player with cockpit message 4 when the mech goes without ejecting */
#define MW2_SELF_DESTRUCT_MESSAGE
/* A mech's kill counts for the player whose shot hit it last, the local player's outcome is 4 unless
   they ejected, and in a network game only the local mech's death is played out (KillMech, which
   DestroySection and DestroyCriticalSlot call whatever the game) */
#define MW2_KILLS_BY_LAST_SHOOTER
/* A critical hit on a flamer and on the AMS has a message of its own (DestroyCriticalSlot) */
#define MW2_MORE_CRITICAL_HITS
/* The AI's attack and target states stop the group fire as well as the fire (AiStateAttack,
   AiStateMove) */
#define MW2_AI_STOPS_GROUP_FIRE
/* Artefact: GBL's source copies some structures with memcpy, which its build calls, where the
   optimized editions show a structure assignment: nav points, rays, players, matrices, weapon slots,
   render settings, sample headers and session descriptions (RemoveNavPoint, CopyRay,
   LogPlayerStatusLines, the object matrices, LayoutWeaponPanels, the mech view panel, StartSample,
   FindSession) */
#define MW2_STRUCT_COPY_MEMCPY
/* Only the local player inspects its target, which counts as inspected once anyone has, and the
   inspect request stays set (UpdateTarget) */
#define MW2_LOCAL_INSPECTION
/* DdrawCreateSurfaces sets no DDSD_CAPS in its surface descriptions */
#define MW2_DDRAW_NO_CAPS_FLAG
/* The cheat codes are a table with the messages each shows, all stored XORed with 0xa, the messages
   decoded at the first key typed; GBL has codes of its own, and six of them do nothing
   (g_cheatCodes, HandleCheatInput) */
#define MW2_CHEAT_TABLE
/* GetIniValue cuts a value at a ';' and trims it, unless its second argument asks for the raw value
   (which error.c's call doesn't pass) */
#define MW2_INI_COMMENTS
/* ReportInputDeviceError reports nothing about the mouse */
#define MW2_NO_MOUSE_ERRORS
/* Artefact: DrawDepthQueue takes the next entry's index into a local before it copies the entry */
#define MW2_DEPTH_QUEUE_INDEX
/* Artefact: the vector lengths sum the squares from z down (FloatVectorLength, ScaleVectorToLength,
   BuildMatrixFromDirection) */
#define MW2_LENGTH_SUM_FROM_Z
/* QueueObjTree draws no shape of type 0x20 */
#define MW2_SKIP_SHAPE_TYPE_20
/* The ALL_PT_MENU, PT_2_MENU and PT_3_MENU game keys (actions 53 to 55) do what the first three
   starmate menu keys do (RunGameKey) */
#define MW2_PT_MENU_KEYS
/* Artefact: RunGameKey's screenshot acquires the frame buffer in an if with an empty else, where 1.1
   has a conditional expression */
#define MW2_SCREENSHOT_ACQUIRE_IF
/* Key 0x3e asks for power down in power state 7 too, where 1.1 ignores it */
#define MW2_NO_STATE_7_POWER_TEST
/* UpdateInputs compares an axis' rate pointer with its position's address */
#define MW2_AXIS_RATE_POINTER_TEST
/* Restarting a star mission keeps its objectives' targets' listed flags */
#define MW2_RESTART_KEEPS_LISTED
/* IsTeamNearTarget tests only the team's leader */
#define MW2_LEADER_NEAR_TARGET
/* The local star's mission fails once the local mech is lost (UpdateObjectives, g_localMechLost) */
#define MW2_LOST_MECH_FAILS_STAR
/* UpdateObjective takes star N as player N in network games */
#define MW2_NETWORK_STAR_PLAYERS
/* Artefact: FindIpxProviderCallback and FillJoystickDeviceInfo compare names with strcmp, not memcmp */
#define MW2_NAME_STRCMP
/* MemCopy and MemSet return the C runtime's memcpy and memset, where 1.1's are assembly */
#define MW2_MEM_HELPERS_CALL_CRT
/* A scrounge record whose id names no class takes a static object's shape, loading the object
   (BwdExecuteStream) */
#define MW2_SCROUNGE_STATIC_OBJECTS
/* A player the stream creates for another machine or the AI gets AI mode 1 in a network game and 2
   otherwise (BwdExecuteStream) */
#define MW2_REMOTE_PLAYER_AI_MODE
/* A mech's heat sinks cool five times as well at -100 degrees or colder, on top of the cold bonus
   (LoadMechConfig) */
#define MW2_DEEP_COLD_COOLING
/* SimMain updates the players every frame, before a network launch too */
#define MW2_PLAYERS_BEFORE_LAUNCH
/* StartSupAnim leaves the palette's first 16 entries as the backdrop set them */
#define MW2_SUPANIM_KEEPS_PALETTE
/* Artefact: InitSoundInfo fills the table with -1, not 0xff (which GBL's memset call shows) */
#define MW2_SOUND_INFO_FILL_MINUS_ONE
/* DdrawStop releases DirectDraw without returning the window to normal cooperation or restoring the
   display mode */
#define MW2_DDRAW_STOP_KEEPS_MODE
/* Artefact: DdrawInit's DDCAPS is DirectX 1's, 0xac bytes (the edition was built with DirectX 1's headers) */
#define MW2_DDRAW_CAPS_DX1
/* In a network game, UpdateMech leaves a player with flag 1 (another machine's) where it is unless it
   is the local one, and holds a truck at its object's height */
#define MW2_REMOTE_MECHS_STAY
#endif

#ifdef MW2_PREMERCS
/* The early Mercenaries build starts with every feature of Ghost Bear's Legacy, whose code it is: each
   stays until its code shows otherwise */
/* JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX (GBL's later build
   probes as 1.1 does) */
#define MW2_JOYSTICK_OPEN_POSEX
/* The scene's shapes are filed in a grid of 12 by 12 cells, 25000 units each, around the origin, each shape
   keeping its cell (Shape::m_gridX, m_gridZ; shapelists.c) */
#define MW2_SHAPE_GRID
/* Players have 0x46 bytes more than GBL's, in three places (Player::m_navTargetInfo, m_unk0x184,
   m_unk0x1a0 and m_unk0x1a4) */
#define MW2_PLAYER_UNK_MEMBERS
/* Mechs, their weapon slots and their ammunition bins have 0x8f, 0xc and 8 bytes more than GBL's (Mech::m_unk0x114,
   WeaponSlot::m_unk0x64 and m_unk0x6c, AmmoBin::m_unk0x14 and WeaponBin::m_unk0x14) */
#define MW2_MECH_UNK_MEMBERS
/* Cockpit panels are 0xf0 bytes, with a member and a setter of their own first (CockpitPanel::m_unk0x30,
   m_unk0xc4) */
#define MW2_COCKPIT_PANEL_UNK_MEMBERS
/* The cockpit has 34 panels, not 30, laid out anew: four more at the end, and the mech view's transition on
   five more of them (c_panelCount; g_cockpitPanelPanes) */
#define MW2_THIRTY_FOUR_PANELS
/* Outside network games the cockpit's map views run with the radar off too (DifficultyCfg::m_radar:
   ShutdownCockpitPanels, UpdateCockpit) */
#define MW2_MAP_VIEW_WITHOUT_RADAR
/* While the HUD is hidden (g_showHud), UpdateCockpit doesn't draw the network panel either */
#define MW2_HIDDEN_HUD_HIDES_NETWORK_PANEL
/* 21 more game keys: the MFDs' (MFD_CYCLE_A to _F, MFDS_RESET, SAVE_HUD, TOGGLE_COMPASS, TOGGLE_WEAPONS), the
   starmates' orders (STARMATE_SET_ALL to STARMATE_SHUTDOWN), two more weapon groups and FIRE_CYCLE_WEAPON_GROUP,
   which renumber the others (GAME_KEY_ACTION; g_gameKeyNames) */
#define MW2_MFD_GAME_KEYS
/* A network game keeps the difficulty file's heat tracking, ammunition and damage settings (LoadDifficultyCfg
   doesn't force them) */
#define MW2_NETWORK_KEEPS_DAMAGE_RULES
/* SatelliteDrawPolygon takes a polygon's kind from bits 12-15 of its flags, not 12-14, as the 3D editions do */
#define MW2_SATELLITE_KIND_FOUR_BITS
/* ReloadPlayerMech presses the torso's reset buttons and clears the mech's NARC mark (Mech::m_unk0x112), and copies
   its fourth argument over its third */
#define MW2_RELOAD_RESETS_TORSO
/* A player keeps a second target, the nav point TargetNavPoint last targeted and CycleTarget chose
   (Player::m_navTargetInfo), which the local player's autopilot steers for (GetTargetBearing), and retargeting hands
   the steering to the autopilot only while it has none (CycleTarget, TargetAtReticle) */
#define MW2_NAV_TARGET_KEPT
/* A player with Player::m_unk0x1b4 1 targets a mech at its feet: its object's origin less its height
   (TargetGamePiece) */
#define MW2_AIM_AT_FEET
/* AssignStarObjective queues the members' state with the objective's target (the leader they follow), not 2 */
#define MW2_STAR_QUEUES_TARGET
/* LeaveAIState releases the group fire button too (PlayerSteering::m_weaponFireGroup) */
#define MW2_LEAVE_STATE_STOPS_GROUP_FIRE
/* The mech view panel is seven MFDs, each showing one of thirteen modes (CockpitPanel::m_mfdMode) in a pane of
   its own (m_weapon): the damage outline, armor bars, rear, eye, overhead, satellite and tracked shot views, the
   target and the speed, jump jet, heat and heat rate panels; SAVE_HUD keeps their modes in MERCS.HUD
   (mechviewpanel.c) */
#define MW2_MFD_PANELS
/* The HUD has no altimeter (DrawAltimeter): the jump jet MFD shows the altitude */
#define MW2_NO_HUD_ALTIMETER
/* A box closes in on the local player's new target, turning as it shrinks and trailed by two more, before the
   brackets show (UpdateLockBox, hud.c); the HUD marks the nav target too (DrawNavTargetMarker) */
#define MW2_LOCK_BOXES
/* The software renderer textures the sky and the ground (skygnd.par: LoadSkyGroundSettings, DrawSkyQuad,
   DrawGroundQuad, as the accelerated editions do), clipping their quads to depths (ClipPolygonDepth) */
#define MW2_TILED_SKY
/* The gauge ellipse can be drawn and filled at a percentage of its pane (DrawScaledGaugeEllipse) */
#define MW2_SCALED_GAUGE_ELLIPSES
/* RememberLoadMech and RememberMechSegments don't log a player index out of range */
#define MW2_QUIET_MECH_RECORDS
/* An object turns about a point of its own (RotateObjAbout), and a part's objects can be destroyed with their
   children (DestroyPartTree, object.c) */
#define MW2_PART_TREES
/* A hit to the head of an AI player's mech shuts it down its first ten times (Mech::m_headHits), and a leg's
   first loss cripples an AI mech 95 times in 100 (DestroySection, DestroySectionEx) */
#define MW2_HEAD_SHUTDOWN
/* The lancemates acknowledge orders and report kills in voices of their own (g_lancemateVoices, SayLancemateAck,
   SayLancemateKill) */
#define MW2_LANCEMATE_VOICES
/* The local star starts at -1, as 1.1's does (g_localStar), though the stars count from 0 (MW2_LOCAL_STAR_FROM_ZERO) */
#define MW2_LOCAL_STAR_UNSET
/* The weapons are the Clans' (GBL's, renamed and retuned) and 31 of the Inner Sphere (WEAPON_DEF_COUNT 66,
   g_weaponDefs) */
#define MW2_CLAN_AND_IS_WEAPONS
/* SpawnShot fires with no ammunition left under the unlimited-ammunition cheat, and starts a tracer ahead of
   each machine-gun shot (premercs/tracers.c) */
#define MW2_SHOT_TRACERS
/* A shot's effect belongs to its shooter, as 1.1's does: DetonateShot takes no owner (UpdateShot) */
#define MW2_SHOT_OWNS_EFFECT
/* ResetEffectSlot leaves the effect's owner as it was */
#define MW2_RESET_EFFECT_KEEPS_OWNER
/* SetShapeState, GetShapeState and SetShapeKind don't test the shape for NULL */
#define MW2_SHAPE_ACCESSORS_UNCHECKED
/* RandomIntBelow and RandomNormal set their table's index back to 0 at 127, where the later builds take it modulo
   127 (RandomIntBelow2 and RandomNormal2 already do) */
#define MW2_RANDOM_INDEX_RESET
/* BWD streams have Mercenaries' campaign records, 15 node tags more (CPGN, PILT, TECH, NEWS…), which have no
   names (g_bwdTypeCodes, GetKeywordName) */
#define MW2_CAMPAIGN_BWD_TAGS
/* The pause sound is another sound of the PRJ's (PlayPauseSound) */
#define MW2_PAUSE_SOUND 0x149
/* FindIniSection and GetIniValue open MW2.INI by that name in capitals */
#define MW2_INI_UPPER_NAME
/* An error's text is the raw value of its code in the "SystemError" section, its comment kept (FormatErrorMessage) */
#define MW2_ERROR_TEXT_RAW
/* ScatterDebris throws its pieces without showing their objects */
#define MW2_DEBRIS_NOT_SHOWN
/* Players of a type 8 bank into their turns (FUN_10037d84 from UpdateAI) */
#define MW2_BANKING_PLAYER_TYPE
/* AimTorsoTilt takes the pitch it aims at, which its callers pass (the target's or another) */
#define MW2_TORSO_TILT_TAKES_PITCH
/* The class table's entries have 4 bytes more, before m_released (ClassEntry::m_unk0x42) */
#define MW2_CLASS_ENTRY_UNK_MEMBER
/* JoystickPoll neither clears its JOYINFOEX nor sets its size */
#define MW2_JOYINFOEX_UNCLEARED
/* FlushSpeechQueue sets g_speechLocked once it has flushed the queue, and not at all when the queue is empty */
#define MW2_SPEECH_LOCKED_AFTER_FLUSH
/* FadeToEndPalette applies the end palette without blending to it first */
#define MW2_FADE_TO_END_UNBLENDED
/* The eye position's overlay line is this many characters wide (HideEyePosition) */
#define MW2_EYE_POSITION_WIDTH 0x32
/* Blit draws the timed and debug overlays and advances the palette fade, which the later builds' SimMain does */
#define MW2_BLIT_DRAWS_OVERLAYS
/* The banner's palette blends in over this many steps (ShowBanner) */
#define MW2_BANNER_BLEND_STEPS 0x3c
/* FindIniSection's errors are 0x41 and 0x4a, the error codes numbered on from GBL's */
#define MW2_INI_ERROR_CODES
/* FindIniSection reads only the lines that open with a bracket, as section headers, and looks past the bracket */
#define MW2_INI_SECTION_HEADERS
/* A nuke's blast reaches this far, whatever its effect's shape (StartNuke) */
#define MW2_NUKE_RADIUS 10000
/* A player in the target state resolves a symbolic target to the nearest one when Player::m_unk0x1a0 is set, else
   to the best, whatever g_aiSpreadTargets (ResolveTarget) */
#define MW2_RESOLVE_NEAREST_FLAG
/* With bit 2 of Mech::m_unk0x112 set the weapons aim along the eye tilted by the torso's pitch, not at the torso's
   pitch (GetEyeAimDirection) */
#define MW2_EYE_AIM_ADDS_PITCH
/* The depth sort's entries are 0x20 bytes, 0x18 nothing decompiled reads (DepthEntry::m_unk0x08) */
#define MW2_DEPTH_ENTRY_UNK_MEMBERS
/* DodgeShot takes the shot's pitch from the target's and the torso's, modulo 360 degrees, not the target's alone
   scaled */
#define MW2_DODGE_TORSO_PITCH
/* A shot fells a game thing whose hit shape is of kind 0xb0 at once (DamageGameThing; GBL's MW2_BUILDINGS_FALL_AT_ONCE
   tests the static object's shape, and adds an effect) */
#define MW2_SHOTS_FELL_BUILDINGS
/* Mechs take heat from outside (Mech::m_externalHeat, which the radius damage adds to and
   CalculateHeat adds to the frame's heat) */
#define MW2_EXTERNAL_HEAT
/* Mechs have a second word of flags (Mech::m_unk0x112, whose bit 0 UpdateShot sets for a NARC hit) */
#define MW2_MECH_SECOND_FLAGS
/* Mechs have 14 weapon slots, not 10 (WEAPON_SLOT_COUNT) */
#define MW2_FOURTEEN_WEAPON_SLOTS
/* An AI player whose gunnery is above 6 takes its skills from a table of pilots, with two more
   values and a flag the AI's fire decisions test (Player::m_unk0x160, m_unk0x1b0, m_unk0x1b4) */
#define MW2_PILOT_TABLE
/* The maneuver tables are data, five of them the pilot table's, and each player has its table and a
   range (Player::m_maneuverTable, m_unk0x1b0: InitializeManeuvers) */
#define MW2_STATIC_MANEUVER_TABLES
/* Representation: a player's AI flags are an int, not a short (PlayerAi::m_flags) */
#define MW2_AI_FLAGS_INT
/* The local star starts at 0, and the player's formation goes to it */
#define MW2_LOCAL_STAR_FROM_ZERO
/* The CHAT_MODE game key, action 66: the later actions are one higher (GAME_KEY_ACTION) */
#define MW2_CHAT_KEY
/* The TOGGLE_AMS game key, action 169 */
#define MW2_AMS_KEY
/* The message boxes and the refresh mode errors name Ghost Bear's Legacy */
#define MW2_GHOST_BEAR_TITLES
/* The game's CD is the drive with GBL_HERC.DRV */
#define MW2_GBL_CD_DRIVER
/* Representation: GetEngagementAIFlags returns an int, not a short */
#define MW2_ENGAGEMENT_FLAGS_INT
/* VFX as released (unaligned fills and runs, word-wise character copies), where 1.1 modified it; the
   assembly gets it from cmake/editions.cmake */
#define MW2_VFX_STOCK
/* VFXA's VFX_lookaside_write, which sets one lookaside entry (cmake/editions.cmake) */
#define MW2_VFX_LOOKASIDE_WRITE
/* VFX3D's VFX_flat_polygon fills its spans with the destination aligned, as 1.1's modified VFX does,
   where its other routines are VFX as released (the assembly gets it from cmake/editions.cmake) */
#define MW2_VFX3D_ALIGNED_FILL
/* A recentering torso snaps to the legs within 1 degree, not 4 (TORSO_SNAP_RANGE) */
#define MW2_TORSO_SNAP_ONE_DEGREE
/* Weapons work through their definitions: each has a launch function (SpawnShot for all but the
   AMS), a function called each frame, flags (the launch sounds, an animation) and an animation, and
   there are 35 of them; shots and effects carry the weapon's flags and a word more, and there are 255
   of each and 36 effect types, with four more words each (WeaponDef, Shot, Effect, EffectInfo) */
#define MW2_WEAPON_FUNCTIONS
/* PlayPositionalSound takes the sample's flags (PlayPositionalSoundEx; the old entry point passes
   0x32), and plays nothing beyond a cutoff distance, g_unk0x100ba608 hundred units, when that is set */
#define MW2_POSITIONAL_SOUND_FLAGS
/* Artefact: weapons.c, ai.c, maneuvers.c and gridobject.c call the C runtime's abs, which GBL's
   other units expand (MW2_GBL's pragma above), and bandpoly.c, AssignStarTarget and
   ChooseArtResolution its labs */
#define MW2_ABS_CALLS
/* Artefact: GetBearingAndRange compares with the C runtime's labs */
#define MW2_BEARING_LABS
/* EjectPlayer tells the local player with cockpit message 4 when the mech goes without ejecting */
#define MW2_SELF_DESTRUCT_MESSAGE
/* A mech's kill counts for the player whose shot hit it last, the local player's outcome is 4 unless
   they ejected, and in a network game only the local mech's death is played out (KillMech, which
   DestroySection and DestroyCriticalSlot call whatever the game) */
#define MW2_KILLS_BY_LAST_SHOOTER
/* A critical hit on a flamer and on the AMS has a message of its own (DestroyCriticalSlot) */
#define MW2_MORE_CRITICAL_HITS
/* The AI's attack and target states stop the group fire as well as the fire (AiStateAttack,
   AiStateMove) */
#define MW2_AI_STOPS_GROUP_FIRE
/* Artefact: GBL's source copies some structures with memcpy, which its build calls, where the
   optimized editions show a structure assignment: nav points, rays, players, matrices, weapon slots,
   render settings, sample headers and session descriptions (RemoveNavPoint, CopyRay,
   LogPlayerStatusLines, the object matrices, LayoutWeaponPanels, the mech view panel, StartSample,
   FindSession) */
#define MW2_STRUCT_COPY_MEMCPY
/* Only the local player inspects its target, which counts as inspected once anyone has, and the
   inspect request stays set (UpdateTarget) */
#define MW2_LOCAL_INSPECTION
/* DdrawCreateSurfaces sets no DDSD_CAPS in its surface descriptions */
#define MW2_DDRAW_NO_CAPS_FLAG
/* The cheat codes are a table with the messages each shows, all stored XORed with 0xa, the messages
   decoded at the first key typed; GBL has codes of its own, and six of them do nothing
   (g_cheatCodes, HandleCheatInput) */
#define MW2_CHEAT_TABLE
/* GetIniValue cuts a value at a ';' and trims it, unless its second argument asks for the raw value
   (which error.c's call doesn't pass) */
#define MW2_INI_COMMENTS
/* ReportInputDeviceError reports nothing about the mouse */
#define MW2_NO_MOUSE_ERRORS
/* Artefact: DrawDepthQueue takes the next entry's index into a local before it copies the entry */
#define MW2_DEPTH_QUEUE_INDEX
/* Artefact: the vector lengths sum the squares from z down (FloatVectorLength, ScaleVectorToLength,
   BuildMatrixFromDirection) */
#define MW2_LENGTH_SUM_FROM_Z
/* QueueObjTree draws no shape of type 0x20 */
#define MW2_SKIP_SHAPE_TYPE_20
/* The ALL_PT_MENU, PT_2_MENU and PT_3_MENU game keys (actions 53 to 55) do what the first three
   starmate menu keys do (RunGameKey) */
#define MW2_PT_MENU_KEYS
/* Artefact: RunGameKey's screenshot acquires the frame buffer in an if with an empty else, where 1.1
   has a conditional expression */
#define MW2_SCREENSHOT_ACQUIRE_IF
/* Key 0x3e asks for power down in power state 7 too, where 1.1 ignores it */
#define MW2_NO_STATE_7_POWER_TEST
/* UpdateInputs compares an axis' rate pointer with its position's address */
#define MW2_AXIS_RATE_POINTER_TEST
/* Restarting a star mission keeps its objectives' targets' listed flags */
#define MW2_RESTART_KEEPS_LISTED
/* IsTeamNearTarget tests only the team's leader */
#define MW2_LEADER_NEAR_TARGET
/* The local star's mission fails once the local mech is lost (UpdateObjectives, g_localMechLost) */
#define MW2_LOST_MECH_FAILS_STAR
/* UpdateObjective takes star N as player N in network games */
#define MW2_NETWORK_STAR_PLAYERS
/* Artefact: FindIpxProviderCallback and FillJoystickDeviceInfo compare names with strcmp, not memcmp */
#define MW2_NAME_STRCMP
/* MemCopy and MemSet return the C runtime's memcpy and memset, where 1.1's are assembly */
#define MW2_MEM_HELPERS_CALL_CRT
/* A scrounge record whose id names no class takes a static object's shape, loading the object
   (BwdExecuteStream) */
#define MW2_SCROUNGE_STATIC_OBJECTS
/* A mech's heat sinks cool five times as well at -100 degrees or colder, on top of the cold bonus
   (LoadMechConfig) */
#define MW2_DEEP_COLD_COOLING
/* SimMain updates the players every frame, before a network launch too */
#define MW2_PLAYERS_BEFORE_LAUNCH
/* StartSupAnim leaves the palette's first 16 entries as the backdrop set them */
#define MW2_SUPANIM_KEEPS_PALETTE
/* Artefact: InitSoundInfo fills the table with -1, not 0xff (which GBL's memset call shows) */
#define MW2_SOUND_INFO_FILL_MINUS_ONE
/* DdrawStop releases DirectDraw without returning the window to normal cooperation or restoring the
   display mode */
#define MW2_DDRAW_STOP_KEEPS_MODE
/* Artefact: DdrawInit's DDCAPS is DirectX 1's, 0xac bytes (the edition was built with DirectX 1's headers) */
#define MW2_DDRAW_CAPS_DX1
/* In a network game, UpdateMech leaves a player with flag 1 (another machine's) where it is unless it
   is the local one, and holds a truck at its object's height */
#define MW2_REMOTE_MECHS_STAY
#endif

/* A game key action's number from 1.1's: with MW2_CHAT_KEY, the actions from 0x42 on are one higher, and with
   MW2_MFD_GAME_KEYS, those from 3 on 10 higher, from 14 on 18 and from 0x42 on 19. */
#if defined(MW2_MFD_GAME_KEYS)
#define GAME_KEY_ACTION(p_action)                                                                                      \
	((p_action) >= 0x42 ? (p_action) + 19                                                                              \
	 : (p_action) >= 14 ? (p_action) + 18                                                                              \
	 : (p_action) >= 3  ? (p_action) + 10                                                                              \
						: (p_action))
#elif defined(MW2_CHAT_KEY)
#define GAME_KEY_ACTION(p_action) ((p_action) >= 0x42 ? (p_action) + 1 : (p_action))
#else
#define GAME_KEY_ACTION(p_action) (p_action)
#endif

/* Boolean typedefs (MechBool / MechBool8 ...) are added when a match proves
   the width and signedness. */

#endif /* TYPES_H */
