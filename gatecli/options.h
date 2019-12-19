/**
 * Stores options through the SET command from the CLI.
 */

#ifndef GATE_OPTIONS_H
#define GATE_OPTIONS_H

// Lifted this from https://stackoverflow.com/a/10966395
#define FOREACH_OPTION_KEY(to_key)      \
        to_key(OBSERVER_BODY)             \
        to_key(OBSERVER_LATITUDE)         \
        to_key(OBSERVER_LONGITUDE)        \
        to_key(STAR_TABLE)                \
        to_key(OPTION_KEY_LENGTH)
#define ENUM_TO_CONSTANT(ENUM) ENUM,

/**
 * An enumeration of different options that may be set.
 */
typedef enum {
    FOREACH_OPTION_KEY(ENUM_TO_CONSTANT)
} option_key;

/**
 * Default observer body constant to avoid issues with
 * freeing default values.
 */
static char *const OBSERVER_BODY_EARTH = "EARTH";

/**
 * Converts a string value into an option_key enum value.
 *
 * @param string the string to convert
 * @return the option_key value, or -1 if the name is not
 * resolvable
 */
option_key string_to_key(char *string);

/**
 * Converts a key to its respective string value.
 *
 * @param key the key to convert
 * @return the name of the key, in string form
 */
const char *key_to_string(option_key key);

/**
 * Assigns the given option to the given value.
 *
 * @param key the option type to set
 * @param value the option value, or NULL to unset
 */
void set_option(option_key key, void *value);

/**
 * Obtains the value of a particular option.
 *
 * @param key the option type to get
 * @return the option value pointer, or NULL if no value
 * set
 */
void *get_option(option_key key);

#endif // GATECLI_OPTIONS_H
