#ifndef _SANIC_FS_H_
#define _SANIC_FS_H_

#define BLOCK_TERMINATOR -2
#define BLOCK_FREE 0

#define SUPER_BLOCK 0

#define MAX_FILES 64
#define MAX_DESCRIPTORS 32
#define MAX_FNAME 16

typedef struct t_directory_entry {
  char filename[MAX_FNAME]; // 16 bytes maximum
  short start; // block offset
  unsigned int size; // file size
} directory_entry;

typedef struct t_file_descriptor {
  int directory_i; // index in directory
  int offset; // seek offset
} file_descriptor;


int make_fs(char* disk_name);


int mount_fs(char* disk_name);


int umount_fs(char* disk_name);


int fs_open(char* name);

int fs_close(int fildes);


int fs_create(char* name);


int fs_delete(char* name);

int fs_read(int fildes, void* buf, size_t nbyte);

int fs_write(int fildes, void* buf, size_t nbyte);

int fs_get_filesize(int fildes);


int fs_lseek(int fildes, off_t offset);


int fs_truncate(int fildes, off_t length);

#endif
