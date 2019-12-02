CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: client server

client: tcp_client.c
	${CC} tcp_client.c  -o client

server: tcp_server.c
	${CC} tcp_server.c  -o server

clean:
	rm -f *.o *~
