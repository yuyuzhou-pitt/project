# Makefile for lsrp-server, lsrp-client, lsrp-router
# gcc -g -pthread -o lsrp-server lsrp-server.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c config/config.c ../endsys/checksum.c

CC = gcc

all:lsrp-server lsrp-client

lsrp-server:
	${CC} -g -pthread -o lsrp-server lsrp-server.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c config/config.c ../endsys/checksum.c

lsrp-client:
	${CC} -g -pthread -o lsrp-client lsrp-client.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c config/config.c ../endsys/checksum.c

clean:
	rm lsrp-server lsrp-client
