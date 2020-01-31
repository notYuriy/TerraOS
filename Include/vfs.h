#ifndef __VFS_H_INCLUDED__
#define __VFS_H_INLCUDED__

typedef struct vfs_file_struct {
    int (*read)(vfs_file_t* file, char* ptr, int size);
    int (*write)(vfs_file_t* file, char* ptr, int size);
    long (*seek)(vfs_file_t* file, long offset, int whence);
    int (*close)(vfs_file_t* file);
    int (*openat)(vfs_file_t* file, char* name);
} vfs_file_t;

void vfs_mount(char letter, vfs_file_t* root);
void vfs_unmount(char letter);
void vfs_open(char* name);
void vfs_open(char* name);

#endif