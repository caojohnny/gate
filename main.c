#include <math.h>
#include <stdio.h>
#include <SpiceUsr.h>

#include "topo.h"

typedef struct {
    SpiceInt cat_num;

    SpiceDouble parallax;
    SpiceDouble ra;
    SpiceDouble dec;

    SpiceDouble ra_epoch;
    SpiceDouble dec_epoch;

    // Proper motion
    SpiceDouble ra_pm;
    SpiceDouble dec_pm;

    // Used to calculate uncertainty
    SpiceDouble ra_sigma;
    SpiceDouble dec_sigma;
    SpiceDouble ra_pm_sigma;
    SpiceDouble dec_pm_sigma;
} star_info;

SpiceInt get_int_from_table(SpiceInt column, SpiceInt row) {
    SpiceInt data;
    SpiceBoolean found;
    SpiceBoolean is_null;
    ekgi_c(column, row, 0, &data, &is_null, &found);

    if (!found || is_null) {
        setmsg_c("No SpiceInt found from column=%d row=%d\n");
        errint_c("%d", column);
        errint_c("%d", row);
        sigerr_c("get_int");
        return 0;
    }

    return data;
}

SpiceDouble get_double_from_table(SpiceInt column, SpiceInt row) {
    SpiceDouble data;
    SpiceBoolean found;
    SpiceBoolean is_null;
    ekgd_c(column, row, 0, &data, &is_null, &found);

    if (!found || is_null) {
        setmsg_c("No SpiceDouble found from column=%d row=%d\n");
        errint_c("%d", column);
        errint_c("%d", row);
        sigerr_c("get_double");
        return 0;
    }

    return data;
}

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

    SpiceChar query[500];
    snprintf(query, 500, "SELECT "
                         "CATALOG_NUMBER, DM_NUMBER, "
                         "PARLAX, RA, DEC, RA_EPOCH, DEC_EPOCH, "
                         "RA_PM, DEC_PM, "
                         "RA_SIGMA, DEC_SIGMA, RA_PM_SIGMA, DEC_PM_SIGMA "
                         "FROM %s WHERE "
                         // Polaris
                         "CATALOG_NUMBER = 11767",
            // "RA >= 216 AND RA <= 218 AND "
            // "DEC >= 61 AND DEC <= 63",
             table_name);

    SpiceInt row_count;
    SpiceBoolean error_occurred;
    SpiceChar error_string[100];
    ekfind_c(query, 100, &row_count, &error_occurred, error_string);
    printf("Found %d relevant rows in table %s\n", row_count, table_name);

    if (error_occurred) {
        puts(error_string);
        return 1;
    }

    star_info matching_stars[row_count];
    for (int i = 0; i < row_count; ++i) {
        SpiceInt cat_num = get_int_from_table(0, i);
        SpiceInt dm_num = get_int_from_table(1, i);
        SpiceDouble parallax = get_double_from_table(2, i);
        SpiceDouble ra = get_double_from_table(3, i);
        SpiceDouble dec = get_double_from_table(4, i);
        SpiceDouble ra_epoch = get_double_from_table(5, i);
        SpiceDouble dec_epoch = get_double_from_table(6, i);
        SpiceDouble ra_pm = get_double_from_table(7, i);
        SpiceDouble dec_pm = get_double_from_table(8, i);
        SpiceDouble ra_sigma = get_double_from_table(9, i);
        SpiceDouble dec_sigma = get_double_from_table(10, i);
        SpiceDouble ra_pm_sigma = get_double_from_table(11, i);
        SpiceDouble dec_pm_sigma = get_double_from_table(12, i);

        star_info info = {cat_num,
                          parallax, ra, dec,
                          ra_epoch, dec_epoch,
                          ra_pm, dec_pm, ra_sigma, dec_sigma, ra_pm_sigma, dec_pm_sigma};
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
    str2et_c("2019 AUG 7 16:31:09", &et);

    SpiceChar topo_frame[13] = "SEATTLE_TOPO";
    load_topo_frame(topo_frame, -122, 42);

    for (int i = 0; i < row_count; ++i) {
        star_info info = matching_stars[i];
        SpiceDouble as;
        convrt_c(info.parallax, "DEGREES", "ARCSECONDS", &as);

        SpiceDouble dist_parsecs = 1 / as;
        SpiceDouble dist_km;
        convrt_c(dist_parsecs, "PARSECS", "KM", &dist_km);
        printf("Star dist = %f km\n", dist_km);

        SpiceDouble t = (et / jyear_c()) + ((j2000_c() - j1950_c()) / (jyear_c() / spd_c()));
        SpiceDouble dtra = t - info.ra_epoch;
        SpiceDouble dtdec = t - info.dec_epoch;
        SpiceDouble ra = info.ra + (dtra * info.ra_pm);
        SpiceDouble dec = info.dec + (dtdec * info.dec_pm);
        SpiceDouble ra_u = sqrt(pow(info.ra_sigma, 2) + pow(dtra * info.ra_pm_sigma, 2));
        SpiceDouble dec_u = sqrt(pow(info.dec_sigma, 2) + pow(dtdec * info.dec_pm_sigma, 2));
        printf("Our best info for star: ra=%f (+/-)%f dec=%f (+/-)%f\n", ra, ra_u, dec, dec_u);

        SpiceDouble star_rec[3];
        SpiceDouble ra_rad = ra * rpd_c();
        SpiceDouble dec_rad = dec * rpd_c();
        radrec_c(dist_km, ra_rad, dec_rad, star_rec);

        printf("Star = (%f, %f, %f)\n", star_rec[0], star_rec[1], star_rec[2]);

        SpiceDouble star_topo_rec[3];
        SpiceDouble star_rot_matrix[3][3];
        pxform_c("J2000", topo_frame, et, star_rot_matrix);
        mxv_c(star_rot_matrix, star_rec, star_topo_rec);

        SpiceDouble topo_r;
        SpiceDouble topo_ra;
        SpiceDouble topo_dec;
        recrad_c(star_topo_rec, &topo_r, &topo_ra, &topo_dec);

        SpiceDouble topo_ra_deg = topo_ra * dpr_c();
        SpiceDouble topo_dec_deg = topo_dec * dpr_c();
        printf("Look angle: ra=%f dec=%f\n", topo_ra_deg, topo_dec_deg);
    }

    unload_topo_frame(topo_frame);

    return 0;
}