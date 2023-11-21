/*
 * macfand - Fan control for Apple Computers
 *
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#ifndef UTIL_H
#define UTIL_H

#ifdef __GLIBC_MINOR__
#if __GLIBC_MINOR >= 38
void strlcpy(char *to, const char *from, int len);
#endif
#endif

int numPlaces(int n);
float ctof(float c);
int ctofi(int c);

#endif
