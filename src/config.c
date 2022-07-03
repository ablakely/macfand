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
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "util.h"
#include "config.h"
#include "control.h"

struct mfdconfig read_cfg(struct mfdconfig config, char *file)
{
    int count, n, minspeed, n2, count2;
    config_t cfg, *cf;
    config_setting_t *fan_ctrl, *blacklist, *fansensors;

    const char *sensor = NULL;
    const char *base = (const char *)malloc(sizeof(char *) * 1024);
    
    cf = &cfg;


    int intbase, boolbase;

    config_init(cf);

    if (!config_read_file(cf, file))
    {
        printf("[macfand.conf:%d] Configuration file error: %s\n",
            config_error_line(cf),
            config_error_text(cf)
        );

        config_destroy(cf);
        exit(-1);
    }

    if (config_lookup_string(cf, "config.modelID", &base))
    {
        //config.modelID = malloc(strlen(base)+1);
        strlcpy(config.modelID, base, strlen(base)+1);
    }

    if (config_lookup_int(cf, "config.log_level", &intbase))
        config.log_level = intbase;

    if (config_lookup_int(cf, "config.temp_avg_floor", &intbase))
        config.temp_avg_floor = intbase;

    if (config_lookup_int(cf, "config.temp_avg_ceiling", &intbase))
        config.temp_avg_ceiling = intbase;

    if (config_lookup_bool(cf, "config.use_avgctrl", &boolbase))
        config.use_avgctrl = boolbase;
    
    blacklist = config_lookup(cf, "config.blacklist");
    if (blacklist != NULL)
    {
        count     = config_setting_length(blacklist);

        for (n = 0; n < count; n++)
        {
            sensor = config_setting_get_string_elem(blacklist, n);
            strlcpy(config.blacklist[n], sensor, SENSKEY_MAXLEN);
        }
    }

    fan_ctrl = config_lookup(cf, "config.fan_ctrl");
    if (fan_ctrl != NULL)
    {
        count    = config_setting_length(fan_ctrl);

        for (n = 0; n < count; n++)
        {
            config_setting_t *fanc = config_setting_get_elem(fan_ctrl, n);
            //config.fanctrl[n] = malloc(sizeof(struct fan_ctrl));


            if (config_setting_lookup_int(fanc, "floor", &intbase))
                config.fanctrl[n].floor = intbase;

            if (config_setting_lookup_int(fanc, "ceiling", &intbase))
                config.fanctrl[n].ceiling = intbase;

            if (config_setting_lookup_int(fanc, "min_speed", &intbase))
                config.fanctrl[n].min_speed = intbase;
            
            if (config_setting_lookup_int(fanc, "max_speed", &intbase))
                config.fanctrl[n].max_speed = intbase;

            if (config_setting_lookup_bool(fanc, "use_avg", &boolbase))
            {
                config.fanctrl[n].use_avgctrl = boolbase;
                printf("dbug: enabling avg_control for fan%d\n", n+1);
            }

            fansensors = config_setting_get_member(fanc, "fan_sensors");
            if (fansensors != NULL)
            {
                config.fanctrl[n].sensor_cnt = config_setting_length(fansensors);

                for (n2 = 0; n2 < config.fanctrl[n].sensor_cnt; n2++)
                {
                    sensor = config_setting_get_string_elem(fansensors, n2);
                    if (sensor != NULL)
                    {
                        strlcpy(config.fanctrl[n].sensors[n2], sensor, SENSKEY_MAXLEN);
                    }
                }
            }
        }
    }

    config_destroy(cf);
    //retconfig = malloc(sizeof(config));
    //confp = &config;
    return config;
};

struct modelProfile *read_profile(struct mfdconfig inscfg, char *modelID)
{
    struct mfdconfig defcfg_, *defcfg;
    defcfg = malloc(sizeof(struct mfdconfig));
    defcfg = &defcfg_;

    struct modelProfile *tmpprofile = calloc(1, sizeof(struct modelProfile));
    tmpprofile->defaultcfg = calloc(1, sizeof(struct mfdconfig));

    int count, n = 0, k, v, n2, x;

    for (n = 0; n < 50; n++)
    {
        tmpprofile->sensordesc[n].id = calloc(SENSKEY_MAXLEN, sizeof(char));
        tmpprofile->sensordesc[n].desc = calloc(500, sizeof(char));
    }

    for (n = 0; n < MAXFANS; n++)
    {
        tmpprofile->sensordesc[n].desc = calloc(500, sizeof(char));
    }


    char profilepath[PATH_MAX];

    sprintf(profilepath, "%s/%s.conf", MACHINESDIR, modelID);
    printf("dbug: opening profile %s\n", profilepath);

    config_t cfg;
    config_setting_t *setting;
    const config_setting_t *fan_ctrl, *blacklist, *fansensors, *minspeed;
    
    int  intbase, boolbase;
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
                strlcpy(tmpprofile->sensordesc[i].id, key, SENSKEY_MAXLEN);
            }

            if (config_setting_lookup_string(sensor_desc, "value", &base))
            {
                strlcpy(tmpprofile->sensordesc[i].desc, base, strlen(base)+1);
            }
        }
    }
    else
    {
        printf("error: profile missing tmpprofile->sensor_desc array\n");
        return NULL;
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
                tmpprofile->fandesc[i].num = intbase;
            }

            if (config_setting_lookup_string(fan_desc, "value", &base))
            {
                tmpprofile->fandesc[i].desc = malloc(sizeof(char*)*50);
                strlcpy(tmpprofile->fandesc[i].desc, base, strlen(base)+1);
            }
        }
    }
    else
    {
        printf("error: profile missing tmpprofile->fan_desc array\n");
        return NULL;
    }

    if (config_lookup_int(&cfg, "presets.temp_avg_floor", &intbase))
        defcfg->temp_avg_floor = intbase;

    if (config_lookup_int(&cfg, "presets.temp_avg_ceiling", &intbase))
        defcfg->temp_avg_ceiling = intbase;

    
    blacklist = config_lookup(&cfg, "presets.blacklist");
    if (blacklist != NULL)
    {
        count     = config_setting_length(blacklist);

        for (n = 0; n < count; n++)
        {
            sensor = config_setting_get_string_elem(blacklist, n);
            strlcpy(defcfg->blacklist[n], sensor, SENSKEY_MAXLEN);
        }
    }

    struct fan_ctrl fcp[MAXFANS];

    for (x = 0; x < MAXFANS; x++)
    {
        fcp[x].sensors = calloc(MAX_TARGETS, sizeof(char *));
        for (n = 0; n < MAX_TARGETS; n++)
        {
            fcp[x].sensors[n] = calloc(SENSKEY_MAXLEN, sizeof(char));
        }
    }


    fan_ctrl = config_lookup(&cfg, "presets.fan_ctrl");
    if (fan_ctrl != NULL)
    {
        count    = config_setting_length(fan_ctrl);

        for (n = 0; n < count; n++)
        {
            config_setting_t *fanc = config_setting_get_elem(fan_ctrl, n);
            fcp[n].active = 1;

            if (config_setting_lookup_int(fanc, "floor", &intbase))
                fcp[n].floor = intbase;

            if (config_setting_lookup_int(fanc, "ceiling", &intbase))
                fcp[n].ceiling = intbase;

            if (config_setting_lookup_int(fanc, "min_speed", &intbase))
                fcp[n].min_speed = intbase;

            if (config_setting_lookup_int(fanc, "max_speed", &intbase))
                fcp[n].max_speed = intbase;

            if (config_setting_lookup_bool(fanc, "use_avg", &boolbase))
                fcp[n].use_avgctrl = boolbase;

            fansensors = config_setting_get_member(fanc, "fan_sensors");
            if (fansensors != NULL)
            {
                fcp[n].sensor_cnt     = config_setting_length(fansensors);

                for (n2 = 0; n2 < fcp[n].sensor_cnt; n2++)
                {
                    sensor = config_setting_get_string_elem(fansensors, n2);
                    if (sensor != NULL)
                    {
                        strlcpy(fcp[n].sensors[n2], sensor, SENSKEY_MAXLEN);
                    }
                }
            }
        }

        for (x = 0; x < MAXFANS; x++)
        {
            tmpprofile->fanctrl[x] = fcp[x];
        }
    }

    config_destroy(&cfg);
    tmpprofile->defaultcfg = defcfg;
    inscfg.profile = tmpprofile;

    return inscfg.profile;
}
