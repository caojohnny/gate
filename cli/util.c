#include <string.h>

int eq_ignore_case(const char *one, const char *two) {
    return strcasecmp(one, two) == 0;
}
