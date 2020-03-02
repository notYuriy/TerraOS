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
#include <kthread.h>
#include <ramdiskfs.h>

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
    kheap_traverse();
    void* result1 = kheap_malloc(16 MB);
    printf("Pointer to the first 16 MB pool %p\n", result1);
    void* result2 = kheap_malloc(16 MB);
    printf("Pointer to the second 16 MB pool %p\n", result2);
    kheap_free(result1); kheap_free(result2);
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
    printf("Splitter test\n");
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
        "/dev/main/lol//kek/cheburek//",
        "//",
        "dev//",
        "//dev",
        "usr//e/",
        ""
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
        //time_sleep(500);
    }
    return true;
}

bool getline_test(){
    printf("Login: ");
    char buf[10000];
    getline(buf, 10000);
    printf("hello, %s\n", buf);
    return true;
}

bool cmd_split_test(){
    char* cmds[] = {
        "",
        "test",
        "test test",
        "d f f",
        "d f f  a",
        "d \t e f",
        " d e f f",
        " dsfsf df e  f",
        "\"kek\"\"lol\"",
        " \"lel\" \"lel\" ",
        "lel \"lol kek\" \"cheburek\"",
        "k\"k"
    };
    for(size_t i = 0; i < ARRSIZE(cmds); ++i){
        printf("cmd: \"%s\" args count: %d\n", cmds[i], get_args_count(cmds[i]));
        cmd_args_t args = split_to_args(cmds[i]);
        for(size_t i = 0; i < (size_t)args.argc; ++i){
            printf("\t%s\n", args.argv[i]);
        }
        time_sleep(1000);
    }
    return true;
}

_Atomic size_t col = 2;
_Atomic size_t returned = 100;
_Atomic size_t kthread_id = 0;

void printer(){
    size_t mycol = (col++) % 14 + 2;
    size_t mythreadid = kthread_id++;
    for(int i = 0; i < 5; ++i){
        video_set_foreground(mycol);
        printf("Hi! I am thread %llu, using color %llu\n", mythreadid, mycol);
        time_sleep(1000);
    }
    video_set_foreground(mycol);
    printf("Thread %llu is terminating...\n", mythreadid);
    returned--;
}

bool multitasking_test(void){
    returned = 100;
    for(size_t i = 0; i < 100; ++i){
        kthread_summon(printer, 0);
    }
    while(atomic_load(&returned) > 0){
        time_sleep(100);
    }
    return true;
}

bool ramdisk_test(void){
    printf("Files in root directory\n");
    void* root = ramdisk_opendir("/");
    ramdisk_dirent_t buf;
    while(ramdisk_readdir(root, &buf, 1)){
        printf("%s\n", buf.file_name);
    }
    return true;
}

bool (*test_suites[])() = { 
    cmd_split_test,
    kheap_test,
    idt_test,
    timer_test,
    splitter_test,
    getline_test,
    multitasking_test,
    ramdisk_test
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
