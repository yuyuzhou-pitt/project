# Makefile for lsrp-server, lsrp-client, lsrp-router
# gcc -g -pthread -o lsrp-router lsrp-router.c socket/server.c socket/client.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c packet/hello.c packet/ping.c packet/lsa.c config/config.c config/libfile.c config/liblog.c ../endsys/checksum.c

CC = gcc

all:lsrp-router

lsrp-router:
	${CC} -g -pthread -o lsrp-router lsrp-router.c socket/server.c socket/client.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c packet/hello.c packet/ping.c packet/lsa.c packet/libqueue.c config/config.c config/libfile.c config/liblog.c ../endsys/checksum.c

#lsrp-server:
#	${CC} -g -pthread -o lsrp-server lsrp-server.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c packet/hello.c config/config.c config/libfile.c ../endsys/checksum.c

#lsrp-client:
#	${CC} -g -pthread -o lsrp-client lsrp-client.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c packet/hello.c config/config.c config/libfile.c ../endsys/checksum.c

clean:
	rm lsrp-router .1*
#	rm lsrp-server lsrp-client
