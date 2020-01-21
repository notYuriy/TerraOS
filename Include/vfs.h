#ifndef __VFS_H_INCLUDED__
#define __VFS_H_INCLUDED__

#include <utils.h>

typedef void* vfs_file_private_t;
typedef void* vfs_file_loc_t;
typedef struct vfs_dirent_struct vfs_dirent_t;
typedef struct vfs_file_struct vfs_file_t;
typedef struct vfs_fs_struct vfs_fs_t;
typedef void* vfs_fs_private_t;

typedef struct vfs_file_struct {
    vfs_file_private_t m_this;
    vfs_file_t* parent;
    void (*init)(vfs_file_private_t);
    void (*finalize)(vfs_file_private_t);
    vfs_file_t* (*openat)(vfs_file_private_t, char*);
    vfs_file_loc_t (*getloc)(vfs_file_private_t);
    int (*linkto)(vfs_file_private_t, vfs_file_loc_t);
    int (*unlink)(vfs_file_private_t);
    int (*read)(vfs_file_private_t, char*, int);
    int (*write)(vfs_file_private_t, char*, int);
    int (*seek)(vfs_file_private_t, int, int);
    int (*readdir)(vfs_file_private_t);
} vfs_file_t;

typedef struct vfs_dirent_struct {
    char* name;
} vfs_dirent_t;

typedef struct vfs_fs_struct {
    vfs_fs_private_t m_this;
    vfs_file_t* root;
    void (*unload_callback)();
} vfs_fs_t;

void vfs_set_root_fs(vfs_fs_t* fs);

//manipulate with files in the directory (represented by fd)

vfs_file_t* vfs_open_at(vfs_file_t* dir, char* path);
int vfs_link_at(vfs_file_t* olddir, char* oldpath, vfs_file_t* newdir, char* newpath);
int vfs_unlink_at(vfs_file_t* file);

//manipulate with files from root directly

int vfs_open(char* path);
int vfs_link(char* oldpath, char* newpath);
int vfs_unlink(char* path);

//read/write/seek files

int vfs_read(vfs_file_t* file, char* buf, int size);
int vfs_write(vfs_file_t* file, char* buf, int size);
int vfs_seek(vfs_file_t* file, int offset, int whence);

#endif