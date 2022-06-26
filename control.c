/*
 *  control.c -  Fan control daemon for MacBook
 *
 *  Copyright(C) 2010  Mikael Strom <mikael@sesamiq.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  macfand - Mac Fan Control Daemon
 *  Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
 */

#include "control.h"

struct
{
	char *key;
	char *desc;
}
sensor_desc[] =
{
	{"TB0T", "Battery TS_MAX Temp"},
	{"TB1T", "Battery TS1 Temp"},
	{"TB2T", "Battery TS2 Temp"},
	{"TB3T", "Battery Temp"},
	{"TC0D", "CPU 0 Die Temp"},
	{"TC0P", "CPU 0 Proximity Temp"},
	{"TG0D", "GPU Die - Digital"},
	{"TG0P", "GPU 0 Proximity Temp"},
	{"TG0T", "GPU 0 Die - Analog Temp"},
	{"TG0H", "Left Heat Pipe/Fin Stack Proximity Temp"},
	{"TG1H", "Left Heat Pipe/Fin Stack Proximity Temp"},
	{"TN0P", "MCP Proximity"},
	{"TN0D", "MCP Die"},
	{"Th2H", "Right Fin Stack Proximity Temp"},
	{"Tm0P", "Battery Charger Proximity Temp"},
	{"Ts0P", "Palm Rest Temp"},
	{"TA0P", "Airflow 1"},
	{"TC0C", "CPU 0 Die Core Temp - Digital"},
	{"TC1C", "CPU Core 1"},
	{"TC2C", "CPU Core 2"},
	{"TC3C", "CPU Core 3"},
	{"TCAH", "CPU 1 Heatsink Alt."},
	{"TCBH", "CPU 2 Heatsink Alt."},
	{"TH0P", "HDD Bay 1"},
	{"TH1P", "HDD Bay 2"},
	{"TH2P", "HDD Bay 3"},
	{"TH3P", "HDD Bay 4"},
	{"THTG", "???"},
	{"TM0P", "Memory Bank A1"},
	{"TM0S", "Memory Module A1"},
	{"TM1P", "Memory Bank A2"},
	{"TM1S", "Memory Module A1"},
	{"TM2P", "Memory Riser A Voltage Regulator Proximity"},
	{"TM2S", "???"},
	{"TM3S", "???"},
	{"TM8P", "Memory Bank B1"},
	{"TM8S", "Memory Module B1"},
	{"TM9P", "Memory Bank B2"},
	{"TM9S", "Memory Module B2"},
	{"TMAP", "Memory Riser B Voltage Regulator Proximity"},
	{"TMAS", "???"},
	{"TMBS", "???"},
	{"TN0H", "Memory Controller Heatsink"},
	{"TS0C", "Expansion Slots"},
	{"Tp0C", "Power Supply 1 Alt."},
	{"Tp1C", "Power Supply 2 Alt."},
	{"Tv0S", "???"},
	{"Tv1S", "???"},
};

#define N_DESC			(sizeof(sensor_desc) / sizeof(sensor_desc[0]))

struct sensor
{
	int id;
	int excluded;
	char name[SENSKEY_MAXLEN];
	char fname[PATH_MAX];
	float value;
};



char base_path[PATH_MAX];
char fan1_min[PATH_MAX];
char fan2_min[PATH_MAX];
char fan3_min[PATH_MAX];
char fan4_min[PATH_MAX];

char fan1_man[PATH_MAX];
char fan2_man[PATH_MAX];
char fan3_man[PATH_MAX];
char fan4_man[PATH_MAX];

int sensor_count = 0;
int fan_count = 0;
float temp_avg = 0;

int fan1_speed;
int fan2_speed;
int fan3_speed;
int fan4_speed;

struct sensor *sensors = NULL;
struct sensor *sensor_TC0P = NULL;
struct sensor *sensor_TM0P = NULL;

#define CTL_NONE	0	// sensor control fan flags
#define CTL_AVG		1
#define CTL_TC0P	2
#define CTL_TM0P	3

int fan_ctl = 0;		// which sensor controls fan


void find_applesmc()
{
	DIR *fd_dir;
	int ret;

	base_path[0] = 0;

	// find and verify applesmc path in /sys/devices

	fd_dir = opendir(HWMON_DIR);

	if(fd_dir != NULL)
	{
		struct dirent *dir_entry;

		while((dir_entry = readdir(fd_dir)) != NULL && base_path[0] == 0)
		{
			if(dir_entry->d_name[0] != '.')
			{
				char name_path[PATH_MAX];
				int fd_name;

				sprintf(name_path, "%s/%s/device/name", HWMON_DIR, dir_entry->d_name);

				fd_name = open(name_path, O_RDONLY);

				if(fd_name > -1)
				{
					char name[sizeof(APPLESMC_ID)];

					ret = read(fd_name, name, sizeof(APPLESMC_ID) - 1);

					close(fd_name);

					if(ret == sizeof(APPLESMC_ID) - 1)
					{
						if(strncmp(name, APPLESMC_ID, sizeof(APPLESMC_ID) - 1) == 0)
						{
							char *dev_path;
							char *last_slash = strrchr(name_path, '/');

							if(last_slash != NULL)
							{
								*last_slash = 0;

								dev_path = realpath(name_path, NULL);

								if(dev_path != NULL)
								{
									strncpy(base_path, dev_path, sizeof(base_path) - 1);
									base_path[sizeof(base_path) - 1] = 0;
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

	// create paths to fan and sensor

	if(base_path[0] == 0)
	{
		printf("Error: Can't find a applesmc device\n");
		exit(-1);
	}

	sprintf(fan1_min, "%s/fan1_output", base_path);
	sprintf(fan2_min, "%s/fan2_output", base_path);
	sprintf(fan3_min, "%s/fan3_output", base_path);
	sprintf(fan4_min, "%s/fan4_output", base_path);

	sprintf(fan1_man, "%s/fan1_manual", base_path);
	sprintf(fan2_man, "%s/fan2_manual", base_path);
	sprintf(fan3_man, "%s/fan3_manual", base_path);
	sprintf(fan4_man, "%s/fan4_manual", base_path);

	printf("Found applesmc at %s\n", base_path);
}

//------------------------------------------------------------------------------

void read_sensors()
{
	int i;
	for(i = 0; i < sensor_count; ++i)
	{
		if(! sensors[i].excluded)
		{
			// read temp value

			int fd = open(sensors[i].fname, O_RDONLY);
			if(fd < 0)
			{
				printf("Error: Can't open %s\n", sensors[i].fname);
				fflush(stdout);
			}
			else
			{
				char val_buf[16];
				int n = read(fd, val_buf, sizeof(val_buf));
				if(n < 1)
				{
					printf("Error: Can't read  %s\n", sensors[i].fname);
				}
				else
				{
					sensors[i].value = (float)atoi(val_buf) / 1000.0;
					if (sensors[i].value <= 0.0 && sensors[i].value != 0.0)
					{
						sensors[i].value *= -1;
					}
				}
				close(fd);
			}
		}
	}

	// calc average

	temp_avg = 0.0;
	int active_sensors = 0;

	for(i = 0; i < sensor_count; i++)
	{
		if(! sensors[i].excluded)
		{
			temp_avg += sensors[i].value;
			active_sensors++;
		}
	}

	temp_avg = temp_avg / active_sensors;
}

//------------------------------------------------------------------------------

void calc_fan()
{
	float temp_window;
	float normalized_temp;
	float fan_window;
	float temp_avg_window;

	float fan1_avg_speed;
	float fan2_avg_speed;
	float fan3_avg_speed;
	float fan4_avg_speed;

	float fan1_TC0P_speed;
	float fan2_TC0P_speed;
	float fan3_TC0P_speed;
	float fan4_TC0P_speed;

	float fan1_TM0P_speed;
	float fan2_TM0P_speed;
	float fan3_TM0P_speed;
	float fan4_TM0P_speed;


	fan1_speed = cpumem_min;
	fan2_speed = expansion_min;
	fan3_speed = exhaust_min;
	fan4_speed = ps_min;

	fan_ctl = CTL_NONE;

	for (int i = 0; i < 3; i++)
	{
		temp_avg_window = temp_avg_ceiling - temp_avg_floor;
		normalized_temp = (temp_avg - temp_avg_floor) / temp_avg_window;

		if (i == 0) { 
			fan_window = fan_max - cpumem_min;
			fan1_avg_speed = (normalized_temp * fan_window);

			if (fan1_avg_speed > fan1_speed) {
				fan1_speed = fan1_avg_speed;
				fan_ctl = CTL_AVG;
			}

			if (sensor_TC0P != NULL)
			{
				temp_window = temp_TC0P_ceiling - temp_TC0P_floor;
				normalized_temp = (sensor_TC0P->value - temp_TC0P_floor) / temp_window;
				fan1_TC0P_speed = (normalized_temp * fan_window);
			
				if (fan1_TC0P_speed > fan1_speed || fan1_TC0P_speed > fan1_avg_speed) {
					fan1_speed = fan1_TC0P_speed;
					fan_ctl = CTL_TC0P;
				}
			}

			if (sensor_TM0P != NULL)
			{
				temp_window = temp_TM0P_ceiling - temp_TM0P_floor;
				normalized_temp =(sensor_TM0P->value - temp_TM0P_floor) / temp_window;
				fan1_TM0P_speed =(normalized_temp * fan_window);

				if (fan1_TM0P_speed > fan1_speed || fan1_TM0P_speed > fan1_avg_speed) {
					fan1_speed = fan1_TM0P_speed;
					fan_ctl = CTL_TM0P;
				}
			}
		}
		if (i == 1) { 
			fan_window = fan_max - expansion_min;
			fan2_avg_speed = (normalized_temp * fan_window);

			if (sensor_TC0P != NULL)
			{
				temp_window = temp_TC0P_ceiling - temp_TC0P_floor;
				normalized_temp = (sensor_TC0P->value - temp_TC0P_floor) / temp_window;
				fan2_TC0P_speed = (normalized_temp * fan_window);

				if (fan2_TC0P_speed > fan2_speed || fan2_TC0P_speed > fan2_avg_speed) {
					fan2_speed = fan2_TC0P_speed;
					fan_ctl = CTL_TC0P;
				}
			}

			if (sensor_TM0P != NULL)
			{
				temp_window = temp_TM0P_ceiling - temp_TM0P_floor;
				normalized_temp =(sensor_TM0P->value - temp_TM0P_floor) / temp_window;
				fan2_TM0P_speed =(normalized_temp * fan_window);

				if (fan2_TM0P_speed > fan2_speed || fan2_TM0P_speed > fan2_avg_speed) {
					fan2_speed = fan2_TM0P_speed;
					fan_ctl = CTL_TM0P;
				}
			}
		}
		if (i == 2) {
			fan_window = fan_max - exhaust_min;
			fan3_avg_speed = (normalized_temp * fan_window);

			if (sensor_TC0P != NULL)
			{
				temp_window = temp_TC0P_ceiling - temp_TC0P_floor;
				normalized_temp = (sensor_TC0P->value - temp_TC0P_floor) / temp_window;
				fan3_TC0P_speed = (normalized_temp * fan_window);

				if (fan3_TC0P_speed > fan3_speed || fan3_TC0P_speed > fan3_avg_speed) {
					fan3_speed = fan3_TC0P_speed;
					fan_ctl = CTL_TC0P;
				}
			}

			if (sensor_TM0P != NULL)
			{
				temp_window = temp_TM0P_ceiling - temp_TM0P_floor;
				normalized_temp =(sensor_TM0P->value - temp_TM0P_floor) / temp_window;
				fan3_TM0P_speed =(normalized_temp * fan_window);

				if (fan3_TM0P_speed > fan3_speed || fan3_TM0P_speed > fan3_avg_speed) {
					fan3_speed = fan3_TM0P_speed;
					fan_ctl = CTL_TM0P;
				}
			}
		}
		if (i == 3) {
			fan_window = fan_max - ps_min;
			fan4_avg_speed = (normalized_temp * fan_window);

			if (sensor_TC0P != NULL)
			{
				temp_window = temp_TC0P_ceiling - temp_TC0P_floor;
				normalized_temp = (sensor_TC0P->value - temp_TC0P_floor) / temp_window;
				fan4_TC0P_speed = (normalized_temp * fan_window);

				if (fan4_TC0P_speed > fan4_speed || fan4_TC0P_speed > fan4_avg_speed) {
					fan4_speed = fan4_TC0P_speed;
					fan_ctl = CTL_TC0P;
				}
			}

			if (sensor_TM0P != NULL)
			{
				temp_window = temp_TM0P_ceiling - temp_TM0P_floor;
				normalized_temp =(sensor_TM0P->value - temp_TM0P_floor) / temp_window;
				fan4_TM0P_speed =(normalized_temp * fan_window);

				if (fan4_TM0P_speed > fan4_speed || fan4_TM0P_speed > fan4_avg_speed) {
					fan4_speed = fan4_TM0P_speed;
					fan_ctl = CTL_TM0P;
				}
			}
		}
		
	}


	// finally clamp

	fan1_speed = min(fan_max, fan1_speed);
	fan2_speed = min(fan_max, fan2_speed);
	fan3_speed = min(fan_max, fan3_speed);
	fan4_speed = min(fan_max, fan4_speed);
}

//------------------------------------------------------------------------------

void set_fan()
{
	char buf[16];

	// update fan 1 [CPU_MEM]

	int fd = open(fan1_min, O_WRONLY);
	if(fd < 0)
	{
		printf("Error: Can't open %s\n", fan1_min);
	}
	else
	{
		sprintf(buf, "%d", fan1_speed);
		write(fd, buf, strlen(buf));
		close(fd);
	}

	// set fan 1 manual to zero

	fd = open(fan1_man, O_WRONLY);
	if(fd < 0)
	{
		printf("Error: Can't open %s\n", fan1_man);
	}
	else
	{
		strcpy(buf, "0");
		write(fd, buf, strlen(buf));
		close(fd);
	}

	// update fan 2

	if(fan_count > 1)
	{
		fd = open(fan2_min, O_WRONLY);
		if(fd < 0)
		{
			printf("Error: Can't open %s\n", fan2_min);
		}
		else
		{
			sprintf(buf, "%d", fan2_speed);
			write(fd, buf, strlen(buf));
			close(fd);
		}

		// set fan 2 manual to zero

		fd = open(fan2_man, O_WRONLY);
		if(fd < 0)
		{
			printf("Error: Can't open %s\n", fan2_man);
		}
		else
		{
			strcpy(buf, "0");
			write(fd, buf, strlen(buf));
			close(fd);
		}
	}

	// update fan 3

	if (fan_count > 2)
	{
		fd = open(fan3_min, O_WRONLY);
		if (fd < 0)
		{
			printf("Error: Can't open %s\n", fan3_min);
		}
		else
		{
			sprintf(buf, "%d", fan3_speed);
			write(fd, buf, strlen(buf));
			close(fd);
		}

		// set fan 3 manual to zero

		fd = open(fan3_man, O_WRONLY);
		if (fd < 0)
		{
			printf("Error: Can't open %s\n", fan3_man);
		}
		else
		{
			strcpy(buf, "0");
			write(fd, buf, strlen(buf));
			close(fd);
		}
	}

	// update fan 4

	if (fan_count > 3)
	{
		fd = open(fan4_min, O_WRONLY);
		if (fd < 0)
		{
			printf("Error: Can't open %s\n", fan4_min);
		}
		else
		{
			sprintf(buf, "%d", fan4_speed);
			write(fd, buf, strlen(buf));
			close(fd);
		}
	}

	fflush(stdout);
}

//------------------------------------------------------------------------------

void adjust()
{
	read_sensors();
	calc_fan();
	set_fan();
}

//------------------------------------------------------------------------------

void scan_sensors()
{
	int i;
	int j;
	struct stat buf;
	int result;

	sensor_TC0P = NULL;
	sensor_TM0P = NULL;

	// get number of fans

	result = stat(fan1_min, &buf);
	if(result != 0)
	{
		printf("No fans detected, terminating!\n");
		exit(-1);
	}
	else
	{
		fan_count = 1;
	}

	result = stat(fan2_min, &buf);
	if(result != 0)
	{
		printf("Found 1 fan [CPU_MEM]");
	}
	else
	{
		fan_count = 2;
		printf("\rFound 2 fans [CPU_MEM, EXPANSION]");
	}

	result = stat(fan3_min, &buf);
	if (result != 0)
	{
		printf("\rFound 2 fans [CPU_MEM, EXPANSION]");
	}
	else
	{
		fan_count = 3;
		printf("\rFound 3 fans [CPU_MEM, EXPANSION, EXHAUST]");
	}

	result = stat(fan4_min, &buf);
	if (result != 0)
	{
		printf("\rFound 3 fans [CPU_MEM, EXPANSION, EXHAUST]");
	}
	else
	{
		fan_count = 4;
		printf("\rFound 4 fans [CPU_MEM, EXPANSION, EXHAUST, POWER_SUPPLY]");
	}

	printf("\n");

	// count number of sensors

	int count = 0;
	while(count < 100)	// more than 100 sensors is an error!
	{
		char fname[512];

		// sensor numbering start at 1
		sprintf(fname, "%s/temp%d_input", base_path, count + 1);
		result = stat(fname, &buf);

		if(result == 0)
		{
			++count;
		}
		else
		{
			break;		// done
		}
	}

	sensor_count = count;

	if(sensor_count > 0)
	{
		// Get sensor id, labels and descriptions, check exclude list

		if(sensors != NULL)
		{
			free(sensors);
		}

		sensors = malloc(sizeof(struct sensor) * sensor_count);
		assert(sensors != NULL);

		printf("Found %d sensors:\n", sensor_count);

		for(i = 0; i < sensor_count; ++i)
		{
			char fname[512];

			// set id, check exclude list and save file name
			sensors[i].id = i + 1;
			sensors[i].excluded = 0;
			sprintf(sensors[i].fname, "%s/temp%d_input", base_path, sensors[i].id);

			for(j = 0; j < MAX_EXCLUDE && exclude[j] != 0; ++j)
			{
				if(exclude[j] == sensors[i].id)
				{
					sensors[i].excluded = 1;
					break;
				}
			}

			// read label
			sprintf(fname, "%s/temp%d_label", base_path, sensors[i].id);

			sensors[i].name[0] = 0; // set zero length

			FILE *fp = fopen(fname, "r");
			if(fp == NULL)
			{
				printf("Error: Can't open %s\n", fname);
			}
			else
			{
				char key_buf[SENSKEY_MAXLEN];
				memset(key_buf, 0, SENSKEY_MAXLEN);

				int n = fread(key_buf, 1, SENSKEY_MAXLEN - 1, fp);
				if(n < 1)
				{
					printf("Error: Can't read  %s\n", fname);
				}
				else
				{
					char *p_endl = strrchr(key_buf, '\n');
					if(p_endl)
					{
						*p_endl = 0; 	// remove '\n'
					}
					strncpy(sensors[i].name, key_buf, SENSKEY_MAXLEN);
				}
				fclose(fp);
			}
		}

		for(i = 0; i < sensor_count; ++i)		// for each label found
		{
			if(! sensors[i].excluded)
			{
				// try to find TC0P and TM0P
				// if found, assign sensor_TC0P and sensor_TM0P for later use

				if(strcmp(sensors[i].name, "TC0P") == 0)
				{
					sensor_TC0P = &sensors[i];
					if (sensor_TC0P->value <= 0.0 && sensor_TC0P->value != 0.0)
					{
						sensor_TC0P->value *= -1;
					}
				}
				else if(strcmp(sensors[i].name, "TM0P") == 0)
				{
					sensor_TM0P = &sensors[i];
					if (sensor_TM0P->value <= 0.0 && sensor_TM0P->value != 0.0)
					{
						sensor_TM0P->value *= -1;
					}
				}
			}

			// print out sensor information.

			printf("\t%2d: ", sensors[i].id);

			int found = 0;
			for(j = 0; j < N_DESC && ! found; ++j)		// find in descriptions table
			{
				if(strcmp(sensors[i].name, sensor_desc[j].key) == 0)
				{
					found = 1;
					printf("%s - %s", sensor_desc[j].key, sensor_desc[j].desc);
				}
			}
			if(! found)
			{
				printf("%s - ?", sensors[i].name);
			}

			printf(" %s\n", sensors[i].excluded ? "   ***EXCLUDED***" : "");
		}
	}
	else
	{
		printf("No sensors detected, terminating!\n");
		exit(-1);
	}

	fflush(stdout);
}

//------------------------------------------------------------------------------

void logger()
{
	int i;

	if(log_level > 0)
	{
		printf("Speed: [CPUMEM %d] [EXPANSION %d] [EXHAUST %d] [POWER_SUPPLY %d], %sAVG: %.1fC" ,
			   fan1_speed,
			   fan2_speed,
			   fan3_speed,
			   fan4_speed,
			   fan_ctl == CTL_AVG ? "*" : " ",
			   temp_avg);

		if(sensor_TC0P != NULL)
		{
			printf(", %sTC0P: %.1fC" ,
				   fan_ctl == CTL_TC0P ? "*" : " ",
				   sensor_TC0P->value);
		}

		if(sensor_TM0P != NULL)
		{
			printf(", %sTM0P: %.1fC" ,
				   fan_ctl == CTL_TM0P ? "*" : " ",
				   sensor_TM0P->value);
		}

		if(log_level > 1)
		{
			printf(", Sensors: ");
			for(i = 0; i < sensor_count; ++i)
			{
				if(! sensors[i].excluded)
				{
					printf("%s:%.0f ", sensors[i].name, sensors[i].value);
				}
			}
		}

		printf("\n");
		fflush(stdout);
	}
}

//------------------------------------------------------------------------------

