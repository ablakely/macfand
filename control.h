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
 *  Updated for controlling 4 fans on intel mac pros
 *  Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
#include "config.h"

#define HWMON_DIR		"/sys/class/hwmon"
#define APPLESMC_ID		"applesmc"
#define SENSKEY_MAXLEN	16

void find_applesmc();	// called once at startup, before anything else!
void scan_sensors();
void adjust();
void logger();



#endif /* CONTROL_H_ */
