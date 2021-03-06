#define		WRITE_BYTE_SUCCESS     19
#define		MAKE_RW_SUCCESS     18
#define		MAKE_RO_SUCCESS     17
#define		DELETE_FILE_SUCCESS	16
#define		READ_BYTE_SUCCESS	12
#define		SEEK_FILE_SUCCESS	13
#define		RENAME_FILE_SUCCESS	14
#define		WRITE_FILE_SUCCESS	6
#define		OPEN_FILE_SUCCESS	4
#define		UNMOUNT_FS_SUCCESS	3
#define		MOUNT_FS_SUCCESS	2
#define		MAKE_FS_SUCCESS		1

#define 	OPENDISK_FAILURE 	-1
#define 	READBLOCK_FAILURE 	-2
#define 	WRITEBLOCK_FAILURE 	-3
#define 	DISK_PAST_LIMITS 	-4
#define 	MAKE_FS_ERROR 		-5
#define 	FS_VERIFY_FAILURE 	-6
#define 	OPEN_FILE_FAILURE 	-7
#define 	RENAME_FILE_FAILURE	-8

#define		SEEK_FILE_FAILURE	-11
#define		READ_BYTE_FAILURE	-10
#define		DELETE_FILE_FAILURE	-12
#define 	WRITE_FILE_FAILURE 	-9
#define		MAKE_RO_FAILURE		-13
#define		MAKE_RW_FAILURE		-14
#define		MOUNT_FS_FAILURE	-15
#define		UNMOUNT_FS_FAILURE	-16
#define		CLOSE_FILE_FAILURE	-17
#define		WRITE_BYTE_FAILURE	-18
#define		READ_FILE_INFO_FAILURE	-19
#define		READ_DIR_FAILURE	-20
#define		REMOVE_DYNAMIC_RESOURCE_ERROR	-21