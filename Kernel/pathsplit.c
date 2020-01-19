#include <pathsplit.h>
#include <kheap.h>
#include <kstub.h>
#include <video.h>

kstub_t stub;

void splitter_init(void){
    kstub_init(&stub, sizeof(splitted_path_node_t));
}

//return null terminated array of given string
// /dev/zero
splitted_path_node_t* splitter_split_path(char* path){
    splitted_path_node_t* result = kstub_new(&stub);
    splitted_path_node_t* current = result;
    result->next = NULL;
    size_t name_start = 0;
    size_t i;
    bool slash_appeared = false;
    for(i = 0; path[i] != '\0'; ++i)
    {
        //ignore slash sequence
        if(path[i] == '/')
        {
            slash_appeared = true;
            size_t len = i - name_start;
            if(len == 0)
            {
                current->name = NULL;
            }
            else
            {
                size_t nullterm_len = len + 1;
                char* copied_name = kheap_malloc(nullterm_len);
                memcpy(copied_name, path + name_start, len);
                copied_name[len] = '\0';
                current->name = copied_name;
            }
            current->next = kstub_new(&stub);
            current = current->next;
            current->next = NULL;
            name_start = i + 1;
        }
    }
    if(!slash_appeared)
    {
        return NULL;
    }
    size_t len = i - name_start;
    if(len == 0)
    {
        current->name = NULL;
    }
    else
    {
        size_t nullterm_len = len + 1;
        char* copied_name = kheap_malloc(nullterm_len);
        memcpy(copied_name, path + name_start, len);
        copied_name[len] = '\0';
        current->name = copied_name;
    }
    current = result;
    while(current != NULL)
    {
        if(current->next == NULL)
        {
            break;
        }
        if(current->next->name == NULL)
        {
            splitted_path_node_t* next = current->next;
            current->next = next->next;
            kstub_delete(&stub, next);
        }
        else
        {
            current = current->next;
        }
    }
    
    return result;
}

void splitter_free_splitted_path(splitted_path_node_t* splitted){
    while(splitted->next != NULL)
    {
        if(splitted->name != NULL)
        {
            kstub_flush(&stub);
        }
        splitted = splitted->next;
    }
}