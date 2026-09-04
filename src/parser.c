#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Command *command_create(void)
{
    Command *command = malloc(sizeof(Command));

    if (command == NULL) {
        perror("malloc");
        return NULL;
    }

    command->argc = 0;
    command->input_file = NULL;
    command->output_file = NULL;
    command->append_file = NULL;
    command->background = 0;
    command->next = NULL;

    for (int i = 0; i < MAX_ARGS; i++) {
        command->argv[i] = NULL;
    }

    return command;
}

static void command_append(Command **head, Command *new_command)
{
    if (head == NULL || new_command == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = new_command;
        return;
    }

    Command *current = *head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = new_command;
}

static int add_argument(Command *command, const char *value)
{
    if (command == NULL || value == NULL) {
        return 0;
    }

    if (command->argc >= MAX_ARGS - 1) {
        fprintf(stderr, "shellforge: too many arguments\n");
        return 0;
    }

    command->argv[command->argc] = strdup(value);

    if (command->argv[command->argc] == NULL) {
        perror("strdup");
        return 0;
    }

    command->argc++;

    /*
     * execvp() requires argv to end with NULL.
     */
    command->argv[command->argc] = NULL;

    return 1;
}

static int set_file(char **destination, const char *filename)
{
    if (destination == NULL || filename == NULL) {
        return 0;
    }

    *destination = strdup(filename);

    if (*destination == NULL) {
        perror("strdup");
        return 0;
    }

    return 1;
}

void command_free(Command *commands)
{
    Command *current = commands;

    while (current != NULL) {
        Command *next = current->next;

        for (int i = 0; i < current->argc; i++) {
            free(current->argv[i]);
        }

        free(current->input_file);
        free(current->output_file);
        free(current->append_file);

        free(current);

        current = next;
    }
}

Command *parser_parse(Token *tokens)
{
    if (tokens == NULL) {
        return NULL;
    }

    Command *commands = NULL;
    Command *current_command = command_create();

    if (current_command == NULL) {
        return NULL;
    }

    Token *current_token = tokens;

    while (current_token != NULL) {

        switch (current_token->type) {

        case TOKEN_WORD:
            if (!add_argument(current_command,
                              current_token->value)) {
                command_free(current_command);
                command_free(commands);
                return NULL;
            }
            break;

        case TOKEN_REDIRECT_IN:
            /*
             * The next token must be the input filename.
             */
            if (current_token->next == NULL ||
                current_token->next->type != TOKEN_WORD) {

                fprintf(stderr,
                        "shellforge: expected filename after '<'\n");

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            current_token = current_token->next;

            if (!set_file(&current_command->input_file,
                          current_token->value)) {

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            break;

        case TOKEN_REDIRECT_OUT:
            /*
             * The next token must be the output filename.
             */
            if (current_token->next == NULL ||
                current_token->next->type != TOKEN_WORD) {

                fprintf(stderr,
                        "shellforge: expected filename after '>'\n");

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            current_token = current_token->next;

            if (!set_file(&current_command->output_file,
                          current_token->value)) {

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            break;

        case TOKEN_REDIRECT_APPEND:
            /*
             * The next token must be the append filename.
             */
            if (current_token->next == NULL ||
                current_token->next->type != TOKEN_WORD) {

                fprintf(stderr,
                        "shellforge: expected filename after '>>'\n");

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            current_token = current_token->next;

            if (!set_file(&current_command->append_file,
                          current_token->value)) {

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            break;

        case TOKEN_BACKGROUND:
            current_command->background = 1;
            break;

        case TOKEN_PIPE:
            /*
             * A pipe separates two commands.
             */
            if (current_command->argc == 0) {
                fprintf(stderr,
                        "shellforge: invalid pipe\n");

                command_free(current_command);
                command_free(commands);
                return NULL;
            }

            command_append(&commands, current_command);

            current_command = command_create();

            if (current_command == NULL) {
                command_free(commands);
                return NULL;
            }

            break;
        }

        current_token = current_token->next;
    }

    /*
     * Don't add an empty final command.
     */
    if (current_command->argc > 0) {
        command_append(&commands, current_command);
    } else {
        free(current_command);
    }

    if (commands == NULL) {
        return NULL;
    }

    return commands;
}
