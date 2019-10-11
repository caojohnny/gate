#include <cspice/SpiceUsr.h>
#include <stdio.h>

#include "clihandler.h"

void load_cmd_files(int argc, char **argv) {
    if (argc == 1) {
        return;
    }

    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];
        char *cmd_argv[3] = { "LOAD", "CMD", arg };

        printf("Loading command file '%s'...\n", arg);
        handle_tokens(3, cmd_argv);
        puts("");
    }
}

int main(int argc, char **argv) {
    erract_c("SET", 0, "REPORT");

    load_cmd_files(argc, argv);
    handle_cli();

    return 0;
}
