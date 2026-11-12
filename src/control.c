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

void adjust(struct applesmc *smc, struct mfdconfig cfg)
{
    read_sensors(smc, cfg);
    calc_fan(smc, cfg);
    set_fan(smc, cfg);
}

void calc_fan(struct applesmc *smc, struct mfdconfig cfg)
{
    int avg_ceiling, avg_floor, fan_min, fan_max;
    float avg_speed, target_avg_speed, temp_window, normalized_temp, fan_window, temp_avg_window;
    int i;

    for (i = 0; i < smc->fan_cnt; i++)
    {
        
        fan_max = (cfg.fanctrl[i].max_speed > 0) ? cfg.fanctrl[i].max_speed : cfg.profile->fanctrl[i].max_speed;
        fan_min = (cfg.fanctrl[i].min_speed > 0) ? cfg.fanctrl[i].min_speed : cfg.profile->fanctrl[i].min_speed;
        
        int use_avg = cfg.fanctrl[i].active ? cfg.fanctrl[i].use_avgctrl
                                            : cfg.profile->fanctrl[i].use_avgctrl;
        if (use_avg == 1)
        {
            avg_ceiling = (cfg.temp_avg_ceiling > 0) ? cfg.temp_avg_ceiling : cfg.profile->defaultcfg->temp_avg_ceiling;
            avg_floor = (cfg.temp_avg_floor > 0) ? cfg.temp_avg_floor : cfg.profile->defaultcfg->temp_avg_floor;

            temp_avg_window = avg_ceiling - avg_floor;
            normalized_temp = ((smc->temp_avg - avg_floor) / temp_avg_window);
        }
        else
        {
            avg_ceiling = (cfg.fanctrl[i].ceiling > 0) ? cfg.fanctrl[i].ceiling : cfg.profile->fanctrl[i].ceiling;
            avg_floor   = (cfg.fanctrl[i].floor > 0) ? cfg.fanctrl[i].floor : cfg.profile->fanctrl[i].floor;

            temp_avg_window = avg_ceiling - avg_floor;
            normalized_temp = ((smc->fans[i].sensor_avg - avg_floor) / temp_avg_window);
        }


        /* Linear ramp: fan_min at/below floor, fan_max at/above ceiling.
         * (Previously |normalized| * window, which made fans speed up as
         * temps fell below the floor and never reached fan_max.) */
        if (normalized_temp < 0.0) normalized_temp = 0.0;
        if (normalized_temp > 1.0) normalized_temp = 1.0;

        fan_window = fan_max - fan_min;
        avg_speed = fan_min + (normalized_temp * fan_window);

        if (avg_speed > fan_max)
            avg_speed = fan_max;
        
        if (avg_speed < fan_min)
            avg_speed = fan_min;

        int want = (int)avg_speed;
        int last = smc->fans[i].last_speed;

        if (last > 0 && want < last)
        {
            /* Deadband never blocks the final step back to fan_min,
             * otherwise a fan can park just above minimum forever. */
            if (last - want < cfg.rampdown_deadband && want > fan_min)
                want = last;                                /* hold */
            else if (cfg.rampdown_step > 0 && last - want > cfg.rampdown_step)
                want = last - cfg.rampdown_step;            /* ease down */
        }

        smc->fans[i].speed = want;
    }
}

void set_fan(struct applesmc *smc, struct mfdconfig cfg)
{
    int i, fd;
    char *buf;

    for (i = 0; i < smc->fan_cnt; i++)
    {
        buf = calloc(16, sizeof(char));

        /* The driver rejects (EINVAL) anything outside the hardware range,
         * e.g. a configured max of 2763 when the SMC reports 2762. */
        if (smc->fans[i].hw_max > 0 && smc->fans[i].speed > smc->fans[i].hw_max)
            smc->fans[i].speed = smc->fans[i].hw_max;
        if (smc->fans[i].hw_min > 0 && smc->fans[i].speed < smc->fans[i].hw_min)
            smc->fans[i].speed = smc->fans[i].hw_min;

        fd = (smc->fans[i].manual_path[0] == 0) ? -2 : open(smc->fans[i].manual_path, O_WRONLY);
        if (fd == -2)
        {
            /* macsmc_hwmon: no manual file, writing the target enables manual */
        }
        else if (fd < 0)
        {
            printf("Error: cannot open fan: %s\n", smc->fans[i].manual_path);
        }
        else
        {
            strcpy(buf, "1");
            write(fd, buf, strlen(buf));
            close(fd);
        }

        fd = open(smc->fans[i].out_path, O_WRONLY);
        if (fd < 0)
        {
            printf("Error: cannot open fan: %s\n", smc->fans[i].out_path);
        }
        else
        {
            sprintf(buf, "%d", smc->fans[i].speed);
            if (write(fd, buf, strlen(buf)) < 0)
                printf("Error: write %s to %s failed\n", buf, smc->fans[i].out_path);
            else
                smc->fans[i].last_speed = smc->fans[i].speed;
            close(fd);
        }
        
        free(buf);
    }

    fflush(stdout);
}