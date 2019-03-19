#include "disk.h"
#include "fs_constants.h"

typedef struct SuperBlock {
	int fat_start;
	int root_dir_start;
	int data_block_start;
} SuperBlock;

typedef struct FileMetadata {
	char name[FILE_NAME_MAX+1];
	int data_block_i; // index of first data block
	int size;
	int valid; // -1 if invalid and 0 if valid
} FileMetadata;

typedef struct Directory {
	FileMetadata files[FILE_MAX];
} Directory;

/*
 * fat[i] = {
 * -1 if data block i unallocated, 
 * x in [0, DATA_BLOCKS) if data block i allocated and is not eof			 
 * DATA_BLOCKS if data block i allocated and is eof
 * }
 */
typedef int FAT[DATA_BLOCKS];

typedef struct FD_Entry {
	char name[FILE_NAME_MAX+1];
	int offset;
	int valid; // -1 if invalid and 0 if valid
} FD_Entry;

typedef FD_Entry FileDescriptorList[NFILE_DESCRIPTOR_MAX];