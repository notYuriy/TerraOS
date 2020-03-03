#ifndef __SHELL_BUILTINS_H_INCLUDED__
#define __SHELL_BUILTINS_H_INLCUDED__

#include <video.h>

typedef struct color_theme_struct {
    video_color_t background_color;
    video_color_t foreground_color;
    video_color_t cmd_color;
    video_color_t path_sep_color;
    video_color_t user_color;
} color_theme_t;

void shellbuiltins_init(void* root, color_theme_t* theme_ref);
void shellbuiltins_version(int argc, char** argv);
void shellbuiltins_man(int argc, char** argv);
void shellbuiltins_cls(int argc, char** argv);
void shellbuiltins_clear(int argc, char** argv);
void shellbuiltins_kernelmem(int argc, char** argv);
void shellbuiltins_tests(int argc, char** argv);
void shellbuiltins_license(int argc, char** argv);
void shellbuiltins_ls(int argc, char** argv);
void shellbuiltins_cd(int argc, char** argv);
void shellbuiltins_cat(int argc, char** argv);
void shellbuiltins_echo(int argc, char** argv);
void shellbuiltins_setcolor(int argc, char** argv);

#endif