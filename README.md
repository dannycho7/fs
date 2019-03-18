# fs

I modeled this implementation of a filesystem using the file allocation table (FAT) design.

## Metadata
Each filesystem would store a superblock, FAT, and root directory in the metadata blocks. The superblock stores information on where to access the FAT, root directory, and data blocks. Each entry in the FAT represented a data block. The values in the FAT represented what type of block and the next block in the file (if there was any). The root directory stored information on which files were contained, the first block of the file, and the size of the file.

## Instructions
In order to create fs.o:
```bash
make fs.o
```
