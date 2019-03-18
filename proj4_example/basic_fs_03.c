#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs1";
	char file_name1[] = "file1";
	char file_name2[] = "file2";

	// Mount filesystem from basic_fs_01.c
	ret = mount_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: mount_fs failed\n");
	}

	// open file from basic_fs_01.c
	fildes = fs_open(file_name1);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// this is the string we wrote to file1 in basic_fs_01.c
	// Lets make sure it was actually written into the file correctly
	char data[] = "This is my data";
	int len = strlen(data);

	int filesize = fs_get_filesize(fildes);
	if(filesize != len) {
		printf("ERROR: /home/fs/file1 does not have correct size!\n");
	}

	char buffer[20];
	ret = fs_read(fildes,buffer,len);
	if(ret != len) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	ret = strncmp(data,buffer,len);
	if(ret != 0) {
		printf("ERROR: /home/fs/file1 does not have correct data!\n");
	}

	// close file descriptor
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// unmount fs from basic_fs_01.c
	ret = umount_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: umount_fs failed\n");
	}

	// mount fs from basic_fs_02.c
	char disk_name2[] = "fs2";
	ret = mount_fs(disk_name2);
	if(ret != 0) {
		printf("ERROR: mount_fs failed on mounting disk_name2\n");
	}

	// open file created in basic_fs_02.c
	fildes = fs_open(file_name2);

	// seek to position right before 'moose'
	ret = fs_lseek(fildes,4094);
	if(ret != 0) {
		printf("ERROR: fs_lseek failed\n");
	}

	// read bytes where 'moose' should be
	ret = fs_read(fildes,buffer,5);
	if(ret != 5) {
		printf("ERROR: fs_read2 failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	char moose[] = "moose";
	ret = strncmp(buffer,moose,5);
	if(ret != 0) {
		printf("ERROR: data read does not match data written!\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// unmount file system
	ret = umount_fs(disk_name2);
	if(ret < 0) {
		printf("ERROR: umount_fs failed\n");
	}

	// done!
	return 0;
}