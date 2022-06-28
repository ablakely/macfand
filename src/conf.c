/*
 * config.c - Configuration File Parser
 *
 * macfand - Fan control daemon for Apple Computers
 * Written by Aaron Blakely <aaron@ephasic.org>
**/

#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>
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

#include "config.h"
#include "control.h"

struct mfdconfig *read_cfg(char *file)
{
    struct mfdconfig config, *retconfig;
    //config.modelID = malloc(sizeof(char*) * 32);

    //config.fanctrl = malloc(MAX_EXCLUDE * sizeof(struct fan_ctrl));

    int count, n, minspeed, n2, count2;
    config_t cfg;
    config_setting_t *fan_ctrl, *fan_min, *blacklist, *fansensors;

    char *sensor = NULL;
    char *base = (const char *)malloc(sizeof(char *) * 1024);
    
    int intbase;
    bool boolbase;

    config_init(&cfg);

    if (!config_read_file(&cfg, file))
    {
        printf("[macfand.conf:%d] Configuration file error: %s\n",
            config_error_line(&cfg),
            config_error_text(&cfg)
        );

        config_destroy(&cfg);
        exit(-1);
    }

    if (config_lookup_string(&cfg, "config.modelID", &base))
    {
        config.modelID = malloc(sizeof(char) * 32);
        strlcpy(config.modelID, base, 32);
    }
        //strlcpy(config.modelID, base, 32);

    if (config_lookup_int(&cfg, "config.log_level", &intbase))
        config.log_level = intbase;

    if (config_lookup_int(&cfg, "config.temp_avg_floor", &intbase))
        config.temp_avg_floor = intbase;

    if (config_lookup_int(&cfg, "config.temp_avg_ceiling", &intbase))
        config.temp_avg_ceiling = intbase;

    if (config_lookup_bool(&cfg, "config.use_avgctrl", &boolbase))
        config.use_avgctrl = boolbase;
    
    blacklist = config_lookup(&cfg, "config.blacklist");
    if (blacklist != NULL)
    {
        count     = config_setting_length(blacklist);

        for (n = 0; n < count; n++)
        {
            sensor = config_setting_get_string_elem(blacklist, n);
            strlcpy(config.blacklist[n], sensor, SENSKEY_MAXLEN);
        }
    }

    fan_min = config_lookup(&cfg, "config.fan_min");
    if (fan_min != NULL)
    {
        count   = config_setting_length(fan_min);

        for (n = 0; n < count; n++)
        {
            minspeed = config_setting_get_int_elem(fan_min, n);
            config.fan_min[n] = minspeed;
        }
    }

    fan_ctrl = config_lookup(&cfg, "config.fan_ctrl");
    if (fan_ctrl != NULL)
    {
        count    = config_setting_length(fan_ctrl);

        for (n = 0; n < count; n++)
        {
            config_setting_t *fanc = config_setting_get_elem(fan_ctrl, n);

            if (config_setting_lookup_int(fanc, "floor", &intbase))
                config.fanctrl[n].floor = intbase;

            if (config_setting_lookup_int(fanc, "ceiling", &intbase))
                config.fanctrl[n].ceiling = intbase;

            fansensors = config_lookup(fanc, "fan_sensors");
            count2     = config_setting_length(fansensors);

            for (n2 = 0; n2 < count; n2++)
            {
                sensor = config_setting_get_string_elem(fansensors, n2);
                strlcpy(config.fanctrl[n].sensors[n2], sensor, SENSKEY_MAXLEN);
            }
        }
    }

    config_destroy(&cfg);
    //retconfig = malloc(sizeof(config));
    retconfig = &config;
    return &config;
};

struct modelProfile *read_profile(char *modelID)
{
    struct modelProfile profile, *pf;

    pf = malloc(sizeof(struct modelProfile));
    profile.defaultcfg = malloc(sizeof(struct mfdconfig));

    pf = &profile;

    int count, n = 0, k, v, count2, n2;

    char profilepath[PATH_MAX];

    sprintf(profilepath, "%s/%s.conf", MACHINESDIR, modelID);
    printf("dbug: opening profile %s\n", profilepath);

    bool boolbase;
    config_t cfg;
    config_setting_t *setting;
    const config_setting_t *fan_ctrl, *fan_min, *blacklist, *fansensors, *minspeed;
    
    const int intbase = malloc(sizeof(int));
    const char *base = (const char*)malloc(sizeof(char *) * 1024);
    const char *key;
    const char *sensor = NULL;

    config_init(&cfg);

    if (!config_read_file(&cfg, profilepath))
    {
        printf("[macfand.conf:%d] Profile file error: %s\n",
            config_error_line(&cfg),
            config_error_text(&cfg)
        );

        config_destroy(&cfg);
        exit(-1);
    }

    setting = config_lookup(&cfg, "profile.sensor_desc");
    if (setting != NULL)
    {
        count = config_setting_length(setting);
        int i;

        for (i = 0; i < count; i++)
        {
            config_setting_t *sensor_desc = config_setting_get_elem(setting, i);

            if (config_setting_lookup_string(sensor_desc, "sensor", &key))
            {
                strlcpy(profile.sensordesc[i].id, key, SENSKEY_MAXLEN);
                //printf("dbug:  key = %s\n", key);
            }

            if (config_setting_lookup_string(sensor_desc, "value", &base))
            {
                strlcpy(profile.sensordesc[i].desc, base, 50);
                //printf("dbug:  value = %s\n", base);
            }
        }
    }
    else
    {
        printf("error: profile missing profile.sensor_desc array\n");
        return;
    }


    setting = config_lookup(&cfg, "profile.fan_desc");
    n = 0;

    if (setting != NULL)
    {
        count = config_setting_length(setting);
        int i;

        for (i = 0; i < count; i++)
        {
            config_setting_t *fan_desc = config_setting_get_elem(setting, i);

            if (config_setting_lookup_int(fan_desc, "num", &intbase))
            {
                profile.fandesc[i].num = intbase;
                //printf("dbug: num = %d\n", intbase);
            }

            if (config_setting_lookup_string(fan_desc, "value", &base))
            {
                strlcpy(profile.fandesc[i].desc, base, 50);
                //printf("dbug: value = %s\n", base);
            }
        }
    }
    else
    {
        printf("error: profile missing profile.fan_desc array\n");
        return;
    }

    

    if (config_lookup_int(&cfg, "presets.temp_avg_floor", &intbase))
        profile.defaultcfg->temp_avg_floor = intbase;

    if (config_lookup_int(&cfg, "presets.temp_avg_ceiling", &intbase))
        profile.defaultcfg->temp_avg_ceiling = intbase;

    if (config_lookup_bool(&cfg, "presets.use_avgctrl", &boolbase))
        profile.defaultcfg->use_avgctrl = boolbase;
    
    blacklist = config_lookup(&cfg, "presets.blacklist");
    if (blacklist != NULL)
    {
        count     = config_setting_length(blacklist);

        for (n = 0; n < count; n++)
        {
            sensor = config_setting_get_string_elem(blacklist, n);
            strlcpy(profile.defaultcfg->blacklist[n], sensor, SENSKEY_MAXLEN);
        }
    }

    fan_min = config_lookup(&cfg, "presets.fan_min");
    if (fan_min != NULL)
    {
        count   = config_setting_length(fan_min);

        for (n = 0; n < count; n++)
        {
            minspeed = config_setting_get_int_elem(fan_min, n);
            profile.defaultcfg->fan_min[n] = minspeed;
        }
    }

    fan_ctrl = config_lookup(&cfg, "presets.fan_ctrl");
    if (fan_ctrl != NULL)
    {
        count    = config_setting_length(fan_ctrl);

        for (n = 0; n < count; n++)
        {
            config_setting_t *fanc = config_setting_get_elem(fan_ctrl, n);

            if (config_setting_lookup_int(fanc, "floor", &intbase))
            {
                pf->fanctrl[n].floor = intbase;
                pf->defaultcfg->fanctrl[n].floor = intbase;
                printf("floor = %d\n", pf->defaultcfg->fanctrl[n].floor);
            }

            if (config_setting_lookup_int(fanc, "ceiling", &intbase))
                pf->fanctrl[n].ceiling = intbase;

            fansensors = config_setting_get_member(fanc, "fan_sensors");
            if (fansensors != NULL)
            {
                count2     = config_setting_length(fansensors);
                //profile.fanctrl[n].sensors = malloc(sizeof(char*) * MAX_EXCLUDE);

                for (n2 = 0; n2 < count; n2++)
                {
                    sensor = config_setting_get_string_elem(fansensors, n2);
                    if (sensor != NULL)
                    {
                        profile.fanctrl[n].sensors = malloc(sizeof(char*) * MAX_EXCLUDE);


                        strlcpy(profile.fanctrl[n].sensors[n2], sensor, SENSKEY_MAXLEN);
                        printf("dbug: fan%d sensor%d = %s\n", n, n2, profile.fanctrl[n].sensors[n2]);
                    }
                }
            }
        }
    }

    config_destroy(&cfg);

    pf = &profile;
    return pf;
}