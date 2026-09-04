#ifndef BUILTIN_H
#define BUILTIN_H

#include "parser.h"

/*
 * Check whether a command is a shell built-in command.
 *
 * Returns:
 *     1 -> built-in
 *     0 -> not a built-in
 */
int builtin_is_command(const char *command);

/*
 * Execute a built-in command.
 *
 * Returns:
 *     0 or greater -> normal shell status
 *     -1           -> command is not a built-in
 *
 * The shell's main loop will handle "exit"
 * separately when necessary.
 */
int builtin_execute(Command *command);

#endif
