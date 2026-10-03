#ifndef WEAPONPANEL_H
#define WEAPONPANEL_H

#include "types.h"

struct CockpitPanel;

// The functions and globals of weaponpanel.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10033280(struct CockpitPanel* p_panel);
	void FUN_100334d3(struct CockpitPanel* p_panel);

#ifdef __cplusplus
}
#endif

#endif // WEAPONPANEL_H
