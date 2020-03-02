#include <tty.h>
#include <video.h>
#include <kybrd.h>
#include <kheap.h>

size_t getline(char* buffer, size_t buffer_size){
    size_t pos = 0;
    kybrd_event_t event;
    while(true){
        asm("pause":::);
        event = kybrd_poll_event();
        if(!(event.mask & KYBRD_FLAG_PRESENT)){
            continue;
        }
        if(event.mask & KYBRD_FLAG_RELEASED){
            continue;
        }
        if(event.code == '\t'){
            continue;
        }
        if(event.code == '\n'){
            video_putc('\n');
            break;
        }
        if(event.code == '\b'){
            if(pos != 0){
                video_putc('\b');
                video_putc(' ');
                video_putc('\b');
                buffer[pos] = '\n';
                pos--;
            }
            continue;
        }
        if(pos == buffer_size - 1){
            continue;
        }
        buffer[pos++] = event.code;
        video_putc(event.code);
    }
    buffer[pos++] = '\0';
    return pos;
}

int cmdeq(char* buf, char* cmd){
    int pos = 0;
    for(; cmd[pos] != '\0'; pos++){
        if(buf[pos] != cmd[pos]){
            return -1;
        }
    }
    if(buf[pos] == ' '){
        return pos + 1;
    }
    if(buf[pos] == '\0'){
        return pos;
    }
    return -1;
}

size_t get_args_count(char* line){
    if(line == NULL){
        return 0;
    }
    if(*line == '\0'){
        return 0;
    }
    size_t result = 0;
    bool skip = false;
    bool arg_separated = false;
    for(size_t i = 0; line[i] != '\0'; ++i){
        if(i == 0){
            arg_separated = true;
        }
        if(line[i] == '\"'){
            if((!arg_separated) || skip){
                result++; arg_separated = true;
            }
            skip = !skip;
        }else if(!skip){
            if(line[i] == ' ' || line[i] == '\t'){
                if(!arg_separated){
                    result++;
                    arg_separated = true;
                }
            }else{
                arg_separated = false;
            }
        }
    }
    if(!arg_separated){
        result++;
    }
    return result;
}

void* get_slice(char* str, size_t start, size_t end){
    void* result = kheap_malloc(end - start + 1);
    memcpy(result, str + start, end - start);
    ((char*)result)[end - start] = '\0';
    return result;
}

cmd_args_t split_to_args(char* cmd){
    cmd_args_t result;
    if(cmd == NULL){
        result.argc = 0;
        result.argv = NULL;
        return result;
    }
    if(*cmd == '\0'){
        result.argc = 0;
        result.argv = NULL;
        return result; 
    }
    size_t args_count = get_args_count(cmd);
    result.argc = args_count;
    result.argv = kheap_malloc((result.argc + 1) * sizeof(*(result.argv)));
    size_t current_arg = 0; size_t current_start_index = 0;
    bool skip = false;
    bool arg_separated = false;
    size_t i = 0;
    for(; cmd[i] != '\0'; ++i){
        if(i == 0){
            arg_separated = true;
        }
        if(cmd[i] == '\"'){
            if((!arg_separated) || skip){
                result.argv[current_arg++] = 
                get_slice(cmd, current_start_index, i);
                arg_separated = true;
            }
            current_start_index = i + 1;
            skip = !skip;
        } else if(!skip){
            if(cmd[i] == ' ' || cmd[i] == '\t'){
                if(!arg_separated){
                    result.argv[current_arg++] = 
                    get_slice(cmd, current_start_index, i);
                    arg_separated = true;
                }
                current_start_index = i + 1;
            }else{
                arg_separated = false;
            }
        }
    }
    if(!arg_separated){
        result.argv[current_arg++] = get_slice(cmd, current_start_index, i);
    }
    result.argv[current_arg] = NULL;
    return result;
}

void free_args(cmd_args_t args){
    for(size_t i = 0; i < (size_t)args.argc; ++i){
        kheap_free(args.argv[i]);
    }
    kheap_free(args.argv);
}