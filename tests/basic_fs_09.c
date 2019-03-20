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

	// Create 16MB file
    printf("Creating data\n");
    const int MAX_SIZE = 4096 * 4096;
    char* data = (char*)malloc((MAX_SIZE + 1) * sizeof(char));
    memset(data, 7, MAX_SIZE + 1);

	ret = fs_write(fildes,data,MAX_SIZE + 1);
	if(ret != MAX_SIZE) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	ret = fs_lseek(fildes,4094);
	if(ret != 0) {
		printf("ERROR: fs_lseek failed\n");
	}

	char buffer[20];
	ret = fs_read(fildes,buffer,5);
	if(ret != 5) {
		printf("%d\n",ret);
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	ret = strncmp(data+4094,buffer,5);
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

    free(data);
	// done!
	return 0;
}
