#include <video.h>
#include <utils.h>
#include <multiboot2.h>
#include <phmmngr.h>
#include <vmcore.h>
#include <kheap.h>
#include <pathsplit.h>
#include <idt.h>
#include <spinlock.h>
#include <ksbrk.h>
#include <portio.h>
#include <pic.h>
#include <timer.h>
#include <time.h>
#include <kybrd.h>
#include <tty.h>
#include <tests.h>
#include <kthread.h>
#include <ramdiskfs.h>
#include <shellbuiltins.h>
#include <shell.h>

#define RUN_SHELL 0

void system_init(void){
    video_set_foreground(light_grey);
    printf("Terra OS. 64 bit operating system project\n");
    printf("Copyright @notYuriy. Project is licensed under MIT license\n");
    void* current_dir = ramdisk_opendir("/");
    shellbuiltins_init(current_dir);
    char buf[160]; memset(buf, 160, '\0');
    while(true){
        video_set_foreground(light_green);
        printf("$");
        video_set_foreground(light_brown);
        printf("root> ");
        video_set_foreground(white);
        size_t size = getline(buf, ARRSIZE(buf));
        video_set_foreground(light_grey);
        shell_run_cmd(buf);
        memset(buf, size, '\0');
    }
#if RUN_SHELL == 1
    void* current_dir = ramdisk_opendir("/");
    while(true){
        char buf[160];
        video_set_foreground(light_green);
        printf("$");
        video_set_foreground(light_brown);
        printf("root> ");
        video_set_foreground(white);
        size_t size = getline(buf, ARRSIZE(buf));
        video_set_foreground(light_grey);
        if(strcmp(buf, "version") == 0){
            printf("Terra OS version -1.0.0\n");
            goto cleanup;
        }
        if(strcmp(buf, "help") == 0){
            printf("help - show this message\n");
            printf("version - show kernel version\n");
            printf("cls/clear - clear screen\n");
            printf("kernelmem - show kernel memory usage statistics\n");
            printf("license - print license\n");
            printf("tests - run test suites\n");
            printf("cd - change directory\n");
            printf("ls - list files in a directory\n");
            printf("cat - show text file contents\n");
            printf("echo - print the message\n");
            goto cleanup;
        }
        if(strcmp(buf, "cls") == 0){
            video_clear_screen();
            goto cleanup;
        }
        if(strcmp(buf, "clear") == 0){
            video_clear_screen();
            goto cleanup;
        }
        if(strcmp(buf, "kernelmem") == 0){
            kheap_traverse();
            goto cleanup;
        }
        if(strcmp(buf, "tests") == 0){
            tests_run();
            goto cleanup;
        }
        if(strcmp(buf, "license") == 0){
            memcpy(buf, "cat /LICENSE", 12);
        }
        if(strcmp(buf, "ls") == 0){
            void* dir = ramdisk_opendirat(current_dir, "");
            ramdisk_dirent_t dirent;
            while(ramdisk_readdir(dir, &dirent, 1)){
                if(dirent.file_type == FT_REGULAR){
                    printf("\'%s\'\n", dirent.file_name);
                }else{
                    printf(" %s \n", dirent.file_name);
                }
            }
            ramdisk_closedir(dir);
            goto cleanup;
        }
        int pos = -1;
        if((pos = cmdeq(buf, "cd")) != -1){
            char* arg = buf + pos;
            void* newdir = ramdisk_opendirat(current_dir, arg);
            if(newdir == NULL){
                printf("cd: %s: No such file or directory\n", arg);
                goto cleanup;
            }
            ramdisk_closedir(current_dir);
            current_dir = newdir;
            goto cleanup;
        }
        if((pos = cmdeq(buf, "cat")) != -1){
            char* arg = buf + pos;
            void* file = ramdisk_openat(current_dir, arg);
            if(file == NULL){
                printf("cat: %s: No such file or directory\n", arg);
            }
            char contents_buf[256];
            memset(contents_buf, 256, '\0');
            size_t read = 0;
            bool place_whitespace = false;
            while((read = ramdisk_read(file, 256, contents_buf)) != 0){
                printf("%s", contents_buf);
                memset(contents_buf, read, '\0');
                place_whitespace = true;
            }
            ramdisk_close(file);
            if(place_whitespace){
                printf("\n");
            }
            goto cleanup;
        }
        if((pos = cmdeq(buf, "echo")) != -1){
            char* arg = buf + pos;
            printf("%s\n", arg);
            goto cleanup;
        }
        printf("Unknown command\n");
cleanup:
        memset(buf, size, '\0');
    }
    return;
#endif
}