#include "commands.h"
#include "util.h"
#include <stdio.h>

void dispatch(int argc, char **argv, volatile int *is_running) {
    char *label = argv[0];

    if (eq_ignore_case("HELP", label)) {
        return help();
    }

    if (eq_ignore_case("GET", label)) {
        return get(argc, argv);
    }

    if (eq_ignore_case("SHOW", label)) {
        return show(argc, argv, is_running);
    }

    if (eq_ignore_case("SET", label)) {
        return set(argc, argv);
    }

    if (eq_ignore_case("LOAD", label)) {
        return load(argc, argv, is_running);
    }

    if (eq_ignore_case("STAR", label)) {
        return star(argc, argv, is_running);
    }

    if (eq_ignore_case("BODY", label)) {
        return body(argc, argv, is_running);
    }

    if (eq_ignore_case("SAT", label)) {
        return sat(argc, argv, is_running);
    }

    if (eq_ignore_case("CALC", label)) {
        return calc(argc, argv, is_running);
    }

    puts("Command not recognized. Try typing 'HELP'");
}
