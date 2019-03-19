CC=gcc
CFLAGS=-c -Wall

main: fs.o disk.o
fs.o: fs.c fs.h
	$(CC) $(CFLAGS) ./fs.c
disk.o: disk.c disk.h
	$(CC) $(CFLAGS) ./disk.c
clean:
	rm -f *.o
	$(MAKE) -C ./tests clean 
