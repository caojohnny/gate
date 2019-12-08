#include <stdio.h>
#include <stdlib.h>
#include "snm.h"

#define FMT_COLUMN_LEN 13

static const int utf8_ctl_layout_len = 3;
static const int utf8_ctl_layout[] = {
        0b11000000, 0b11100000, 0b11110000
};

static void intlbuf_append(long *buf, int buf_size, long item) {
    for (int i = 1; i < buf_size; ++i) {
        buf[i - 1] = buf[i];
    }

    buf[buf_size - 1] = item;
}

static void skip_line(FILE *file) {
    while (1) {
        int c = fgetc(file);
        if (c == '\n') {
            break;
        }
    }
}

static int parse_fmt(int *data_fmt, FILE *file, long fmt_off) {
    long cur = ftell(file);
    fseek(file, fmt_off, SEEK_SET);

    int column = 0;
    int fmt_column = 0;
    while (1) {
        int c = fgetc(file);
        if (c == '\n' || c == EOF) {
            break;
        }

        if (c == '(') {
            if (fmt_column > FMT_COLUMN_LEN) {
                printf("More columns than expected (%d), try updating\n", FMT_COLUMN_LEN);
                return 0;
            }

            data_fmt[fmt_column] = column;
            fmt_column++;
        }

        column++;
    }

    // First column is offset by the preceding # comment symbol
    data_fmt[0] -= 1;

    fseek(file, cur, SEEK_SET);
    return 1;
}

static void read_utf8(FILE *file, int start, int until, char **out, int *len) {
    *len = until - start + 1;
    *out = malloc(*len * sizeof(char));
    if (*out == NULL) {
        puts("Failed to allocate string");
        return;
    }

    int trailing = 0;
    for (int i = 0; i < *len - 1; ++i) {
        int c = fgetc(file);
        if (c == ' ') {
            trailing++;
        } else {
            trailing = 0;
        }

        (*out)[i] = (char) c;

        for (int ctl_idx = 0; ctl_idx < utf8_ctl_layout_len; ++ctl_idx) {
            int mask = utf8_ctl_layout[ctl_idx];
            if ((c & mask) == mask) {
                int additional_bytes = ctl_idx + 1;
                *len += additional_bytes;

                char *new_out = realloc(*out, *len * sizeof(char));
                if (new_out == NULL) {
                    puts("Failed to reallocate for extra bytes");
                    return;
                }
            }
        }
    }

    if (trailing > 0) {
        *len -= trailing;
        char *new_out = realloc(*out, *len * sizeof(char));
        if (new_out == NULL) {
            puts("Failed to trim string");
            return;
        }

        *out = new_out;
    }

    (*out)[*len - 1] = '\0';
}

static void read_double(FILE *file, int start, int until, double *out) {
    char *string;
    int string_len;
    read_utf8(file, start, until, &string, &string_len);

    char *end;
    double result = strtod(string, &end);
    if (string == end) {
        // This is due to unassigned values
        // printf("Error decoding double: %s\n", string);
        free(string);
        *out = 0;
        return;
    }

    *out = result;
    free(string);
}

static void read_int(FILE *file, int start, int until, int *out) {
    char *string;
    int string_len;
    read_utf8(file, start, until, &string, &string_len);

    char *end;
    long result = strtol(string, &end, 10);
    if (string == end) {
        // This is due to unassigned values
        // printf("Error decoding integer: %s\n", string);
        free(string);
        *out = 0;
        return;
    }

    *out = (int) result;
    free(string);
}

static void read_date(FILE *file, int start, int until, int *year, int *month, int *day) {
    char *string;
    int string_len;
    read_utf8(file, start, until, &string, &string_len);

    sscanf(string, "%d-%d-%d", year, month, day);

    free(string);
}

void snm_parse_data(FILE *file, int *parsed_data_len, csn_data **parsed_data) {
    long prev_comment_offsets[2];
    int data_fmt[FMT_COLUMN_LEN];

    while (1) {
        int c = fgetc(file);

        // Skip comments
        if (c == '#') {
            intlbuf_append(prev_comment_offsets, 2, ftell(file) - 1);
            skip_line(file);
            continue;
        }

        if (!parse_fmt(data_fmt, file, prev_comment_offsets[0])) {
            return;
        } else {
            break;
        }
    }

    // Move cursor behind the first line of actual data
    fseek(file, -1, SEEK_CUR);

    *parsed_data_len = 0;
    while (1) {
        int c = fgetc(file);

        // Bottom comment block starts again
        if (c == '#') {
            return;
        }

        // End of line artifacts
        if (c == ' ' || c == '\n') {
            continue;
        }

        // Rewind to beginning of line
        fseek(file, -1, SEEK_CUR);

        csn_data csn;
        read_utf8(file, data_fmt[0], data_fmt[1], &csn.name, &csn.name_len);
        read_utf8(file, data_fmt[1], data_fmt[2], &csn.fd, &csn.fd_len);
        read_utf8(file, data_fmt[2], data_fmt[3], &csn.id, &csn.id_len);
        read_utf8(file, data_fmt[3], data_fmt[4], &csn.id_utf8, &csn.id_utf8_len);
        read_utf8(file, data_fmt[4], data_fmt[5], &csn.constellation, &csn.constellation_len);
        read_utf8(file, data_fmt[5], data_fmt[6], &csn.wds_comp_id, &csn.wds_comp_id_len);
        read_utf8(file, data_fmt[6], data_fmt[7], &csn.wds_j, &csn.wds_j_len);
        read_double(file, data_fmt[7], data_fmt[8], &csn.visual_magnitude);
        read_int(file, data_fmt[8], data_fmt[9], &csn.hip);
        read_int(file, data_fmt[9], data_fmt[10], &csn.hd);
        read_double(file, data_fmt[10], data_fmt[11], &csn.ra);
        read_double(file, data_fmt[11], data_fmt[12], &csn.dec);
        read_date(file, data_fmt[12], data_fmt[12] + 10, &csn.date_year, &csn.date_month, &csn.date_day);

        csn_data *new_parsed_data = realloc(*parsed_data, (*parsed_data_len + 1) * sizeof(*new_parsed_data));
        if (new_parsed_data == NULL) {
            puts("Failed to reallocate extra memory for CSN data");
            return;
        }

        *parsed_data = new_parsed_data;
        (*parsed_data)[*parsed_data_len] = csn;
        (*parsed_data_len)++;
    }
}
