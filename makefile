all: crack dh

crack: crack.c sha256.c
	gcc -Wall -o crack crack.c sha256.c
dh: dh.c
	gcc -Wall -o dh dh.c
dh_scp: dh.c
	gcc -Wall -o dh dh.c
	scp dh.c aturner2@172.26.37.44:dh.c
	openssl sha256 dh.c
clean:
	-rm -f *.o crack dh
