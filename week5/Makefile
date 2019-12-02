CFLAGS = -c -Wall
CC = gcc
LIBS = -lm

all: server client

server: TCPServer.o
	${CC} TCPServer.o -o server
TCPServer.o: TCPServer.c
	${CC} ${CFLAGS} TCPServer.c

client: TCPClient.o
	${CC} TCPClient.o -o client
TCPClient.o: TCPClient.c
	${CC} ${CFLAGS} TCPClient.c
clean:
	rm -f *.o *~

