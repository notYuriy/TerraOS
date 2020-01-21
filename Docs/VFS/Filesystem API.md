The roal of filesystem driver is to implement routines on files described in "Files Operations.md"

The filesystem object represents mount filesystem without any mount points
Effectively, it stores one callback for unmounting and root file

Hence, vfs_fs_t looks like this

    typedef void* vfs_fs_private_t;
    typedef struct vfs_fs_struct {
        vfs_fs_private_t this;
        vfs_file_t* root;
        void (*unload_callback)();
    } vfs_fs_t;
