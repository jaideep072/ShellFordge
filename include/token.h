#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_BACKGROUND
} TokenType;

typedef struct Token {
    TokenType type;
    char *value;
    struct Token *next;
} Token;

/* Create a new token */
Token *token_create(TokenType type, const char *value);

/* Add a token to the end of the token list */
void token_append(Token **head, Token *new_token);

/* Free the complete token list */
void token_free(Token *head);

/* Convert token type to readable text */
const char *token_type_to_string(TokenType type);

#endif
