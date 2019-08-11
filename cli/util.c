#include "util.h"

#include <string.h>

int eq_ignore_case(const char *one, const char *two) {
    return strcasecmp(one, two) == 0;
}

int is_empty(const char *string) {
    return string[0] == '\0';
}
