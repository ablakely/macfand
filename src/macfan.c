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
#include "logger.h"
#include "applesmc.h"

#define PID_FILE	"/var/run/macfand.pid"
#define LOG_FILE	"/var/log/macfand.log"
#define CFG_FILE	"/etc/macfand.conf"

int running = 1;
int lock_fd = -1;
int reload  = 0;


struct mfdconfig cf;
struct modelProfile *profile;
struct applesmc *smc;

void init()
{
    cf.modelID = malloc(sizeof(char*) *32);
    smc = calloc(1, sizeof(struct applesmc *));
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
    printf("  -f   print temperatures in farenhiet\n");
    printf("  -n   run in foreround\n");
    printf("  -h   prints this message\n");
    printf("  -d   disables output formatting\n");
    printf("\nmacfand - Mac Fan Control Daemon\nCopyright 2022 (C) Aaron Blakely <aaron@ephasic.org>\n");
    exit(-1);
}

int main(int argc, char *argv[])
{
    init();
    int daemon = 1, i, targetlen, j, fancy = 1, usef = 0;

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
            usef = 1;
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            daemon = 0;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage();
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--no-formating") == 0)
        {
            fancy = 0;
        }
        else
        {
            usage();
        }
    }

    if (daemon) daemonize();

    cf = read_cfg(cf, CFG_FILE);

    printf("Loading profile for model: %s\n", cf.modelID);
    cf.profile = read_profile(cf, cf.modelID);
    smc->defaults = cf.profile->defaultcfg;

    find_applesmc(smc);
    scan_sensors(smc, cf);

    running = 1;
    while(running)
    {
        adjust(smc, cf);
        logger(smc, cf, fancy, usef);

        if (reload)
        {
            printf("Reloading...\n");
            cf = read_cfg(cf, CFG_FILE);
            cf.profile = read_profile(cf, cf.modelID);

            scan_sensors(smc, cf);
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