#include <stdio.h>
#include <SpiceZpr.h>

int main() {
    furnsh_c("./kernels/hipparcos.bin"); // GENERIC STARS KERNEL
    furnsh_c("./kernels/naif0011.tls");  // GENERIC LSK KERNEL
    furnsh_c("./kernels/de435.bsp");     // JPL PLANETARY SPK
    // furnsh_c("./kernels/mar097s.bsp");   // MARS INSIGHT

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

    SpiceChar query[100];
    snprintf(query, 200, "SELECT CATALOG_NUMBER, RA, DEC FROM %s "
                         // Polaris
                         "WHERE RA >= 37.94 AND "
                         "RA <= 37.96 AND DEC >= 89.1 AND DEC <= 89.3", table_name);

    SpiceInt row_count;
    SpiceBoolean error_occurred;
    SpiceChar error_string[100];
    ekfind_c(query, 100, &row_count, &error_occurred, error_string);
    printf("Found %d relevant rows in table %s\n", row_count, table_name);

    int ignored_bodies = 0;
    for (int i = 0; i < row_count; ++i) {
        SpiceInt naif_id;
        SpiceBoolean is_naif_id_null;
        SpiceBoolean is_naif_id_found;
        ekgi_c(0, i, 0, &naif_id, &is_naif_id_null, &is_naif_id_found);

        SpiceDouble ra;
        SpiceBoolean is_ra_null;
        SpiceBoolean is_ra_found;
        ekgd_c(1, i, 0, &ra, &is_ra_null, &is_ra_found);

        SpiceDouble dec;
        SpiceBoolean is_dec_null;
        SpiceBoolean is_dec_found;
        ekgd_c(2, i, 0, &dec, &is_dec_null, &is_dec_found);

        SpiceChar body_name[100];
        SpiceBoolean has_name;
        bodc2n_c(naif_id, 100, body_name, &has_name);
        if (has_name) {
            printf("Body name for NAIF ID=%d is %s\n", naif_id, body_name);
        } else {
            printf("Unnamed body with NAIF ID=%d at ra=%f and dec=%f\n", naif_id, ra, dec);
            ignored_bodies++;
        }
    }
    printf("Number of ignored/unnmaed bodies=%d\n", ignored_bodies);

    SpiceDouble et;
    str2et_c("2019 AUG 02 22:10:55", &et);

    SpiceDouble vector[6];
    SpiceDouble lt;
    spkezr_c("11767", et, "J2000", "NONE", "EARTH", vector, &lt);

    printf("%f %f %f %f %f %f", vector[0], vector[1], vector[2], vector[3], vector[4], vector[5]);

    return 0;
}