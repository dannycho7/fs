#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 16777216

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs8";
	char file_name[] = "file1";


	int data1_len = BLOCK_SIZE * 2;
	char* data1 = malloc(data1_len);
	memset(data1, 'a', data1_len);

	int data2_len = BLOCK_SIZE * 2;
	char* data2 = malloc(data2_len);
	memset(data2, 'b', data2_len);

	int data3_len = MAX_FILE_SIZE;
	char* data3 = malloc(data3_len);
	memset(data3, 'c', data3_len);

	int expected_data3_write_len = MAX_FILE_SIZE - data1_len - data2_len;
	char* expected = malloc(MAX_FILE_SIZE);
	memset(expected, 'a', data1_len);
	memset(expected + data1_len, 'b', data2_len);
	memset(expected + data1_len + data2_len, 'c', expected_data3_write_len);
	char* buf = malloc(MAX_FILE_SIZE);

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
	ret = fs_write(fildes,data1,data1_len);
	if(ret != data1_len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_write(fildes,data2,data2_len);
	if(ret != data2_len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_write(fildes,data3,data3_len);
	if(ret != expected_data3_write_len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_get_filesize(fildes);
	if (ret != MAX_FILE_SIZE)
		printf("ERROR: fs_get_filesize returned wrong nbytes.\n");
	

	ret = fs_lseek(fildes, 0);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	// read what we just wrote into buffer
	ret = fs_read(fildes,buf,MAX_FILE_SIZE);
	if(ret != MAX_FILE_SIZE) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(expected,buf,MAX_FILE_SIZE);
	if(ret != 0) {
		printf("ERROR: data read does not match data written!\n");
	}

	ret = fs_lseek(fildes, 0);
	if (ret != 0)
		printf("ERROR: fs_lseek failed\n");

	ret = fs_write(fildes,data1,data1_len);
	if(ret != data2_len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_get_filesize(fildes);
	if (ret != MAX_FILE_SIZE)
		printf("ERROR: fs_get_filesize returned wrong nbytes after writing to fd not pointing to eof.\n");
	

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

	// Mount filesystem again.
	ret = mount_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: mount_fs failed\n");
	}

	// open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	ret = fs_get_filesize(fildes);
	if (ret != MAX_FILE_SIZE)
		printf("ERROR: fs_get_filesize after re-mounting returned wrong nbytes.\n");
	
	// read what we just wrote into buffer
	ret = fs_read(fildes,buf,MAX_FILE_SIZE);
	if(ret != MAX_FILE_SIZE) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(expected,buf,MAX_FILE_SIZE);
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