/*
 * Currently working on:
 * - cd, relative paths
 *
 * TODOS:
 * - Write the shell builtin commands (cd, echo, exit(J), help, pwd(J), ...)
 * - Add malloc error checking
 * - Cant open posh in posh due to signal 11:
 *   Line 142: printf("killed by signal %d\n", WTERMSIG(wstatus));
 */

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "Builtin.h"

void poshcd(char *path) {
  // UNIMPLEMENTED;

  bool absolute = false;

  if (path[0] == '/')
    absolute = true;

  if (absolute) {
    // just chdir and set PWD
    if (!chdir(path)) {
      setenv("PWD", path, 1);
    }
    else {
      printf("chdir failure!\n");
    }
  }
  else {
    // TODO relative to absolute path isn't reliable (random bytes)
    // make an absolute path out of PWD/path
    const char *PWD = getenv("PWD");
    int PWD_len = strlen(PWD);
    int path_len = strlen(path);
    char *new_abs_path = malloc(sizeof(char) * (PWD_len + path_len));
    new_abs_path[0] = '\0';
    
    int i;
    for (i = 0; i < PWD_len; i++) {
      new_abs_path[i] = PWD[i];
    }
    new_abs_path[i] = '/';
    i++;
    for (int j = 0; j < path_len; j++) {
      new_abs_path[i] = path[j];
      i++;
    }

    printf("%s/%s\n", PWD, path);
    printf("%s\n", new_abs_path);
    
    // check if new_abs_path even exists
    DIR *dir = opendir(new_abs_path);
    if (dir) {
      closedir(dir);

      // chdir and set PWD
      if (!chdir(path)) {
	setenv("PWD", new_abs_path, 1);
      }
      else {
	printf("chdir failure!\n");
      }
    }
    else if (ENOENT == errno) {
      printf("posh-: Directory doesn't exist\n");
    }
    else {
      printf("opendir failure!\n");
    }
    
    free(new_abs_path);
  }

  return;
}


void poshecho() {
  UNIMPLEMENTED;
}


void poshhelp() {
  UNIMPLEMENTED;
}


void poshpwd() {
  const char *PWD = getenv("PWD");
  printf("%s\n", PWD);
}


void poshclear() {
  UNIMPLEMENTED;
}

