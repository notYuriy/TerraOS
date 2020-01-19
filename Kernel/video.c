#include <video.h>
#include <utils.h>
#include <stdbool.h>
#include <spinlock.h>

typedef uint16_t video_char_t;

spinlock_t video_spinlock;

video_char_t* video_buffer;
const video_coord_t video_width = 80;
const video_coord_t video_height = 25;
video_coord_t video_row, video_column;
video_color_t video_background_color;
video_color_t video_foreground_color;
video_packed_color_t video_packed_color;
video_coord_t video_tab_size;

inline video_packed_color_t video_pack_color(video_color_t foreground, video_color_t background){
    return foreground | background << 4;
}

inline video_char_t video_pack_color_and_char(char character, video_packed_color_t color){
    return (video_char_t)character | ((video_char_t)color << 8);
}

inline void video_update_packed_color(void){
    video_packed_color = video_pack_color(video_foreground_color, video_background_color);
}

void video_clear_screen(void){
    spinlock_lock(&video_spinlock);
    video_row = 0;
    video_column = 0;
    for(size_t i = 0; i < video_width * video_height; ++i)
    {
        video_buffer[i] = video_pack_color_and_char(' ', video_packed_color);
    }
    spinlock_unlock(&video_spinlock);
}

inline void video_scroll(void){
    size_t i;
    for(i = 0; i < video_width * (video_height - 1); ++i)
    {
        video_buffer[i] = video_buffer[video_width + i];
    }
    for(; i < video_width * video_height; ++i)
    {
        video_buffer[i] = video_pack_color_and_char(' ', video_packed_color);
    }
    video_column = 0;
    video_row = video_height - 1;
}

inline void video_put_at(video_coord_t x, video_coord_t y, char c){
    video_buffer[x + y * video_width] = video_pack_color_and_char(c, video_packed_color);
}

inline void video_putrawc(char c){
    if(video_column == video_width)
    {
        video_column = 0; video_row++;
        if(video_row == video_height)
        {
            video_scroll();
        }
    }
    video_put_at(video_column++, video_row, c);
}

void video_set_background(video_color_t color){
    spinlock_lock(&video_spinlock);
    video_background_color = color;
    video_update_packed_color();
    spinlock_unlock(&video_spinlock);
}

void video_set_foreground(video_color_t color){
    spinlock_lock(&video_spinlock);
    video_foreground_color = color;
    video_update_packed_color();
    spinlock_unlock(&video_spinlock);
}

void __video_putc(char c){
    if(c == '\n')
    {
        size_t to_skip = video_width - video_column;
        if(video_column == video_width)
        {
            to_skip = video_width;
        }
        for(size_t i = 0; i < to_skip; ++i)
        {
            video_putrawc(' ');
        } 
        return;
    }
    if(c == '\t')
    {
        size_t to_skip = 
        ((((video_column + video_tab_size))
        /video_tab_size)*video_tab_size) - video_column;
        for(size_t i = 0; i < to_skip; ++i)
        {
            video_putrawc(' ');
        }
        return;
    }
    if(c == '\r')
    {
        return;
    }
    if(c == '\x08')
    {
        --video_column;
        if(video_column == -1)
        {
            video_column = video_width - 1;
            video_row = (video_row == 0)?(0):(video_row - 1);
        }
    }
    video_putrawc(c);
}

void video_putc(char c){
    spinlock_lock(&video_spinlock);
    __video_putc(c);
    spinlock_unlock(&video_spinlock);
}

void _putchar(char c){
    video_putc(c);
}

void video_write(char* str, size_t len){
    spinlock_lock(&video_spinlock);
    for(size_t i = 0; i < len; ++i)
    {
        __video_putc(str[i]);
    }
    spinlock_unlock(&video_spinlock);
}

void video_init(void){
    video_buffer = (video_char_t*)(KERNEL_MAPPING_BASE + 0xb8000);
    video_background_color = black;
    video_foreground_color = cyan;
    video_tab_size = 4;
    video_update_packed_color();
    video_clear_screen();
    video_spinlock = 0;
}

void video_puts(char* str){
    video_write(str, strlen(str));
}

void video_set_tab_size(video_coord_t size){
    spinlock_lock(&video_spinlock);
    video_tab_size = size;
    spinlock_unlock(&video_spinlock);
}

void video_move_to_x(video_coord_t x){
    spinlock_lock(&video_spinlock);
    video_column = x;
    spinlock_unlock(&video_spinlock);
}

void video_move_to_y(video_coord_t y){
    spinlock_lock(&video_spinlock);
    video_row = y;
    spinlock_unlock(&video_spinlock);
}

char video_get_digit(int a){
    static const char digits[] = "0123456789ABCDEF";
    return digits[a];
}

void video_write_hex(char* str, size_t size){
    spinlock_lock(&video_spinlock);
    unsigned char* ustr = (unsigned char*)str;
    for(size_t i = 0; i < size; ++i){
        unsigned char byte = ustr[i];
        int div = byte / 16;
        int mod = byte % 16;
        char divdigit = video_get_digit(div);
        char moddigit = video_get_digit(mod);
        video_putrawc(divdigit);
        video_putrawc(moddigit);
    }
    spinlock_unlock(&video_spinlock);
}

int8_t video_get_screen_width(void){
    return video_width;
}

int8_t video_get_screen_height(void){
    return video_height;
}

video_packed_color_t video_get_packed_color(void){
    spinlock_lock(&video_spinlock);
    video_packed_color_t result = video_packed_color;
    spinlock_unlock(&video_spinlock);
    return result;
}

void video_set_packed_color(video_packed_color_t color){
    spinlock_lock(&video_spinlock);
    video_packed_color = color;
    spinlock_unlock(&video_spinlock);
}