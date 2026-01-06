/*
 * Currently working on:
 * - cd, relative paths
 *
 * TODOS:
 * - Write the shell builtin commands (cd, echo, exit(J), help, pwd(J), ...)
 * - Add malloc error checking
 * - Piping (stdout to stdin)
 * - Command history with <Arrow-Up> and <Arrow-Down>
 * - TAB Completion
 * - ... (TBD)
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#include "Builtin.h"

int posh_count_substrings(const char *str, const char *sub) {
    int count = 0;
    size_t subLen = strlen(sub);

    if (subLen == 0)
        return 0;

    const char *p = str;
    while ((p = strstr(p, sub)) != NULL) {
        count++;
        p += subLen;
    }

    return count;
}

void posh_remove_substring(char *str, int start, int end) {

  assert(end > start);
  memmove(str + start, str + end + 1, strlen(str) - end + 1);
  
}

void posh_remove_dotdot(char *str) {
  // Search for ".." in path and remove it plus previous directory
  char *position = NULL;
  position = strstr(str, "..");
  if (position) {
    
    int dotIndex = position - str;
    
    // Find previous '/'
    int start = dotIndex - 2;
    while (start >= 0 && str[start] != '/')
      start--;

    // Remove "/smth/.."
    if (start >= 0)
      posh_remove_substring(str, start, dotIndex + 1);

    if (strlen(str) == 0)
      str[0] = '/';
  }
}

void posh_cd(char *path) {

  // TODO Open Edge Case where PWD: "/", and cmd: "cd home/../home/uisfzuk/..", where there are
  // more ".." in path than in PWD, but not more than in path itself.
  // Might need: int slashes_in_path = countsubstrings(path, "/");
  
  // Edge Case: "cd ." or "cd ./"
  if (strcmp(path, ".") == 0 || strcmp(path, "./") == 0)
    return;

  // Check if there are more ".." than pwd is directories deep
  const char *PWD = getenv("PWD");
  int depth = posh_count_substrings(PWD, "/");
  int dotdots = posh_count_substrings(path, "..");
  
  // Edge Case: More ".." in path than "/" in PWD
  // AND
  // Edge Case: PWD: "/" -> "cd .."
  if (dotdots > depth || (strcmp(PWD, "/") == 0 && path[0] == '.' && path[1] == '.')) {
    if(!chdir("/")) {
      setenv("PWD", "/", 1);
    }
    else {
      perror("posh: cd");
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
        perror("posh: cd");
      }
      return;  
    }
    
    if (!chdir(path)) {

      // Remove .. before assigning path to PWD Variable
      while (strstr(path, ".."))
	posh_remove_dotdot(path);
      setenv("PWD", path, 1);

    }
    else {
      perror("posh: cd");
    }
    return;

  }
  else {
    
    // Make an absolute path out of PWD/path
    int PWDLen = strlen(PWD);
    int pathLen = strlen(path);
    char *newAbsPath = calloc(PWDLen + pathLen, sizeof(char));

    if (newAbsPath) {

      newAbsPath[0] = '\0';
      
      int i;
      for (i = 0; i < PWDLen; i++) {
        newAbsPath[i] = PWD[i];
      }
      
      if (strcmp(PWD, "/") != 0) {
        newAbsPath[i] = '/';
        i++;
      }
      
      for (int j = 0; j < pathLen; j++) {
        newAbsPath[i] = path[j];
	i++;
      }

      // chdir and set PWD
      if (!chdir(newAbsPath)) {

	// Remove every "/smth/.." before assigning the path to PWD Variable
	while (strstr(newAbsPath, ".."))
	  posh_remove_dotdot(newAbsPath);
	setenv("PWD", newAbsPath, 1);

      }
      else {
        perror("posh: cd");
      }
      
    }
    
   free(newAbsPath);

  }

  return;
}


void posh_echo() {
  UNIMPLEMENTED;
}


void posh_help() {
  UNIMPLEMENTED;
}


void posh_pwd() {
  const char *PWD = getenv("PWD");
  printf("%s\n", PWD);
}


void posh_clear() {
  // TODO Next feature to implement
  UNIMPLEMENTED;
}

