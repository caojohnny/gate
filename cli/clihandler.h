/**
 * A CLI handler acts as the entry point for a program that
 * handles terminal input.
 */

#ifndef GATE_CLIHANDLER_H
#define GATE_CLIHANDLER_H

/**
 * Called by a program to initiate the CLI handler loop,
 * which only exits when a SIGINT is received and is not
 * caught by the currently running command.
 */
void handle_cli();

/**
 * Handles prompting and input from the terminal.
 *
 * This should be called in a loop from the handle_cli()
 * procedure.
 *
 * Additional parsing and tokenizing should also be handled
 * by this procedure prior to the input being passed off to
 * the handle_tokens() procedure.
 */
void handle_input();

/**
 * Handles the tokenized input from the handle_input()
 * procedure in order to pass it to a dispatcher.
 *
 * @param argc the argument count
 * @param argv the argument vector containing the input
 * tokens split by a space character
 */
void handle_tokens(int argc, char **argv);

/**
 * Handles a signal, such as SIGKILL, SIGINT, etc if
 * desired.
 *
 * Signal handlers should be registered by the entry point,
 * i.e. by the handle_cli() procedure.
 *
 * @param signal the signal passed to the handler
 */
void handle_signal(int signal);

#endif // GATE_CLIHANDLER_H
