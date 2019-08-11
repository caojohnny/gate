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

    if (argc == 2) {
        if (eq_ignore_case("GET", label)) {
            return get(argv);
        }

        if (eq_ignore_case("SHOW", label)) {
            return show(argv);
        }
    }

    if (argc == 3) {
        if (eq_ignore_case("SET", label)) {
            return set(argv);
        }

        if (eq_ignore_case("LOAD", label)) {
            return load(argv);
        }
    }

    puts("Command not recognized. Try typing 'HELP'");
}
