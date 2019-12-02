CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: client server

client: TCPClient.c
	${CC} TCPClient.c  -o client

server: TCPServer.c
	${CC} TCPServer.c  -o server

clean:
	rm -f *.o *~
