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
 * Handles a command which loads a kernel or a metakernel
 * into the SPICE Toolkit kernel pool.
 *
 * Usage: LOAD KERNEL <filename>
 *
 * @param argc the argument count
 * @param argv the argument vector
 */
void load_kernel(int argc, char **argv);

#endif // GATE_COMMANDS_H
