/*
 * applesmc.h - Fan control daemon for Apple Computers
 *
 * macfand - Mac Fan Control Daemon
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#ifndef LOGGER_H_
#define LOGGER_H_

void logger(struct applesmc *smc, struct mfdconfig cfg, int fancy, int usef);

#endif