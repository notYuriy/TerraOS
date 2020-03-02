#ifndef __SHELL_BUILTINS_H_INCLUDED__
#define __SHELL_BUILTINS_H_INLCUDED__

void shellbuiltins_init(void* root);
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

#endif