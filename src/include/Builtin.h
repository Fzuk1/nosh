#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>

#define UNIMPLEMENTED						\
  do {								\
    fprintf(stderr, "%s:%d: %s is not implemented yet\n",	\
	    __FILE__, __LINE__, __func__);			\
  } while(0)


void nosh_set_pwd();
void nosh_cd(char *path);
void nosh_echo(char **args);
void nosh_help();
void nosh_pwd();
void nosh_clear();
void nosh_remove_arg0(char **args);
void nosh_exec(char **args);

#endif
