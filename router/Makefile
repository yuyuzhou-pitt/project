# Makefile for lsrp-router

CC = gcc

all:lsrp-router

lsrp-router:
	${CC} -g -pthread -o lsrp-router lsrp-router.c socket/server.c socket/client.c socket/libsocket.c socket/getaddrinfo.c packet/neighbor.c packet/hello.c packet/ping.c packet/lsa.c packet/data.c packet/libqueue.c packet/dijkstra.c config/config.c config/libfile.c config/liblog.c ../endsys/checksum.c

clean:
	rm lsrp-router .1* ../.1* .*graph.ospf
