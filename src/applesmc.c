/*
 * applesmc.c - Fan control daemon for Apple Computers
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
#include "applesmc.h"

int countFans(char *basePath)
{
	DIR *fan_dir;
	struct dirent *dir_entry;
	int fnlen;
	int fancnt = 0;

	fan_dir = opendir(basePath);
	if (fan_dir != NULL)
	{
		while ((dir_entry = readdir(fan_dir)) != NULL)
		{
			if (dir_entry->d_name[0] != '.')
			{
				fnlen = strlen(dir_entry->d_name);
				if (fnlen >= 8 &&
					(dir_entry->d_name[0] == 'f') &&
					(dir_entry->d_name[1] == 'a') &&
					 (dir_entry->d_name[2] == 'n') &&
					(dir_entry->d_name[fnlen-1] == 'n') &&
					(dir_entry->d_name[fnlen-2] == 'i') &&
					(dir_entry->d_name[fnlen-3] == 'm') &&
					(dir_entry->d_name[fnlen-4] == '_'))
                {
                    // fanX_min 
                    fancnt++;
                }
                else if (fnlen >= 9 &&
                    (dir_entry->d_name[0] == 'f') &&
                    (dir_entry->d_name[1] == 'a') &&
                    (dir_entry->d_name[2] == 'n') &&
                    (dir_entry->d_name[fnlen-1] == 'n') &&
                    (dir_entry->d_name[fnlen-2] == 'i') &&
                    (dir_entry->d_name[fnlen-3] == 'm') &&
                    (dir_entry->d_name[fnlen-4] == '_'))
                {
                    // fanXX_min - Xserve has up to 18 fans
                    fancnt++;
                }
			}
		}
	} else {
		printf("Cannot open %s", basePath);
	}

	return fancnt;
}

static int read_sysfs_int(const char *path, int *out)
{
    char buf[32];
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 1)
        return -1;
    buf[n] = 0;
    *out = atoi(buf);
    return 0;
}

/* Kernel >= 7.3 (T2 Macs): macsmc_hwmon. Attributes live directly in the
 * hwmon directory and the hwmon "name" file identifies it. */
static void find_macsmc_hwmon(struct applesmc *smc)
{
    DIR *d = opendir(HWMON_DIR);
    struct dirent *e;
    char p[PATH_MAX];
    char name[64];

    if (d == NULL)
        return;

    while ((e = readdir(d)) != NULL && smc->path[0] == 0)
    {
        if (e->d_name[0] == '.')
            continue;

        snprintf(p, sizeof(p), "%s/%s/name", HWMON_DIR, e->d_name);
        int fd = open(p, O_RDONLY);
        if (fd < 0)
            continue;
        int n = read(fd, name, sizeof(name) - 1);
        close(fd);
        if (n < 1)
            continue;
        name[n] = 0;
        char *nl = strchr(name, '\n');
        if (nl) *nl = 0;

        if (strcmp(name, MACSMC_HWMON_ID) == 0)
        {
            snprintf(p, sizeof(p), "%s/%s", HWMON_DIR, e->d_name);
            char *real = realpath(p, NULL);
            if (real != NULL)
            {
                strlcpy(smc->path, real, PATH_MAX);
                free(real);
                smc->is_macsmc = true;
            }
        }
    }
    closedir(d);
}

void find_applesmc(struct applesmc *smc)
{
    int i, ret, fd_name;
    char *name_path = calloc(PATH_MAX, sizeof(char));

    smc->fans = calloc(MAXFANS, sizeof(struct fan));
    smc->sensors = calloc(MAXSENSORS, sizeof(struct sensor));

    DIR *fd_dir;
    smc->path[0] = 0;

    fd_dir = opendir(HWMON_DIR);
    if (fd_dir != NULL)
    {
        struct dirent *dir_entry;

        while ((dir_entry = readdir(fd_dir)) != NULL && smc->path[0] == 0)
        {
            if (dir_entry->d_name[0] != '.')
            {
                sprintf(name_path, "%s/%s/device/name", HWMON_DIR, dir_entry->d_name);
                fd_name = open(name_path, O_RDONLY);

                if (fd_name > -1)
                {
                    char name[sizeof(APPLESMC_ID)];

                    ret = read(fd_name, name, sizeof(APPLESMC_ID) - 1);
                    close(fd_name);

                    if (ret == sizeof(APPLESMC_ID) - 1)
                    {
                        if (strncmp(name, APPLESMC_ID, sizeof(APPLESMC_ID) - 1) == 0)
                        {
                            char *dev_path;
                            char *last_slash = strrchr(name_path, '/');

                            if (last_slash != NULL)
                            {
                                *last_slash = 0;
                                dev_path = realpath(name_path, NULL);

                                if (dev_path != NULL)
                                {
                                    strlcpy(smc->path, dev_path, PATH_MAX);
                                    free(dev_path);
                                }
                            }
                        }
                    }
                }
            }
        }
        closedir(fd_dir);
    }

    smc->is_macsmc = false;
    if (smc->path[0] == 0)
        find_macsmc_hwmon(smc);

    if (smc->path[0] == 0)
    {
        printf("Error: Cannot find applesmc or macsmc_hwmon device.\n");
        exit(-1);
    }

    smc->fan_cnt = countFans(smc->path);
    
    for (i = 0; i < smc->fan_cnt; i++)
    {
        char path[PATH_MAX];

        smc->fans[i].id = i + 1;

        if (smc->is_macsmc)
        {
            /* macsmc_hwmon: writing a value in [min,max] to fanN_target
             * switches the fan to manual; writing 0 returns it to the SMC. */
            sprintf(path, "%s/fan%d_target", smc->path, i+1);
            strlcpy(smc->fans[i].out_path, path, PATH_MAX);
            smc->fans[i].manual_path[0] = 0;
        }
        else
        {
            sprintf(path, "%s/fan%d_output", smc->path, i+1);
            strlcpy(smc->fans[i].out_path, path, PATH_MAX);

            sprintf(path, "%s/fan%d_manual", smc->path, i+1);
            strlcpy(smc->fans[i].manual_path, path, PATH_MAX);
        }

        smc->fans[i].hw_min = 0;
        smc->fans[i].hw_max = 0;
        sprintf(path, "%s/fan%d_min", smc->path, i+1);
        read_sysfs_int(path, &smc->fans[i].hw_min);
        sprintf(path, "%s/fan%d_max", smc->path, i+1);
        read_sysfs_int(path, &smc->fans[i].hw_max);
    }

    printf("Using %s at %s (%d fans)\n",
           smc->is_macsmc ? MACSMC_HWMON_ID : APPLESMC_ID, smc->path, smc->fan_cnt);

    free(name_path);
}

void scan_sensors(struct applesmc *smc, struct mfdconfig cfg)
{
    int i, j, k, result, count = 0, skip = 0;
    struct stat buf;
    char *fname = calloc(512, sizeof(char));
    char *key_buf = calloc(SENSKEY_MAXLEN, sizeof(char));

    FILE *fp;

    int sensordesclen = sizeof(cfg.profile->sensordesc) / sizeof(cfg.profile->sensordesc[0]);

    while (count < 100)
    {
        char fname[512];
        sprintf(fname, "%s/temp%d_input", smc->path, count + 1);
        result = stat(fname, &buf);

        if (result == 0)
        {
            count++;
        }
        else
        {
            break;
        }
    }

    smc->sensor_cnt = count;

    if (smc->sensor_cnt > 0)
    {
        printf("Found %d sensors:\n", smc->sensor_cnt);
        for (i = 0; i < smc->sensor_cnt; i++)
        {
            smc->sensors[i].id = i + 1;
            smc->sensors[i].blacklisted = 0;
            sprintf(smc->sensors[i].fname, "%s/temp%d_input", smc->path, smc->sensors[i].id);

            sprintf(fname, "%s/temp%d_label", smc->path, smc->sensors[i].id);

            fp = fopen(fname, "r");
            if (fp == NULL)
            {
                printf("Error: cannot open %s\n", fname);
            }
            else if (smc->is_macsmc)
            {
                char label[256] = {0};
                int n = fread(label, 1, sizeof(label) - 1, fp);
                char *open_p = (n > 0) ? strrchr(label, '(') : NULL;
                char *close_p = open_p ? strchr(open_p, ')') : NULL;

                memset(smc->sensors[i].key, 0, SENSKEY_MAXLEN);
                if (open_p && close_p && close_p > open_p + 1)
                {
                    int klen = min((int)(close_p - open_p - 1), SENSKEY_MAXLEN - 1);
                    memcpy(smc->sensors[i].key, open_p + 1, klen);
                }
                else
                {
                    printf("Error: cannot parse label in %s\n", fname);
                }
                fclose(fp);
            }
            else
            {
                int n = fread(key_buf, 1, SENSKEY_MAXLEN - 1, fp);
                if (n < 1)
                {
                    printf("Error: cannot read %s\n", fname);
                }
                else
                {
                    char *p_endl = strrchr(key_buf, '\n');
                    if (p_endl)
                        *p_endl = 0;

                    strncpy(smc->sensors[i].key, key_buf, SENSKEY_MAXLEN);
                }
                fclose(fp);
            }
        }

        for (i = 0; i < smc->sensor_cnt; i++)
        {
            if (cfg.blacklist_cnt > 0)
            {
                skip = 0;
                for (k = 0; k < cfg.blacklist_cnt; k++)
                {
                    if (strcmp(smc->sensors[i].key, cfg.blacklist[k]) == 0)
                    {
                        skip = 1;
                        smc->sensors[i].blacklisted = true;
                        printf("Skipping blacklisted sensor: %s\n", smc->sensors[i].key);
                    }
                }

                if (skip == 1)
                    continue;
            }
            else if (cfg.profile->defaultcfg->blacklist_cnt > 0)
            {
                skip = 0;
                for (k = 0; k < cfg.profile->defaultcfg->blacklist_cnt; k++)
                {
                    if (strcmp(smc->sensors[i].key, cfg.profile->defaultcfg->blacklist[k]) == 0)
                    {
                        skip = 1;
                        smc->sensors[i].blacklisted = true;
                        printf("Skipping blacklisted sensor: %s\n", smc->sensors[i].key);
                    }
                }

                if (skip == 1)
                    continue;
            }

            printf("\t%2d: ", smc->sensors[i].id);

            int found = 0;
            for (j = 0; j < sensordesclen && !found; j++)
            {
                if (strcmp(smc->sensors[i].key, cfg.profile->sensordesc[j].id) == 0)
                {
                    found = 1;
                    printf("%s - %s\n", cfg.profile->sensordesc[j].id, cfg.profile->sensordesc[j].desc);
                }
            }

            if (!found)
            {
                printf("%s - ???\n", smc->sensors[i].key);
            }
        }
    }
    else
    {
        printf("Error: No sensors detected, terminating!\n");
        exit(-1);
    }

    fflush(stdout);
    free(fname);
    free(key_buf);
}

void read_sensors(struct applesmc *smc, struct mfdconfig cfg)
{
    int i, j, k, cnt, avgcnt, sensorcnt;
    float tempavg;

    for (i = 0; i < smc->sensor_cnt; i++)
    {
        for (k = 0; k < smc->sensor_cnt; k++)
        {
            if (strcmp(cfg.profile->sensordesc[k].id, smc->sensors[i].key) >= 0)
                strlcpy(smc->sensors[i].name, cfg.profile->sensordesc[k].desc, strlen(cfg.profile->sensordesc[k].desc)+1);
        }

        smc->sensors[i].valid = false;
        if (smc->sensors[i].blacklisted != true)
        {
            int fd = open(smc->sensors[i].fname, O_RDONLY);
            if (fd < 0)
            {
                printf("Error: cannot open %s\n", smc->sensors[i].fname);
                fflush(stdout);
            }
            else
            {
                char val_buf[16];
                int n = read(fd, val_buf, sizeof(val_buf));
                
                if (n < 1)
                {
                    printf("Error: cannot read %s\n", smc->sensors[i].fname);
                }
                else
                {
                    val_buf[min(n, (int)sizeof(val_buf) - 1)] = 0;
                    smc->sensors[i].valid = true;
                    smc->sensors[i].value = (float)atoi(val_buf) / 1000.0;
                    /* macsmc exposes PECI-relative, trend and error keys that
                     * are <= 0 or nonsense; never let them into averages. */
                    if (smc->is_macsmc && (smc->sensors[i].value <= 0.0 || smc->sensors[i].value > 125.0))
                        smc->sensors[i].valid = false;
                    if (smc->sensors[i].value <= 0.0 && smc->sensors[i].value != 0.0)
                        smc->sensors[i].value *= -1;
                }

                close(fd);
            }
        }
    }

    smc->temp_avg = 0.0;
    smc->active_sensors = 0;

    for (i = 0; i < smc->sensor_cnt; i++)
    {
        if (smc->sensors[i].blacklisted != true && smc->sensors[i].valid)
        {
            smc->temp_avg += smc->sensors[i].value;
            smc->active_sensors++;
        }
    }

    if (smc->active_sensors > 0)
        smc->temp_avg = smc->temp_avg / smc->active_sensors;

    for (i = 0; i < smc->fan_cnt; i++)
    {
        tempavg = 0;
        avgcnt  = 0;

        cnt = (cfg.fanctrl[i].sensor_cnt > 0) ? cfg.fanctrl[i].sensor_cnt : cfg.profile->fanctrl[i].sensor_cnt;

        for (j = 0; j < cnt; j++)
        {
            if (cfg.fanctrl[i].sensor_cnt > 0)
            {
                for (k = 0; k < MAXSENSORS; k++)
                {
                    if (k < smc->sensor_cnt && strcmp(cfg.fanctrl[i].sensors[j], smc->sensors[k].key) == 0 && smc->sensors[k].blacklisted != true && smc->sensors[k].valid)
                    { 
                        tempavg += smc->sensors[k].value;    
                        avgcnt++;
                    }
                }
            }
            else
            {
                for (k = 0; k < MAXSENSORS; k++)
                {
                    if (k < smc->sensor_cnt && strcmp(cfg.profile->fanctrl[i].sensors[j], smc->sensors[k].key) == 0 && smc->sensors[k].blacklisted != true && smc->sensors[k].valid)
                    {
                        tempavg += smc->sensors[k].value;
                        avgcnt++;
                    }
                }
            }
        }

        if (avgcnt > 0)
            tempavg = tempavg / avgcnt;

        if (tempavg <= 0.0 && tempavg != 0)
            tempavg *= -1;

        smc->fans[i].sensor_avg = floor(tempavg);
    }
}

/* Hand the fans back to the SMC's own control (called on shutdown). */
void release_fans(struct applesmc *smc)
{
    int i, fd;

    for (i = 0; i < smc->fan_cnt; i++)
    {
        const char *p = smc->is_macsmc ? smc->fans[i].out_path : smc->fans[i].manual_path;
        if (p[0] == 0)
            continue;
        fd = open(p, O_WRONLY);
        if (fd < 0)
        {
            printf("Error: cannot release fan: %s\n", p);
            continue;
        }
        if (write(fd, "0", 1) < 0)
            printf("Error: cannot release fan: %s\n", p);
        close(fd);
    }
    fflush(stdout);
}
