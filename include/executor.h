#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"

/*
 * Execute a complete command pipeline.
 *
 * Supports:
 *     normal commands
 *     pipes
 *     input redirection
 *     output redirection
 *     append redirection
 *     background execution
 */
int executor_execute(Command *commands);

#endif
