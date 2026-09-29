/* Miles Sound System (WAIL32.DLL) timer services: only what the game calls. They live apart
   from mss.h because every declaration in a unit's symbol table can flip the operand order of
   its comparisons, and the shell, which includes mss.h widely, uses no timers. */
#ifndef MSSTIMER_H
#define MSSTIMER_H

#include "mss.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef long HTIMER;
	typedef void (*AILTIMERCB)(unsigned int p_user);

	AILIMPORT HTIMER AILCALL AIL_register_timer(AILTIMERCB p_callback);
	AILIMPORT void AILCALL AIL_set_timer_divisor(HTIMER p_timer, unsigned int p_divisor);
	AILIMPORT void AILCALL AIL_start_timer(HTIMER p_timer);
	AILIMPORT void AILCALL AIL_release_timer_handle(HTIMER p_timer);

#ifdef __cplusplus
}
#endif

#endif /* MSSTIMER_H */
