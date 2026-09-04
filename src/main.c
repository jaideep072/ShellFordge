#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "builtin.h"
#include "expand.h"
#include "executor.h"
#include "history.h"
#include "lexer.h"
#include "parser.h"

static void expand_command_arguments(Command *commands)
{
    Command *current = commands;

    while (current != NULL) {

        for (int i = 0; i < current->argc; i++) {

            char *expanded = expand_variables(current->argv[i]);

            if (expanded != NULL) {
                free(current->argv[i]);
                current->argv[i] = expanded;
            }
        }

        if (current->input_file != NULL) {

            char *expanded =
                expand_variables(current->input_file);

            if (expanded != NULL) {
                free(current->input_file);
                current->input_file = expanded;
            }
        }

        if (current->output_file != NULL) {

            char *expanded =
                expand_variables(current->output_file);

            if (expanded != NULL) {
                free(current->output_file);
                current->output_file = expanded;
            }
        }

        if (current->append_file != NULL) {

            char *expanded =
                expand_variables(current->append_file);

            if (expanded != NULL) {
                free(current->append_file);
                current->append_file = expanded;
            }
        }

        current = current->next;
    }
}

int main(void)
{
    printf("===================================\n");
    printf("            Shellforge             \n");
    printf("  A Unix Style Shell written in C  \n");
    printf("===================================\n");

    using_history();

    while (1) {

        char *line = readline("shellforge$ ");

        /*
         * Ctrl+D
         */
        if (line == NULL) {
            printf("\nGoodbye!\n");
            break;
        }

        /*
         * Ignore empty input.
         */
        if (line[0] == '\0') {
            free(line);
            continue;
        }

        /*
         * Add command to readline history.
         */
        add_history(line);

        /*
         * Keep the existing custom history command.
         */
        if (strcmp(line, "history") == 0) {
            print_history();
            free(line);
            continue;
        }

        /*
         * Convert input into tokens.
         */
        Token *tokens = lexer_tokenize(line);

        free(line);

        if (tokens == NULL) {
            continue;
        }

        /*
         * Convert tokens into commands.
         */
        Command *commands = parser_parse(tokens);

        token_free(tokens);

        if (commands == NULL) {
            continue;
        }

        /*
         * Expand environment variables.
         */
        expand_command_arguments(commands);

        /*
         * Handle exit before forking.
         */
        if (commands->next == NULL &&
            commands->argc > 0 &&
            strcmp(commands->argv[0], "exit") == 0) {

            command_free(commands);

            printf("Exiting...\n");
            break;
        }

        /*
         * Execute the parsed command.
         */
        executor_execute(commands);

        command_free(commands);
    }

    return 0;
}
