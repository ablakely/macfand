/*
 * macfand - Fan control for Apple Computers
 *
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#ifndef UTIL_H
#define UTIL_H

void strlcpy(char *to, const char *from, int len);
int numPlaces(int n);
float ctof(float c);
int ctofi(int c);

#endif