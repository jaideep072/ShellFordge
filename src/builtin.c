#include "builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int builtin_is_command(const char *command)
{
    if (command == NULL) {
        return 0;
    }

    return strcmp(command, "cd") == 0 ||
       strcmp(command, "pwd") == 0 ||
       strcmp(command, "echo") == 0 ||
       strcmp(command, "export") == 0 ||
       strcmp(command, "unset") == 0 ||
       strcmp(command, "exit") == 0;
}

int builtin_execute(Command *command)
{
    if (command == NULL ||
        command->argc == 0 ||
        command->argv[0] == NULL) {
        return -1;
    }

    const char *name = command->argv[0];

    /* cd */
    if (strcmp(name, "cd") == 0) {

        const char *directory;

        if (command->argc >= 2) {
            directory = command->argv[1];
        } else {
            directory = getenv("HOME");

            if (directory == NULL) {
                fprintf(stderr, "shellforge: HOME not set\n");
                return 1;
            }
        }

        if (chdir(directory) != 0) {
            perror("shellforge: cd");
            return 1;
        }

        return 0;
    }
/* echo */
if (strcmp(name, "echo") == 0) {

    for (int i = 1; i < command->argc; i++) {

        if (i > 1) {
            putchar(' ');
        }

        printf("%s", command->argv[i]);
    }

    putchar('\n');

    return 0;
}
    /* pwd */
    if (strcmp(name, "pwd") == 0) {

        char *current_directory = getcwd(NULL, 0);

        if (current_directory == NULL) {
            perror("shellforge: pwd");
            return 1;
        }

        printf("%s\n", current_directory);

        free(current_directory);

        return 0;
    }

    /* export */
    if (strcmp(name, "export") == 0) {

        if (command->argc == 1) {
            /*
             * Display environment variables.
             */
            extern char **environ;

            for (char **env = environ; *env != NULL; env++) {
                printf("%s\n", *env);
            }

            return 0;
        }

        for (int i = 1; i < command->argc; i++) {

            char *argument = command->argv[i];

            char *equals = strchr(argument, '=');

            if (equals == NULL) {
                fprintf(stderr,
                        "shellforge: export: invalid format: %s\n",
                        argument);
                return 1;
            }

            *equals = '\0';

            const char *variable = argument;
            const char *value = equals + 1;

            if (setenv(variable, value, 1) != 0) {
                perror("shellforge: export");
                *equals = '=';
                return 1;
            }

            *equals = '=';
        }

        return 0;
    }

    /* unset */
    if (strcmp(name, "unset") == 0) {

        for (int i = 1; i < command->argc; i++) {

            if (unsetenv(command->argv[i]) != 0) {
                perror("shellforge: unset");
                return 1;
            }
        }

        return 0;
    }

    /* exit */
    if (strcmp(name, "exit") == 0) {
        return 0;
    }

    return -1;
}
