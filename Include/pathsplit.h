#ifndef __PATH_SPLIT_H_INCLUDED__
#define __PATH_SPLIT_H_INCLUDED__

typedef struct splitted_path_node_struct
splitted_path_node_t;

typedef struct splitted_path_node_struct {
    char* name;
    splitted_path_node_t* next;
} splitted_path_node_t;

void splitter_init(void);
splitted_path_node_t* splitter_split_path(char* path);
void splitter_free_splitted_path(splitted_path_node_t* splitted);

#endif