#include <stdio.h>
#include "fs.h"

int main() {
	printf("%d\n", make_fs("begin.filesys"));
	printf("%d\n", mount_fs("begin.filesys"));
	printf("%d\n", fs_create("file1"));
	int wfd = fs_open("file1");
	char* buf = malloc(6);
	buf[5] = '\0';
	printf("%d\n", fs_write(wfd, "hello", 6));	
	int rfd = fs_open("file1");
	printf("%d\n", fs_read(rfd, buf, 8));
	printf("read: %s\n", buf);	
	printf("%d\n", fs_close(wfd));
	printf("%d\n", fs_close(rfd));
	printf("%d\n", fs_delete("file1"));	
	printf("%d\n", umount_fs("begin.filesys"));
}