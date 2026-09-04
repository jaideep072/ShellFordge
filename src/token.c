#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token *token_create(TokenType type, const char *value)
{
    Token *token = malloc(sizeof(Token));

    if (token == NULL) {
        perror("malloc");
        return NULL;
    }

    token->type = type;
    token->next = NULL;

    if (value != NULL) {
        token->value = strdup(value);

        if (token->value == NULL) {
            perror("strdup");
            free(token);
            return NULL;
        }
    } else {
        token->value = NULL;
    }

    return token;
}

void token_append(Token **head, Token *new_token)
{
    if (head == NULL || new_token == NULL) {
        return;
    }

    if (*head == NULL) {
        *head = new_token;
        return;
    }

    Token *current = *head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = new_token;
}

void token_free(Token *head)
{
    Token *current = head;

    while (current != NULL) {
        Token *next = current->next;

        free(current->value);
        free(current);

        current = next;
    }
}

const char *token_type_to_string(TokenType type)
{
    switch (type) {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_REDIRECT_IN:
            return "REDIRECT_IN";

        case TOKEN_REDIRECT_OUT:
            return "REDIRECT_OUT";

        case TOKEN_REDIRECT_APPEND:
            return "REDIRECT_APPEND";

        case TOKEN_BACKGROUND:
            return "BACKGROUND";

        default:
            return "UNKNOWN";
    }
}
