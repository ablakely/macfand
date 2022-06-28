/*
 *  macfand - Mac Fan Control Daemon
 *  Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>

#define MAX_EXCLUDE		20
#define MAX_TARGETS     10

#define HWMON_DIR		"/sys/class/hwmon"
#define APPLESMC_ID		"applesmc"
#define SENSKEY_MAXLEN	16
#define MAXFANS         15

#define MACHINESDIR "./machines"

#define max(a,b)	(a > b ? a : b)
#define min(a,b)	(a < b ? a : b)

struct fan_ctrl
{
    int *num;
    char **sensors;
    int *floor;
    int *ceiling;
};

struct mfdconfig
{
    char   *modelID;
    int    *log_level;
	int    *fan_min[MAXFANS];
    int    *temp_avg_floor;
    int    *temp_avg_ceiling;
    bool   use_avgctrl;
    char   blacklist[MAX_EXCLUDE][SENSKEY_MAXLEN];
    struct fan_ctrl fanctrl[MAXFANS];
    struct modelProfile *profile;
};

struct modelProfile
{
    struct sensor_desc
    {
        char id[SENSKEY_MAXLEN];
        char desc[150];
    } sensordesc[50];

    struct fan_desc
    {
        int num;
        char desc[150];
    } fandesc[MAXFANS];

    struct fan_ctrl fanctrl[MAXFANS];
    struct mfdconfig *defaultcfg;
};

struct mfdconfig *read_cfg(char *file);
struct modelProfile *read_profile(char *modelID);

#endif /* CONFIG_H_ */
