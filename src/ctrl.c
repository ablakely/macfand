/*
 * control.c - Fan control daemon for Apple Computers
 *
 * macfand - Mac Fan Control Daemon
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>

#include "util.h"
#include "config.h"
#include "control.h"
#include "logger.h"
#include "applesmc.h"

void adjust(struct applesmc *smc, struct mfdconfig *cfg)
{
    read_sensors(smc);
    calc_fan(smc, cfg);
    set_fan(smc, cfg);
}

void calc_fan(struct applesmc *smc, struct mfdconfig *cfg)
{
    float avg_speed, target_avg_speed, temp_window, normalized_temp, fan_window, temp_avg_window;

    //for (int i = 0; i <)
}

void set_fan(struct applesmc *smc, struct mfdconfig *cfg)
{
    int i;
    char buf[10];

    for (i = 0; i < smc->fan_cnt; i++)
    {
        int fd = open(smc->fans[i].out_path, O_WRONLY);
        if (fd < 0)
        {
            printf("Error: cannot open fan: %s\n", smc->fans[i].out_path);
        }
        else
        {
            sprintf(buf, "%d", smc->fans[i].speed);
            write(fd, buf, strlen(buf));
            close(fd);
        }

        fd = open(smc->fans[i].manual_path, O_WRONLY);
        if (fd < 0)
        {
            printf("Error: cannot open fan: %s\n", smc->fans[i].manual_path);
        }
        else
        {
            strcpy(buf, "0");
            write(fd, buf, strlen(buf));
            close(fd);
        }
    }
}