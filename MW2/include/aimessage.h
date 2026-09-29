#ifndef AIMESSAGE_H
#define AIMESSAGE_H

#include "types.h"

// A message posted to an AI player (FUN_10054d4c): a rule's message and its target, which
// the player's rules match before their own conditions. Posted to a star's leader, it is an
// order (FUN_1005579a).
// SIZE 0x04
typedef struct AiMessage {
	MechS16 m_message; // 0x00
	MechU16 m_target;  // 0x02
} AiMessage;

#endif // AIMESSAGE_H
