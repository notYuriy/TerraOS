#ifndef __RAMDISKFS_H_INCLUDED__
#define __RAMDISKFS_H_INCLUDED__

#include <utils.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define FT_REGULAR 0
#define FT_DIRECTORY 1

void ramdisk_init(void* ramdisk_addr);
void* ramdisk_open(char* path);
size_t ramdisk_read(void* file, size_t size, char* buffer);
size_t ramdisk_seek(void* file, size_t offset, size_t whence);
void ramdisk_close(void* file);
void* ramdisk_opendir(char* path);

typedef struct ramdisk_dirent_struct {
    char   file_name[100];
    size_t file_size;
    size_t file_type;
} ramdisk_dirent_t;

size_t ramdisk_readdir(void* dir, ramdisk_dirent_t* buf, size_t count);
void* ramdisk_openat(void* dir, char* path);
void* ramdisk_opendirat(void* dir, char* path);
void ramdisk_closedir(void* dir);

#endif