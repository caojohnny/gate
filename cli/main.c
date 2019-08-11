#include <cspice/SpiceUsr.h>
#include "clihandler.h"

void load_kernels() {
    furnsh_c("./kernels/hipparcos.bin");   // GENERIC STARS KERNEL
    furnsh_c("./kernels/naif0011.tls");    // GENERIC LSK KERNEL
    furnsh_c("./kernels/pck00010.tpc");    // GENERIC PCK KERNEL
    furnsh_c("./kernels/earth_fixed.tf");  // EARTH_FIXED ALIAS
    furnsh_c("./kernels/earth_000101_191026_190804.bpc");  // GENERIC ITRF93 KERNEL
}

int main() {
    erract_c("SET", 0, "REPORT");

    // TODO: Move this to a metakernel
    load_kernels();

    handle_cli();
    return 0;
}
