#include <math.h>
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
        printf("table_count=%d is greater than 1", table_count);
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

        SpiceDouble as;
        convrt_c(info.parallax, "DEGREES", "ARCSECONDS", &as);
        SpiceDouble dist_parsecs = 1 / as;

        SpiceDouble ra;
        SpiceDouble dec;
        SpiceDouble ra_u;
        SpiceDouble dec_u;
        gate_calc_star_pos(info, et, &ra, &dec, &ra_u, &dec_u);
        printf("Our best info for star: ra=%f (+/-)%f dec=%f (+/-)%f\n", ra, ra_u, dec, dec_u);

        SpiceDouble star_rec[3];
        SpiceDouble ra_rad = ra * rpd_c();
        SpiceDouble dec_rad = dec * rpd_c();
        radrec_c(dist_parsecs, ra_rad, dec_rad, star_rec);

        SpiceDouble star_rot_matrix[3][3];
        pxform_c("J2000", seattle_topo.frame_name, et, star_rot_matrix);

        SpiceDouble star_topo_rec[3];
        mxv_c(star_rot_matrix, star_rec, star_topo_rec);

        gate_conv_adjust_topo_rec(seattle_topo, "PARSECS", star_topo_rec);

        SpiceDouble topo_r;
        SpiceDouble topo_ra;
        SpiceDouble topo_dec;
        recrad_c(star_topo_rec, &topo_r, &topo_ra, &topo_dec);

        SpiceDouble topo_ra_deg = topo_ra * dpr_c();
        SpiceDouble topo_dec_deg = topo_dec * dpr_c();
        printf("Look angle: az=%.30f dec=%.30f\n", 360 - topo_ra_deg, topo_dec_deg);
    }

    gate_unload_topo_frame(seattle_topo);

    return 0;
}