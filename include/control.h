/*
 * control.h - Fan control daemon for Apple Computers
 *
 * macfand - Mac Fan Control Daemon
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#ifndef CONTROL_H_
#define CONTROL_H_

#include "config.h"
#include "applesmc.h"

void adjust(struct applesmc *smc, struct mfdconfig cfg);
void calc_fan(struct applesmc *smc, struct mfdconfig cfg);
void set_fan(struct applesmc *smc, struct mfdconfig cfg);

#endif /* CONTROL_H_ */
