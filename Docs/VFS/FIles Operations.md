File descriptor - special structure stored somewhere on disk

Userland process refers to file descriptor using a file descriptor number

Here is the list of operations on files offered by VFS
- Init (gets called when file descriptor is created for read/write/link/unlink/seek/readdir access)
- Finalize (gets called before file descriptor is freed)
- Openat (open file in the directory (if it presents on disk) represented by this descriptor)
- Get hard location (return hard location)
- Link (link this file to given hard location)
- Unlink (delete yhis file / decrease reference counter)
- Read (given buffer offset and size, write data from file to buffer)
- Write (given buffer offset and size, write data from buffer to file)
- Seek (given position, move cursor to that position)
- Readdir (get next directory descriptor)
 
Also, we need to remember the fd of the directory, where this file is located to open special subdirectories.

Next pointer store the location of the next entry.
Prev pointer is required to perform deletition given pointer to list node only.
Child pointer points to one of the children

Finally, mounted pointer points to filesystem if it is mounted

To ensure threading safety, every file/folder in vfs file tree is protected using spinlock (better thread syncronisation will be added later)

Hence, vfs_file_t looks like this

    typedef void* vfs_file_private_t;
    typedef void* vfs_file_loc_t;
    typedef struct vfs_dirent_struct vfs_dirent_t;

    typedef struct vfs_file_struct {
        vfs_file_private_t m_this;
        vfs_file_t* parent;
        vfs_file_t* prev;
        vfs_file_t* next;
        vfs_file_t* chld;
        spinlock_t spinlock;
        void (*init)(vfs_file_private_t);
        void (*finalize)(vfs_file_private_t);
        vfs_file_t* (*openat)(vfs_file_private_t, char*);
        vfs_file_loc_t (*getloc)(vfs_file_private_t);
        int (*linkto)(vfs_file_private_t, vfs_file_loc_t);
        int (*unlink)(vfs_file_private_t);
        int (*read)(vfs_file_private_t, char*, int);
        int (*write)(vfs_file_private_t, char*, int);
        int (*seek)(vfs_file_private_t, int, int);
        int (*readdir)(vfs_file_private_t, vfs_dirent_t*);
    } vfs_file_t;