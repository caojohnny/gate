/**
 * SNM stands for "Star Name Mapper," which is basically a
 * tool that was written to parse the IAU-CSN.txt file
 * into a useful data structure. Because the provided SPICE
 * 1 formatted star catalogs do not have star names
 * mapped, this adds support for name lookups based on
 * catalog IDs, hence the name. In addition to name lookup,
 * other information available in the IAU-CSN data file is
 * also parsed, some available in those catalogs, some not.
 */

#ifndef GATE_SNM_H
#define GATE_SNM_H

#include <stdio.h>

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

/**
 * Parses the IAU-CSN.txt file into an array of
 * {@code csn_data}.
 *
 * @param file the file which to parse the data from
 * (input)
 * @param parsed_data_len the length of the
 * {@code parsed_data} array (output)
 * @param parsed_data the array populated with the
 * parsed star data (output)
 */
void snm_parse_data(FILE *file, int *parsed_data_len, csn_data **parsed_data);

#endif // GATESNM_SNM_H
