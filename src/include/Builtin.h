#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>
#include <stdlib.h>

#define UNIMPLEMENTED						\
  do {								\
    fprintf(stderr, "%s:%d: %s is not implemented yet\n",	\
	    __FILE__, __LINE__, __func__);			\
  } while(0)


int countsubstrings(const char *str, const char *sub);
void removedotdot(char *str);
void removesubstring(char *str, int start, int end);
void poshcd(char *path);
void poshecho();
void poshhelp();
void poshpwd();
void poshclear();

#endif
