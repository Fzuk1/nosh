#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>
#include <stdlib.h>

#define UNIMPLEMENTED						\
  do {								\
    fprintf(stderr, "%s:%d: %s is not implemented yet\n",	\
	    __FILE__, __LINE__, __func__);			\
  } while(0)


int count_substrings(const char *str, const char *sub);
void remove_dotdot(char *str);
void remove_substring(char *str, int start, int end);
void posh_cd(char *path);
void posh_echo();
void posh_help();
void posh_pwd();
void posh_clear();

#endif
