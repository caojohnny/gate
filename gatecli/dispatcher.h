/**
 * A dispatcher acts as a bridge between a CLI handler and
 * the command handlers.
 *
 * A dispatcher decides which handler gets what command,
 * in essence, dispatching the command to their respective
 * handlers.
 */

#ifndef GATE_DISPATCHER_H
#define GATE_DISPATCHER_H

/**
 * Dispatches commands to be handled by the individual
 * handlers.
 *
 * This procedure should only be invoked if there are
 * tokens available to be parsed, otherwise, the caller
 * should handle the case where no input has been entered
 * (i.e. argc = 0).
 *
 * @param argc the argument count
 * @param argv the argument vector, containing tokens
 * separated by a space character
 * @param is_running 1 to represent the fact that a SIGINT
 * has not been received yet
 */
void dispatch(int argc, char **argv, volatile int *is_running);

#endif // GATE_DISPATCHER_H
