#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs3";
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
    
    ret = fs_create(file_name);
    if(ret != 0) {
        printf("ERROR: fs_create failed\n");
    }

    fildes = fs_open(file_name);
    if(fildes < 0) {
        printf("ERROR: fs_open failed\n");
    }

    const int size = 10000;
    char buffer[size];
    memset(buffer, 69, size);
    ret = fs_write(fildes, buffer, size);
    if(ret != size) {
        printf("ERROR: fs_write did not return correct amount\n");
    }
    ret = fs_truncate(fildes, 7);
    if(ret != 0) {
        printf("ERROR: truncate failed\n");
    }

    ret = fs_lseek(fildes, fs_get_filesize(fildes));
    if(ret != 0) printf("ERROR: lseek failed\n");

    int len = fs_get_filesize(fildes);
    if(len != 7) {
        printf("Wrong length\n");
    }
  
    fs_close(fildes); 
    ret = fs_delete(file_name); 
    
	// unmount file system
	ret = umount_fs(disk_name);
	if(ret < 0) {
		printf("ERROR: umount_fs failed\n");
	}
	// done!
	return 0;
}
