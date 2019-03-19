#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 16777216

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs7";
	char file_name[] = "file1";
	char* buffer;

	// Make filesystem.
	ret = make_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: make_fs failed\n");
	}

	// Mount filesystem.
	ret = mount_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: mount_fs failed\n");
	}

	// create file
	ret = fs_create(file_name);
	if(ret != 0) {
		printf("ERROR: fs_create failed\n");
	}

	// open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// write to file
	ret = fs_write(fildes,"ab",2);
	if(ret != 2) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_lseek(fildes, 0);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	// read what we just wrote into buffer
	buffer = malloc(2);
	ret = fs_read(fildes,buffer,2);
	if(ret != 2) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp("ab",buffer,2);
	if(ret != 0) {
		printf("ERROR: data2 read does not match data2 written!\n");
	}

	ret = fs_lseek(fildes, 2);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	ret = fs_write(fildes,"cd",2);
	if(ret != 2) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_lseek(fildes, 0);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	free(buffer);
	buffer = malloc(4);
	// read what we just wrote into buffer
	ret = fs_read(fildes,buffer,4);
	if(ret != 4) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp("abcd",buffer,4);
	if(ret != 0) {
		printf("ERROR: data2 read does not match data2 written!\n");
	}

	ret = fs_lseek(fildes, 3);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	ret = fs_write(fildes,"lmao",4);
	if(ret != 4) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_lseek(fildes, 0);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	// read what we just wrote into buffer
	free(buffer);
	buffer = malloc(7);
	ret = fs_read(fildes,buffer,7);
	if(ret != 7) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp("abclmao",buffer, 7);
	if(ret != 0) {
		printf("ERROR: data2 read does not match data2 written!\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// unmount file system
	ret = umount_fs(disk_name);
	if(ret < 0) {
		printf("ERROR: umount_fs failed\n");
	}

	// done!
	return 0;
}