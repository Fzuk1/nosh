#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>

#define UNIMPLEMENTED						\
  do {								\
    fprintf(stderr, "%s:%d: %s is not implemented yet\n",	\
	    __FILE__, __LINE__, __func__);			\
  } while(0)


int nosh_count_substrings(const char *str, const char *sub);
void nosh_remove_dotdot(char *str);
void nosh_remove_substring(char *str, int start, int end);
void nosh_cd(char *path);
void nosh_echo(char **args);
void nosh_help();
void nosh_pwd();
void nosh_clear();
void nosh_remove_arg0(char **args);
void nosh_exec(char **args);

#endif
