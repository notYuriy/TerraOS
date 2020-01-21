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

void report_test_result(bool success){
    video_packed_color_t color = video_get_packed_color();
    if(success){
        video_set_foreground(light_green);
        printf("OK");
    }else{
        video_set_foreground(red);
        printf("Successful failure");
    }
    video_set_packed_color(color);
}

bool kheap_test(){
    void* result1 = kheap_malloc(16 MB);
    printf("Pointer to the first 16 MB pool %p\n", result1);
    void* result2 = kheap_malloc(16 MB);
    printf("Pointer to the second 16 MB pool %p\n", result2);
    return result1 != result2 && result1 != NULL && result2 != NULL;
}

bool idt_test(){
    printf("Calling int 57 just for fun\n");
    asm("int $57":::);
    return true;
}

bool timer_test(){
    for(size_t i = 0; i < 3; ++i){
        printf("sleeping for %llu seconds\n", i);
        time_sleep(i * 1000);
    }
    return true;
}

bool splitter_test(){
    char* paths[] = {
        "kek",
        ".",
        "*",
        "/",
        "/dev",
        "dev/",
        "/dev/",
        "/dev/str",
        "/dev/strio",
        "//",
        "dev//",
        "//dev",
        "usr//e/"
    };
    for(size_t i = 0; i < ARRSIZE(paths); ++i){
        splitted_path_node_t* node = splitter_split_path(paths[i]);
        printf("\nSplitting path \"%s\"\n", paths[i]);
        while(node != NULL){
            if(node->name == NULL){
                printf("This path is rooted\n");
            }else{
                printf("Going to the subdirectory \"%s\"\n", node->name);
            }
            node = node->next;
        }
        splitter_free_splitted_path(node);
    }
    return true;
}

bool getline_test(){
    printf("Login: ");
    char buf[10000];
    getline(buf, 10000);
    printf("\nhello, %s\n", buf);
    return true;
}

bool (*test_suites[])() = { 
    kheap_test,
    idt_test,
    timer_test,
    splitter_test,
    getline_test
};

void tests_run_list(bool (**tests)(void), size_t count){
    printf("Running test suites\n");
    for(size_t i = 0; i < count; ++i){
        printf("Running test %llu\n", i);
        video_set_foreground(light_grey);
        bool result = tests[i]();
        video_set_foreground(white);
        printf("Test result: ");
        report_test_result(result);
        video_putc('\n');
        if(!result){
            video_set_foreground(light_red);
            printf("As we can clearly see, this os is a +-><[].* piece of junk\n");
            printf("This can be reasonably confirmed from test results\n");
            printf("You should delete it and install linux instead\n");
            return;
        }
    }
    video_set_foreground(light_green);
    printf("Wow! All tests suites are completed!\n");
    printf("Nothing else to do for now, shutting down...\n");
    return;
}

void tests_run(void){
    tests_run_list(test_suites, ARRSIZE(test_suites));
}
