#include <pathsplit.h>
#include <kheap.h>
#include <kstub.h>
#include <video.h>

kstub_t splitter_stub;

void splitter_init(void){
    kstub_init(&splitter_stub, sizeof(splitted_path_node_t));
}

//return null terminated array of given string
// /dev/zero
splitted_path_node_t* splitter_split_path(char* path){
    size_t len = strlen(path);
    char* path_copy = kheap_malloc(len + 1);
    path_copy[len] = '\0';
    memcpy(path_copy, path, len + 1);
    splitted_path_node_t* result = kstub_new(&splitter_stub);
    splitted_path_node_t* current = result;
    current->next = NULL;
    current->name = NULL;
    size_t start = 0;
    for(size_t i = 0; i < len; ++i){
        if(path_copy[i] == '/'){
            path_copy[i] = '\0';
            if(i == 0){
                current->next = kstub_new(&splitter_stub);
                current->name = NULL;
                current = current->next;
            }else{
                //zeros are now slashes
                //ignore repeatable slashes
                //and slashes at the end of the path
                if(path_copy[i - 1] != '\0' && i != len - 1){
                    current->name = path_copy + start;
                    current->next = kstub_new(&splitter_stub);
                    current = current->next;
                }
            }
            start = i + 1;
        }
    }
    current->name = path_copy + start;
    current->next = NULL;
    return result;
}

void splitter_free_splitted_path(splitted_path_node_t* splitted){
    bool path_freed = false;
    while(splitted != NULL){
        if(!path_freed && splitted->name){
            kheap_free(splitted->name);
            path_freed = true;
        }
        splitted_path_node_t* next = splitted->next;
        kstub_delete(&splitter_stub, splitted);
        splitted = next;
    }
}