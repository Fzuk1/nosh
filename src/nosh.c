/* Sources:
 * - https://medium.com/@nyangaresibrian/simple-shell-b7014425601f
 * - https://www.ibm.com/docs/en/aix/7.2.0?topic=administration-operating-system-shells
 * - https://www.gnu.org/software/bash/manual/html_node/Bash-Builtins.html
 * - https://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html
 * - https://brennan.io/2015/01/16/write-a-shell-in-c/
 *
 * Currently working on:
 * - Done for now
 *
 * TODOS:
 * - Hook up some package manager (apt?, pacman?, ...?)
 * - Write better code like here: "https://brennan.io/2015/01/16/write-a-shell-in-c/"
 * - Add malloc error checking
 * -
 */

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Builtin.h"

#define MAX_LINE_SIZE 1024

void nosh_shell_loop();
char *nosh_read_line();
char **nosh_str_split(char *aStr, const char aDelim);
void nosh_search_split_quotes(char **args);
int nosh_execute_cmd(char **args);
int nosh_launch_cmd(char **args);

int main() {
    /*
      Entry point.
      Setup configuration.
      Cleanup after finishing execution.
    */

    // Load config files, like cmd history

    // Set the SHELL and PWD enviroment variables
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1)
        perror("nosh: readlink");
    else {
        path[len] = '\0';
        if (setenv("SHELL", path, 1) == -1)
            perror("nosh: setenv");
        if (!getenv("PWD"))
            if (setenv("PWD", getenv("HOME"), 1) == -1)
                perror("nosh: setenv");

        // Main loop
        nosh_shell_loop();
    }

    // Cleanup

    return 0;
}

void nosh_shell_loop() {
    /*
      Print prompt.
      Read in line.
      Split line into args.
      Execute cmd with args.
    */

    char *line;
    char **args;
    int status;

    do {
        // Print "PATH-#" as the input prompt, to always know where you are
        char *curr_path = getenv("PWD");
        printf("\033[34;1m%s\033[0m$ ", curr_path);

        line = nosh_read_line();
        // Check for enter press
        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        args = nosh_str_split(line, ' ');
        // Search for split up args of type ["Hello,] [World"]
        nosh_search_split_quotes(args);
        status = nosh_execute_cmd(args);

        // Free all mallocs of current loop
        free(line);
        for (int i = 0; *(args + i); i++)
            free(*(args + i));
        free(args);
    } while (!status);
}

char *nosh_read_line() {
    /*
      Read a line from stdin to buffer.
    */

    int bufsize = MAX_LINE_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * MAX_LINE_SIZE);
    int c;

    if (!buffer) {
        perror("nosh: malloc");
        exit(EXIT_FAILURE);
    }

    while (true) {
        // Read next char
        c = getchar();

        // Check for EOF or \n
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
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

void nosh_search_split_quotes(char **args) {
    /*
      Search in the arguments for quotes and combine the args between quotes.
    */

    char *firstQuote = NULL;
    char *secondQuote = NULL;
    int i = 0;
    int j;

    do {
        // Find out where first quote is
        int done = false;
        firstQuote = NULL;
        for (; args[i] && !done; i++) {
            for (j = 0; j < (int)strlen(args[i]); j++) {
                if (args[i][j] == '"') {
                    firstQuote = &args[i][j];
                    done = true;
                    i--;
                    break;
                }
            }
        }

        // Find out where second quote is
        int k = i;
        int l = j;
        secondQuote = NULL;
        done = false;
        for (k = i; args[k] && !done; k++) {
            for (l = j + 1; l < (int)strlen(args[k]); l++) {
                if (args[k][l] == '"') {
                    secondQuote = &args[k][l];
                    done = true;
                    k--;
                    break;
                }
            }
        }

        // Move quoted args into one arg
        if (firstQuote && secondQuote) {
            for (int m = i + 1; m <= k; m++) {
                strcat(args[i], " ");
                strcat(args[i], args[m]);
            }

            // Clean up remainders
            for (; k - 1 > 0; k--)
                for (int m = i + 1; args[m]; m++) {
                    args[m] = args[m + 1];
                }

            i++;
        }
    } while (firstQuote && secondQuote);
}

int nosh_execute_cmd(char **args) {
    /*
      If cmd is builtin execute the command.
      If cmd is found in PATH, launch it.
    */

    const char *builtins[] = {"cd", "echo", "help", "pwd", "clear", "exec", "exit", NULL};

    // Check for ./ file execution
    if (args[0][0] == '.' && args[0][1] == '/') {
        nosh_dot_slash(args);
        return 0;
    }

    // Then check for builtin commands
    int i;
    for (i = 0; builtins[i]; i++) {
        if (strcmp(args[0], builtins[i]) == 0)
            break;
    }

    // Execute based on checks
    switch (i) {
    case 0: {
        if (!args[1] || args[2]) {
            printf("Usage: cd [dir]\n");
            return 0;
        }
        nosh_cd(args[1]);
        break;
    }
    case 1: {
        if (!args[1]) {
            printf("Usage: echo [text or $ENVVAR]\n");
            return 0;
        }
        nosh_echo(args);
        break;
    }
    case 2: {
        if (args[1]) {
            printf("Usage: help\n");
            return 0;
        }
        nosh_help();
        break;
    }
    case 3: {
        if (args[1]) {
            printf("Usage: pwd\n");
            return 0;
        }
        nosh_pwd();
        break;
    }
    case 4: {
        if (args[1]) {
            printf("Usage: clear\n");
            return 0;
        }
        nosh_clear();
        break;
    }
    case 5: {
        if (!args[1]) {
            printf("Usage: exec [program]\n");
            return 0;
        }
        nosh_exec(args);
        break;
    }
    case 6: {
        if (args[1]) {
            printf("Usage: exit\n");
            return 0;
        }
        return 1;
        break;
    }
    default: {
        nosh_launch_cmd(args);
        break;
    }
    }

    return 0;
}

int nosh_launch_cmd(char **args) {
    /*
      Create a child process and execute the cmd there.
      Parent waits for child to finish execution.
    */

    // Create a child process to execute a non builtin command
    int wStatus;
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
            printf("Command '%s' not found, but may be installable\n", args[0]);
        }
        exit(EXIT_FAILURE);

    } else {
        // Parent waits for child
        do {
            w = waitpid(cpid, &wStatus, WUNTRACED);
            if (w == -1) {
                perror("nosh: waitpid");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(wStatus)) {
                return 0;
            } else if (WIFSIGNALED(wStatus)) {
                printf("killed by signal %d\n", WTERMSIG(wStatus));
            } else if (WIFSTOPPED(wStatus)) {
                printf("stopped by signal %d\n", WSTOPSIG(wStatus));
            } else if (WIFCONTINUED(wStatus)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(wStatus) && !WIFSIGNALED(wStatus));

        return 1;
    }

    return 0;
}

char **nosh_str_split(char *aStr, const char aDelim) {
    /*
      Split aStr at every instance of aDelim.
      Return a char* array of split aStr.
    */

    char **result = 0;
    size_t count = 0;
    char *tmp = aStr;
    char *lastDelim = 0;
    char delim[2];
    delim[0] = aDelim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {

        if (aDelim == *tmp) {
            count++;
            lastDelim = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += lastDelim < (aStr + strlen(aStr) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (!result) {
        perror("nosh: malloc");
        exit(EXIT_FAILURE);
    }

    size_t idx = 0;
    char *token = strtok(aStr, delim);

    while (token) {

        assert(idx < count);
        // Equal to "result[idx++] = strdup(token)"
        *(result + idx++) = strdup(token);
        token = strtok(NULL, delim);
    }

    assert(idx == count - 1);
    *(result + idx) = NULL;

    return result;
}
