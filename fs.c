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
	char* tmp_buf = malloc(BLOCK_SIZE);
	while (nbytes_written != size) {
		int needed = (size - nbytes_written > BLOCK_SIZE) ? BLOCK_SIZE : size - nbytes_written;
		if (block_read(block, tmp_buf) == -1)
			break;
		memcpy(tmp_buf, buf, needed);
		if (block_write(block, tmp_buf) == -1)
			break;
		buf += needed;
		nbytes_written += needed;
		block = get_next_block(block);
	}
	free(tmp_buf);
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
			return i;
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

static int fat_next(int block_i) {
	int data_block_i = block_i - sblock.data_block_start;
	int next_data_block_i = fat[data_block_i];
	if (next_data_block_i < 0 || next_data_block_i >= DATA_BLOCKS)
		return -1;
	return next_data_block_i + sblock.data_block_start;
}

static int fat_get_free_block() {
	for (int i = 0; i < DATA_BLOCKS; i++) {
		if (fat[i] == -1)
			return i + sblock.root_dir_start;
	}
	return -1;
}

static int fat_next_alloc(int block_i) {
	int free_block_i = fat_get_free_block();
	if (free_block_i == -1)
		return -1;
	if (block_i != -1)
		fat[block_i - sblock.root_dir_start] = free_block_i;
	fat[free_block_i] = DATA_BLOCKS; // eof
	return free_block_i;
}

static int fildes_get_block_i(int fildes, struct FileMetadata fm) {
	int offset = fildes_arr[fildes].offset;
	if (fs_get_filesize(fildes) == 0)
		return -1;
	int block_i = fm.data_block_i + sblock.data_block_start;
	while (offset >= BLOCK_SIZE) {
		if (block_i == -1 && (block_i = fat_next_alloc(block_i)) == -1) {
			return -1;
		}
		offset -= BLOCK_SIZE;
		block_i = fat_next(block_i);
	}
	return block_i;
}

int fs_read(int fildes, void* buf, size_t nbyte) {
	if (fildes_arr[fildes].valid == -1)
		return -1;
	struct FileMetadata fm = root_dir.files[fs_find_file(fildes_arr[fildes].name)];
	if (fildes_arr[fildes].offset > fs_get_filesize(fildes))
		return 0;
	size_t extra_to_read = fildes_arr[fildes].offset % BLOCK_SIZE;
	size_t bytes_to_read = nbyte + extra_to_read;
	void* tmp_buf = malloc(bytes_to_read);
	int bytes_read = batch_block_read(fildes_get_block_i(fildes, fm), tmp_buf, bytes_to_read, fat_next);
	bytes_read -= extra_to_read;
	memcpy(buf, tmp_buf + extra_to_read, bytes_read);
	free(tmp_buf);
	fildes_arr[fildes].offset += bytes_read;
	return bytes_read;
}

int fs_write(int fildes, void* buf, size_t nbyte) {
	if (fildes_arr[fildes].valid == -1)
		return -1;
	if (nbyte == 0)
		return 0;
	size_t extra_to_write = fildes_arr[fildes].offset % BLOCK_SIZE;
	size_t bytes_to_write = nbyte + extra_to_write;

	void* tmp_buf = malloc(bytes_to_write);
	int file_i = fs_find_file(fildes_arr[fildes].name);
	if (fs_get_filesize(fildes) == 0)
		root_dir.files[file_i].data_block_i = fat_next_alloc(-1);
	int block_i = fildes_get_block_i(fildes, root_dir.files[file_i]);
	if (block_i == -1)
		return -1; // TODO: What happens ...? invalid fildes
	if (block_read(block_i, tmp_buf) == -1)
		return 0;
	memcpy(tmp_buf + extra_to_write, buf, nbyte);

	int bytes_written = batch_block_write(block_i, tmp_buf, bytes_to_write, fat_next_alloc);
	bytes_written -= extra_to_write;
	free(tmp_buf);
	root_dir.files[file_i].size += bytes_written;
	fildes_arr[fildes].offset += bytes_written;
	return bytes_written;
}

int fs_get_filesize(int fildes) {
	if (fildes_arr[fildes].valid == -1)
		return -1;
	return root_dir.files[fs_find_file(fildes_arr[fildes].name)].size;
}

int fs_lseek(int fildes, off_t offset) {
	if (fildes_arr[fildes].valid == -1 || offset > fs_get_filesize(fildes) || offset < 0)
		return -1;
	fildes_arr[fildes].offset = offset;
	return 0;
}

int fs_truncate(int fildes, off_t length) {
	if (fildes_arr[fildes].valid == -1 || length > fs_get_filesize(fildes) || length < 0)
		return -1;
	int file_i = fs_find_file(fildes_arr[fildes].name);
	int del_block_offset = get_nblocks(length) + 1;
	int curr_block_offset = 1;
	int curr_block = root_dir.files[file_i].data_block_i;

	while (curr_block != DATA_BLOCKS) {
		int next_block = fat[curr_block];
		if (curr_block_offset <= del_block_offset)
			fat[curr_block] = -1;
		curr_block = next_block;
		curr_block_offset += 1;
	}
	root_dir.files[file_i].size = length;
	return 0;
}
