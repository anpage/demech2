#include "debris.h"

#include "approxlen.h"
#include "clock.h"
#include "collision.h"
#include "debrischunk.h"
#include "debrispiece.h"
#include "decomp.h"
#include "environment.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "inradius.h"
#include "integrate.h"
#include "mechclass.h"
#include "object.h"
#include "random.h"
#include "shape.h"
#include "shots.h"
#include "types.h"

DECOMP_SIZE_ASSERT(DebrisPiece, 0x24)
DECOMP_SIZE_ASSERT(DebrisChunk, 0x14)

// Wreckage: scene objects knocked off a model fly as debris pieces, bounce on the terrain and
// come to rest; the chunks among them can be shot to pieces and time out.

// GLOBAL: MW2 0x100a1150
MechS32 g_debrisCount = 0;

// GLOBAL: MW2 0x100a1158
DebrisChunk g_emptyDebrisChunk = {0};

// GLOBAL: MW2 0x10179ec0
DebrisPiece g_debrisPieces[0x80];

// GLOBAL: MW2 0x1017b0c0
DebrisChunk g_debrisChunks[0x80];

// FUNCTION: MW2 0x100040b0
MechS32 FUN_100040b0(void)
{
	MechS32 i;

	for (i = 0; i < 0x80 && g_debrisPieces[i].m_obj; i++) {
	}

	return i == 0x80;
}

// Stack-slot permutation: i and piece.
// FUNCTION: MW2 0x10004111
MechS32 FUN_10004111(SceneObject* p_obj, MechS32 p_unk0x00)
{
	MechS32 i;
	DebrisPiece* piece;

	if (p_obj == NULL) {
		return -1;
	}

	for (i = 0; i < 0x80 && g_debrisPieces[i].m_obj != p_obj; i++) {
	}

	if (i == 0x80) {
		for (i = 0; i < 0x80 && g_debrisPieces[i].m_obj; i++) {
		}
	}

	if (i == 0x80) {
		return -1;
	}

	FUN_10004c06(i);
	g_debrisCount++;
	FUN_10001e32(p_obj);

	piece = &g_debrisPieces[i];
	piece->m_unk0x00 = p_unk0x00;
	piece->m_obj = p_obj;
	piece->m_acceleration = -g_unk0x100ba600;
	return i;
}

// Throws the piece off in a random direction, spinning.
// FUNCTION: MW2 0x10004218
void FUN_10004218(MechS32 p_index)
{
	DebrisPiece* piece;

	piece = &g_debrisPieces[p_index];
	if (!piece->m_obj) {
		return;
	}

	piece->m_velocityX = FixedMul16((FUN_100736f5() << 16) / ((g_unk0x100ba604 << 10) >> 16), 0x57e98);
	piece->m_velocityZ = FixedMul16((FUN_100736f5() << 16) / ((g_unk0x100ba604 << 10) >> 16), 0x57e98);
	piece->m_velocityY = FixedMul16(((FUN_100736f5() + 0x400) << 16) / ((g_unk0x100ba604 << 10) >> 16), 0x57e98);
	piece->m_spinX = FUN_100736f5() * 0x7e98 / 0x400;
	piece->m_spinY = FUN_100736f5() * 0x7e98 / 0x400;
	piece->m_spinZ = FUN_100736f5() * 0x7e98 / 0x400;
}

// Blows p_obj off its model as a chunk of debris; p_callback gets it when it's gone.
// Stack-slot permutation: i and chunk.
// FUNCTION: MW2 0x10004356
void FUN_10004356(SceneObject* p_obj, ObjectCallback p_callback, MechU32 p_unk0x16)
{
	MechS32 i;
	MechS32 destroy;
	MechS32 piece;
	DebrisChunk* chunk;

	if (!p_obj || !p_obj->m_unk0x6c) {
		return;
	}

	destroy = FALSE;
	if ((p_obj->m_unk0x6c->m_unk0x02 & 0xf0) == 0x70 || (p_unk0x16 && p_obj->m_unk0x6c->m_unk0x16 == 0)) {
		FUN_1003ad2d(p_obj->m_unk0x6c, 0x50);
		destroy = TRUE;
	}

	for (i = 0; i < 0x80 && g_debrisChunks[i].m_active; i++) {
	}

	if (i == 0x80 || destroy) {
		FUN_10001e32(p_obj);
		FUN_1000457e(p_obj, p_callback);
		return;
	}
	else {
		chunk = &g_debrisChunks[i];
	}

	piece = FUN_10004111(p_obj, 2);
	if (piece >= 0) {
		chunk->m_active = TRUE;
		chunk->m_obj = p_obj;
		chunk->m_callback = p_callback;
		chunk->m_startTime = g_currentClock;
		chunk->m_health = 0x100000;
		FUN_10001bce(p_obj, 0x300);
		SetObjTreeFlag(p_obj, 0x50);
		FUN_10001b0c(p_obj, i);
		FUN_10004218(piece);
	}
	else {
		FUN_1000457e(p_obj, p_callback);
	}
}

// Stack-slot permutation: child and sibling.
// FUNCTION: MW2 0x100044f3
void FUN_100044f3(SceneObject* p_obj, ObjectCallback p_callback, MechU32 p_unk0x16)
{
	SceneObject* child;
	SceneObject* sibling;

	if (!p_obj) {
		return;
	}

	child = FUN_10001da4(p_obj);
	if (child) {
		FUN_100044f3(child, p_callback, p_unk0x16);
	}

	sibling = FUN_10001dba(p_obj);
	if (sibling) {
		FUN_100044f3(sibling, p_callback, p_unk0x16);
	}

	FUN_10004356(p_obj, p_callback, p_unk0x16);
}

// FUNCTION: MW2 0x1000457e
void FUN_1000457e(SceneObject* p_obj, ObjectCallback p_callback)
{
	MechS32 index;

	if (p_obj) {
		FUN_10004e4d(p_obj);
		if (p_callback) {
			p_callback(p_obj);
		}

		index = FUN_10004c86(p_obj);
		if (index != -1) {
			FUN_10004c06(index);
		}
	}
}

// FUNCTION: MW2 0x100045db
void UpdateDebris(void)
{
	MechS32 i;

	for (i = 0; i < 0x80; i++) {
		UpdateDebrisPiece(i);
	}

	for (i = 0; i < 0x80; i++) {
		if (g_debrisChunks[i].m_active == TRUE && g_currentClock - g_debrisChunks[i].m_startTime > 0xe24 &&
			!g_unk0x100a2c04) {
			FUN_1000457e(g_debrisChunks[i].m_obj, g_debrisChunks[i].m_callback);
			g_debrisChunks[i] = g_emptyDebrisChunk;
		}
	}
}

// Blows the chunk up.
// FUNCTION: MW2 0x100046b2
void FUN_100046b2(MechS32 p_index)
{
	DebrisChunk* chunk;
	SceneObject* obj;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 piece;

	chunk = &g_debrisChunks[p_index];
	obj = chunk->m_obj;
	if (obj) {
		FUN_1003adc9(FUN_1000154d(obj), &x, &y, &z);
		FUN_1006b152(-2, 7, x, y, z, x, y, z);
		FUN_10004e4d(obj);
		if (chunk->m_callback) {
			chunk->m_callback(obj);
		}

		piece = FUN_10004c86(obj);
		if (piece != -1) {
			FUN_10004c06(piece);
		}

		*chunk = g_emptyDebrisChunk;
	}
}

// FUNCTION: MW2 0x10004783
void FUN_10004783(MechS32 p_index, MechS32 p_damage)
{
	g_debrisChunks[p_index].m_health -= p_damage;
	if (g_debrisChunks[p_index].m_health < 0) {
		FUN_100046b2(p_index);
	}
}

// Stack-slot permutation of the locals, and the operand order of newY <= ground.
// FUNCTION: MW2 0x100047c2
void UpdateDebrisPiece(MechS32 p_index)
{
	MechS32 velocityY;
	MechS32 rising;
	MechS32 landed;
	DebrisPiece* piece;
	MechS32 ground;
	MechS32 radius;
	MechS32 z;
	MechS32 spinZ;
	MechS32 y;
	MechS32 spinY;
	MechS32 x;
	MechS32 spinX;
	MechS32 dz;
	MechS32 newY;
	MechS32 dx;
	MechS32 dy;

	landed = FALSE;
	piece = &g_debrisPieces[p_index];
	if (!piece->m_obj) {
		return;
	}

	radius = FUN_1003adc9(FUN_1000154d(piece->m_obj), &x, &y, &z);
	y = y - (radius >> 1);
	newY = y;
	velocityY = piece->m_velocityY;
	rising = velocityY > 0;
	IntegrateMidpoint(&newY, &velocityY, piece->m_acceleration, g_deltaTime);

	if (velocityY <= 0 && newY <= (ground = GetTerrainHeight(x, y, z))) {
		if (rising) {
			FUN_10004e4d(piece->m_obj);
			landed = TRUE;
		}

		if (velocityY > -0x8d6f) {
			landed = TRUE;
		}

		newY = ground;
		velocityY = -(velocityY >> 2);
		if (RandomIntBelow(2)) {
			velocityY >>= 1;
			piece->m_velocityX = -(piece->m_velocityX >> 1);
			piece->m_velocityZ = -(piece->m_velocityZ >> 1);
		}

		piece->m_spinX = -(piece->m_spinX >> 1);
		piece->m_spinY = -(piece->m_spinY >> 1);
		piece->m_spinZ = -(piece->m_spinZ >> 1);
	}

	piece->m_velocityY = velocityY;
	dx = FixedMul16(piece->m_velocityX, g_deltaTime);
	dy = newY - y;
	dz = FixedMul16(piece->m_velocityZ, g_deltaTime);
	spinX = FixedMul16(piece->m_spinX, g_deltaTime << 16);
	spinY = FixedMul16(piece->m_spinY, g_deltaTime << 16);
	spinZ = FixedMul16(piece->m_spinZ, g_deltaTime << 16);
	FUN_10001667(piece->m_obj, dx, dy, dz);
	FUN_1000184b(piece->m_obj, spinX, spinY, spinZ, 0);
	FUN_10001cf8(piece->m_obj);

	if (landed) {
		g_debrisCount--;
		FUN_10004c06(p_index);
	}
}

// Pushes the piece by (p_x, p_y, p_z) on top of a random throw.
// Stack-slot permutation: length, speed and piece; and the operand order of length < speed.
// FUNCTION: MW2 0x10004a45
void FUN_10004a45(MechS32 p_index, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 length;
	MechS32 speed;
	DebrisPiece* piece;

	piece = &g_debrisPieces[p_index];
	if (!piece->m_obj) {
		return;
	}

	length = ApproximateVectorLength(p_x, p_y, p_z);
	if (!length) {
		return;
	}

	FUN_10004218(p_index);
	speed = ApproximateVectorLength(piece->m_velocityX, piece->m_velocityY, piece->m_velocityZ);
	if (length < speed) {
		speed = FixedDiv16(length * 2, speed);
		piece->m_velocityX = FixedMul16(piece->m_velocityX, speed);
		piece->m_velocityY = FixedMul16(piece->m_velocityY, speed);
		piece->m_velocityZ = FixedMul16(piece->m_velocityZ, speed);
	}

	piece->m_velocityX += p_x;
	piece->m_velocityY += p_y;
	piece->m_velocityZ += p_z;
}

// The only diff is the indirect call's displacement (g_debrisChunks[0].m_callback), which
// reccmp leaves unmapped.
// FUNCTION: MW2 0x10004b4f
void ZeroChunx(void)
{
	MechS32 i;
	SceneObject* obj;

	for (i = 0; i < 0x80; i++) {
		obj = g_debrisChunks[i].m_obj;
		if (obj && g_debrisChunks[i].m_callback) {
			g_debrisChunks[i].m_callback(obj);
		}

		g_debrisChunks[i] = g_emptyDebrisChunk;
	}

	for (i = 0; i < 0x80; i++) {
		FUN_10004c06(i);
	}
}

// FUNCTION: MW2 0x10004c06
void FUN_10004c06(MechS32 p_index)
{
	DebrisPiece* piece;

	piece = &g_debrisPieces[p_index];
	piece->m_unk0x00 = 0;
	piece->m_obj = NULL;
	piece->m_velocityX = piece->m_velocityY = piece->m_velocityZ = 0;
	piece->m_spinX = piece->m_spinY = piece->m_spinZ = 0;
	piece->m_acceleration = 0;
}

// Stack-slot permutation: index and i.
// FUNCTION: MW2 0x10004c86
MechS32 FUN_10004c86(SceneObject* p_obj)
{
	MechS32 index;
	MechS32 i;

	index = -1;
	for (i = 0; i < 0x80; i++) {
		if (g_debrisPieces[i].m_obj == p_obj) {
			index = i;
			break;
		}
	}

	return index;
}

// Damages the chunks within p_radius of (p_x, p_y, p_z) by p_damage per second.
// Stack-slot permutation of the locals, and the operand order of radius + p_radius.
// FUNCTION: MW2 0x10004ce5
void FUN_10004ce5(MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_radius, MechS32 p_damage)
{
	DebrisChunk* chunk;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;
	MechS32 reach;
	MechS32 i;

	i = 0x80;
	while (i--) {
		chunk = &g_debrisChunks[i];
		if (!chunk->m_active || !chunk->m_obj) {
			continue;
		}

		radius = FUN_1003adc9(FUN_1000154d(chunk->m_obj), &x, &y, &z);
		dx = x - p_x;
		dy = y - p_y;
		dz = z - p_z;
		reach = radius + p_radius;
		if (FUN_10004ec0(dx, dy, dz, reach)) {
			FUN_10004783(i, FixedMul16(p_damage, g_deltaTime));
		}
	}
}

// FUNCTION: MW2 0x10004dcb
void FUN_10004dcb(SceneObject* p_obj, ObjectCallback p_callback)
{
	MechS32 i;

	if (p_obj == NULL) {
		return;
	}

	for (i = 0; i < 0x80; i++) {
		if (g_debrisChunks[i].m_obj == p_obj) {
			g_debrisChunks[i] = g_emptyDebrisChunk;
			break;
		}
	}

	FUN_1000457e(p_obj, p_callback);
}

// FUNCTION: MW2 0x10004e4d
void FUN_10004e4d(SceneObject* p_obj)
{
	if (p_obj) {
		FUN_100018ca(p_obj);
		FUN_1000199a(p_obj);
		FUN_10001cf8(p_obj);
	}
}
