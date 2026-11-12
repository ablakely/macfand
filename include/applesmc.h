/*
 * applesmc.h - Fan control daemon for Apple Computers
 *
 * macfand - Mac Fan Control Daemon
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#ifndef APPLESMC_H
#define APPLESMC_H


#include <limits.h>
#include "config.h"

struct sensor
{
    int id;
    bool blacklisted;
    char key[SENSKEY_MAXLEN];
    char name[50];
    char fname[PATH_MAX];
    float value;
    bool valid;
};

struct fan
{
    int  id;
    char out_path[PATH_MAX];
    char manual_path[PATH_MAX];
    int  speed;
    int  last_speed;  /* last speed written, 0 = none yet */
    int  hw_min;      /* fanN_min from sysfs, 0 if unknown */
    int  hw_max;      /* fanN_max from sysfs, 0 if unknown */
    float sensor_avg;
};

struct applesmc
{
    char   path[PATH_MAX];
    int    sensor_cnt;
    int    fan_cnt;
    float  temp_avg;
    int    active_sensors;
    bool   is_macsmc;  /* macsmc_hwmon (kernel >= 7.3) instead of applesmc */
    
    struct sensor *sensors;
    struct fan *fans;
    struct mfdconfig *defaults;
};

int countFans(char *basePath);
void find_applesmc(struct applesmc *smc);
void scan_sensors(struct applesmc *smc, struct mfdconfig cfg);
void read_sensors(struct applesmc *smc, struct mfdconfig cfg);
void release_fans(struct applesmc *smc);

#endif