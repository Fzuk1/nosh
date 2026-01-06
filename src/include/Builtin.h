#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>

#define UNIMPLEMENTED						\
  do {								\
    fprintf(stderr, "%s:%d: %s is not implemented yet\n",	\
	    __FILE__, __LINE__, __func__);			\
  } while(0)


int posh_count_substrings(const char *str, const char *sub);
void posh_remove_dotdot(char *str);
void posh_remove_substring(char *str, int start, int end);
void posh_cd(char *path);
void posh_echo();
void posh_help();
void posh_pwd();
void posh_clear();

#endif
