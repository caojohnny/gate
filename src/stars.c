#include "stars.h"
#include "constants.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

void gate_load_stars(ConstSpiceChar *table, ConstSpiceChar const *filter, SpiceInt *rows) {
    ConstSpiceChar *actual_filter = "";
    if (filter != NULL) {
        actual_filter = filter;
    }

    SpiceInt query_len = 174 + strlen(table) + strlen(actual_filter);
    SpiceChar query[query_len];
    snprintf(query, query_len, "SELECT "
                               "CATALOG_NUMBER, "
                               "DEC, DEC_EPOCH, DEC_PM, DEC_PM_SIGMA, DEC_SIGMA, "
                               "DM_NUMBER, PARLAX, "
                               "RA, RA_EPOCH, RA_PM, RA_PM_SIGMA, RA_SIGMA, "
                               "SPECTRAL_TYPE, VISUAL_MAGNITUDE FROM %s %s",
             table, actual_filter);

    SpiceBoolean error;
    SpiceChar error_message[GATE_SPICE_MSG_MAX_LEN];
    ekfind_c(query, GATE_SPICE_MSG_MAX_LEN, rows, &error, error_message);

    if (error) {
        setmsg_c("Error occurred, long msg was '%s'. Query string is '%s'");
        errch_c("%s", error_message);
        errch_c("%s", query);
        sigerr_c("query");
        return;
    }
}

void gate_parse_stars(SpiceInt max_size, gate_star_info_spice1 *array) {
    SpiceBoolean is_null;
    SpiceBoolean found;
    for (int i = 0; i < max_size; ++i) {
        gate_star_info_spice1 info;
        ekgi_c(0, i, 0, &info.catalog_number, &is_null, &found);
        ekgd_c(1, i, 0, &info.dec, &is_null, &found);
        ekgd_c(2, i, 0, &info.dec_epoch, &is_null, &found);
        ekgd_c(3, i, 0, &info.dec_pm, &is_null, &found);
        ekgd_c(4, i, 0, &info.dec_pm_sigma, &is_null, &found);
        ekgd_c(5, i, 0, &info.dec_sigma, &is_null, &found);
        ekgi_c(6, i, 0, &info.dm_number, &is_null, &found);
        ekgd_c(7, i, 0, &info.parallax, &is_null, &found);
        ekgd_c(8, i, 0, &info.ra, &is_null, &found);
        ekgd_c(9, i, 0, &info.ra_epoch, &is_null, &found);
        ekgd_c(10, i, 0, &info.ra_pm, &is_null, &found);
        ekgd_c(11, i, 0, &info.ra_pm_sigma, &is_null, &found);
        ekgd_c(12, i, 0, &info.ra_sigma, &is_null, &found);
        ekgc_c(13, i, 0, 5, info.spectral_type, &is_null, &found);
        ekgd_c(14, i, 0, &info.visual_magnitude, &is_null, &found);

        array[i] = info;
    }
}

void gate_calc_star_pos(gate_star_info_spice1 info, SpiceDouble et,
                        SpiceDouble *ra, SpiceDouble *dec, SpiceDouble *ra_u, SpiceDouble *dec_u) {
    SpiceDouble t = (et / jyear_c()) + ((j2000_c() - j1950_c()) / (jyear_c() / spd_c()));
    SpiceDouble dtra = t - info.ra_epoch;
    SpiceDouble dtdec = t - info.dec_epoch;

    if (ra != NULL) {
        *ra = info.ra + (dtra * info.ra_pm);
    }

    if (dec != NULL) {
        *dec = info.dec + (dtdec * info.dec_pm);
    }

    if (ra_u != NULL) {
        *ra_u = sqrt(pow(info.ra_sigma, 2) + pow(dtra * info.ra_pm_sigma, 2));
    }

    if (dec_u != NULL) {
        *dec_u = sqrt(pow(info.dec_sigma, 2) + pow(dtdec * info.dec_pm_sigma, 2));
    }
}

void gate_calc_star_topo(gate_topo_frame observer_frame, gate_star_info_spice1 info, SpiceDouble et,
                         SpiceDouble *range, SpiceDouble *azimuth, SpiceDouble *elevation) {
    SpiceDouble parallax_as;
    convrt_c(info.parallax, "DEGREES", "ARCSECONDS", &parallax_as);
    SpiceDouble dist_parsecs = 1 / parallax_as;

    SpiceDouble dist_km;
    convrt_c(dist_parsecs, "PARSECS", "KILOMETERS", &dist_km);

    SpiceDouble current_ra;
    SpiceDouble current_dec;
    gate_calc_star_pos(info, et, &current_ra, &current_dec, NULL, NULL);

    SpiceDouble ra_rad = current_ra * rpd_c();
    SpiceDouble dec_rad = current_dec * rpd_c();
    SpiceDouble star_pos_j2000_rec[3];
    radrec_c(dist_km, ra_rad, dec_rad, star_pos_j2000_rec);

    SpiceDouble frame_transform_matrix[3][3];
    pxform_c("J2000", observer_frame.frame_name, et, frame_transform_matrix);

    SpiceDouble star_topo_rec[3];
    mxv_c(frame_transform_matrix, star_pos_j2000_rec, star_topo_rec);

    gate_adjust_topo_rec(observer_frame, star_topo_rec);
    gate_conv_rec_azel(star_topo_rec, range, azimuth, elevation);
}
