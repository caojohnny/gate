#include <cspice/SpiceUsr.h>
#include "clihandler.h"

int main() {
    erract_c("SET", 0, "REPORT");
    handle_cli();

    return 0;
}
