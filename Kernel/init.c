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

//#define RUN_TEST_SUITES 1

bool allocator_test_suite(){
    void* result1 = kheap_malloc(16 MB);
    printf("Pointer to the first 16 MB pool %p\n", result1);
    void* result2 = kheap_malloc(16 MB);
    printf("Pointer to the second 16 MB pool %p\n", result2);
    return result1 != result2 && result1 != NULL && result2 != NULL;
}

bool interrupt_handling_test(){
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

bool kr_ot_shterna(){
    printf("Haha, you will fail me!\n");
    return false;
}

bool path_splitter_test(){
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
            time_sleep(500);
        }
        splitter_free_splitted_path(node);
    }
    return true;
}

bool (*test_suites[])() = { 
    allocator_test_suite, 
    interrupt_handling_test, 
    timer_test,
    path_splitter_test,
    kr_ot_shterna
};

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

void system_init(void){
    printf("Terra OS. 64 bit operating system project\n");
    printf("Copyright @notYuriy. Project is licensed under MIT license\n");
#ifdef RUN_TEST_SUITES
        printf("Running test suites\n");
        for(size_t i = 0; i < ARRSIZE(test_suites); ++i){
            printf("Running test %llu\n", i);
            video_set_foreground(light_grey);
            bool result = test_suites[i]();
            video_set_foreground(cyan);
            printf("Test result: ");
            report_test_result(result);
            video_putc('\n');
            if(!result){
                video_set_foreground(light_red);
                printf("As we can clearly see, this os is a +-><[].* piece of junk\n");
                printf("This can be reasonably confirmed from test results\n");
                printf("You should delete it and install linux instead\n");
                printf("Nothing else to do for now, shutting down...\n");
                return;
            }
        }
        video_set_foreground(light_green);
        printf("Wow! All tests suites are completed!\n");
        printf("Nothing else to do for now, shutting down...\n");
        return;
#endif
    printf("Keyboard test now\n");
    kybrd_event_t c;
    while(true){
        printf("%c", (c = kybrd_poll_event()).code);
    }
}