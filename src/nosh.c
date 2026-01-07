/* Sources:
 * - https://medium.com/@nyangaresibrian/simple-shell-b7014425601f 
 * - https://www.ibm.com/docs/en/aix/7.2.0?topic=administration-operating-system-shells
 * - https://www.gnu.org/software/bash/manual/html_node/Bash-Builtins.html
 * - https://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html
 * - https://brennan.io/2015/01/16/write-a-shell-in-c/
 *
 * Currently working on:
 * - builtin.c
 *
 * TODOS:
 * - Write better code like here: "https://brennan.io/2015/01/16/write-a-shell-in-c/"
 * - Maybe "cd.c", "clear.c", "echo.c", ... better than just "builtin.c"
 * - Add malloc error checking
 * -
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <limits.h>

#include "Builtin.h"


#define MAX_LINE_SIZE 1024


void nosh_shell_loop();
char *nosh_read_line();
char **nosh_str_split(char *aStr, const char aDelim);
int nosh_execute_cmd(char **args);

//char *nosh_find_cmd(char *cmd, char **pathTokens);

/*------------------------------------------------------------------------------------------*/

int main(int /*argc*/, char **/*argv*/, char **/*envp*/) {

  // Load config files, like cmd history

  
  // Set the SHELL enviroment variable
  char path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len == -1)
    perror("nosh: readlink");
  else {
    path[len] = '\0';
    if (setenv("SHELL", path, 1) == -1)
      perror("nosh: setenv");


    // Main loop
    nosh_shell_loop();
  }

  
  // Cleanup

  
  return 0;
}

/*------------------------------------------------------------------------------------------*/

void nosh_shell_loop() {
  
  char *line;
  char **args;
  int status;
    
  do {
    printf("# ");
    
    line = nosh_read_line();
    args = nosh_str_split(line, ' ');
    status = nosh_execute_cmd(args);

    free(line);
    for (int i = 0; *(args + i); i++)
      free(*(args + i));
    free(args);
  } while (!status);
  
}

/*------------------------------------------------------------------------------------------*/

char *nosh_read_line() {
  int bufsize = MAX_LINE_SIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * MAX_LINE_SIZE);
  int c;

  if (!buffer) {
    fprintf(stderr, "nosh: malloc error\n");
    exit(EXIT_FAILURE);
  }

  while (true) {
    // Read next char
    c = getchar();

    // Check for EOF or \n
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }
    else {
      buffer[position] = c;
    }
    position++;

    // If bufsize is too small
    if (position >= bufsize) {
      bufsize += MAX_LINE_SIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
	fprintf(stderr, "nosh: realloc error\n");
	exit(EXIT_FAILURE);
      }
    }
  }
}

/*------------------------------------------------------------------------------------------*/

int nosh_execute_cmd(char **args) {

  // Check if command matches builtin commands
  if (strcmp(args[0], "cd") == 0) {
    nosh_cd(args[1]);
  }
  else if (strcmp(args[0], "exit") == 0) {
    // Exit the do-while-loop by returning a non 0 value 
    return 1;
  }
  else if (strcmp(args[0], "echo") == 0) {
    nosh_echo(args);
  }
  else if (strcmp(args[0], "help") == 0) {
    nosh_help();
  }
  else if (strcmp(args[0], "pwd") == 0) {
    nosh_pwd();
  }
  else if (strcmp(args[0], "clear") == 0) {
    // Set the EnvVar to the same value as Bash
    // setenv("TERM", "xterm-256color", 1);
    nosh_clear();
  }
  else if (strcmp(args[0], "exec") == 0) {
    nosh_exec(args);
  }
  else if (args[0][0] == '.' && args[0][1] == '/') {
    printf("File execution \"./\" is not implemeted yet\n");
  }
  else {
    // Create a child process to execute a non builtin command
    int wstatus;
    pid_t cpid, w;

    cpid = fork();
    if (cpid == -1) {
      perror("nosh: fork");
      exit(EXIT_FAILURE);
    }

    if (cpid == 0) {
      // Child executes command
      if (execvp(args[0], args) == -1) {
	// Print an error message if execvp fails
	perror("nosh: execvp");
      }
      exit(EXIT_FAILURE);

    }
    else {
      // Parent waits for child
      do {
	w = waitpid(cpid, &wstatus, WUNTRACED);
	if (w == -1) {
	  perror("nosh: waitpid");
	  exit(EXIT_FAILURE);
	}
	if (WIFEXITED(wstatus)) {
	  // printf("exited, status=%d\n", WEXITSTATUS(wstatus));
	  return 0;
	}
	else if (WIFSIGNALED(wstatus)) {
	  printf("killed by signal %d\n", WTERMSIG(wstatus));
	}
	else if (WIFSTOPPED(wstatus)) {
	  printf("stopped by signal %d\n", WSTOPSIG(wstatus));
	}
	else if (WIFCONTINUED(wstatus)) {
	  printf("continued\n");
	}
      } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    
      return 1;

    }
  }
  return 0;
}

/*------------------------------------------------------------------------------------------

char *nosh_find_cmd(char *cmd, char **pathTokens) {
 
  DIR *currDir;
  int i = 0;
  bool done = false;
  char *retPath = NULL;
  
  while (pathTokens[i] != NULL && !done) {
    // printf("Path %d: %s\n", i, *(path_tokens + i));
    currDir = opendir(pathTokens[i]);
    if (currDir) {

      // printf("Opened: %s\n", path_tokens[i]);
      struct dirent *direntry;
      while ((direntry = readdir(currDir))) {

	if (strcmp(cmd, direntry->d_name) == 0) {

	  retPath = pathTokens[i];
	  done = true;
	  break;

	}

      }
      closedir(currDir);

    }
    i++;
  }
  
  if (!done)
    printf("-nosh: %s: Command not found!\n", cmd);
  
  
  return retPath;
}
*/

/*------------------------------------------------------------------------------------------*/
/*                                       DONE                                               */
/*------------------------------------------------------------------------------------------*/

char **nosh_str_split(char *aStr, const char aDelim) {

  char** result = 0;
  size_t count = 0;
  char* tmp = aStr;
  char* lastSpc = 0;
  char delim[2];
  delim[0] = aDelim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp) {

    if (aDelim == *tmp) {
      count++;
      lastSpc = tmp;
    }
    tmp++;

  }

  /* Add space for trailing token. */
  count += lastSpc < (aStr + strlen(aStr) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = malloc(sizeof(char*) * count);

  if (result) {

    size_t idx  = 0;
    char* token = strtok(aStr, delim);

    while (token) {

      assert(idx < count);
      *(result + idx++) = strdup(token);
      token = strtok(NULL, delim);

    }

    assert(idx == count - 1);
    *(result + idx) = NULL;

  }

  return result;
  
}


