/*
 * Currently working on:
 * - echo or clear, gotta decide
 *
 * TODOS:
 * - Write the shell builtin commands (cd(J), echo, exit(J), help(J), pwd(J), exec(J), clear(J),
 * ...)
 * - Add malloc error checking
 * - Piping (stdout to stdin)
 * - Command history with <Arrow-Up> and <Arrow-Down>
 * - TAB Completion
 * - In command navigation with <Arrow-Left> and <Arrow-right>
 * - (TBD)
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Builtin.h"

#define MAX_PATH_SIZE 512

void nosh_set_pwd() {
    /*
      Get current working directory.
      Set PWD enviroment variable.
    */

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
    /*
      Change current working directory to path.
      Handle special inputs like '~' for the home directory.
      Keep PWD enviroment variable up to date.
    */

    // Make '~' = $HOME
    if (path[0] == '~') {
        char *changedPath = malloc(sizeof(char) * (strlen(getenv("HOME")) + strlen(path) + 2));
        if (!changedPath) {
            perror("nosh: malloc");
            exit(EXIT_FAILURE);
        }

        // Write the path from $HOME to changedPaths memory address
        char *home_path = getenv("HOME");
        for (int i = 0; home_path[i]; i++) {
            changedPath[i] = home_path[i];
        }

        // Remove first char (~) from path, to combine $HOME with path
        for (int i = 0; i < (int)strlen(path); i++) {
            path[i] = path[i + 1];
        }
        strcat(changedPath, path);

        // Chdir normally
        if (!chdir(changedPath)) {
            nosh_set_pwd();
        } else {
            perror("nosh: cd");
        }

        free(changedPath);
    } else {
        // Just change directory to path
        if (!chdir(path)) {
            nosh_set_pwd();
        } else {
            perror("nosh: cd");
        }
    }
}

void nosh_echo(char **args) {
    /*
      Print args back out to stdout.
      Handle $ENVVAR for enviroment variables.
      TODO: Handle #(EQUATION), to add math capabilities.
    */

    nosh_remove_arg0(args);
    if (args[0][0] == '$') {
        // Remove '$' from args[0], to pass it correctly to getenv
        for (int i = 0; i < (int)strlen(args[0]); i++) {
            args[0][i] = args[0][i + 1];
        }
        char *envVar = getenv(args[0]);
        if (envVar)
            printf("%s\n", envVar);
    } else if (args[0][0] == '#') {
        // TODO: Add math capabilities like #(1 + 2)
    } else {
        for (int i = 0; args[i]; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    }
}

void nosh_help() {
    /*
      Print helpful information for the user.
    */

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
    /*
      Print current working directory.
    */

    char *pwd = getenv("PWD");
    printf("%s\n", pwd);
}

void nosh_clear() {
    /*
      Clear the terminal emulators screen.
    */

    // TODO: Maybe make it better.

    // \033[3J → clear scrollback buffer (not POSIX, but widely supported)
    printf("\033[3J\033[2J\033[H");
    fflush(stdout);
    // Twice to also get last scrollback buffer
    printf("\033[3J\033[2J\033[H");
    fflush(stdout);
}

void nosh_remove_arg0(char **args) {
    /*
      Remove the first argument of args.
    */

    for (int i = 0; args[i]; i++) {
        args[i] = args[i + 1];
    }
}

void nosh_exec(char **args) {
    /*
      Replace current process with program and args.
    */

    nosh_remove_arg0(args);
    if (execvp(args[0], args) == -1)
        perror("nosh: exec");
}

void nosh_dot_slash(char **args) {
    /*
      Create a child process.
      Execute a program not found in PATH.
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
        if (execv(args[0], args) == -1) {
            // Print an error message if execv fails
            perror("nosh: execv");
            // printf("Command '%s' not found\n", args[0]);
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
                return;
            } else if (WIFSIGNALED(wStatus)) {
                printf("killed by signal %d\n", WTERMSIG(wStatus));
            } else if (WIFSTOPPED(wStatus)) {
                printf("stopped by signal %d\n", WSTOPSIG(wStatus));
            } else if (WIFCONTINUED(wStatus)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(wStatus) && !WIFSIGNALED(wStatus));

        return;
    }
}
