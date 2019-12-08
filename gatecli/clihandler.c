#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cspice/SpiceUsr.h>
#include "clihandler.h"
#include "dispatcher.h"
#include "util.h"

static volatile int is_running = 1;

void handle_cli() {
    struct sigaction signal_handler = {handle_signal};
    sigaction(SIGINT, &signal_handler, NULL);

    while (is_running) {
        handle_input();
        puts("");
    }
}

void handle_input() {
    SpiceChar input_buffer[MAX_BUFFER_LEN];
    prompt_c("gate> ", MAX_BUFFER_LEN, input_buffer);

    if (input_buffer[0] == '\0') {
        puts("No input");
        return;
    }

    int argc = 0;
    char **argv = malloc(1 * sizeof(*argv));
    for (char *token = strtok(input_buffer, " ");
         token != NULL;
         token = strtok(NULL, " ")) {
        int idx = argc++;

        char **argv_new = realloc(argv, argc * sizeof(*argv));
        if (argv_new == NULL) {
            setmsg_c("Error allocating args vector");
            sigerr_c("alloc");
            handle_signal(SIGINT);
            return;
        } else {
            argv = argv_new;
        }

        argv[idx] = token;
    }

    if (argc == 0) {
        argc = 1;
        argv[0] = input_buffer;
    }

    handle_tokens(argc, argv);

    free(argv);
}

void handle_tokens(int argc, char **argv) {
    if (argc == 1 && eq_ignore_case("exit", argv[0])) {
        exit(0);
    }

    dispatch(argc, argv, &is_running);
}

void handle_signal(int signal) {
    is_running = 0;
}
