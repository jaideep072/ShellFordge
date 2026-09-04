#include "expand.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int append_char(char **buffer, size_t *length, size_t *capacity, char c)
{
    if (*length + 1 >= *capacity) {
        size_t new_capacity = (*capacity) * 2;

        char *temp = realloc(*buffer, new_capacity);

        if (temp == NULL) {
            return 0;
        }

        *buffer = temp;
        *capacity = new_capacity;
    }

    (*buffer)[(*length)++] = c;
    (*buffer)[*length] = '\0';

    return 1;
}

static int append_string(char **buffer,
                         size_t *length,
                         size_t *capacity,
                         const char *string)
{
    if (string == NULL) {
        return 1;
    }

    while (*string != '\0') {
        if (!append_char(buffer, length, capacity, *string)) {
            return 0;
        }

        string++;
    }

    return 1;
}

char *expand_variables(const char *input)
{
    if (input == NULL) {
        return NULL;
    }

    size_t capacity = 64;
    size_t length = 0;

    char *result = malloc(capacity);

    if (result == NULL) {
        perror("malloc");
        return NULL;
    }

    result[0] = '\0';

    const char *p = input;

    while (*p != '\0') {

        /*
         * Handle environment variables:
         *
         * $HOME
         * $USER
         * $PATH
         */
        if (*p == '$') {

            p++;

            /*
             * Handle "$?"
             * This will be supported properly later
             * through shell status tracking.
             */
            if (*p == '?') {
                const char *status = "0";

                if (!append_string(&result,
                                   &length,
                                   &capacity,
                                   status)) {
                    free(result);
                    return NULL;
                }

                p++;
                continue;
            }

            /*
             * Environment variable names:
             * letters, digits and underscore.
             */
            if (isalpha((unsigned char)*p) || *p == '_') {

                char variable_name[256];
                size_t name_length = 0;

                while ((isalnum((unsigned char)*p) || *p == '_') &&
                       name_length < sizeof(variable_name) - 1) {

                    variable_name[name_length++] = *p;
                    p++;
                }

                variable_name[name_length] = '\0';

                const char *value = getenv(variable_name);

                if (value != NULL) {
                    if (!append_string(&result,
                                       &length,
                                       &capacity,
                                       value)) {
                        free(result);
                        return NULL;
                    }
                }

                continue;
            }

            /*
             * If '$' is not followed by a valid variable name,
             * keep the '$' literally.
             */
            if (!append_char(&result, &length, &capacity, '$')) {
                free(result);
                return NULL;
            }

            continue;
        }

        /*
         * Normal character.
         */
        if (!append_char(&result, &length, &capacity, *p)) {
            free(result);
            return NULL;
        }

        p++;
    }

    return result;
}
