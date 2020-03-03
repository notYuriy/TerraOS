#include <shell.h>
#include <tty.h>
#include <shellbuiltins.h>
#include <utils.h>
#include <video.h>

char* builtins_names[] = {
    "version",
    "man",
    "cls",
    "clear",
    "kernelmem",
    "tests",
    "license",
    "ls",
    "cd",
    "cat",
    "echo",
    "setcolor"
};

void (*builtins_functions[])(int, char**) = {
    shellbuiltins_version,
    shellbuiltins_man,
    shellbuiltins_cls,
    shellbuiltins_clear,
    shellbuiltins_kernelmem,
    shellbuiltins_tests,
    shellbuiltins_license,
    shellbuiltins_ls,
    shellbuiltins_cd,
    shellbuiltins_cat,
    shellbuiltins_echo,
    shellbuiltins_setcolor
};

void shell_run_cmd(char* cmd){
    cmd_args_t args = split_to_args(cmd);
    if(args.argc == 0){
        goto cleanup;
    }
    for(size_t i = 0; i < ARRSIZE(builtins_names); ++i){
        if(strcmp(args.argv[0], builtins_names[i]) == 0){
            (builtins_functions[i])(args.argc, args.argv);
            goto cleanup;
        }
    }
    printf("%s: command not found\n", args.argv[0]);
cleanup:
    free_args(args);
}