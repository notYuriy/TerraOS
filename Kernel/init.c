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

#define RUN_TEST_SUITES 0

void system_init(void){
    video_set_foreground(light_grey);
    printf("Terra OS. 64 bit operating system project\n");
    printf("Copyright @notYuriy. Project is licensed under MIT license\n");
#if RUN_TEST_SUITES == 1
    tests_run();
#endif
    while(true){
        video_set_foreground(light_green);
        printf("$");
        video_set_foreground(light_brown);
        printf("root> ");
        video_set_foreground(white);
        char buf[160];
        size_t size = getline(buf, ARRSIZE(buf));
        video_set_foreground(light_grey);
        if(strcmp(buf, "version") == 0){
            printf("Terra OS. 64 bit operating system project\n");
            printf("Copyright @notYuriy. Project is licensed under MIT license\n");
            goto cleanup;
        }
        if(strcmp(buf, "help") == 0){
            printf("help - show this message\n");
            printf("version - show kernel version\n");
            printf("cls - clear screen\n");
            printf("kernelmem - show kernel memory usage statistics\n");
            printf("license - print license\n");
            printf("tests - run test suites\n");
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
        }
        if(strcmp(buf, "license") == 0){
            printf(
                "MIT License\n"
                "\n"
                "Copyright (c) 2020 Yuriy Zamyatin\n"
                "\n"
                "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
                "of this software and associated documentation files (the \"Software\"), to deal\n"
                "in the Software without restriction, including without limitation the rights\n"
                "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
                "copies of the Software, and to permit persons to whom the Software is\n"
                "furnished to do so, subject to the following conditions:\n"
                "\n"
                "The above copyright notice and this permission notice shall be included in all\n"
                "copies or substantial portions of the Software.\n"
                "\n"
                "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
                "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
                "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
                "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
                "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
                "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
                "SOFTWARE.\n"
            );
            goto cleanup;
        }
        printf("Unknown command\n");
cleanup:
        memset(buf, size, '\0');
    }
    return;
}