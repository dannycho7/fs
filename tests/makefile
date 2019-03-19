CC=gcc
CFLAGS=-lm

all: disk.o basic_fs_01 basic_fs_02 basic_fs_03 basic_fs_04 basic_fs_05

basic_fs_01: 
	$(CC) $(CFLAGS) -o basic_fs_01.run basic_fs_01.c fs.o disk.o

basic_fs_02: 
	$(CC) $(CFLAGS) -o basic_fs_02.run basic_fs_02.c fs.o disk.o

basic_fs_03: 
	$(CC) $(CFLAGS) -o basic_fs_03.run basic_fs_03.c fs.o disk.o

basic_fs_04: 
	$(CC) $(CFLAGS) -o basic_fs_04.run basic_fs_04.c fs.o disk.o

basic_fs_05: 
	$(CC) $(CFLAGS) -o basic_fs_05.run basic_fs_05.c fs.o disk.o

disk.o:
	gcc -c disk.c

clean:
	rm disk.o *.run fs1 fs2
