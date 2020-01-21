File descriptor - special structure stored somewhere on disk

Userland process refers to file descriptor using a file descriptor number

Here is the list of operations on files offered by VFS
- Init (gets called when file descriptor is created)
- Finalize (gets called before file descriptor is freed)
- Openat (open file in the directory (if it presents on disk) represented by this descriptor)
- Linkat (given pointer to other's file private pointer, hard link file in this directory to that file)
- Unlink (delete file / decrease reference counter)
- Read (given buffer offset and size, write data from file to buffer)
- Write (given buffer offset and size, write data from buffer to file)
- Seek (given position, move cursor to that position)
- Readdir (get next directory descriptor)

Hence, vfs_file_t looks like this

    typedef void* vfs_file_private_t;
    typedef struct vfs_dirent_struct vfs_dirent_t;

    typedef struct vfs_file_struct {
        vfs_file_private_t private_t;
        void (*init)(vfs_file_private_t);
        void (*finalize)(vfs_file_private_t);
        vfs_file_t* (*openat)(vfs_file_private_t, char*);
        int (*linkat)(vfs_file_private_t, vfs_file_unique_t, char*);
        int (*unlinkat)(vfs_file_private_t);
        int (*read)(vfs_file_private_t, char*, int);
        int (*write)(vfs_file_private_t, char*, int);
        int (*seek)(vfs_file_private_t, int, int);
        int (*readdir)(vfs_file_private_t);
    } vfs_file_t;