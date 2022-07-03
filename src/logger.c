/*
 * logger.c - Fan control daemon for Apple Computers
 *
 * macfand - Mac Fan Control Daemon
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#include <stdio.h>

#include "util.h"
#include "config.h"
#include "control.h"
#include "logger.h"
#include "applesmc.h"

void logger(struct applesmc *smc, struct mfdconfig cfg)
{
    int i;

    if (cfg.log_level > 0)
    {
        printf("Speed:");

        for (i = 0; i < smc->fan_cnt; i++)
        {
            printf(" [Fan%d: %dRPM / Target Avg: %.1fC]", i+1, smc->fans[i].speed, smc->fans[i].sensor_avg);
        }

        printf("\n");
    }
    
    if (cfg.log_level > 1)
    {
        printf("Sensors:");
        for (i = 0; i < smc->sensor_cnt; i++)
        {
            printf(" %s=%.0fC", smc->sensors[i].key, smc->sensors[i].value);
        }

        printf("\n");
    }

    fflush(stdout);
}