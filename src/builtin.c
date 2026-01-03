/*
 * Currently working on:
 * - cd, relative paths
 *
 * TODOS:
 * - Write the shell builtin commands (cd, echo, exit(J), help, pwd(J), ...)
 * - Add malloc error checking
 * - Piping (stdout to stdin)
 * - Command history with <Arrow-Up> and <Arrow-Down>
 */

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#include "Builtin.h"

int countsubstrings(const char *str, const char *sub) {
    int count = 0;
    size_t sublen = strlen(sub);

    if (sublen == 0)
        return 0;

    const char *p = str;
    while ((p = strstr(p, sub)) != NULL) {
        count++;
        p += sublen;
    }

    return count;
}

void removesubstring(char *str, int start, int end) {

  assert(end > start);
  memmove(str + start, str + end + 1, strlen(str) - end + 1);
  
}

void removedotdot(char *str) {
  // Search for ".." in path and remove it plus previous directory
  char *position = NULL;
  position = strstr(str, "..");
  if (position) {
    
    int dot_index = position - str;
    
    // Find previous '/'
    int start = dot_index - 2;
    while (start >= 0 && str[start] != '/')
      start--;

    // Remove "/smth/.."
    if (start >= 0)
      removesubstring(str, start, dot_index + 1);

    if (strlen(str) == 0)
      str[0] = '/';
  }
}

void poshcd(char *path) {

  // TODO Open Edge Case where PWD: "/", and cmd: "cd home/../home/uisfzuk/..", where there are
  // more ".." in path than in PWD, but not more than in path itself.
  // Might need: int slashes_in_path = countsubstrings(path, "/");
  
  // Edge Case: "cd ." or "cd ./"
  if (strcmp(path, ".") == 0 || strcmp(path, "./") == 0)
    return;

  // Check if there are more ".." than pwd is directories deep
  const char *PWD = getenv("PWD");
  int depth = countsubstrings(PWD, "/");
  int dotdots = countsubstrings(path, "..");
  printf("%d, %d\n", depth, dotdots);

  // Edge Case: More ".." in path than "/" in PWD
  // AND
  // Edge Case: PWD: "/" -> "cd .."
  if (dotdots > depth || (strcmp(PWD, "/") == 0 && path[0] == '.' && path[1] == '.')) {
    if(!chdir("/")) {
      setenv("PWD", "/", 1);
    }
    else {
      printf("-posh: cd: %s: No such file or directory\n", path);
    }
    return;
  }

  
  if (path[0] == '/') {

    // Edge Case: "cd /../smth"
    if (path[1] == '.' && path[2] == '.') {

      if(!chdir("/")) {
	setenv("PWD", "/", 1);
      }
      else {
	printf("-posh: cd: %s: No such file or directory\n", path);
      }
      return;
      
    }

    if (!chdir(path)) {

      // Remove .. before assigning path to PWD Variable
      while (strstr(path, ".."))
	removedotdot(path);
      setenv("PWD", path, 1);

    }
    else {
      printf("-posh: cd: %s: No such file or directory\n", path);
    }
    return;

  }
  else {
    
    // Make an absolute path out of PWD/path
    int PWD_len = strlen(PWD);
    int path_len = strlen(path);
    char *new_abs_path = calloc(PWD_len + path_len, sizeof(char));

    if (new_abs_path) {

      new_abs_path[0] = '\0';
      
      int i;
      for (i = 0; i < PWD_len; i++) {
	new_abs_path[i] = PWD[i];
      }
      
      if (strcmp(PWD, "/") != 0) {
        new_abs_path[i] = '/';
        i++;
      }
      
      for (int j = 0; j < path_len; j++) {
	new_abs_path[i] = path[j];
	i++;
      }


      // check if new_abs_path even exists
      DIR *dir = opendir(new_abs_path);
      if (dir) {

	closedir(dir);

	// chdir and set PWD
	if (!chdir(new_abs_path)) {

	  // Remove every "/smth/.." before assigning the path to PWD Variable
	  while (strstr(new_abs_path, ".."))
	    removedotdot(new_abs_path);
	  setenv("PWD", new_abs_path, 1);

	}
	else {
	  printf("chdir failure!\n");
	}

      }
      else if (ENOENT == errno) {
	printf("-posh: cd: %s: No such file or directory\n", path);
      }
      else {
	printf("opendir failure!\n");
      }
    
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

