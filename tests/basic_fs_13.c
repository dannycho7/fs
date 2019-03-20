#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 16777216

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs13";
	char file_name[] = "abcdefghijklmnop"; // 16 length

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
    if(ret == 0) {
        printf("ERROR: fs_create succeeded when it should have failed.\n");
    }
    
	// unmount file system
	ret = umount_fs(disk_name);
	if(ret < 0) {
		printf("ERROR: umount_fs failed\n");
	}
	// done!
	return 0;
}
