#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_special_char(char c)
{
    return c == '|' || c == '<' || c == '>' || c == '&';
}

static char *copy_word(const char *start, size_t length)
{
    char *word = malloc(length + 1);

    if (word == NULL) {
        perror("malloc");
        return NULL;
    }

    memcpy(word, start, length);
    word[length] = '\0';

    return word;
}

static char *read_quoted_word(const char **input)
{
    const char *p = *input;
    char quote = *p;

    p++;

    size_t capacity = 64;
    size_t length = 0;

    char *word = malloc(capacity);

    if (word == NULL) {
        perror("malloc");
        return NULL;
    }

    while (*p != '\0' && *p != quote) {
        if (length + 1 >= capacity) {
            capacity *= 2;

            char *temp = realloc(word, capacity);

            if (temp == NULL) {
                perror("realloc");
                free(word);
                return NULL;
            }

            word = temp;
        }

        word[length++] = *p;
        p++;
    }

    if (*p == quote) {
        p++;
    } else {
        fprintf(stderr, "shellforge: unmatched quote\n");
        free(word);
        return NULL;
    }

    word[length] = '\0';
    *input = p;

    return word;
}

static char *read_word(const char **input)
{
    const char *start = *input;
    const char *p = *input;

    size_t capacity = 64;
    size_t length = 0;

    char *word = malloc(capacity);

    if (word == NULL) {
        perror("malloc");
        return NULL;
    }

    while (*p != '\0' &&
           !isspace((unsigned char)*p) &&
           !is_special_char(*p) &&
           *p != '\'' &&
           *p != '"') {

        if (length + 1 >= capacity) {
            capacity *= 2;

            char *temp = realloc(word, capacity);

            if (temp == NULL) {
                perror("realloc");
                free(word);
                return NULL;
            }

            word = temp;
        }

        word[length++] = *p;
        p++;
    }

    /*
     * If the word contains normal characters, return it.
     * Otherwise the caller will handle the special character/quote.
     */
    if (length == 0) {
        free(word);

        if (p == start) {
            return NULL;
        }

        return copy_word(start, 0);
    }

    word[length] = '\0';
    *input = p;

    return word;
}

static int add_token(Token **tokens, TokenType type, const char *value)
{
    Token *token = token_create(type, value);

    if (token == NULL) {
        return 0;
    }

    token_append(tokens, token);

    return 1;
}

Token *lexer_tokenize(const char *input)
{
    if (input == NULL) {
        return NULL;
    }

    Token *tokens = NULL;
    const char *p = input;

    while (*p != '\0') {

        /* Skip whitespace */
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }

        /* Pipe */
        if (*p == '|') {
            if (!add_token(&tokens, TOKEN_PIPE, "|")) {
                token_free(tokens);
                return NULL;
            }

            p++;
            continue;
        }

        /* Input redirection */
        if (*p == '<') {
            if (!add_token(&tokens, TOKEN_REDIRECT_IN, "<")) {
                token_free(tokens);
                return NULL;
            }

            p++;
            continue;
        }

        /* Output redirection / append */
        if (*p == '>') {

            if (*(p + 1) == '>') {
                if (!add_token(&tokens, TOKEN_REDIRECT_APPEND, ">>")) {
                    token_free(tokens);
                    return NULL;
                }

                p += 2;
            } else {
                if (!add_token(&tokens, TOKEN_REDIRECT_OUT, ">")) {
                    token_free(tokens);
                    return NULL;
                }

                p++;
            }

            continue;
        }

        /* Background execution */
        if (*p == '&') {
            if (!add_token(&tokens, TOKEN_BACKGROUND, "&")) {
                token_free(tokens);
                return NULL;
            }

            p++;
            continue;
        }

        /*
         * Quoted string.
         * The contents of the quotes become one WORD token.
         */
        if (*p == '\'' || *p == '"') {
            char *word = read_quoted_word(&p);

            if (word == NULL) {
                token_free(tokens);
                return NULL;
            }

            if (!add_token(&tokens, TOKEN_WORD, word)) {
                free(word);
                token_free(tokens);
                return NULL;
            }

            free(word);
            continue;
        }

        /*
         * Normal word.
         */
        {
            char *word = read_word(&p);

            if (word == NULL) {
                token_free(tokens);
                return NULL;
            }

            /*
             * This should only happen if there was an unexpected
             * zero-length word.
             */
            if (word[0] == '\0') {
                free(word);
                p++;
                continue;
            }

            if (!add_token(&tokens, TOKEN_WORD, word)) {
                free(word);
                token_free(tokens);
                return NULL;
            }

            free(word);
        }
    }

    return tokens;
}
