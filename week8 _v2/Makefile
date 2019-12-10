CFLAGS = -c -Wall
CC = gcc
LIBS = -lm

all: server client

server: Server.c
	${CC} -o server Server.c
client: Client.c
	${CC} -o client Client.c
clean:
	rm -f *.o *~

