#
# Makefile for macprofanctl
#
# Mikael Strom, August 2010
# 
# Updated for Mac Pro(1,1) Support
# Aaron Blakely, June 2022

CC = gcc
CFLAGS = -Wall
SBIN_DIR = $(DESTDIR)/usr/sbin
ETC_DIR = $(DESTDIR)/etc

all: macprofanctld

macprofanctld: macfanctl.c control.c config.c control.h config.h
	$(CC) $(CFLAGS) macfanctl.c control.c config.c -o macprofanctld 

clean:
	dh_testdir
	dh_clean
	rm -rf *.o macprofanctld

install:
	dh_installdirs
	chmod +x macprofanctld
	cp macprofanctld $(SBIN_DIR)
	cp macprofanctl.conf $(ETC_DIR)

uninstall:
	rm $(SBIN_DIR)/macprofanctld $(INITD_DIR)/macprofanctl $(ETC_DIR)/macprofanctl.conf

