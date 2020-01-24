#include <pathsplit.h>
#include <kheap.h>
#include <kslub.h>
#include <video.h>

kslub_t splitter_stub;

void splitter_init(void){
    kslub_init(&splitter_stub, sizeof(splitted_path_node_t));
}

//return null terminated array of given string
// /dev/zero
splitted_path_node_t* splitter_split_path(char* path){
    size_t len = strlen(path);
    char* path_copy = kheap_malloc(len + 1);
    path_copy[len] = '\0';
    memcpy(path_copy, path, len + 1);
    splitted_path_node_t head;
    head.next = NULL;
    head.name = NULL;
    splitted_path_node_t* current = &head;
    current->next = NULL;
    current->name = NULL;
    size_t start = 0;
    for(size_t i = 0; i < len; ++i){
        if(path_copy[i] == '/'){
            path_copy[i] = '\0';
            if(i == 0){
                current->next = kslub_new(&splitter_stub);
                current = current->next;
                current->name = NULL;
            }
            else{
                //zeros are now slashes
                //ignore repeatable slashes
                //and slashes at the end of the path
                if((path_copy[i - 1] != '\0') && (i != len - 1)){
                    char* pointer = path_copy + start;
                    if(*(pointer) != '\0'){
                        current->next = kslub_new(&splitter_stub);
                        current = current->next;
                        current->name = pointer;
                    }
                }
            }
            if(i != len - 1)
            start = i + 1;
        }
    }
    char* pointer = path_copy + start;
    if(*pointer != '\0'){
        current->next = kslub_new(&splitter_stub);
        current = current->next;
        current->name = pointer;
    }
    current->next = NULL;
    return head.next;
}

void splitter_free_splitted_path(splitted_path_node_t* splitted){
    bool path_freed = false;
    while(splitted != NULL){
        if(!path_freed && splitted->name){
            kheap_free(splitted->name);
            path_freed = true;
        }
        splitted_path_node_t* next = splitted->next;
        kslub_delete(&splitter_stub, splitted);
        splitted = next;
    }
}