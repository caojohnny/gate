#include <stdio.h>
#include <cspice/SpiceUsr.h>
#include <string.h>
#include <time.h>
#include <timeconv.h>

#include "stars.h"
#include "topo.h"

int main() {
    furnsh_c("./kernels/hipparcos.bin");   // GENERIC STARS KERNEL
    furnsh_c("./kernels/naif0011.tls");    // GENERIC LSK KERNEL
    furnsh_c("./kernels/pck00010.tpc");    // GENERIC PCK KERNEL
    furnsh_c("./kernels/earth_fixed.tf");  // EARTH_FIXED ALIAS
    furnsh_c("./kernels/earth_000101_191026_190804.bpc");  // GENERIC ITRF93 KERNEL

    SpiceChar input[300];
    prompt_c("gate> ", 300, input);
    if (strcmp(input, "STAR") != 0) {
        printf("'%s' is not a valid command!");
        return 1;
    }

    gate_topo_frame seattle_topo;
    gate_load_earth_topo_frame("SEATTLE_TOPO", 48, -122, 0, &seattle_topo);

    SpiceInt table_count;
    ekntab_c(&table_count);
    if (table_count > 1) {
        setmsg_c("table_count=%d is greater than 1");
        errint_c("%d", table_count);
        sigerr_c("tablecount");

        return 1;
    }

    SpiceChar table_name[100];
    ektnam_c(0, 100, table_name);
    printf("Name of table at idx=0: %s\n", table_name);

    SpiceInt rows;
    gate_load_stars(table_name, "WHERE CATALOG_NUMBER = 70890", &rows);

    gate_star_info_spice1 stars[rows];
    gate_parse_stars(rows, stars);

    struct timespec cur_time;
    clock_gettime(CLOCK_REALTIME, &cur_time);

    SpiceDouble et;
    gate_unix_clock_to_et(cur_time, &et);

    for (int i = 0; i < rows; ++i) {
        gate_star_info_spice1 info = stars[i];

        SpiceDouble azimuth;
        SpiceDouble inclination;
        gate_calc_star_topo(seattle_topo, info, et, NULL, &azimuth, &inclination);
        printf("Look angle: az=%f inc=%f\n", azimuth, inclination);
    }

    gate_unload_topo_frame(seattle_topo);

    return 0;
}
