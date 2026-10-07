#ifndef PATH_H
#define PATH_H

#include "fixedfloat.h"
#include "types.h"

// A path's waypoint: a position and three 16.16 values the path record gives as integers.
// SIZE 0x1c
typedef struct PathPoint {
	MechScalar m_x;       // 0x00
	MechScalar m_y;       // 0x04
	MechScalar m_z;       // 0x08
	MechScalar m_pitch;   // 0x0c
	MechScalar m_heading; // 0x10
	MechScalar m_roll;    // 0x14
	MechS32 m_duration;   // 0x18
} PathPoint;

// A named path of up to 64 waypoints, loaded from a mission's path records.
// SIZE 0x744
typedef struct Path {
	MechChar m_name[0x40];    // 0x00
	MechS32 m_count;          // 0x40
	PathPoint m_points[0x40]; // 0x44
} Path;

#endif // PATH_H
