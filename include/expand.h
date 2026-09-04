#ifndef EXPAND_H
#define EXPAND_H

/*
 * Expand environment variables in a string.
 *
 * Example:
 *
 *     Input:  "Hello $USER"
 *     Output: "Hello jaideep"
 *
 * The returned string is dynamically allocated
 * and must be freed by the caller.
 */
char *expand_variables(const char *input);

#endif
