#ifndef __VIDEO_H_INCLUDED__
#define __VIDEO_H_INCLUDED__

#include <stdint.h>
#include <stddef.h>
#include <printf.h>
#include <utils.h>

typedef enum video_color_enum {
    black = 0,
    blue = 1,
    green = 2,
    cyan = 3,
    red = 4,
    magenta = 5,
    brown = 6,
    light_grey = 7,
    dark_grey = 8,
    light_blue = 9,
    light_green = 10,
    light_cyan = 11,
    light_red = 12,
    light_magenta = 13,
    light_brown = 14,
    white = 15
} video_color_t;

typedef uint8_t video_packed_color_t;
typedef int8_t video_coord_t;

void video_init(void);
void video_lock(void);
void video_unlock(void);
void video_set_background(video_color_t color);
void video_set_foreground(video_color_t color);
void video_putc(char c);
void video_puts(char* str);
void video_write(char* str, size_t size);
void video_write_hex(char* str, size_t size);
void video_move_to_x(video_coord_t coord);
void video_move_to_y(video_coord_t coord);
void video_set_tab_size(video_coord_t size);
void video_clear_screen(void);
video_packed_color_t video_get_packed_color(void);
void video_set_packed_color(video_packed_color_t color);
video_coord_t video_get_screen_width(void);
video_coord_t video_get_screen_height(void);

inline void video_move_to_xy(video_coord_t x, video_coord_t y){
    video_move_to_x(x);
    video_move_to_y(y);
}

#endif