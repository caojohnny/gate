#include "options.h"
#include "util.h"
#include <stdlib.h>

#define ENUM_TO_STRING(STRING) #STRING,

static const char *OPTION_KEY_STRINGS[] = {
        FOREACH_OPTION_KEY(ENUM_TO_STRING)
};

static void *options[OPTION_KEY_LENGTH] = {
        OBSERVER_BODY_EARTH, NULL, NULL, NULL
};

option_key string_to_key(char *string) {
    for (int i = 0; i < OPTION_KEY_LENGTH; ++i) {
        const char *key_name = OPTION_KEY_STRINGS[i];
        if (eq_ignore_case(key_name, string)) {
            return i;
        }
    }

    return -1;
}

const char *key_to_string(option_key key) {
    return OPTION_KEY_STRINGS[key];
}

void set_option(option_key key, void *value) {
    void *current_value = options[key];
    if (current_value != OBSERVER_BODY_EARTH && current_value != NULL) {
        free(current_value);
    }

    options[key] = value;
}

void *get_option(option_key key) {
    return options[key];
}
