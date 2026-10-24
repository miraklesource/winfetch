#include <stdio.h>

#include "util.h"

int print_blue(const char *str) {
    if (!str) {
        fprintf(stderr, str,  "print_blue: ERROR - str argument is NULL.");
        return -1;
    }

    int byte_count = printf("\033[38;2;39;179;255m%s", str);
    // Reset color
    printf("\033[39m\033[49m");

    return byte_count;
}
