#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdio.h>
#include <stdlib.h>

#define UNIMPLEMENTED						\
  do {								\
    fprintf(stderr, "%s:%d: %s is not implemented yet\n",	\
	    __FILE__, __LINE__, __func__);			\
  } while(0)


void poshcd();
void poshecho();
void poshhelp();
void poshpwd();
void poshclear();

#endif
