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

struct fanStruct
{
	int id;
	bool fanManualMode;
	char rname[PATH_MAX];
	char sensors[50][SENSKEY_MAXLEN];
	char fan_out_path[PATH_MAX];
	char fan_man_path[PATH_MAX];
	int minSpeed;
	int maxSpeed;
	int speed;
};

struct fanStruct *fans = (struct fans*)malloc(MAXFANS*sizeof(struct fanStruct));

char base_path[PATH_MAX];

int sensor_count = 0;
int fan_count = 0;
float temp_avg = 0;
int fans;

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
	int fd;
	int speed;

	for (int i = 0; i < fans; i++)
	{
		if (i == 0) { speed =}

		printf("Set fan %d [%s] to %d rpm\n", i+1, fan_out_path[i], )
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

