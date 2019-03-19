#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 16777216

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs6";
	char file_name[] = "file1";

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
	char data1[] = "This is my data";
	int len = strlen(data1);
	ret = fs_write(fildes,data1,len);
	if(ret != len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	// delete file when there is an open fd
	ret = fs_delete(file_name);
	if(ret == 0) {
		printf("ERROR: fs_delete passed when it should have failed\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// delete file
	ret = fs_delete(file_name);
	if(ret != 0)
		printf("ERROR: fs_delete failed\n");

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
	char* data2 = malloc(MAX_FILE_SIZE);
	memset(data2, 'a', MAX_FILE_SIZE);
	len = MAX_FILE_SIZE;
	ret = fs_write(fildes,data2,len);
	if(ret != len) {
		printf("Expected: %d\n", len);
		printf("Returned: %d\n", ret);
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	// re-open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// read what we just wrote into buffer
	char* buffer = malloc(MAX_FILE_SIZE);
	ret = fs_read(fildes,buffer,len);
	if(ret != len) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(data2,buffer,len);
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