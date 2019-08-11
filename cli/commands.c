#include "commands.h"
#include <stdio.h>
#include <cspice/SpiceUsr.h>
#include <string.h>
#include <time.h>
#include <timeconv.h>

#include "stars.h"
#include "topo.h"

void help() {
    printf("HELP - prints this message\n");
    printf("LOAD KERNEL <filename> - loads a kernel or metakernel from file\n");
}

void load_kernel(int argc, char **argv) {
    furnsh_c(argv[2]);
}

void handle_star() {
    gate_topo_frame seattle_topo;
    gate_load_earth_topo_frame("SEATTLE_TOPO", 48, -122, 0, &seattle_topo);

    SpiceInt table_count;
    ekntab_c(&table_count);
    if (table_count > 1) {
        setmsg_c("table_count=%d is greater than 1");
        errint_c("%d", table_count);
        sigerr_c("tablecount");

        return;
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
        SpiceDouble elevation;
        gate_calc_star_topo(seattle_topo, info, et, NULL, &azimuth, &elevation);
        printf("Look angle: az=%f el=%f\n", azimuth, elevation);
    }

    gate_unload_topo_frame(seattle_topo);
}
