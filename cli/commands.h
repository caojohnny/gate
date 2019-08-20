/**
 * Contains the prototypes for each command that gatecli
 * handles.
 */

#ifndef GATE_COMMANDS_H
#define GATE_COMMANDS_H

/**
 * Print the help message.
 */
void help();

/**
 * Sets a CLI-specific option.
 *
 * Usage: SET <key> <value>
 *
 * Options:
 *   - OBSERVER_BODY <body name>
 *     (default='Earth')
 *   - OBSERVER_LATITUDE <latitude>
 *     (default=NULL)
 *   - OBSERVER_LONGITUDE <longitude>
 *     (default=NULL)
 *   - STAR_TABLE <table name>
 *     (default=NULL)
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 */
void set(int argc, char **argv);

/**
 * Gets a CLI-specific option.
 *
 * Usage: GET <key>
 *
 * See the set() procedure for a full list of options.
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 */
void get(int argc, char **argv);

/**
 * Handles a command to load a file to obtain options or
 * kernels.
 *
 * Usage: LOAD <CMD | KERNEL> <filename>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void load(int argc, char **argv, volatile int *is_running);

/**
 * Handles a command to show tables.
 *
 * Usage: SHOW <TABLES>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 */
void show(int argc, char **argv);

/**
 * Handles a command to show star info or observation
 * position.
 *
 * Usage:
 * - STAR INFO <catalog number>
 * - STAR AZEL <CONT | count> <catalog number>
 *   <ISO time | NOW>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void star(int argc, char **argv, volatile int *is_running);

#endif // GATE_COMMANDS_H
