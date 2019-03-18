CC=gcc
CFLAGS=-c -Wall

main: fs.o disk.o
	$(CC) ./test.c ./fs.o ./disk.o -o ./test.out
fs.o: fs.c fs.h
	$(CC) $(CFLAGS) ./fs.c
disk.o: disk.c disk.h
	$(CC) $(CFLAGS) ./disk.c