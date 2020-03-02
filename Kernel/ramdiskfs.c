#include <ramdiskfs.h>
#include <pathsplit.h>
#include <kslub.h>

typedef struct ramdisk_header_struct {
    size_t file_size;
    char   file_name[100];
    size_t file_type;
    size_t file_parent_off;
} __attribute__((packed)) ramdisk_header_t;

ramdisk_header_t* ramdisk_root_offset;
kslub_t ramdisk_handlers_slub;

typedef struct ramdisk_file_struct {
    ramdisk_header_t* header;
    size_t offset;
} ramdisk_file_t;

typedef struct ramdisk_dir_struct {
    ramdisk_header_t* header;
    ramdisk_header_t* current;
} ramdisk_dir_t;

void ramdisk_init(void* ramdisk_addr){
    ramdisk_root_offset = ramdisk_addr;
    kslub_init(&ramdisk_handlers_slub, sizeof(ramdisk_file_t));
}

ramdisk_header_t* ramdisk_lookup(ramdisk_header_t* header, char* name){
    if(header->file_type != FT_DIRECTORY)
        return NULL;
    if((strcmp(name, ".") == 0) || *(name) == '\0'){
        return header;
    }
    if(strcmp(name, "..") == 0){
        return (ramdisk_header_t*)(((char*)ramdisk_root_offset) + header->file_parent_off);
    }
    ramdisk_header_t* end_header = (ramdisk_header_t*)(((char*)header) + header->file_size);
    ramdisk_header_t* current_header = header + 1;
    while(current_header != end_header){
        if(strcmp(current_header->file_name, name) == 0){
            return current_header;
        }
        current_header = (ramdisk_header_t*)(((char*)current_header) + current_header->file_size);
    }
    return NULL;
}

void* ramdisk_open(char* path){
    splitted_path_node_t* splitted = splitter_split_path(path);
    splitted_path_node_t* subdir_name = splitted;
    if(splitted->name == NULL){
        subdir_name = subdir_name->next;
    }
    ramdisk_header_t* current = ramdisk_root_offset;
    while(subdir_name != NULL){
        if(subdir_name->name == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        current = ramdisk_lookup(current, subdir_name->name);
        if(current == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        subdir_name = subdir_name->next;
    }
    splitter_free_splitted_path(splitted);
    if(current->file_type != FT_REGULAR){
        return NULL;
    }
    ramdisk_file_t* file = kslub_new(&ramdisk_handlers_slub);
    if(file == NULL){
        return NULL;
    }
    file->header = current;
    file->offset = 0;
    return file;
}

size_t ramdisk_read(void* _file, size_t size, char* buffer){
    if(_file == NULL){
        return 0;
    }
    ramdisk_file_t* file = (ramdisk_file_t*)_file;
    char* pos = (char*)(file->header + 1) + file->offset;
    char* end = (char*)(file->header + 1) + file->header->file_size;
    size_t read = size;
    if(end < pos + read){
        uint64_t fsize = file->header->file_size - sizeof(ramdisk_header_t);
        if(file->offset >= fsize){
            return 0;
        }
        read = fsize - file->offset;
    }
    memcpy(buffer, pos, read);
    file->offset += read;
    return read;
}

size_t ramdisk_seek(void* _file, size_t offset, size_t whence){
    if(_file == NULL){
        return 0;
    }
    ramdisk_file_t* file = (ramdisk_file_t*)_file;
    size_t maximum_bound = file->header->file_size - sizeof(ramdisk_header_t);
    if(whence == SEEK_SET){
        if(offset <= maximum_bound){
            file->offset = offset;
            return offset;
        }else{
            file->offset = maximum_bound;
            return maximum_bound;
        }
    }
    if(whence == SEEK_CUR){
        offset += file->offset;
        if(offset <= maximum_bound){
            file->offset = offset;
            return offset;
        }else{
            file->offset = maximum_bound;
            return maximum_bound;
        }
    }
    if(whence == SEEK_END){
        file->offset = maximum_bound;
        return maximum_bound;
    }
    return file->offset;
}

void ramdisk_close(void* file){
    if(file == NULL){
        return;
    }
    kslub_delete(&ramdisk_handlers_slub, file);
}

void* ramdisk_opendir(char* path){
    splitted_path_node_t* splitted = splitter_split_path(path);
    splitted_path_node_t* subdir_name = splitted;
    if(splitted->name == NULL){
        subdir_name = subdir_name->next;
    }
    ramdisk_header_t* current = ramdisk_root_offset;
    while(subdir_name != NULL){
        if(subdir_name->name == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        current = ramdisk_lookup(current, subdir_name->name);
        if(current == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        subdir_name = subdir_name->next;
    }
    splitter_free_splitted_path(splitted);
    if(current->file_type != FT_DIRECTORY){
        return NULL;
    }
    ramdisk_dir_t* dir = kslub_new(&ramdisk_handlers_slub);
    if(dir == NULL){
        return NULL;
    }
    dir->header = current;
    dir->current = dir->header + 1;
    return dir;
}

size_t ramdisk_readdir(void* _dir, ramdisk_dirent_t* buf, size_t count){
    ramdisk_dir_t* dir = (ramdisk_dir_t*)_dir;
    ramdisk_header_t* end = (ramdisk_header_t*)((char*)(dir->header) + dir->header->file_size);
    ramdisk_header_t* cur = dir->current;
    size_t result;
    for(result = 0; (cur != end) && (result < count); ++result){
        memcpy(buf[result].file_name, cur->file_name, 100);
        buf[result].file_size = cur->file_size - sizeof(ramdisk_header_t);
        buf[result].file_type = cur->file_type;
        cur = (ramdisk_header_t*)(((char*)cur) + cur->file_size);
        dir->current = cur;
    }
    return result;
}

void* ramdisk_openat(void* dir, char* path){
    splitted_path_node_t* splitted = splitter_split_path(path);
    ramdisk_header_t* current = ((ramdisk_dir_t*)dir)->header;
    if(splitted == NULL){
        goto make_file;
    }
    splitted_path_node_t* subdir_name = splitted;
    if(splitted->name == NULL){
        subdir_name = subdir_name->next;
    }
    while(subdir_name != NULL){
        if(subdir_name->name == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        current = ramdisk_lookup(current, subdir_name->name);
        if(current == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        subdir_name = subdir_name->next;
    }
    splitter_free_splitted_path(splitted);
make_file:
    if(current->file_type != FT_REGULAR){
        return NULL;
    }
    ramdisk_file_t* file = kslub_new(&ramdisk_handlers_slub);
    if(file == NULL){
        return NULL;
    }
    file->header = current;
    file->offset = 0;
    return file;
}

void* ramdisk_opendirat(void* dir, char* path){
    splitted_path_node_t* splitted = splitter_split_path(path);
    ramdisk_header_t* current = ((ramdisk_dir_t*)dir)->header;
    if(splitted == NULL){
        goto make_file;
    }
    splitted_path_node_t* subdir_name = splitted;
    if(splitted->name == NULL){
        subdir_name = subdir_name->next;
    }
    while(subdir_name != NULL){
        if(subdir_name->name == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        current = ramdisk_lookup(current, subdir_name->name);
        if(current == NULL){
            splitter_free_splitted_path(splitted);
            return NULL;
        }
        subdir_name = subdir_name->next;
    }
    splitter_free_splitted_path(splitted);
make_file:
    if(current->file_type != FT_DIRECTORY){
        return NULL;
    }
    ramdisk_dir_t* file = kslub_new(&ramdisk_handlers_slub);
    if(file == NULL){
        return NULL;
    }
    file->header = current;
    file->current = file->header + 1;
    return file;
}

void ramdisk_closedir(void* dir){
    kslub_delete(&ramdisk_handlers_slub, dir);
}