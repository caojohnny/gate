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
 * @param argv the argument vector
 */
void set(char **argv);

/**
 * Gets a CLI-specific option.
 *
 * Usage: GET <key>
 *
 * See the set() procedure for a full list of options.
 *
 * @param argv the argument vector
 */
void get(char **argv);

/**
 * Handles a command to load a file to obtain options or
 * kernels.
 *
 * Usage: LOAD <OPTIONS | KERNEL> <filename>
 *
 * @param argv the argument vector
 */
void load(char **argv);

/**
 * Handles a command to show tables.
 *
 * Usage: SHOW <TABLES>
 *
 * @param argv the argument vector
 */
void show(char **argv);

#endif // GATE_COMMANDS_H
