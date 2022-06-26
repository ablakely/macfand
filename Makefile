#
# Makefile for macfan
#
# Origin: macfanctld Mikael Strom, August 2010
#
# macfand
# Aaron Blakely, June 2022

VERSION = 1.0.0

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = gcc
CFLAGS = -Wall
ETC_DIR = /etc

all: macfand

macfand: macfan.c control.c config.c control.h config.h
	$(CC) $(CFLAGS) macfan.c control.c config.c -o macfand 

clean:
	dh_testdir
	dh_clean
	rm -rf *.o macfand

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp macfand $(DESTDIR)$($PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/macfand
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < macfand.1 > $(DESTDIR)($MANPREFIX)/man1/macfand.1
	chmod 644 $(DESTDIR)($MANPREFIX)/man1/macfand.1


	@echo "Done!"
	@echo " "
	@echo "Be sure to enable macfand with systemd. This can be done with:"
	@echo "   sudo systemctl enable --now macfand"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/macfand
	$(ETC_DIR)/macfan.conf

.PHONY: all clean install uninstall