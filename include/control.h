/*
 *  control.h -  Fan control daemon for MacBook
 *
 *  Copyright (C) 2010  Mikael Strom <mikael@sesamiq.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  macfand - Mac Fan Control Daemon
 *  Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include "config.h"
#include "applesmc.h"

void adjust(struct applesmc *smc, struct mfdconfig cfg);
void calc_fan(struct applesmc *smc, struct mfdconfig cfg);
void set_fan(struct applesmc *smc, struct mfdconfig cfg);

#endif /* CONTROL_H_ */
