#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_ARGS 64

typedef struct Command {
    char *argv[MAX_ARGS];
    int argc;

    char *input_file;
    char *output_file;
    char *append_file;

    int background;

    struct Command *next;
} Command;

/*
 * Convert a token list into a linked list of commands.
 *
 * Example:
 *
 *     ls -l | grep txt > output.txt
 *
 * becomes:
 *
 *     Command 1:
 *         argv = ["ls", "-l"]
 *
 *     Command 2:
 *         argv = ["grep", "txt"]
 *         output_file = "output.txt"
 */
Command *parser_parse(Token *tokens);

/* Free the complete command list */
void command_free(Command *commands);

#endif
