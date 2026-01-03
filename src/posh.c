/* Sources:
 * - https://medium.com/@nyangaresibrian/simple-shell-b7014425601f 
 * - https://www.ibm.com/docs/en/aix/7.2.0?topic=administration-operating-system-shells
 *
 * Currently working on:
 * - builtin.c
 *
 * TODOS:
 * - (FIXED line: 75) When cmd == "" strsplit assertion failes
 * - Add malloc error checking
 * - Cant open posh in posh due to signal 11:
 *   Line 142: printf("killed by signal %d\n", WTERMSIG(wstatus));
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>

#include "Builtin.h"


#define MAX_CMD_SIZE 100


void handleinput(char *cmd, char **path_tokens);
char *findcmd(char *cmd, char **path_tokens);
void execcmd(char **cmd_tokens, char *path);
char **strsplit(char *a_str, const char a_delim);

/*------------------------------------------------------------------------------------------*/

int main(int /*argc*/, char **/*argv*/, char **/*envp*/) {

  //pwd Env Variable
  //const char *PWD = getenv("PWD");
  //printf("%s\n%s\n", PATH, PWD);

  const char *PATH = getenv("PATH");
  
  char *path = strdup(PATH);
  char **path_tokens = strsplit(path, ':');
  free(path);

  
  char *cmd = malloc(MAX_CMD_SIZE * sizeof(char));

  if (cmd) {
    bool running = true;
    while (running) {

      free(cmd);
      cmd = malloc(MAX_CMD_SIZE * sizeof(char));
      if (cmd) {
	printf("$ ");
    
	// Don't use scanf because it comes with a lot of error possibilities
	// Commands longer than MAX_CMD_SIZE are not read by fgets
	if (fgets(cmd, MAX_CMD_SIZE, stdin)) {
	  if (!strchr(cmd, '\n')) {
	    int tmp;
	    // Get rid of the rest of the line
	    while ((tmp = getchar()) != '\n' && tmp != EOF) {
	      ;
	    }
	    printf("Command too long!\n");
	    continue;
	  }
	  if (cmd[0] == '\n' && cmd[1] == '\0') {
	    continue;
	  }
	  
	  cmd[strcspn(cmd, "\n")] = 0;
	  if (strcmp(cmd, "exit") == 0) {
	    running = false;
	    break;
	  }
	  handleinput(cmd, path_tokens);
	  
	}
      }
    }
  }
  
  // Free Malloced Memory of the paths
  if (path_tokens) {
    int i;
    for (i = 0; *(path_tokens + i); i++) {
      // printf("Path %d: %s\n", i, *(path_tokens + i));
      free(*(path_tokens + i));
    }
    free(path_tokens);
  }

  
  free(cmd);


  return 0;
}

/*------------------------------------------------------------------------------------------*/

void handleinput(char *cmd, char **path_tokens) {

  char **cmd_tokens = strsplit(cmd, ' ');
  // Search for "cmd" in the paths and pass cmd_tokens and the valid path to execcmd


  if (strcmp(cmd, "cd") == 0) {
    poshcd(cmd_tokens[1]);
  }
  else if (strcmp(cmd, "echo") == 0) {
    poshecho();
  }
  else if (strcmp(cmd, "help") == 0) {
    poshhelp();
  }
  else if (strcmp(cmd, "pwd") == 0) {
    poshpwd();
  }
  else if (strcmp(cmd, "clear") == 0) {

    // Set the EnvVar to the same value as Bash
    setenv("TERM", "xterm-256color", 1);
    poshclear();

  }
  else if (cmd[0] == '.' && cmd[1] == '/') {
    printf("File execution \"./\" is not implemeted yet\n");
  }
  else {

    char *path = NULL;
    path = findcmd(cmd, path_tokens);
    if (path)
      execcmd(cmd_tokens, path);

  }

  // Free malloced memory of the cmd_tokens
  if (cmd_tokens) {

    int i;
    for (i = 0; *(cmd_tokens + i); i++) {
      free(*(cmd_tokens + i));
    }
    free(cmd_tokens);

  }

}

/*------------------------------------------------------------------------------------------*/

void execcmd(char **cmd_tokens, char *path) {
  // Add the command to the path
  char *program = malloc(sizeof(char) * (strlen(path) + strlen(cmd_tokens[0]) + 1));

  program[0] = '\0';
  
  int wstatus;
  pid_t cpid, w;

  cpid = fork();
  if (cpid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (cpid == 0) {

    strcat(program, path);
    strcat(program, "/");
    strcat(program, cmd_tokens[0]);
    // printf("Program: %s\n", program);

    execve(program, cmd_tokens, NULL);

    perror("execve");  // Print an error message if execve fails
    exit(EXIT_FAILURE);

  }
  else {

    do {
      w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
      if (w == -1) {
	perror("waitpid");
	exit(EXIT_FAILURE);
      }
      if (WIFEXITED(wstatus)) {
	// printf("exited, status=%d\n", WEXITSTATUS(wstatus));
	free(program);
	return;
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
    
    free(program);
    exit(EXIT_SUCCESS);

  }


  return;
}

/*------------------------------------------------------------------------------------------*/

char *findcmd(char *cmd, char **path_tokens) {
 
  DIR *curr_dir;
  int i = 0;
  bool done = false;
  char *ret_path = NULL;
  
  while (path_tokens[i] != NULL && !done) {
    // printf("Path %d: %s\n", i, *(path_tokens + i));
    curr_dir = opendir(path_tokens[i]);
    if (curr_dir) {

      // printf("Opened: %s\n", path_tokens[i]);
      struct dirent *direntry;
      while ((direntry = readdir(curr_dir))) {

	if (strcmp(cmd, direntry->d_name) == 0) {

	  ret_path = path_tokens[i];
	  done = true;
	  break;

	}

      }
      closedir(curr_dir);

    }
    i++;
  }
  
  if (!done)
    printf("-posh: %s: Command not found!\n", cmd);
  
  
  return ret_path;
}

/*------------------------------------------------------------------------------------------*/
/*                                       DONE                                               */
/*------------------------------------------------------------------------------------------*/

char **strsplit(char *a_str, const char a_delim) {

  char** result = 0;
  size_t count = 0;
  char* tmp = a_str;
  char* last_spc = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp) {

    if (a_delim == *tmp) {
      count++;
      last_spc = tmp;
    }
    tmp++;

  }

  /* Add space for trailing token. */
  count += last_spc < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = malloc(sizeof(char*) * count);

  if (result) {

    size_t idx  = 0;
    char* token = strtok(a_str, delim);

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


