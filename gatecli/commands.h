/**
 * Contains the prototypes for each command that gatecli
 * handles.
 */

#ifndef GATE_COMMANDS_H
#define GATE_COMMANDS_H

#include <cspice/SpiceUsr.h>

/**
 * Represents a satellite that has been added to the
 * gatecli database using orbital elements such as the
 * NORAD Keplerian TLE format.
 */
typedef struct {
    char *lines[2];

    SpiceDouble epoch;
    SpiceDouble elements[10];

    int is_deep_space;
} sat_data;

/**
 * Represents an object that has been added to the gatecli
 * database for the purpose of computing the position of an
 * object in the sky that does not have a NAIF ID and is
 * not a star mapped in a database, e.g. supernovae, black
 * holes, pulsars and other interesting objects.
 */
typedef struct {
    double ra;
    double dec;
    double r;

    double ra_pm;
    double dec_pm;
} calc_data;

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
 * kernels or an IAU Catalog of Star Names.
 *
 * The CSN file can be found at
 * https://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt
 *
 * Usage: LOAD <CMD | KERNEL | CSN> <filename>
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
 * Usage: SHOW <TABLES | FRAMES | CSN | BODIES | CALC>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void show(int argc, char **argv, volatile int *is_running);

/**
 * Handles a command to show star info or observation
 * position.
 *
 * Usage:
 * - STAR INFO <catalog number>
 * - STAR AZEL <catalog number> <CONT | count>
 *   <ISO time | NOW>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void star(int argc, char **argv, volatile int *is_running);

/**
 * Handles a command to show body info or observation
 * position.
 *
 * Usage:
 * - BODY INFO <naif id>
 * - BODY AZEL <naif id> <CONT | count>
 *   <ISO time | NOW>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void body(int argc, char **argv, volatile int *is_running);

/**
 * Handles a command to show satellite info or observation
 * position of a satellite given a TLE element set.
 *
 * Usage: TBD
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void sat(int argc, char **argv, volatile int *is_running);

/**
 * Perform a calculation on the observation position of
 * some other miscellaneous body, such as supernovae
 * or other bodies that do not have a  kernel or
 * NAIF data set.
 *
 * Usage:
 * - CALC ADD
 * - CALC REM <id>
 * - CALC INFO <id>
 * - CALC AZEL <id> <CONT | count> <ISO time | NOW>
 *
 * @param argc the number of arguments
 * @param argv the argument vector
 * @param is_running whether or not the program is or
 * should be running
 */
void calc(int argc, char **argv, volatile int *is_running);

#endif // GATECLI_COMMANDS_H
