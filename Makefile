#
# Makefile for macprofanctl
#
# Origin: macfanctld Mikael Strom, August 2010
#
# macprofanctld
# Aaron Blakely, June 2022

CC = gcc
CFLAGS = -Wall
SBIN_DIR = /usr/sbin
ETC_DIR = /etc
UNIT_DIR = /usr/lib/systemd/system

all: macprofanctld

macprofanctld: macprofanctl.c control.c config.c control.h config.h
	$(CC) $(CFLAGS) macprofanctl.c control.c config.c -o macprofanctld 

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

