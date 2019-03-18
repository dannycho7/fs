#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs1";
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
	char data[] = "This is my data";
	int len = strlen(data);
	ret = fs_write(fildes,data,len);
	if(ret != len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// re-open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// read what we just wrote into buffer
	char buffer[4096];
	ret = fs_read(fildes,buffer,len);
	if(ret != len) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(data,buffer,len);
	if(ret != 0) {
		printf("ERROR: data read does not match data written!\n");
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