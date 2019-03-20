#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 16777216

int main() {
	int fildes = 0;
	int fd = 0, fd2 = 0;
	int ret = 0;
	char disk_name[] = "fs3";
	char file_name[] = "file1";
	char file_name2[] = "file2";

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
	char* data = malloc(MAX_FILE_SIZE);
	memset(data, 'a', MAX_FILE_SIZE);
	int len = MAX_FILE_SIZE;
	ret = fs_write(fildes,data,len);
	if(ret != len) {
		printf("Expected: %d\n", len);
		printf("Returned: %d\n", ret);
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
	char* buffer = malloc(MAX_FILE_SIZE);
	ret = fs_read(fildes,buffer,len);
	if(ret != len) {
		printf("Expected: %d\n", len);
		printf("Returned: %d\n", ret);
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

	ret = fs_create(file_name2);
	if (ret != 0) {
		printf("ERROR: fs_create failed\n");
	}

	fd = fs_open(file_name2);
	if (fd < 0) {
		printf("ERROR: fs_open failed\n");
	}

	char message[5000];
	int len2 = 5000;
	memset(message, 'a', len2);
	ret = fs_write(fd, message, len2);
	if (ret > 0) {
		printf("ERROR: write was successful eventhough it should have failed\n");
	}

	// re-open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	ret = fs_truncate(fildes, MAX_FILE_SIZE - BLOCK_SIZE);
	if (ret != 0) {
		printf("ERROR: fs_truncate failed\n");
	}

	// read what we just wrote into buffer
	//char buffer[MAX_FILE_SIZE];
	ret = fs_read(fildes,buffer,len);
	if(ret != len - BLOCK_SIZE) {
		printf("Expected: %d\n", len - BLOCK_SIZE);
		printf("Returned: %d\n", ret);
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(data,buffer,len - BLOCK_SIZE);
	if(ret != 0) {
		printf("ERROR: data read does not match data written!\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	fd = fs_open(file_name2);
	if (fd < 0) {
		printf("ERROR: fs_open failed\n");
	}

	ret = fs_write(fd, message, len2);
	if (ret < 0) {
		printf("ERROR: fs_write was unsuccessful\n");
	}

	ret = fs_lseek(fd, 0);
	if (ret != 0) {
		printf("ERROR: fs_lseek failed\n");
	}

	char buffer2[5000];
	ret = fs_read(fd, buffer2, len2);
	if (ret != BLOCK_SIZE) {
		printf("Expected: %d\n", BLOCK_SIZE);
		printf("Returned: %d\n", ret);
		printf("ERROR: wrote wrong bytes than it should have\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(message, buffer2, BLOCK_SIZE);
	if(ret != 0) {
		printf("Expected: %s\n\n", message);
		printf("Returned: %s\n", buffer2);
		printf("ERROR: data read does not match data written!\n");
	}

	fd2 = fs_open(file_name2);
	if (fd2 < 0) {
		printf("ERROR: fs_open failed\n");
	}

	ret = fs_truncate(fd2, 0);
	if (ret != 0) {
		printf("ERROR: fs_truncate failed\n");
	}

	ret = fs_get_filesize(fd);
	if (ret != 0) {
		printf("ERROR: filesize is incorrect after truncation\n");
	}

	ret = fs_read(fd, buffer2, len2);
	if (ret != 0) {
		printf("ERROR: read bytes eventhough it shouldn't have\n");
	}

	ret = fs_read(fd2, buffer2, len2);
	if (ret != 0) {
		printf("ERROR: read bytes eventhough it shouldn't have\n");
	}

	ret = fs_write(fd, "abcd", 4);
	if (ret != 0) {
		printf("Wrote bytes when it shouldn't have\n");
	}

	ret = fs_get_filesize(fd);
	if (ret != BLOCK_SIZE) {
		printf("ERROR: filesize was not increased by holes in fs_write.\n");
	}

	ret = fs_read(fd2, buffer2, len2);
	if (ret != BLOCK_SIZE) {
		printf("Expected: %d\n", BLOCK_SIZE);
		printf("Returned: %d\n", ret);
		printf("ERROR: Didn't read the proper number of bytes\n");
	}

	// close file
	ret = fs_close(fd2);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// close file
	ret = fs_close(fd);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// unmount file system
	ret = umount_fs(disk_name);
	if(ret < 0) {
		printf("ERROR: umount_fs failed\n");
	}

	free(data);
	free(buffer);

	// done!
	return 0;
}
