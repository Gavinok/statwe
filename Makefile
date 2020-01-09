# CC: gcc
# BINARY: statwe
# statwe version
VERSION = 2.1

all: statwe
	gcc -o statwe statwe.c -std=c99 -Os -Wall -Wextra -s -lasound -lX11

statwe: statwe.c
	gcc -o statwe statwe.c -std=c99 -Os -Wall -Wextra -s -lasound -lX11

uninstall:
	rm -f /usr/local/bin/statwe\
		# /usr/local/bin/man1/statwe.1

install: all
	mkdir -p /usr/local/bin
	cp -f statwe /usr/local/bin
	chmod 755 /usr/local/bin/statwe
	mkdir -p /usr/local/statwe
	# sed "s/VERSION/${VERSION}/g" < statwe.1 > /usr/local/man1/statwe.1
	# chmod 644 /usr/local/man1/statwe.1
clean:
	rm -f statwe 
