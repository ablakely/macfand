#
# Makefile for macfan
#
# Origin: macfanctld Mikael Strom, August 2010
#
# macfand
# Aaron Blakely, June 2022

CC = gcc
CFLAGS = -Wall
SBIN_DIR = /usr/sbin
ETC_DIR = /etc
UNIT_DIR = /usr/lib/systemd/system

all: macfand

macfand: macfan.c control.c config.c control.h config.h
	$(CC) $(CFLAGS) macfan.c control.c config.c -o macfand 

clean:
	dh_testdir
	dh_clean
	rm -rf *.o macfand

install:
	chmod +x macfand
	cp macfand $(SBIN_DIR)
	cp macfan.conf $(ETC_DIR)
	cp macfand.service $(UNIT_DIR)

uninstall:
	rm $(SBIN_DIR)/macfand $(INITD_DIR)/macfan $(ETC_DIR)/macfan.conf $(UNIT_DIR)/macfand.service

