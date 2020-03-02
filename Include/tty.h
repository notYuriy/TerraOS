#ifndef __TTY_H_INCLUDED__
#define __TTY_H_INCLUDED__

#include <utils.h>

size_t getline(char* buffer, size_t buffer_size);
int cmdeq(char* buf, char* cmd);

typedef struct cmd_args_struct {
    int argc;
    char** argv;
} cmd_args_t;

size_t get_args_count(char* line);
cmd_args_t split_to_args(char* str);
void free_args(cmd_args_t args);
#endif