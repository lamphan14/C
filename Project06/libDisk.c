#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "tinyFS.h"
#include "tinyFS_errno.h"

void addDisk(Disk disk);
Disk *findDisk(int diskNum);

DiskNode *head;

int diskCount = 0;

/* This functions opens a regular UNIX file and designates the first nBytes of it as 
 * space for the emulated disk. nBytes should be an integral number of the block size.
 * If nBytes > 0 and there is already a file by the given filename, that file’s contents
 * may be overwritten. If nBytes is 0, an existing disk is opened, and should not be
 * overwritten. There is no requirement to maintain integrity of any file content beyond
 * nBytes. The return value is -1 on failure or a disk number on success.
 */
int openDisk(char *filename, int nBytes) {
	int diskNum = 0;
	FILE *file;
	Disk disk;
	char *permissions = "w+";
	
	if(nBytes % BLOCKSIZE != 0) {
		return OPENDISK_FAILURE;
	}

	//	file doesn't exist if access returns -1
	if(access(filename, F_OK) == -1) {
		permissions = "w+";

		//	doesn't exist but only read, so failure
		if(nBytes == 0) {
			return OPENDISK_FAILURE;
		}
	}
	else {
		permissions = "r+";

		//	read-only if nBytes is 0
		if(nBytes == 0) {
			permissions = "r";
		}
	}

	file = fopen(filename, permissions);
	
	if(file == NULL) {
		return OPENDISK_FAILURE;
	}
	
	diskNum = diskCount++;
		
	disk = (Disk) {
		file,
		diskNum,
		nBytes,
		1
	};
	
	addDisk(disk);
	
	return diskNum;
}

void addDisk(Disk disk) {
	DiskNode *curr;

	Disk *diskPtr = malloc(sizeof(Disk));
	memcpy(diskPtr, &disk, sizeof(Disk));
	
	//	create head if it is null
	if(head == NULL) {
		head = malloc(sizeof(DiskNode));
		*head = (DiskNode) {
			diskPtr,
			NULL
		};
	}
	else {
		curr = head;
		
		while(curr->next != NULL) curr = curr->next;
				
		curr->next = malloc(sizeof(DiskNode));
		*(curr->next) = (DiskNode) {
			diskPtr,
			NULL
		};
	}
}

Disk *findDisk(int diskNum) {
	DiskNode *curr = head;
	
	if(curr == NULL) {
		return NULL;
	}
	
	else {
		if(curr->disk->diskNum == diskNum) return curr->disk;
		
		while(curr->next != NULL) {
			curr = curr->next;

			if(curr->disk->diskNum == diskNum) return curr->disk;
		}
		
		return NULL;
	}
}

/* readBlock() reads an entire block of BLOCKSIZE bytes from the open disk (identified by
 * ‘disk’) and copies the result into a local buffer (must be at least of BLOCKSIZE
 * bytes). The bNum is a logical block number, which must be translated into a byte
 * offset within the disk. The translation from logical to physical block is
 * straightforward: bNum=0 is the very first byte of the file. bNum=1 is BLOCKSIZE
 * bytes into the disk, bNum=n is n*BLOCKSIZE bytes into the disk. On success, it
 * returns 0. -1 or smaller is returned if disk is not available (hasn’t been opened)
 * or any other failures. You must define your own error code system.
 */
int readBlock(int disk, int bNum, void *block) {
	Disk *diskPtr;
	int byteOffset;
	
	diskPtr = findDisk(disk);
	
	if(diskPtr == NULL || !diskPtr->open) {
		return READBLOCK_FAILURE;
	}
	
	byteOffset = bNum * BLOCKSIZE;
	
	if(byteOffset + BLOCKSIZE > diskPtr->space) {
		return DISK_PAST_LIMITS;
	}
	
	//	seek to correct location in file
	if(fseek(diskPtr->file, byteOffset, SEEK_SET) != 0) {
		return READBLOCK_FAILURE;
	}
	
	//	read from file into block buffer
	fread(block, BLOCKSIZE, 1, diskPtr->file);
	
	return 0;
}

/* writeBlock() takes disk number ‘disk’ and logical block number ‘bNum’ and writes the
 * content of the buffer ‘block’ to that location. ‘block’ must be integral with
 * BLOCKSIZE. The disk must be open. Just as in readBlock(), writeBlock() must translate
 * the logical block bNum to the correct byte position in the file. On success, it
 * returns 0. -1 or smaller is returned if disk is not available (i.e. hasn’t been
 * opened) or any other failures. You must define your own error code system.
*/
int writeBlock(int disk, int bNum, void *block) {
	Disk *diskPtr;
	int byteOffset;
	
	diskPtr = findDisk(disk);
	
	if(diskPtr == NULL || !diskPtr->open) {
		return WRITEBLOCK_FAILURE;
	}
	
	byteOffset = bNum * BLOCKSIZE;
	
	if(byteOffset + BLOCKSIZE > diskPtr->space) {
		return DISK_PAST_LIMITS;
	}

	//	seek to correct location in file
	if(fseek(diskPtr->file, byteOffset, SEEK_SET) != 0) {
		return WRITEBLOCK_FAILURE;
	}

	//	write from block buffer into file
	fwrite(block, BLOCKSIZE, 1, diskPtr->file);

	return 0;
}

/* closeDisk() takes a disk number ‘disk’ and makes the disk closed to further I/O;
 * i.e. any subsequent reads or writes to a closed disk should return an error. Closing
 * a disk should also close the underlying file, committing any buffered writes. 
 */
void closeDisk(int disk) {
	Disk *diskPtr;
	
	diskPtr = findDisk(disk);
	
	if(diskPtr == NULL || !diskPtr->open) {
		return;
	}
		
	fclose(diskPtr->file);

	diskPtr->open = 0;
}