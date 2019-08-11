#include "dispatcher.h"
#include "commands.h"
#include "util.h"
#include <stdio.h>

void dispatch(int argc, char **argv, volatile int *is_running) {
    char *label = argv[0];

    if (argc == 1) {
        if (eq_ignore_case("HELP", label)) {
            return help();
        }
    }

    if (argc == 3) {
        if (eq_ignore_case("LOAD", label)) {
            return load_kernel(argc, argv);
        }
    }

    printf("Command not recognized. Try typing 'help'.\n");
}
