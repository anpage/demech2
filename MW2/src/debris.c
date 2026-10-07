#include "debris.h"

#include "approxlen.h"
#include "clock.h"
#include "collision.h"
#include "debrischunk.h"
#include "debrispiece.h"
#include "decomp.h"
#include "environment.h"
#include "fixeddiv.h"
#include "fixedfloat.h"
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
// GLOBAL: MW2MATROX 0x100a5a28
MechS32 g_debrisCount = 0;

// GLOBAL: MW2 0x100a1158
// GLOBAL: MW2MATROX 0x100a5a30
DebrisChunk g_emptyDebrisChunk = {0};

// GLOBAL: MW2 0x10179ec0
// GLOBAL: MW2MATROX 0x101d5630
DebrisPiece g_debrisPieces[0x80];

// GLOBAL: MW2 0x1017b0c0
// GLOBAL: MW2MATROX 0x101d4c30
DebrisChunk g_debrisChunks[0x80];

// FUNCTION: MW2 0x100040b0
// FUNCTION: MW2MATROX 0x1002a420
MechS32 IsDebrisFull(void)
{
	MechS32 i;

	for (i = 0; i < 0x80 && g_debrisPieces[i].m_obj; i++) {
	}

	return i == 0x80;
}

// Stack-slot permutation: i and piece.
// FUNCTION: MW2 0x10004111
// FUNCTION: MW2MATROX 0x1002a481
MechS32 AddDebrisPiece(SceneObject* p_obj, MechS32 p_unk0x00)
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

	ResetDebrisPiece(i);
	g_debrisCount++;
	DetachObj(p_obj);

	piece = &g_debrisPieces[i];
	piece->m_unk0x00 = p_unk0x00;
	piece->m_obj = p_obj;
	piece->m_acceleration = -g_gravity;
	return i;
}

// Throws the piece off in a random direction, spinning.
// FUNCTION: MW2 0x10004218
// FUNCTION: MW2MATROX 0x1002a589
void ThrowDebrisPiece(MechS32 p_index)
{
	DebrisPiece* piece;

	piece = &g_debrisPieces[p_index];
	if (!piece->m_obj) {
		return;
	}

#ifdef MW2_MATROX
	piece->m_velocityX = RandomNormal() / (g_gravityScale * 1024.0f) * 5.5f;
	piece->m_velocityZ = RandomNormal() / (g_gravityScale * 1024.0f) * 5.5f;
	piece->m_velocityY = (RandomNormal() + 0x400) / (g_gravityScale * 1024.0f) * 5.5f;
	piece->m_spinX = RandomNormal() * 0.5f / 1024.0f;
	piece->m_spinY = RandomNormal() * 0.5f / 1024.0f;
	piece->m_spinZ = RandomNormal() * 0.5f / 1024.0f;
#else
	piece->m_velocityX = FixedMul16((RandomNormal() << 16) / ((g_gravityScale << 10) >> 16), 0x57e98);
	piece->m_velocityZ = FixedMul16((RandomNormal() << 16) / ((g_gravityScale << 10) >> 16), 0x57e98);
	piece->m_velocityY = FixedMul16(((RandomNormal() + 0x400) << 16) / ((g_gravityScale << 10) >> 16), 0x57e98);
	piece->m_spinX = RandomNormal() * 0x7e98 / 0x400;
	piece->m_spinY = RandomNormal() * 0x7e98 / 0x400;
	piece->m_spinZ = RandomNormal() * 0x7e98 / 0x400;
#endif
}

// Blows p_obj off its model as a chunk of debris; p_callback gets it when it's gone.
// Stack-slot permutation: i and chunk.
// FUNCTION: MW2 0x10004356
// FUNCTION: MW2MATROX 0x1002a703
void BlowOffChunk(SceneObject* p_obj, ObjectCallback p_callback, MechU32 p_unk0x16)
{
	MechS32 i;
	MechS32 destroy;
	MechS32 piece;
	DebrisChunk* chunk;

	if (!p_obj || !p_obj->m_shape) {
		return;
	}

	destroy = FALSE;
	if ((p_obj->m_shape->m_kind & 0xf0) == 0x70 || (p_unk0x16 && p_obj->m_shape->m_partId == 0)) {
		SetShapeKind(p_obj->m_shape, 0x50);
		destroy = TRUE;
	}

	for (i = 0; i < 0x80 && g_debrisChunks[i].m_active; i++) {
	}

	if (i == 0x80 || destroy) {
		DetachObj(p_obj);
		DisposeDebris(p_obj, p_callback);
		return;
	}
	else {
		chunk = &g_debrisChunks[i];
	}

	piece = AddDebrisPiece(p_obj, 2);
	if (piece >= 0) {
		chunk->m_active = TRUE;
		chunk->m_obj = p_obj;
		chunk->m_callback = p_callback;
		chunk->m_startTime = g_currentClock;
#ifdef MW2_MATROX
		chunk->m_health = 16.0f;
#else
		chunk->m_health = 0x100000;
#endif
		ClearObjTreeKind(p_obj, 0x300);
		SetObjTreeKind(p_obj, 0x50);
		SetObjTreeOwner(p_obj, i);
		ThrowDebrisPiece(piece);
	}
	else {
		DisposeDebris(p_obj, p_callback);
	}
}

// Stack-slot permutation: child and sibling.
// FUNCTION: MW2 0x100044f3
// FUNCTION: MW2MATROX 0x1002a8a0
void BlowOffObjTree(SceneObject* p_obj, ObjectCallback p_callback, MechU32 p_unk0x16)
{
	SceneObject* child;
	SceneObject* sibling;

	if (!p_obj) {
		return;
	}

	child = GetObjFirstChild(p_obj);
	if (child) {
		BlowOffObjTree(child, p_callback, p_unk0x16);
	}

	sibling = GetObjNextSibling(p_obj);
	if (sibling) {
		BlowOffObjTree(sibling, p_callback, p_unk0x16);
	}

	BlowOffChunk(p_obj, p_callback, p_unk0x16);
}

// FUNCTION: MW2 0x1000457e
// FUNCTION: MW2MATROX 0x1002a92b
void DisposeDebris(SceneObject* p_obj, ObjectCallback p_callback)
{
	MechS32 index;

	if (p_obj) {
		HideDebrisObj(p_obj);
		if (p_callback) {
			p_callback(p_obj);
		}

		index = FindDebrisPiece(p_obj);
		if (index != -1) {
			ResetDebrisPiece(index);
		}
	}
}

// FUNCTION: MW2 0x100045db
// FUNCTION: MW2MATROX 0x1002a988
void UpdateDebris(void)
{
	MechS32 i;

	for (i = 0; i < 0x80; i++) {
		UpdateDebrisPiece(i);
	}

	for (i = 0; i < 0x80; i++) {
		if (g_debrisChunks[i].m_active == TRUE && g_currentClock - g_debrisChunks[i].m_startTime > 0xe24 &&
			!g_localMechLost) {
			DisposeDebris(g_debrisChunks[i].m_obj, g_debrisChunks[i].m_callback);
			g_debrisChunks[i] = g_emptyDebrisChunk;
		}
	}
}

// Blows the chunk up.
// FUNCTION: MW2 0x100046b2
// FUNCTION: MW2MATROX 0x1002aa5f
void ExplodeChunk(MechS32 p_index)
{
	DebrisChunk* chunk;
	SceneObject* obj;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechS32 piece;

	chunk = &g_debrisChunks[p_index];
	obj = chunk->m_obj;
	if (obj) {
		GetShapeBounds(GetObjShape(obj), &x, &y, &z);
		SpawnEffect(-2, 7, x, y, z, x, y, z);
		HideDebrisObj(obj);
		if (chunk->m_callback) {
			chunk->m_callback(obj);
		}

		piece = FindDebrisPiece(obj);
		if (piece != -1) {
			ResetDebrisPiece(piece);
		}

		*chunk = g_emptyDebrisChunk;
	}
}

// FUNCTION: MW2 0x10004783
// FUNCTION: MW2MATROX 0x1002ab32
void DamageChunk(MechS32 p_index, MechScalar p_damage)
{
	g_debrisChunks[p_index].m_health -= p_damage;
	if (FIXED_IS_NEGATIVE(g_debrisChunks[p_index].m_health)) {
		ExplodeChunk(p_index);
	}
}

// Stack-slot permutation of the locals, and the operand order of newY <= ground. MW2MATROX
// stores velocityY and ground before comparing them (fst, fcomp) where the rebuild compares first
// (fcom, fstp), the symbol-order entropy of UpdateWrappedRamp's comparison.
// FUNCTION: MW2 0x100047c2
// FUNCTION: MW2MATROX 0x1002ab7a
void UpdateDebrisPiece(MechS32 p_index)
{
	MechScalar velocityY;
	MechS32 rising;
	MechS32 landed;
	DebrisPiece* piece;
	MechScalar ground;
	MechScalar radius;
	MechScalar z;
	MechScalar spinZ;
	MechScalar y;
	MechScalar spinY;
	MechScalar x;
	MechScalar spinX;
	MechScalar dz;
	MechScalar newY;
	MechScalar dx;
	MechScalar dy;

	landed = FALSE;
	piece = &g_debrisPieces[p_index];
	if (!piece->m_obj) {
		return;
	}

	radius = GetShapeBounds(GetObjShape(piece->m_obj), &x, &y, &z);
	y = y - FIXED_SHR(radius, 1);
	newY = y;
	velocityY = piece->m_velocityY;
#ifdef MW2_MATROX
	rising = velocityY > 1e-07f;
#else
	rising = velocityY > 0;
#endif
	IntegrateMidpoint(&newY, &velocityY, piece->m_acceleration, g_deltaTime);

#ifdef MW2_MATROX
	if (velocityY < 1e-07f && newY <= (ground = GetTerrainHeight(x, y, z))) {
#else
	if (velocityY <= 0 && newY <= (ground = GetTerrainHeight(x, y, z))) {
#endif
		if (rising) {
			HideDebrisObj(piece->m_obj);
			landed = TRUE;
		}

		if (velocityY > FIXED_LITERAL(-0x8d6f, -0.5524862f)) {
			landed = TRUE;
		}

		newY = ground;
		velocityY = -FIXED_SHR(velocityY, 2);
		if (RandomIntBelow(2)) {
#ifdef MW2_MATROX
			velocityY /= 2;
#else
			velocityY >>= 1;
#endif
			piece->m_velocityX = -FIXED_SHR(piece->m_velocityX, 1);
			piece->m_velocityZ = -FIXED_SHR(piece->m_velocityZ, 1);
		}

		piece->m_spinX = -FIXED_SHR(piece->m_spinX, 1);
		piece->m_spinY = -FIXED_SHR(piece->m_spinY, 1);
		piece->m_spinZ = -FIXED_SHR(piece->m_spinZ, 1);
	}

	piece->m_velocityY = velocityY;
#ifdef MW2_MATROX
	dx = piece->m_velocityX * g_deltaTime;
	dy = newY - y;
	dz = piece->m_velocityZ * g_deltaTime;
	spinX = piece->m_spinX * g_deltaTime;
	spinY = piece->m_spinY * g_deltaTime;
	spinZ = piece->m_spinZ * g_deltaTime;
#else
	dx = FixedMul16(piece->m_velocityX, g_deltaTime);
	dy = newY - y;
	dz = FixedMul16(piece->m_velocityZ, g_deltaTime);
	spinX = FixedMul16(piece->m_spinX, g_deltaTime << 16);
	spinY = FixedMul16(piece->m_spinY, g_deltaTime << 16);
	spinZ = FixedMul16(piece->m_spinZ, g_deltaTime << 16);
#endif
	MoveObj(piece->m_obj, dx, dy, dz);
	RotateObj(piece->m_obj, spinX, spinY, spinZ, 0);
	UpdateObj(piece->m_obj);

	if (landed) {
		g_debrisCount--;
		ResetDebrisPiece(p_index);
	}
}

// Pushes the piece by (p_x, p_y, p_z) on top of a random throw.
// Stack-slot permutation: length, speed and piece; and the operand order of length < speed.
// FUNCTION: MW2 0x10004a45
// FUNCTION: MW2MATROX 0x1002aeed
void PushDebrisPiece(MechS32 p_index, MechScalar p_x, MechScalar p_y, MechScalar p_z)
{
	MechScalar length;
	MechScalar speed;
	DebrisPiece* piece;

	piece = &g_debrisPieces[p_index];
	if (!piece->m_obj) {
		return;
	}

	length = ApproximateVectorLength(p_x, p_y, p_z);
	if (!length) {
		return;
	}

	ThrowDebrisPiece(p_index);
	speed = ApproximateVectorLength(piece->m_velocityX, piece->m_velocityY, piece->m_velocityZ);
	if (length < speed) {
#ifdef MW2_MATROX
		speed = length * 2 / speed;
		piece->m_velocityX *= speed;
		piece->m_velocityY *= speed;
		piece->m_velocityZ *= speed;
#else
		speed = FixedDiv16(length * 2, speed);
		piece->m_velocityX = FixedMul16(piece->m_velocityX, speed);
		piece->m_velocityY = FixedMul16(piece->m_velocityY, speed);
		piece->m_velocityZ = FixedMul16(piece->m_velocityZ, speed);
#endif
	}

	piece->m_velocityX += p_x;
	piece->m_velocityY += p_y;
	piece->m_velocityZ += p_z;
}

// The only diff is the indirect call's displacement (g_debrisChunks[0].m_callback), which
// reccmp leaves unmapped.
// FUNCTION: MW2 0x10004b4f
// FUNCTION: MW2MATROX 0x1002affe
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
		ResetDebrisPiece(i);
	}
}

// FUNCTION: MW2 0x10004c06
// FUNCTION: MW2MATROX 0x1002b0b5
void ResetDebrisPiece(MechS32 p_index)
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
// FUNCTION: MW2MATROX 0x1002b135
MechS32 FindDebrisPiece(SceneObject* p_obj)
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
// FUNCTION: MW2MATROX 0x1002b194
void DamageChunksInRadius(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechScalar p_radius, MechScalar p_damage)
{
	DebrisChunk* chunk;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar radius;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechScalar reach;
	MechS32 i;

	i = 0x80;
	while (i--) {
		chunk = &g_debrisChunks[i];
		if (!chunk->m_active || !chunk->m_obj) {
			continue;
		}

		radius = GetShapeBounds(GetObjShape(chunk->m_obj), &x, &y, &z);
		dx = x - p_x;
		dy = y - p_y;
		dz = z - p_z;
		reach = radius + p_radius;
		if (IsWithinRadius(dx, dy, dz, reach)) {
#ifdef MW2_MATROX
			DamageChunk(i, p_damage * g_deltaTime);
#else
			DamageChunk(i, FixedMul16(p_damage, g_deltaTime));
#endif
		}
	}
}

// FUNCTION: MW2 0x10004dcb
// FUNCTION: MW2MATROX 0x1002b29d
void RemoveChunk(SceneObject* p_obj, ObjectCallback p_callback)
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

	DisposeDebris(p_obj, p_callback);
}

// FUNCTION: MW2 0x10004e4d
// FUNCTION: MW2MATROX 0x1002b31f
void HideDebrisObj(SceneObject* p_obj)
{
	if (p_obj) {
		HideObjTree(p_obj);
		DisableObjTreeCollision(p_obj);
		UpdateObj(p_obj);
	}
}
