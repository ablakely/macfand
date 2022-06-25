#
# Makefile for macprofanctl
#
# Mikael Strom, August 2010
# 
# Updated for Mac Pro(1,1) Support
# Aaron Blakely, June 2022

CC = gcc
CFLAGS = -Wall
SBIN_DIR = /usr/sbin
ETC_DIR = /etc
UNIT_DIR = /usr/lib/systemd/system

all: macprofanctld

macprofanctld: macfanctl.c control.c config.c control.h config.h
	$(CC) $(CFLAGS) macfanctl.c control.c config.c -o macprofanctld 

clean:
	dh_testdir
	dh_clean
	rm -rf *.o macprofanctld

install:
	chmod +x macprofanctld
	cp macprofanctld $(SBIN_DIR)
	cp macprofanctl.conf $(ETC_DIR)
	cp macprofanctld.service $(UNIT_DIR)

uninstall:
	rm $(SBIN_DIR)/macprofanctld $(INITD_DIR)/macprofanctl $(ETC_DIR)/macprofanctl.conf $(UNIT_DIR)/macprofanctld.service

