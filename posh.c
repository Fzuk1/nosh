/* Sources:
 * - https://medium.com/@nyangaresibrian/simple-shell-b7014425601f 
 * - https://www.ibm.com/docs/en/aix/7.2.0?topic=administration-operating-system-shells
 *
 * Currently working on:
 * - findcmd
 *
 * TODOS:
 * - Write a tokcmd function, to correctly handle flags like "ls -la" or "cd ~/"
 * - Write the shell builtin commands (cd, echo, exit, help, pwd, ...)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_SINGLE_PATH_SIZE 100
#define MAX_CMD_SIZE 100


void findcmd(char *cmd);
char **prspth(const char *PATH, int amount_of_paths);
char **tokcmd(const char *cmd, int amount_of_flags);

/*------------------------------------------------------------------------------------------*/

int main(int /*argc*/, char **/*argv*/, char **/*envp*/) {

  //pwd Env Variable
  //const char *PWD = getenv("PWD");
  //printf("%s\n%s\n", PATH, PWD);

  const char *PATH = getenv("PATH");

  // Get amount of /something/bin paths in $PATH
  int path_len = strlen(PATH);
  int amount_of_paths = 1;
  for (int i = 0; i < path_len; i++) {
    if (PATH[i] == ':') {
      amount_of_paths += 1;
    }
  }

  char **parsed_paths = prspth(PATH, amount_of_paths);

  // Print parsed paths
  for (int i = 0; i < amount_of_paths; i++) {
    printf("Path %d: %s\n", i, parsed_paths[i]);
  }

  
  char *cmd = malloc(MAX_CMD_SIZE * sizeof(char));

  
  while ((strcmp(cmd, "QUIT") != 0) && (strcmp(cmd, "exit") != 0)) {

    free(cmd);
    cmd = malloc(MAX_CMD_SIZE * sizeof(char));
    printf("$ ");
    //  %[^\n] makes scanf read until end of line instead of until whitespace
    scanf(" %[^\n]", cmd);
    findcmd(cmd);

  }

  
  // Free Malloced Memory of the paths
  for (int i = 0; i < amount_of_paths; i++) {
    free(parsed_paths[i]);
  }
  free(parsed_paths);
  
  
  free(cmd);


  return 0;
}

/*------------------------------------------------------------------------------------------*/

void findcmd(char *cmd) {

  // Get amount of flags in cmd
  int cmd_len = strlen(cmd);
  int amount_of_flags = 1;
  for (int i = 0; i < cmd_len; i++) {
    if (cmd[i] == ' ') {
      amount_of_flags += 1;
    }
  }
  
  
  char **tok_cmd = tokcmd(cmd, amount_of_flags);
  // TODO Search for "cmd" in the paths
  
  
  // And of the cmd
  for (int i = 0; i < amount_of_flags; i++) {
    free(tok_cmd[i]);
  }
  free(tok_cmd);


  return;
}

/*------------------------------------------------------------------------------------------*/

char **tokcmd(const char *cmd, int amount_of_flags) {

  // TODO Fix the bug where after the tokenized flags there appear random bytes of data

  int cmd_len = strlen(cmd);

  char **ret = malloc(amount_of_flags * sizeof(*ret));
  for (int i = 0; i < amount_of_flags; i++) {
    ret[i] = malloc(MAX_CMD_SIZE * sizeof(char));
  }

  int flag_i = 0;
  int last_spc = -1;
  for (int i = 0; i < cmd_len; i++) {
    if (cmd[i] == ' ') {
      int idx = 0;
      for (int j = last_spc + 1; j < i; j++) {
	ret[flag_i][idx] = cmd[j];
	idx += 1;
      }
      printf("Cmd flag %d: %s\n", flag_i, ret[flag_i]);
      flag_i += 1;
      last_spc = i;
    }
  }

  int idx = 0;
  for (int j = last_spc + 1; j < cmd_len; j++) {
    ret[flag_i][idx] = cmd[j];
    idx += 1;
  }
  printf("Cmd flag %d: %s\n", flag_i, ret[flag_i]);
  
  return ret;
}

/*------------------------------------------------------------------------------------------*/

// -----DONE-----
char **prspth(const char *PATH, int amount_of_paths) {
  int path_len = strlen(PATH);

  // Malloc memory for the parsed paths
  char **ret = malloc(amount_of_paths * sizeof(*ret));
  for (int i = 0; i < amount_of_paths; i++) {
    ret[i] = malloc(MAX_SINGLE_PATH_SIZE * sizeof(char));
  }

  // Parse the paths
  int path_i = 0;
  int last_col = -1;
  for (int i = 0; i < path_len; i++) {
    if (PATH[i] ==  ':') {
      int idx = 0;
      for (int j = last_col + 1; j < i; j++) {
	ret[path_i][idx] = PATH[j];
	idx += 1;
      }
      // printf("Path %d: %s\n", path_i, ret[path_i]);
      path_i += 1;
      last_col = i;
    }
  }

  // Parse the last path
  int idx = 0;
  for (int j = last_col + 1; j < path_len; j++) {
    ret[path_i][idx] = PATH[j];
    idx += 1;
  }
  
  return ret;
}


