#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	int ret = 0;
	char disk_name[] = "fs2";
	char file_name[] = "file2";

	// Mount filesystem from basic_fs_02.c
	ret = mount_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: mount_fs failed\n");
	}

	// open file from basic_fs_02.c
	int fildes_file1 = fs_open(file_name);
	if(fildes_file1 < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// create another file (there should be one from basic_fs_02.c)
	char file2[] = "file3";
	ret = fs_create(file2);
	if(ret != 0) {
		printf("ERROR: fs_create failed\n");
	}

	int fildes_file2 = fs_open(file2);
	if(fildes_file2 < 0) {
		printf("ERROR: fs_open failed\n");
	}


	// transfer contents of file2 to file3
	int len = fs_get_filesize(fildes_file1);
	char *buffer = (char*)malloc(len*sizeof(char));

	ret = fs_read(fildes_file1,buffer,len);
	if(ret != len) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}
	
	ret = fs_write(fildes_file2,buffer,len);
	if(ret != len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	// truncate file1
	ret = fs_truncate(fildes_file1,0);
	if(ret != 0) {
		printf("ERROR: fs_truncate failed\n");
	}

	// make sure file sizes are correct
	int len1 = fs_get_filesize(fildes_file1);
	int len2 = fs_get_filesize(fildes_file2);

	if(len1 != 0) {
		printf("ERROR: file1 has incorrect size\n");
	}
	if(len2 != len) {
		printf("ERROR: file2 has incorrect size\n");
	}

	ret = fs_close(fildes_file1);
	ret = fs_close(fildes_file2);
	ret = umount_fs(disk_name);

	// done!
	return 0;
}