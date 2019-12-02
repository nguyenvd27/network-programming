CFLAGS = -c -Wall
CC = gcc
LIBS = -lm

all: user_management

user_management: main.o
	${CC} main.o -o user_management
main.o: main.c
	${CC} ${CFLAGS} main.c
clean:
	rm -f *.o *~
