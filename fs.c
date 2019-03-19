#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs.h"
#include "fs_constants.h"
#include "fs_types.h"
#define get_nblocks(nbyte) (int) ceil((double)(nbyte)/BLOCK_SIZE)
#define get_nblocks_obj(obj) get_nblocks(sizeof(obj))

static SuperBlock sblock;
static FAT fat;
static Directory root_dir;
static FileDescriptorList fildes_list;

static int min(int a, int b) { return a <= b ? a : b; }
static int max(int a, int b) { return a >= b ? a : b; }

static int batch_block_write(int block_start, char* buf, size_t size, int (*get_next_block)(int)) {
	int block = block_start;
	int nbytes_written = 0;
	char* tmp_buf = malloc(BLOCK_SIZE);
	while (nbytes_written != size) {
		if (nbytes_written != 0)
			block = get_next_block(block);
		int needed = min(size - nbytes_written, BLOCK_SIZE);
		if (block < 0 || block_read(block, tmp_buf) == -1)
			break;
		memcpy(tmp_buf, buf, needed);
		if (block < 0 || block_write(block, tmp_buf) == -1)
			break;
		buf += needed;
		nbytes_written += needed;
	}
	free(tmp_buf);
	return nbytes_written;
}

static int batch_block_read(int block_start, char* buf, size_t size, int (*get_next_block)(int)) {
	int block = block_start;
	int nbytes_read = 0;
	char* tmp_buf = (char*) malloc(BLOCK_SIZE);
	while (nbytes_read != size) {
		if (nbytes_read != 0)
			block = get_next_block(block);
		if (block < 0 || block_read(block, tmp_buf) == -1)
			break;
		int needed = min(size - nbytes_read, BLOCK_SIZE);
		memcpy(buf, tmp_buf, needed);
		buf += needed;
		nbytes_read += needed;
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
	SuperBlock fs_sblock;
	fs_sblock.fat_start = get_nblocks_obj(fs_sblock);
	int fs_fat[DATA_BLOCKS];
	for (int i = 0; i < DATA_BLOCKS; i++) {
		fs_fat[i] = -1;
	}
	fs_sblock.root_dir_start = fs_sblock.fat_start + get_nblocks_obj(fs_fat);
	Directory fs_root_dir;
	for (int i = 0; i < FILE_MAX; i++) {
		fs_root_dir.files[i].valid = -1;
	}
	fs_sblock.data_block_start = fs_sblock.root_dir_start + get_nblocks_obj(fs_root_dir);

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
		fildes_list[i].valid = -1;
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
		if (fildes_list[i].valid == -1) {
			strcpy(fildes_list[i].name, name);
			fildes_list[i].offset = 0;
			fildes_list[i].valid = 0;
			return i;
		}
	}
	return -1;
}

int fs_close(int fildes) {
	if (fildes_list[fildes].valid == -1)
		return -1;
	fildes_list[fildes].valid = -1;
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
	if (strlen_valid(name) == -1 || fs_find_file(name) != -1)
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
		if (fildes_list[i].valid == 0 && strcmp(fildes_list[i].name, name) == 0)
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
			return i;
	}
	return -1;
}

static int fat_next_alloc(int block_i) {
	int data_block_i = block_i - sblock.data_block_start;
	if (block_i != -1 && fat[data_block_i] >= 0 && fat[data_block_i] < DATA_BLOCKS)
		return fat[data_block_i] + sblock.data_block_start;
	int free_data_block_i = fat_get_free_block();
	if (free_data_block_i == -1)
		return -1;
	if (block_i != -1)
		fat[data_block_i] = free_data_block_i;
	fat[free_data_block_i] = DATA_BLOCKS; // eof
	return free_data_block_i + sblock.data_block_start;
}

static int fildes_get_block_i(int fildes, FileMetadata fm) {
	int offset = fildes_list[fildes].offset;
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
	if (fildes_list[fildes].valid == -1)
		return -1;
	FileMetadata fm = root_dir.files[fs_find_file(fildes_list[fildes].name)];
	if (fildes_list[fildes].offset >= fs_get_filesize(fildes))
		return 0;
	size_t extra_to_read = fildes_list[fildes].offset % BLOCK_SIZE;
	size_t bytes_to_read = min(nbyte, fm.size - fildes_list[fildes].offset) + extra_to_read;
	void* tmp_buf = malloc(bytes_to_read);
	int bytes_read = batch_block_read(fildes_get_block_i(fildes, fm), tmp_buf, bytes_to_read, fat_next);
	bytes_read -= extra_to_read;
	memcpy(buf, tmp_buf + extra_to_read, bytes_read);
	free(tmp_buf);
	fildes_list[fildes].offset += bytes_read;
	return bytes_read;
}

static int get_nblock_size(int file_i) {
	if (file_i < 0 || file_i > FILE_NAME_MAX)
		return -1;
	if (root_dir.files[file_i].valid == -1)
		return 0;
	int curr_block = root_dir.files[file_i].data_block_i;
	int nblocks = 1;
	while (curr_block != DATA_BLOCKS) {
		nblocks++;
		curr_block = fat[curr_block];
	}
	return nblocks * BLOCK_SIZE;
}

int fs_write(int fildes, void* buf, size_t nbyte) {
	if (fildes_list[fildes].valid == -1)
		return -1;
	if (nbyte == 0)
		return 0;
	size_t extra_to_write = fildes_list[fildes].offset % BLOCK_SIZE;
	size_t bytes_to_write = nbyte + extra_to_write;

	int file_i = fs_find_file(fildes_list[fildes].name);
	int block_i;
	if (fs_get_filesize(fildes) == 0) {
		if ((block_i = fat_next_alloc(-1)) == -1)
			return 0; // no more memory for initial block
		root_dir.files[file_i].data_block_i = block_i - sblock.data_block_start;
	} else if ((block_i = fildes_get_block_i(fildes, root_dir.files[file_i])) == -1) {
		root_dir.files[file_i].size = get_nblock_size(file_i);
		return 0; // there was not enough memory for the holes
	}
	void* tmp_buf = malloc(max(bytes_to_write, BLOCK_SIZE));
	if (block_read(block_i, tmp_buf) == -1) {
		free(tmp_buf);
		return 0;
	}
	memcpy(tmp_buf + extra_to_write, buf, nbyte);
	
	int bytes_written = batch_block_write(block_i, tmp_buf, bytes_to_write, fat_next_alloc);
	bytes_written -= extra_to_write;
	free(tmp_buf);
	fildes_list[fildes].offset += bytes_written;
	root_dir.files[file_i].size += fildes_list[fildes].offset;
	return bytes_written;
}

int fs_get_filesize(int fildes) {
	if (fildes_list[fildes].valid == -1)
		return -1;
	return root_dir.files[fs_find_file(fildes_list[fildes].name)].size;
}

int fs_lseek(int fildes, off_t offset) {
	if (fildes_list[fildes].valid == -1 || offset > fs_get_filesize(fildes) || offset < 0)
		return -1;
	fildes_list[fildes].offset = offset;
	return 0;
}

int fs_truncate(int fildes, off_t length) {
	if (fildes_list[fildes].valid == -1 || length > fs_get_filesize(fildes) || length < 0)
		return -1;
	int file_i = fs_find_file(fildes_list[fildes].name);
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
