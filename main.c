#include <stdio.h>
#include <SpiceZpr.h>

typedef struct {
    SpiceInt cat_num;
    SpiceDouble parallax;
    SpiceDouble ra;
    SpiceDouble dec;
} star_info;

int main() {
    furnsh_c("./kernels/hipparcos.bin");   // GENERIC STARS KERNEL
    furnsh_c("./kernels/naif0011.tls");    // GENERIC LSK KERNEL
    furnsh_c("./kernels/de435.bsp");       // JPL PLANETARY SPK
    furnsh_c("./kernels/pck00010.tpc");    // GENERIC PCK KERNEL
    furnsh_c("./kernels/earth_fixed.tf");  // EARTH_FIXED ALIAS
    furnsh_c("./frame-seattle-topo.tk");  // EARTH_FIXED ALIAS

    SpiceChar file_type[100];
    SpiceChar source[100];
    SpiceInt handle;
    SpiceBoolean found;
    kinfo_c("./kernels/tycho2.bin", 100, 100, file_type, source, &handle, &found);
    printf("Kernel data:\n"
           "\tfile_type=%s\n"
           "\tsource=%s\n"
           "\thandle=%d\n"
           "\tfound=%d\n",
           file_type, source, handle, found);

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

    SpiceInt column_count;
    ekccnt_c(table_name, &column_count);
    printf("Columns in table_name=%s: %d\n", table_name, column_count);

    for (int i = 0; i < column_count; ++i) {
        SpiceChar column_name[100];
        SpiceEKAttDsc dsc;
        ekcii_c(table_name, i, 100, column_name, &dsc);
        printf("Column info for table_name=%s and cidx=%d:\n"
               "\tcolumn_name=%s\n"
               "\tclass=%d\n"
               "\ttypeId=%d\n"
               "\tsize=%d\n",
               table_name, i, column_name, dsc.cclass, dsc.dtype, dsc.size);
    }

    SpiceChar query[200];
    snprintf(query, 200, "SELECT CATALOG_NUMBER, DM_NUMBER, PARLAX, RA, DEC FROM %s WHERE "
                         // Proxima Centauri
                         "CATALOG_NUMBER = 70890",
            // "RA >= 216 AND RA <= 218 AND "
            // "DEC >= 61 AND DEC <= 63",
             table_name);

    SpiceInt row_count;
    SpiceBoolean error_occurred;
    SpiceChar error_string[100];
    ekfind_c(query, 100, &row_count, &error_occurred, error_string);
    printf("Found %d relevant rows in table %s\n", row_count, table_name);

    star_info matching_stars[row_count];
    for (int i = 0; i < row_count; ++i) {
        SpiceInt cat_num;
        SpiceBoolean is_cat_num_null;
        SpiceBoolean is_cat_num_found;
        ekgi_c(0, i, 0, &cat_num, &is_cat_num_null, &is_cat_num_found);

        SpiceInt dm_num;
        SpiceBoolean is_dm_num_null;
        SpiceBoolean is_dm_num_found;
        ekgi_c(1, i, 0, &dm_num, &is_dm_num_null, &is_dm_num_found);

        SpiceDouble parallax;
        SpiceBoolean is_parallax_null;
        SpiceBoolean is_parallax_found;
        ekgd_c(2, i, 0, &parallax, &is_parallax_null, &is_parallax_found);

        SpiceDouble ra;
        SpiceBoolean is_ra_null;
        SpiceBoolean is_ra_found;
        ekgd_c(3, i, 0, &ra, &is_ra_null, &is_ra_found);

        SpiceDouble dec;
        SpiceBoolean is_dec_null;
        SpiceBoolean is_dec_found;
        ekgd_c(4, i, 0, &dec, &is_dec_null, &is_dec_found);

        star_info info = {cat_num, parallax, ra, dec};
        matching_stars[i] = info;

        SpiceChar body_name[100];
        SpiceBoolean has_name;
        bodc2n_c(dm_num, 100, body_name, &has_name);
        if (has_name) {
            printf("Body name for cat_num=%d and dm_num=%d at parallax=%0.20f ra=%f dec=%f is %s\n",
                   cat_num, dm_num, parallax, ra, dec, body_name);
        } else {
            printf("Unnamed body with cat_num=%d and dm_num=%d at parallax=%0.20f ra=%f dec=%f\n",
                   cat_num, dm_num, parallax, ra, dec);
        }
    }

    SpiceDouble et;
    str2et_c("2019 AUG 02 12:10:55", &et);

    SpiceDouble lon = -122 * rpd_c();
    SpiceDouble lat = 47 * rpd_c();
    SpiceDouble alt = 0;

    SpiceInt radii_count;
    SpiceDouble earth_radii_data[3];
    bodvrd_c("EARTH", "RADII", 3, &radii_count, earth_radii_data);
    SpiceDouble major_axis_rad = earth_radii_data[0];
    SpiceDouble minor_axis_rad = earth_radii_data[2];
    SpiceDouble f = (major_axis_rad - minor_axis_rad) / major_axis_rad;

    // This is also not technically the correct ra/dec
    // Additional transformations needed
    // TODO
    SpiceDouble observer_rec[3];
    georec_c(lon, lat, alt, major_axis_rad, f, observer_rec);

    for (int i = 0; i < row_count; ++i) {
        star_info info = matching_stars[i];
        SpiceDouble as;
        convrt_c(info.parallax, "DEGREES", "ARCSECONDS", &as);

        SpiceDouble dist_parsecs = 1 / as;
        SpiceDouble dist_km;
        convrt_c(dist_parsecs, "PARSECS", "KM", &dist_km);
        printf("Star dist = %f km\n", dist_km);

        SpiceDouble star_rec[3];
        SpiceDouble ra_rad = info.ra * rpd_c();
        SpiceDouble dec_rad = info.dec * rpd_c();
        radrec_c(dist_km, ra_rad, dec_rad, star_rec);

        printf("Star = (%f, %f, %f)\n", star_rec[0], star_rec[1], star_rec[2]);

        SpiceDouble star_topo_rec[3];
        SpiceDouble star_rot_matrix[3][3];
        pxform_c("J2000", "SEATTLE_TOPO", et, star_rot_matrix);
        mxv_c(star_rot_matrix, star_rec, star_topo_rec);

        SpiceDouble topo_r;
        SpiceDouble topo_ra;
        SpiceDouble topo_dec;
        recrad_c(star_topo_rec, &topo_r, &topo_ra, &topo_dec);
        SpiceDouble topo_ra_deg = topo_ra * dpr_c();
        SpiceDouble topo_dec_deg = topo_dec * dpr_c();
        printf("Look angle: ra=%f dec=%f\n", topo_ra_deg, topo_dec_deg);
    }

    return 0;
}