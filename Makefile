# CC: gcc
# BINARY: statwe
# statwe version
CC=gcc
VERSION = 2.1
INCS = -s -lasound -lX11
BINARY = statwe
CFLAGS = -Wpedantic -Wall -Wconversion -Wextra  -Wno-deprecated-declarations -Os ${INCS}

all: statwe

statwe: statwe.c
	$(CC) $(BINARY).c -O2 -g $(CFLAGS) -o $(BINARY)

uninstall:
	rm -f /usr/local/bin/$(BINARY)\

install: all
	mkdir -p /usr/local/bin
	cp -f statwe /usr/local/bin
	chmod 755 /usr/local/bin/$(BINARY)
	mkdir -p /usr/local/$(BINARY)

clean:
	rm -f $(BINARY)
