CFLAGS = -c -Wall
CC = gcc
LIBS = -lm

all: server client

server: UDP_Server.o
	${CC} UDP_Server.o -o server
UDP_Server.o: UDP_Server.c
	${CC} ${CFLAGS} UDP_Server.c

client: UDP_Client.o
	${CC} UDP_Client.o -o client
UDP_Client.o: UDP_Client.c
	${CC} ${CFLAGS} UDP_Client.c
clean:
	rm -f *.o *~

