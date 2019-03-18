#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs.h"
#define DATA_BLOCKS 4096
#define FILE_MAX 64
#define FILE_NAME_MAX 15
#define NFILE_DESCRIPTOR_MAX 32
#define get_nblocks(obj) (int) ceil((double)(sizeof(obj))/BLOCK_SIZE)

static struct SuperBlock {
	int fat_start;
	int root_dir_start;
	int data_block_start;
} sblock;
/*
 * fat[i] = {
 * -1 if data block i unallocated, 
 * x in [0, DATA_BLOCKS) if data block i allocated and is not eof			 
 * DATA_BLOCKS if data block i allocated and is eof
 * }
 */
static int fat[DATA_BLOCKS];

struct FileMetadata {
	char name[FILE_NAME_MAX+1];
	int data_block_i; // index of first data block
	int size;
	int valid; // -1 if invalid and 0 if valid
};

struct Directory {
	struct FileMetadata files[FILE_MAX];
};

static struct Directory root_dir;

struct FD_Entry {
	char name[FILE_NAME_MAX+1];
	int offset;
	int valid; // -1 if invalid and 0 if valid
};
static struct FD_Entry fildes_arr[NFILE_DESCRIPTOR_MAX];

static int batch_block_write(int block_start, char* buf, size_t size, int (*get_next_block)(int)) {
	int block = block_start;
	int nbytes_written = 0;
	while (nbytes_written != size) {
		if (block_write(block, buf) == -1)
			break;
		int needed = (size - nbytes_written > BLOCK_SIZE) ? BLOCK_SIZE : size - nbytes_written;
		buf += needed;
		nbytes_written += needed;
		block = get_next_block(block);
	}
	return nbytes_written;
}

static int batch_block_read(int block_start, char* buf, size_t size, int (*get_next_block)(int)) {
	int block = block_start;
	int nbytes_read = 0;
	char* tmp_buf = (char*) malloc(BLOCK_SIZE);
	while (nbytes_read != size) {
		if (block_read(block, tmp_buf) == -1)
			break;
		int needed = (size - nbytes_read > BLOCK_SIZE) ? BLOCK_SIZE : size - nbytes_read;
		memcpy(buf, tmp_buf, needed);
		buf += needed;
		nbytes_read += needed;
		block = get_next_block(block);
	}
	free(tmp_buf);
	return nbytes_read;
}

static int incr(int x) { return x + 1; }
static int write_metadata(int block_start, char* buf, size_t size) {
	return batch_block_write(block_start, buf, size, incr);
}
static int read_metadata(int block_start, char* buf, size_t size) {
	return batch_block_read(block_start, buf, size, incr);
}

int make_fs(char* disk_name) {
	if (make_disk(disk_name) == -1)
		return -1;
	struct SuperBlock fs_sblock;
	fs_sblock.fat_start = get_nblocks(fs_sblock);
	int fs_fat[DATA_BLOCKS];
	for (int i = 0; i < DATA_BLOCKS; i++) {
		fs_fat[i] = -1;
	}
	fs_sblock.root_dir_start = fs_sblock.fat_start + get_nblocks(fs_fat);
	struct Directory fs_root_dir;
	for (int i = 0; i < FILE_MAX; i++) {
		fs_root_dir.files[i].valid = -1;
	}
	fs_sblock.data_block_start = fs_sblock.root_dir_start + get_nblocks(fs_root_dir);

	if (open_disk(disk_name) == -1 ||
		write_metadata(0, (char*) (&fs_sblock), sizeof(fs_sblock)) != sizeof(fs_sblock) ||
		write_metadata(fs_sblock.fat_start, (char*) (&fs_fat), sizeof(fs_fat)) != sizeof(fs_fat) ||
		write_metadata(fs_sblock.root_dir_start, (char*) (&fs_root_dir), sizeof(fs_root_dir)) != sizeof(fs_root_dir) ||
		close_disk() == -1)
		return -1;
	return 0;
}

int mount_fs(char* disk_name) {
	if (open_disk(disk_name) == -1 ||
		read_metadata(0, (char*) (&sblock), sizeof(sblock)) != sizeof(sblock) ||
		read_metadata(sblock.fat_start, (char*) (&fat), sizeof(fat)) != sizeof(fat) ||
		read_metadata(sblock.root_dir_start, (char*) (&root_dir), sizeof(root_dir)) != sizeof(root_dir))
		return -1;
	for (int i = 0; i < NFILE_DESCRIPTOR_MAX; i++)
		fildes_arr[i].valid = -1;
	return 0;
}

int umount_fs(char* disk_name) {
	if (write_metadata(0, (char*) (&sblock), sizeof(sblock)) != sizeof(sblock) ||
		write_metadata(sblock.fat_start, (char*) (&fat), sizeof(fat)) != sizeof(fat) ||
		write_metadata(sblock.root_dir_start, (char*) (&root_dir), sizeof(root_dir)) != sizeof(root_dir) ||
		close_disk() == -1)
		return -1;
	return 0;
}

int fs_open(char* name) {
	for (int i = 0; i < NFILE_DESCRIPTOR_MAX; i++) {
		if (fildes_arr[i].valid == -1) {
			strcpy(fildes_arr[i].name, name);
			fildes_arr[i].offset = 0;
			fildes_arr[i].valid = 0;
			return 0;
		}
	}
	return -1;
}

int fs_close(int fildes) {
	if (fildes_arr[fildes].valid == -1)
		return -1;
	fildes_arr[fildes].valid = -1;
	return 0;
}

static int fs_find_file(char* name) {
	for (int i = 0; i < FILE_MAX; i++) {
		if (root_dir.files[i].valid == 0 && strcmp(root_dir.files[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

// return: -1 if invalid length and 0 if valid
static int strlen_valid(char* name) {
	for (int i = 0; i < FILE_NAME_MAX + 1; i++) {
		if (name[i] == '\0')
			return 1;
	}
	return -1;
}

int fs_create(char* name) {
	if (strlen_valid(name) == -1 || fs_find_file(name) == -1)
		return -1;
	for (int i = 0; i < FILE_MAX; i++) {
		if (root_dir.files[i].valid == -1) {
			strcpy(root_dir.files[i].name, name);
			root_dir.files[i].size = 0;
			root_dir.files[i].valid = 0;
			return 0;
		}
	}
	return -1;
}

int fs_delete(char* name) {
	int file_i = fs_find_file(name);
	if (file_i == -1)
		return -1;
	for (int i = 0; i < NFILE_DESCRIPTOR_MAX; i++) {
		if (strcmp(fildes_arr[i].name, name) == 0)
			return -1;
	}
	if (root_dir.files[file_i].size == 0)
		return 0;

	int curr_block = root_dir.files[file_i].data_block_i;
	while (curr_block != DATA_BLOCKS) {
		int next_block = fat[curr_block];
		fat[curr_block] = -1;
		curr_block = next_block;
	}

	return 0;
}

int fs_read(int fildes, void* buf, size_t nbyte);
int fs_write(int fildes, void* buf, size_t nbyte);
int fs_get_filesize(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);
