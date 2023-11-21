/*
 * macfand - Fan control for Apple Computers
 *
 * Copyright (C) 2022 Aaron Blakely <aaron@ephasic.org>
*/

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "util.h"

#ifdef __STDC_VERSION__
#if __STDC_VERSION >= 199901L
void strlcpy(char *to, const char *from, int len)
{
    memccpy(to, from, '\0', len);
    to[len-1] = '\0';
}
#endif
#endif

int numPlaces(int n)
{
	if (n == 0) return 1;
	return floor(log10(abs(n))) + 1;
}

float ctof(float c)
{
    return (c * 9/5)+32;
}

int ctofi(int c)
{
    return (c * 9/5)+32;
}
