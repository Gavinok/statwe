# CC: gcc
# BINARY: statwe
# statwe version
CC=gcc
VERSION = 2.1
CFLAGS = -Os -Wall -pedantic -Wextra -s -lasound -lX11
BINARY = statwe

all: statwe
	$(CC)  $(BINARY).c $(CFLAGS) -o $(BINARY)

statwe: statwe.c
	$(CC)  $(BINARY).c $(CFLAGS) -o $(BINARY)

uninstall:
	rm -f /usr/local/bin/$(BINARY)\

install: all
	mkdir -p /usr/local/bin
	cp -f statwe /usr/local/bin
	chmod 755 /usr/local/bin/$(BINARY)
	mkdir -p /usr/local/$(BINARY)

clean:
	rm -f $(BINARY)
