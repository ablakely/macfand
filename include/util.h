/*
 * macfand - Fan control for Apple Computers
 *
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#ifndef UTIL_H
#define UTIL_H

#ifdef __STDC_VERSION__
#if __STDC_VERSION >= 199901L
#ifdef __USE_XOPEN2K
void strlcpy(char *to, const char *from, int len);
#endif
#endif
#endif

int numPlaces(int n);
float ctof(float c);
int ctofi(int c);

#endif
