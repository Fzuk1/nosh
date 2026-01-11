/*
 * Currently working on:
 * - echo or clear, gotta decide
 *
 * TODOS:
 * - Write the shell builtin commands (cd(J), echo, exit(J), help(J), pwd(J), exec(J), clear, ...)
 * - 
 * - Piping (stdout to stdin)
 * - Command history with <Arrow-Up> and <Arrow-Down>
 * - TAB Completion
 * - In command navigation with <Arrow-Left> and <Arrow-right>
 * - ... (TBD)
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "Builtin.h"


#define MAX_PATH_SIZE 512


void nosh_set_pwd() {
  char *cwd = malloc(sizeof(char) * MAX_PATH_SIZE);
  if (!cwd) {
    perror("nosh: malloc");
    exit(EXIT_FAILURE);
  }

  // TODO: MAX_PATH_SIZE may not be enough, then what?
  getcwd(cwd, MAX_PATH_SIZE);

  setenv("PWD", cwd, 1);
  free(cwd);
}

void nosh_cd(char *path) {
  // Make '~' = $HOME
  if (path[0] == '~') {
    char *changed_path = malloc(sizeof(char) * (strlen(getenv("HOME")) + strlen(path) + 2));
    if (!changed_path) {
      perror("nosh: malloc");
      exit(EXIT_FAILURE);
    }

    // Write the path from $HOME to changed_paths memory address
    char *home_path = getenv("HOME");
    for (int i = 0; home_path[i]; i++) {
      changed_path[i] = home_path[i];
    }

    // Remove first char (~) from path, to combine $HOME with path
    for (int i = 0; i < (int)strlen(path); i++) {
      path[i] = path[i + 1];
    }
    strcat(changed_path, path);
    
    // Chdir normally
    if (!chdir(changed_path)) {
      nosh_set_pwd();
    }
    else {
      perror("nosh: cd");
    }

    free(changed_path);
  }
  else {
    // Just change directory to path
    if (!chdir(path)) {
      nosh_set_pwd();
    }
    else {
      perror("nosh: cd");
    }
  }
}


void nosh_echo(char **args) {
  nosh_remove_arg0(args);
  if (args[0][0] == '$') {
    // Remove '$' from args[0], to pass it correctly to getenv
    for (int i = 0; i < (int)strlen(args[0]); i++) {
      args[0][i] = args[0][i + 1];
    }
    char *envVar = getenv(args[0]);
    if (envVar)
      printf("%s\n", envVar);
  }
  else if (args[0][0] == '#') {
    // TODO: Add math capabilities like #(1 + 2)
  }
  else {
    for (int i = 0; args[i]; i++) {
      printf("%s ", args[i]);
    }
    printf("\n");
  }
}


void nosh_help() {
  printf("Nosh Shell ");
  printf("Builtin Commands:\n");
  printf("\tcd [PATH] - Changes directory to PATH.\n");
  printf("\techo [SMTH] - Prints SMTH to stdout.\n");
  printf("\thelp - Lists builtin commands.\n");
  printf("\tpwd - Prints working directory to stdout.\n");
  printf("\tclear - Clears the screen.\n");
  printf("\texec [PROG] - Replaces current process with PROG.\n");
  printf("\texit - Exits the shell.\n");
  printf("For other commands use their man pages.\n");
}


void nosh_pwd() {
  char *pwd = getenv("PWD");
  printf("%s\n", pwd);
}


void nosh_clear() {
  // TODO: Next feature to implement
  // UNIMPLEMENTED;

  // \033[3J → clear scrollback buffer (not POSIX, but widely supported)
  printf("\033[3J\033[2J\033[H");
  fflush(stdout);
  // Twice to also get last scrollback buffer
  printf("\033[3J\033[2J\033[H");
  fflush(stdout);
}


void nosh_remove_arg0(char **args) {
  for (int i = 0; args[i]; i++) {
    args[i] = args[i + 1];
  }
}

void nosh_exec(char **args) {
  nosh_remove_arg0(args);
  if (execvp(args[0], args) == -1)
    perror("nosh: exec");
}


void nosh_dot_slash(char **args) {
  
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
    if (execv(args[0], args) == -1) {
      // Print an error message if execv fails
      perror("nosh: execv");
      // printf("Command '%s' not found\n", args[0]);
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
    
    return;

  }
}
