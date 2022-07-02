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

void strlcpy(char *to, const char *from, int len)
{
    //from[len-1] = '\0';
    memccpy(to, from, '\0', len);
    to[len-1] = '\0';
}

int numPlaces(int n) {
	if (n == 0) return 1;
	return floor(log10(abs(n))) + 1;
}