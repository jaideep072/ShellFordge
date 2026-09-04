#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/*
 * Convert a command line into a linked list of tokens.
 *
 * Example:
 *     ls -l | grep txt > output.txt
 *
 * becomes:
 *     WORD(ls)
 *     WORD(-l)
 *     PIPE(|)
 *     WORD(grep)
 *     WORD(txt)
 *     REDIRECT_OUT(>)
 *     WORD(output.txt)
 */
Token *lexer_tokenize(const char *input);

#endif
