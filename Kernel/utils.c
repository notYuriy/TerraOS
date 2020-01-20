#include <utils.h>
#include <video.h>

size_t strlen(char* str){
    size_t result = 0;
    for(;str[result] != '\0'; ++result);
    return result;
}

void panic(char* error){
    video_set_foreground(red);
    printf("Error: %s\n", error);
    while(true);
}

void memcpy(void* dst, void* src, size_t count){
    for(size_t i = 0; i < count; ++i){
        ((char*)dst)[i] = ((char*)src)[i];
    }
}

int strcmp(char* str1, char* str2){
    while((*str1 == *str2) && *str1 == '\0'){
        str1++; str2++;
    }
    if(*str1 < *str2){
        return -1;
    }
    if(*str1 > *str2){
        return 1;
    }
    return 0;

}