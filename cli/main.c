#include <stdio.h>
#include <cspice/SpiceUsr.h>
#include <time.h>

#include "stars.h"
#include "topo.h"

int main() {
    furnsh_c("./kernels/hipparcos.bin");   // GENERIC STARS KERNEL
    furnsh_c("./kernels/naif0011.tls");    // GENERIC LSK KERNEL
    furnsh_c("./kernels/pck00010.tpc");    // GENERIC PCK KERNEL
    furnsh_c("./kernels/earth_fixed.tf");  // EARTH_FIXED ALIAS
    furnsh_c("./kernels/earth_000101_191026_190804.bpc");  // GENERIC ITRF93 KERNEL

    SpiceInt table_count;
    ekntab_c(&table_count);
    printf("Number of loaded tables: %d\n", table_count);

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

    time_t current_epoch_time = time(NULL);
    struct tm *local_time = localtime(&current_epoch_time);
    printf("Current local time: %s", asctime(local_time));

    struct tm *utc_time = gmtime(&current_epoch_time);
    SpiceChar utc_time_string[20];
    strftime(utc_time_string, 20, "%Y-%m-%dT%H:%M:%S", utc_time);
    printf("UTC time: %s\n", utc_time_string);

    SpiceDouble et;
    str2et_c(utc_time_string, &et);

    gate_topo_frame seattle_topo;
    gate_load_earth_topo_frame("SEATTLE_TOPO", 48, -122, 0, &seattle_topo);

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