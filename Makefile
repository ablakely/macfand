#
# Makefile for macfan
#
# Origin: macfanctld Mikael Strom, August 2010
#
# macfand
# Aaron Blakely, June 2022

VERSION = 1.1.0b
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
CC = gcc
CFLAGS =-g3 -std=gnu99 -c -Iinclude -lconfig -lm
OBJ=./build
SRC=./src
EXEC=macfand
BINFLAGS=-g -lconfig -lm
ETC_DIR = /etc
all: macfand

macfand:
	@rm -rf $(OBJ) && mkdir $(OBJ)
	$(CC) $(CFLAGS)  $(SRC)/config.c -o $(OBJ)/config.o 
	$(CC) $(CFLAGS)  $(SRC)/control.c -o $(OBJ)/control.o
	$(CC) $(CFLAGS)  $(SRC)/macfan.c -o $(OBJ)/macfan.o
	$(CC) -o $(EXEC) $(OBJ)/*.o $(BINFLAGS)
	@echo "Build complete"

dev:
	@rm -rf $(OBJ) && mkdir $(OBJ)
	$(CC) $(CFLAGS) $(SRC)/conf.c     -o $(OBJ)/config.o
	$(CC) $(CFLAGS) $(SRC)/ctrl.c     -o $(OBJ)/control.o
	$(CC) $(CFLAGS) $(SRC)/macfand.c  -o $(OBJ)/macfan.o
	$(CC) $(CFLAGS) $(SRC)/util.c     -o $(OBJ)/util.o
	$(CC) $(CFLAGS) $(SRC)/applesmc.c -o $(OBJ)/applesmc.o
	$(CC) -o $(EXEC) $(OBJ)/*.o $(BINFLAGS)


clean:
	rm -rf build macfand

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp macfand $(DESTDIR)$($PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/macfand
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/MFDVERSION/$(VERSION)/g" < macfand.1 > $(DESTDIR)($MANPREFIX)/man1/macfand.1
	chmod 644 $(DESTDIR)($MANPREFIX)/man1/macfand.1
	cp macfand.conf $(ETC_DIR)


	@echo "Done!"
	@echo " "
	@echo "Be sure to enable macfand with systemd. This can be done with:"
	@echo "   sudo systemctl enable --now macfand"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/macfand
	$(ETC_DIR)/macfan.conf

.PHONY: clean install uninstall