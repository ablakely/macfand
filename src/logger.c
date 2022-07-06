/*
 * logger.c - Fan control daemon for Apple Computers
 *
 * macfand - Mac Fan Control Daemon
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "config.h"
#include "control.h"
#include "logger.h"
#include "applesmc.h"

int printedNewLines = 0;

void logger(struct applesmc *smc, struct mfdconfig cfg, int fancy, int usef)
{
    int i, j, k;

    if (fancy == 1)
    {
        if (printedNewLines == 0)
        {
            for (i = 0; i < smc->fan_cnt*2; i++)
                printf("\n");

            printedNewLines = 1;
        }

        printf("\033[s\033[%dA", smc->fan_cnt*2);
    }

    if (cfg.log_level > 0)
    {
        for (j = 0; j < smc->fan_cnt; j++)
        {
            if (fancy == 1)
            {
                if (usef == 1)
                {
                    printf("\033[u\033[0m\033[1;33mFan %d [%s]: %d RPM / Target Avg: %.0fF\033[u\033[0m\n", j+1, cfg.profile->fandesc[j].desc, smc->fans[j].speed, ctof(smc->fans[j].sensor_avg));
                }
                else
                {
                    printf("\033[u\033[0m\033[1;33mFan %d [%s]: %d RPM / Target Avg: %.0fC\033[u\033[0m\n", j+1, cfg.profile->fandesc[j].desc, smc->fans[j].speed, smc->fans[j].sensor_avg);
                }
            }
            else
            {
                if (usef == 1)
                {
                    printf("Fan %d [%s]: %d RPM / Target Avg: %.0fF\n", j+1, cfg.profile->fandesc[j].desc, smc->fans[j].speed, ctof(smc->fans[j].sensor_avg));
                }
                else
                {
                    printf("Fan %d [%s]: %d RPM / Target Avg: %.0fC\n", j+1, cfg.profile->fandesc[j].desc, smc->fans[j].speed, smc->fans[j].sensor_avg);

                }
            }

            for (k = 0; k < cfg.profile->fanctrl[j].sensor_cnt; k++)
            {
                for (i = 0; i < smc->sensor_cnt; i++)
                {
                    if (strcmp(smc->sensors[i].key, cfg.profile->fanctrl[j].sensors[k]) == 0)
                    {
                        if (fancy == 1)
                        {
                            if (usef == 1)
                            {
                                printf("\t\033[1;36m%s = %.0fF\033[0m", smc->sensors[i].key, ctof(smc->sensors[i].value));
                            }
                            else
                            {
                                printf("\t\033[1;36m%s = %.0fC\033[0m", smc->sensors[i].key, smc->sensors[i].value);
                            }
                        }
                        else
                        {
                            if (usef == 1)
                            {
                                printf("\t%s = %.0fF", smc->sensors[i].key, ctof(smc->sensors[i].value));
                            }
                            else
                            {
                                printf("\t%s = %.0fC", smc->sensors[i].key, smc->sensors[i].value);
                            }
                        }
                    }
                }
            }
            printf("\n");
        }

        if (fancy == 1)
            printf("\033[u\033[0m");
    }
    
    if (cfg.log_level > 1)
    {
        printf("Sensors:");
        for (j = 0; j < smc->sensor_cnt; j++)
        {
            if (fancy == 1)
            {
                if (usef == 1)
                {
                    printf("\033[u\033[0m\033[1;33m\t%s = %.0fF\033[0m", smc->sensors[j].key, ctof(smc->sensors[j].value));
                }
                else
                {
                    printf("\033[u\033[0m\033[1;33m\t%s = %.0fC\033[0m", smc->sensors[j].key, smc->sensors[j].value);
                }
                
            } else 
            {
                if (usef == 1)
                {
                    printf("\t%s=%.0fF", smc->sensors[j].key, ctof(smc->sensors[j].value));
                }
                else
                {
                    printf("\t%s=%.0fC", smc->sensors[j].key, smc->sensors[j].value);
                }
            }
        }

        printf("\n");
    }

    fflush(stdout);
}