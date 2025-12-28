/* Sources:
 * - https://medium.com/@nyangaresibrian/simple-shell-b7014425601f 
 * - https://www.ibm.com/docs/en/aix/7.2.0?topic=administration-operating-system-shells
 *
 * Currently working on:
 * - findcmd, prspath is done!
 *
 * TODOS:
 * - Write a tokenize_cmd function, to correctly handle flags like "ls -la" or "cd ~/"
 * - 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_SINGLE_PATH_SIZE 100


void findcmd(char *cmd);
char **prspth(const char *PATH, int amount_of_paths);


int main(int /*argc*/, char **/*argv*/, char **/*envp*/) {

  
  char *cmd_buf = malloc(100 * sizeof(char));

  
  while (strcmp(cmd_buf, "QUIT") != 0) {

    free(cmd_buf);
    cmd_buf = malloc(100 * sizeof(char));
    printf("$ ");
    //  %[^\n] makes scanf read until end of line instead of until whitespace
    scanf(" %[^\n]", cmd_buf);
    findcmd(cmd_buf);

  }

  
  free(cmd_buf);
  return 0;
}


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
// -----DONE-----


void findcmd(char *cmd) {
  const char *PATH = getenv("PATH");
  //pwd Env Variable
  //const char *PWD = getenv("PWD");
  //printf("%s\n%s\n", PATH, PWD);

  
  // Get Amount of /something/bin paths in $PATH
  int path_len = strlen(PATH);
  int amount_of_paths = 1;
  for (int i = 0; i < path_len; i++) {
    if (PATH[i] == ':') {
      amount_of_paths += 1;
    }
  }

  
  char **parsed_paths = prspth(PATH, amount_of_paths);
  // TODO Search for "cmd" in the paths
  

  // Print parsed paths
  /*
  for (int i = 0; i < amount_of_paths; i++) {
   printf("Path %d: %s\n", i, parsed_paths[i]);
  }
  */

  
  // Free Malloced Memory of the paths
  for (int i = 0; i < amount_of_paths; i++) {
    free(parsed_paths[i]);
  }
  free(parsed_paths);


  return;
}

