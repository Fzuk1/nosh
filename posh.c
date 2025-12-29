/* Sources:
 * - https://medium.com/@nyangaresibrian/simple-shell-b7014425601f 
 * - https://www.ibm.com/docs/en/aix/7.2.0?topic=administration-operating-system-shells
 *
 * Currently working on:
 * - findcmd
 *
 * TODOS:
 * - (CHECK) Write a tokcmd function, to correctly handle flags like "ls -la" or "cd ~/"
 * - Write the shell builtin commands (cd, echo, exit, help, pwd, ...)
 * - Add malloc error checking
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_SINGLE_PATH_SIZE 100
#define MAX_CMD_SIZE 100
#define MAX_FLAG_SIZE 30


void findcmd(char *cmd);
char **prspth(const char *PATH, int amount_of_paths);
char **tokcmd(char *a_str, const char a_delim);

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
  // TODO introduce more malloc error checking like this 
  if (parsed_paths) {
    for (int i = 0; i < amount_of_paths; i++) {
      printf("Path %d: %s\n", i, parsed_paths[i]);
    }
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

  char **tokens = tokcmd(cmd, ' ');
  // TODO Search for "cmd" in the paths and pass tokens to cmd
  printf("Cmd: %s\n", tokens[0]);
  
  if (tokens) {
    int i;
    for (i = 0; *(tokens + i); i++) {
      printf("flag %d: %s\n", i, *(tokens + i));
      free(*(tokens + i));
    }
    free(tokens);
  }

  
  return;
}

/*------------------------------------------------------------------------------------------*/
/*                                       DONE                                               */
/*------------------------------------------------------------------------------------------*/

char **tokcmd(char *a_str, const char a_delim) {

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
      token = strtok(0, delim);
    }
    assert(idx == count - 1);
    *(result + idx) = 0;
  }

  return result;
  
}

/*------------------------------------------------------------------------------------------*/
/*                                       DONE                                               */
/*------------------------------------------------------------------------------------------*/

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


