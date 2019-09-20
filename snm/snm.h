#ifndef GATE_SNM_H
#define GATE_SNM_H

// TODO: Implement

/**
 * Structure representing a row of data from the IAU
 * Catalog of Star Names (CSN). Descriptions for each
 * column can be found on the IAU-CSN.txt file from which
 * this data should be parsed from.
 *
 * https://www.pas.rochester.edu/~emamajek/WGSN/IAU-CSN.txt
 */
typedef struct {
    char *name;
    int name_len;

    char *fd;
    int fd_len;

    char *id;
    int id_len;

    char *id_utf8;
    int id_utf8_len;

    char *constellation;
    int constellation_len;

    char *wds_comp_id;
    int wds_comp_id_len;

    char *wds_j;
    int wds_j_len;

    double visual_magnitude;

    int hip;
    int hd;

    double ra;
    double dec;

    int date_year;
    int date_month;
    int date_day;
} csn_data;

#endif // GATE_SNM_H
