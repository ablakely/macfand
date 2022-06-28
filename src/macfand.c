/*
 * macfand.c - Fan control daemon for Apple Computers
 *
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
 *
 * Note:
 *   Requires applesmc kernel module
**/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>

#include "control.h"
#include "config.h"
#include "applesmc.h"

#define PID_FILE	"/var/run/macfand.pid"
#define LOG_FILE	"/var/log/macfand.log"
#define CFG_FILE	"./newconf.conf"

int running = 1;
int lock_fd = -1;
int reload  = 0;


struct mfdconfig *cfg;
struct applesmc *smc;

void init()
{
    cfg = malloc(sizeof(struct mfdconfig));
    cfg->profile = malloc(sizeof(struct modelProfile));
    //cfg->modelID = malloc(sizeof(char*) * 32);

    smc = malloc(sizeof(struct applesmc));
}


void signal_handler(int sig)
{
     if (sig == SIGHUP) reload = 1;
     if (sig == SIGINT || sig == SIGTERM) running = 0;
}

void daemonize()
{
    if (getppid() == 1) return;

    pid_t pid = fork();

    if (pid < 0) exit(1);
    if (pid > 0) exit(0);

    setsid();
    umask(022);

    freopen(LOG_FILE, "w", stdout);
    freopen("/dev/null", "r", stdin);
    chdir("/");

    lock_fd = open(PID_FILE, O_RDWR | O_CREAT, 0640);
    if (lock_fd < 0) exit(1);
    if (lockf(lock_fd, F_TLOCK, 0) < 0) exit(0);

    char str[32];
    sprintf(str, "%d\n", getpid());
    write(lock_fd, str, strlen(str));
}

void usage()
{
    printf("usage: macfand [-fh]\n");
    printf("  -f   run in foreground\n");
    printf("  -h   prints this message\n");
    printf("\nmacfand - Mac Fan Control Daemon\nCopyright 2022 (C) Aaron Blakely <aaron@ephasic.org>\n");
    exit(-1);
}

int main(int argc, char *argv[])
{
    init();
    int daemon = 1, i;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            daemon = 0;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage();
        }
        else
        {
            usage();
        }
    }

    if (daemon) daemonize();

    cfg = read_cfg(CFG_FILE);
    printf("Loading profile for model: %s\n", cfg->modelID);

    cfg->profile = read_profile(cfg->modelID);

    printf("dbug: temp_avg_floor = %d\n", cfg->profile->defaultcfg->temp_avg_floor);

    smc = find_applesmc();
    scan_sensors(smc, cfg);

    printf("dbug: fanslen = %d, sensorlen = %d\n", smc->fan_cnt, smc->sensor_cnt);

    printf("dbug: fan0target0 = %d\n", cfg->profile->fanctrl[0].floor);

    for (i = 0; i < smc->fan_cnt; i++)
    {
        if (smc->fans[i].id != 0)
        {
            printf("dbug: found fan #%d in smc: %s [%s]\n", i+1, cfg->profile->fandesc[i].desc, smc->fans[i].out_path);
        }
    }

    running = 1;
    while(running)
    {
        //adjust(smc, cfg);
        //logger(smc, cfg);

        if (reload)
        {
            printf("Reloading...\n");
            cfg = read_cfg(CFG_FILE);
            cfg->profile = read_profile(cfg->modelID);

            scan_sensors(smc, cfg);
            reload = 0;
        }

        sleep(5);
    }

    if (lock_fd != -1)
    {
        close(lock_fd);
        unlink(PID_FILE);
    }

    printf("Exiting.\n");
    return 0;
}