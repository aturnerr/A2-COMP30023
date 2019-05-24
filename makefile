all: crack dh

crack: crack.c sha256.c
	gcc -Wall -o crack crack.c sha256.c
dh: dh.c
	gcc -Wall -o dh dh.c
