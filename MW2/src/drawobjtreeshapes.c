/* Draws a scene tree's shapes. Not a unit of its own: depthsort.c includes it, 1.1's build before
   QueueObjTree and the Matrox edition's after it, where each object has it. */
#include "decomp.h"
#include "depthsort.h"
#include "object.h"
#include "recordstacks.h"
#include "types.h"

// Draws the shapes of the scene tree p_root, farthest first.
// MW2MATROX: the loop's comparison loads its operands in the opposite order, and the load of
// g_drawList after the sort is duplicated into both paths.
// FUNCTION: MW2 0x10033b9e
// FUNCTION: MW2MATROX 0x10023860
void DrawObjTreeShapes(SceneObject* p_root)
{
	MechS32 i;

	ResetDrawBuffer();
	g_depthEntryCount = 0;
	g_polygonCount = 0;
	g_queueHasRoom = 1;
	g_depthList = g_drawList;
	QueueObjTree(p_root);
	if (g_depthEntryCount > 1) {
		SortDepthEntries(g_drawList, &g_drawList[g_depthEntryCount - 1]);
	}

	for (i = 0; i < g_depthEntryCount; i++) {
		DrawQueuedPolygon(g_drawList[i].m_poly);
	}
}
