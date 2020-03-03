#include <shellbuiltins.h>
#include <utils.h>
#include <ramdiskfs.h>
#include <video.h>
#include <kheap.h>
#include <tests.h>

void* shellbuiltins_current_dir;
color_theme_t* shellbuiltins_theme;

void shellbuiltins_init(void* root, color_theme_t* theme_ref){
    shellbuiltins_current_dir = root;
    shellbuiltins_theme = theme_ref;
}

void shellbuiltins_version(int argc, char** argv){
    if(argc == 1){
        printf("TerraOS version 0.1.0\n");
    } else if (argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("usage: version. Prints current os version\n");
        }
        if(strcmp(argv[1], "--help") == 0){
            printf("usage: version. Prints current os version\n");
        }
    }
}

void print_file(void* file){
    char buf[256]; memset(buf, 256, '\0');
    size_t read;
    bool print_endl = false;
    while((read = ramdisk_read(file, 256, buf)) != 0){
        printf("%s", buf);
        print_endl = true;
        memset(buf, read, '\0');
    }
    if(print_endl){
        printf("\n");
    }
}

void shellbuiltins_man(int argc, char** argv){
    if(argc == 1){
        printf("What manual page you want?\n");
    } else if (argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("usage: man <command>. Prints manual about command\n");
            return;
        }
        if(strcmp(argv[1], "--help") == 0){
            printf("usage: man <command>. Prints manual about command\n");
            return;
        }
        char* cmdname = argv[1];
        size_t cmdnamelen = strlen(cmdname);
        size_t filenamelen = cmdnamelen + 5;
        char* filename = kheap_malloc(filenamelen + 1);
        memcpy(filename, "/man/", 5);
        memcpy(filename + 5, cmdname, cmdnamelen);
        filename[filenamelen] = '\0';
        void* file = ramdisk_open(filename);
        if(file == NULL){
            printf("No manual entry for %s\n", cmdname);
            return;
        }
        print_file(file);
        kheap_free((void*)filename);
        ramdisk_close(file);
    }
}

void shellbuiltins_cls(int argc, UNUSED char** argv){
    if(argc == 1){
        video_clear_screen();
    }else{
        printf("usage: cls/clear. Clears terminal screen\n");
    }
}

void shellbuiltins_clear(int argc, char** argv){
    shellbuiltins_cls(argc, argv);
}

void shellbuiltins_kernelmem(UNUSED int argc, UNUSED char** argv){
    kheap_traverse();
}

void shellbuiltins_tests(UNUSED int argc, UNUSED char** argv){
    tests_run();
}

void shellbuiltins_license(UNUSED int argc, UNUSED char** argv){
    void* license_file = ramdisk_open("/shell/LICENSE");
    print_file(license_file);
    ramdisk_close(license_file);
}

void shellbuiltins_ls(int argc, char** argv){
    if(argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            printf("usage: ls [<directory>]. Prints directory contents\n");
            return;
        }else if(strcmp(argv[1], "--h") == 0){
            printf("usage: ls [<directory>]. Prints directory contents\n");
            return;
        }
    }
    void* dir;
    if(argc == 1){
        dir = ramdisk_opendirat(shellbuiltins_current_dir, ".");
    }else{
        dir = ramdisk_opendirat(shellbuiltins_current_dir, argv[1]); 
    }
    if(dir == NULL){
        if(argc != 1){
            printf("ls: %s: No such file or directory\n", argv[1]);
        }
        return;
    }
    ramdisk_dirent_t dirent;
    while(ramdisk_readdir(dir, &dirent, 1)){
        if(dirent.file_type == FT_REGULAR){
            printf("\'%s\'\n", dirent.file_name);
        }else{
            printf(" %s \n", dirent.file_name);
        }
    }
    ramdisk_closedir(dir);
}

void shellbuiltins_cd(int argc, char** argv){
    if(argc != 2){
        printf("usage: cd <directory>. Changes current directory.\n");
        return;
    }
    if(strcmp(argv[1], "-h") == 0){
        printf("usage: cd <directory>. Changes current directory.\n");
        return;
    } else if (strcmp(argv[1], "--help") == 0){
        printf("usage: cd <directory>. Changes current directory.\n");
        return;
    }
    void* newdir = ramdisk_opendirat(shellbuiltins_current_dir, argv[1]);
    if(newdir == NULL){
        printf("cd: %s: No such file or directory\n", argv[1]);
        return;
    }
    ramdisk_closedir(shellbuiltins_current_dir);
    shellbuiltins_current_dir = newdir;
}

void shellbuiltins_cat(int argc, char** argv){
    if(argc != 2){
        printf("usage: cat <name>. Prints file contents\n");
        return;
    }else if(strcmp(argv[1], "-h") == 0){
        printf("usage: cat <name>. Prints file contents\n");
        return;
    }else if(strcmp(argv[1], "--help") == 0){
        printf("usage: cat <name>. Prints file contents\n");
        return;
    }
    void* file = ramdisk_openat(shellbuiltins_current_dir, argv[1]);
    if(file == NULL){
        printf("cat: %s: No such file or directory\n", argv[1]);
        return;
    }
    print_file(file);
    ramdisk_close(file);
}

void shellbuiltins_echo(int argc, char** argv){
    for(int i = 1; i < argc; ++i){
        printf("%s ", argv[i]);
    }
    printf("\n");
}

char* color_names[] = {
    "black",
    "blue",
    "green",
    "cyan",
    "red",
    "magenta",
    "brown",
    "light_grey",
    "dark_grey",
    "light_blue",
    "light_green",
    "light_cyan",
    "light_red",
    "light_magenta",
    "light_brown",
    "white"
};

void shellbuiltins_setcolor(int argc, char** argv){
    if(argc != 3){
        goto help;
    }
    size_t param_type = 0;
    if(strcmp(argv[1], "background") == 0){
        param_type = 1;
    }
    if(strcmp(argv[1], "foreground") == 0){
        param_type = 2;
    }
    if(strcmp(argv[1], "cmd") == 0){
        param_type = 3;
    }
    if(strcmp(argv[1], "user") == 0){
        param_type = 4;
    }
    if(strcmp(argv[1], "$") == 0){
        param_type = 5;
    }
    if(param_type == 0){
        goto help;
    }
    for(size_t i = 0; i < ARRSIZE(color_names); ++i){
        if(strcmp(color_names[i], argv[2]) == 0){
            switch(param_type){
                case 1:
                    shellbuiltins_theme->background_color = i;
                    video_set_background(i);
                    break;
                case 2:
                    shellbuiltins_theme->foreground_color = i;
                    break;
                case 3:
                    shellbuiltins_theme->cmd_color = i;
                    break;
                case 4:
                    shellbuiltins_theme->user_color = i;
                    break;
                case 5:
                    shellbuiltins_theme->path_sep_color = i;
                    break;
            }
            video_clear_screen();
            return;
        }
    }
help:
    printf("usage: setcolor <param> <color>. Available colors:\n");
    for(size_t i = 0; i < ARRSIZE(color_names); ++i){
        printf("%s\n", color_names[i]);
    }
    printf("Params: background, foreground, cmd, user, $\n");
}