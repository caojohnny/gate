#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cspice/SpiceUsr.h>
#include <cspice/SpiceZfc.h>

#include <gate/stars.h>
#include <gate/timeconv.h>
#include <gate/topo.h>
#include <gatesnm/snm.h>

#include "options.h"
#include "util.h"
#include "dispatcher.h"
#include "table.h"

#define TAB_NAME_MAX_LEN 100
#define FILTER_MAX_LEN 100
#define TIME_OUT_MAX_LEN 30
#define BODY_NAME_MAX_LEN 100
#define NAIF_ID_MIN -100000     // These are arbitrary
#define NAIF_ID_MAX 100000000
#define FRAME_NAME_MAX_LEN 33
#define KERNEL_FRAMES_MAX_LEN 500
#define TLE_MIN_YEAR 1960
#define TLE_LINE_LEN 70

static int csn_data_len = -1;
static csn_data *csn_data_array;

// https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/FORTRAN/spicelib/ev2lin.html
static const SpiceDouble GEO_CONSTANTS[] =
        {1.082616e-3, -2.53881e-6, -1.65597e-6, 7.43669161e-2, 120.0, 78.0, 6378.135, 1.0};
static gatecli_table sat_data_array;

static gatecli_table calc_data_array;

void help() {
    puts("You can Ctrl+C any time to halt continuous output");
    puts("");

    puts("--- HELP ---");
    puts("EXIT - Quits the command line");
    puts("HELP - prints this message");
    puts("LOAD <CMD | KERNEL | CSN> <filename> - loads a set of commands or a kernel or CSN from file");
    puts("SET <option> <value> - sets the value of a particular option");
    puts("GET <option> - prints the value of a particular option");
    puts("SHOW <TABLES | FRAMES | CSN | BODIES | CALC> - prints the available table, frame, named star, body, or custom calc object names");
    puts("STAR INFO <catalog number> - prints information for a star with the given catalog number");
    puts("STAR AZEL <catalog number> <CONT | count> <ISO time | NOW> - prints the observation position for the star with the given catalog number");
    puts("BODY INFO <naif id> - prints information for a body with the given NAIF ID");
    puts("BODY AZEL <naif id> <CONT | count> <ISO time | NOW> - prints the observation position for the satellite with the given NAIF ID");
    puts("SAT ADD <id> - adds a satellite with the given ID to the internal database (non persistent)");
    puts("SAT REM <id> - removes the satellite with the given ID from the internal database");
    puts("SAT INFO <id> - prints information for a satellite added with the given ID");
    puts("SAT AZEL <id> <CONT | count> <ISO time | NOW> - prints the observation position for the satellite added with the given ID");
    puts("CALC ADD <id> <RANGE> <RA deg> <DEC deg> [<RA_PM deg/yr> <DEC_PM deg/yr>] - adds a body with the given ID to the internal database (non persistent)");
    puts("CALC REM <id> - removes a body with the given ID from the internal database");
    puts("CALC INFO <id> - prints information for a custom calculated body with the given ID");
    puts("CALC AZEL <id> <CONT | count> <ISO time | NOW> - prints the observation for position the calculated body added with the given ID");

    puts("");

    puts("--- OPTIONS ---");
    for (int i = 0; i < OPTION_KEY_LENGTH; ++i) {
        printf("%s\n", key_to_string(i));
    }
}

static char *set_option_string(option_key key, char **argv) {
    char *value_copy = strdup(argv[2]);
    set_option(key, value_copy);
    return value_copy;
}

static SpiceDouble *set_option_double(option_key key, char **argv) {
    SpiceDouble *heap_double = malloc(1 * sizeof(*heap_double));
    char *end;
    heap_double[0] = strtod(argv[2], &end);
    if (argv[2] == end) {
        printf("Not a valid number: '%s'\n", argv[2]);
        return NULL;
    }
    set_option(key, heap_double);
    return heap_double;
}

SpiceBoolean is_table_valid(char *table_name) {
    SpiceInt table_count;
    ekntab_c(&table_count);
    if (table_count == 0) {
        return SPICEFALSE;
    }

    for (int i = 0; i < table_count; ++i) {
        SpiceChar valid_name[TAB_NAME_MAX_LEN];
        ektnam_c(0, TAB_NAME_MAX_LEN, valid_name);

        if (strcmp(valid_name, table_name) == 0) {
            return SPICETRUE;
        }
    }

    return SPICEFALSE;
}

void set(int argc, char **argv) {
    if (argc != 3) {
        puts("This command requires 2 arguments");
        return;
    }

    option_key key = string_to_key(argv[1]);
    if (key == -1) {
        printf("No such option for key '%s'\n", argv[1]);
        return;
    }

    const char *key_name = key_to_string(key);
    switch (key) {
        case OBSERVER_BODY:
        case STAR_TABLE: {
            char *option = set_option_string(key, argv);
            if (key == STAR_TABLE && !is_table_valid(option)) {
                printf("Table '%s' could not be found. Try SHOW TABLES?\n", option);
                set_option(key, NULL);
                break;
            }
            printf("%s = %s\n", key_name, option);

            break;
        }
        case OBSERVER_LATITUDE:
        case OBSERVER_LONGITUDE: {
            SpiceDouble *option = set_option_double(key, argv);
            if (option != NULL) {
                printf("%s = %f\n", key_name, *option);
            }

            break;
        }
        case OPTION_KEY_LENGTH:
            puts("Internal option cannot be set.");
            break;
        default:
            puts("Unhandled option.");
            break;
    }
}

void get(int argc, char **argv) {
    if (argc != 2) {
        puts("This command requires 1 argument");
        return;
    }

    option_key key = string_to_key(argv[1]);
    if (key == -1) {
        printf("No such option for key '%s'\n", argv[1]);
        return;
    }

    switch (key) {
        case OBSERVER_BODY: {
            char *option = (char *) get_option(key);
            printf("Observer body is set to '%s'\n", option);

            break;
        }
        case OBSERVER_LATITUDE: {
            double *option = (double *) get_option(key);
            if (option == NULL) {
                puts("Observer latitude is not set to anything.");
            } else {
                printf("Observer latitude is %f\n", *option);
            }

            break;
        }
        case OBSERVER_LONGITUDE: {
            double *option = (double *) get_option(key);
            if (option == NULL) {
                puts("Observer longitude is not set to anything.");
            } else {
                printf("Observer longitude  is %f\n", *option);
            }

            break;
        }
        case OPTION_KEY_LENGTH:
            puts("Internal option cannot be retrieved.");
            break;
        default:
            puts("Unhandled option.");
            break;
    }
}

static void file_read_free(FILE *file, int argc, char **argv) {
    if (argc > 0) {
        for (int i = 0; i < argc; ++i) {
            free(argv[i]);
        }
        free(argv);
    }

    fclose(file);
}

void load(int argc, char **argv, volatile int *is_running) {
    if (argc != 3) {
        puts("This command requires 2 arguments");
        return;
    }

    if (eq_ignore_case("KERNEL", argv[1])) {
        furnsh_c(argv[2]);
        printf("Loaded kernel for file '%s'\n", argv[2]);
        return;
    }

    if (eq_ignore_case("CMD", argv[1])) {
        FILE *cmd_file = fopen(argv[2], "r");
        if (cmd_file == NULL) {
            printf("Error occurred opening file handle for '%s'\n", argv[2]);
            return;
        }

        int ch_idx_counter = 0;
        int cmd_file_argc = 0;
        char **cmd_file_argv = NULL;
        while (SPICETRUE) {
            int c = fgetc(cmd_file);

            // Ignore comment lines
            if (c == '#') {
                while (c != '\n') {
                    c = fgetc(cmd_file);
                }
                c = fgetc(cmd_file);
            }

            // Line or file has ended
            if (c == EOF || c == '\n') {
                // Process and clear the line if available
                if (cmd_file_argc > 0) {
                    cmd_file_argv[cmd_file_argc - 1][ch_idx_counter] = '\0';
                    dispatch(cmd_file_argc, cmd_file_argv, is_running);

                    for (int i = 0; i < cmd_file_argc; ++i) {
                        free(cmd_file_argv[i]);
                    }
                    free(cmd_file_argv);

                    cmd_file_argc = 0;
                    ch_idx_counter = 0;
                    cmd_file_argv = NULL;
                }

                if (c == EOF) {
                    break;
                }

                continue;
            }

            // We're on a new argument now
            if (cmd_file_argc == 0 || c == ' ') {
                int prev_cmd_file_argc = cmd_file_argc++;
                if (prev_cmd_file_argc > 0) {
                    cmd_file_argv[prev_cmd_file_argc - 1][ch_idx_counter] = '\0';
                }

                ch_idx_counter = 0;

                char **new_cmd_file_argv = realloc(cmd_file_argv, cmd_file_argc * sizeof(*cmd_file_argv));
                if (new_cmd_file_argv != NULL) {
                    cmd_file_argv = new_cmd_file_argv;
                    cmd_file_argv[cmd_file_argc - 1] = NULL;
                } else {
                    file_read_free(cmd_file, cmd_file_argc, cmd_file_argv);
                    sigerr_c("alloc");
                    return;
                }

                if (c == ' ') {
                    continue;
                }
            }

            int arg_idx = cmd_file_argc - 1;
            int ch_idx = ch_idx_counter++;
            char *arg = realloc(cmd_file_argv[arg_idx], (ch_idx_counter + 1) * sizeof(*arg));
            if (arg != NULL) {
                cmd_file_argv[arg_idx] = arg;
            } else {
                file_read_free(cmd_file, cmd_file_argc, cmd_file_argv);
                sigerr_c("alloc");
                return;
            }

            arg[ch_idx] = c;
        }

        fclose(cmd_file);

        return;
    }

    if (eq_ignore_case("CSN", argv[1])) {
        FILE *file = fopen(argv[2], "r");
        if (file == NULL) {
            printf("No such file with name: %s\n", argv[2]);
            fclose(file);
            return;
        }

        snm_parse_data(file, &csn_data_len, &csn_data_array);

        fclose(file);
        printf("Parsed CSN file '%s'\n", argv[2]);

        return;
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

static void print_frames(SpiceCell *frame_id_cells, char *frame_type) {
    SpiceInt cells_len = card_c(frame_id_cells);
    if (cells_len > 0) {
        printf("Printing %d '%s' frames:\n", cells_len, frame_type);
        for (int i = 0; i < cells_len; ++i) {
            SpiceInt frame_id = ((SpiceInt *) frame_id_cells->data)[i];
            SpiceChar frame_name[FRAME_NAME_MAX_LEN];
            frmnam_c(frame_id, FRAME_NAME_MAX_LEN, frame_name);

            printf("%d: %s\n", frame_id, frame_name);
        }
    } else {
        printf("No '%s' frames found", frame_type);
    }
}

void show(int argc, char **argv, volatile int *is_running) {
    if (argc != 2) {
        puts("This command requires 1 argument");
        return;
    }

    if (eq_ignore_case("TABLES", argv[1])) {
        SpiceInt table_count;
        ekntab_c(&table_count);
        if (table_count == 0) {
            puts("No tables found. Try LOAD KERNEL?");
            return;
        }

        for (int i = 0; i < table_count; ++i) {
            SpiceChar table_name[TAB_NAME_MAX_LEN];
            ektnam_c(0, TAB_NAME_MAX_LEN, table_name);

            printf("%s\n", table_name);
        }

        return;
    }

    if (eq_ignore_case("FRAMES", argv[1])) {
        SPICEINT_CELL(blt_frames, SPICE_NFRAME_NINERT + SPICE_NFRAME_NNINRT);
        bltfrm_c(SPICE_FRMTYP_ALL, &blt_frames);
        print_frames(&blt_frames, "built-in");

        puts("");

        SPICEINT_CELL(k_frames, KERNEL_FRAMES_MAX_LEN);
        kplfrm_c(SPICE_FRMTYP_ALL, &k_frames);
        print_frames(&k_frames, "kernel-loaded");

        return;
    }

    if (eq_ignore_case("CSN", argv[1])) {
        if (csn_data_len == -1) {
            puts("No CSN file loaded. Try LOAD CSN?");
            return;
        }

        printf("Showing names for %d named stars:\n", csn_data_len);
        for (int i = 0; i < csn_data_len; ++i) {
            csn_data data = csn_data_array[i];
            printf("%s (HIP %d)\n", data.name, data.hip);
        }

        return;
    }

    if (eq_ignore_case("BODIES", argv[1])) {
        puts("Showing loaded body NAIF IDs and their respective names...");
        puts("(This might take a while)");
        for (int i = NAIF_ID_MIN; i < NAIF_ID_MAX; ++i) {
            if (!*is_running) {
                *is_running = SPICETRUE;
                return;
            }

            SpiceChar body_name[BODY_NAME_MAX_LEN];
            SpiceBoolean found;
            bodc2n_c(i, BODY_NAME_MAX_LEN, body_name, &found);

            if (found) {
                printf("%d: %s\n", i, body_name);
            }
        }

        return;
    }

    if (eq_ignore_case("SAT", argv[1])) {
        if (sat_data_array.size == 0) {
            printf("No satellites added\n");
            return;
        }

        printf("Showing %d satellite IDs:\n", sat_data_array.size);
        for (int i = 0; i < sat_data_array.buckets_len; ++i) {
            for (gatecli_table_entry *entry = sat_data_array.buckets[i];
                 entry != NULL;
                 entry = entry->next) {
                printf("%s\n", entry->key);
            }
        }

        return;
    }

    if (eq_ignore_case("CALC", argv[1])) {
        if (calc_data_array.size == 0) {
            printf("No custom objects added\n");
            return;
        }

        printf("Showing %d custom body IDs:\n", calc_data_array.size);
        for (int i = 0; i < calc_data_array.buckets_len; ++i) {
            for (gatecli_table_entry *entry = calc_data_array.buckets[i];
                 entry != NULL;
                 entry = entry->next) {
                printf("%s\n", entry->key);
            }
        }

        return;
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

static void *check_and_get_option(option_key key) {
    void *option = get_option(key);
    if (option == NULL) {
        const char *option_string = key_to_string(key);
        setmsg_c("Option '%s' should not be NULL. Try SET %s <value>?\n");
        errch_c("%s", option_string);
        errch_c("%s", option_string);
        sigerr_c("option unset");

        return NULL;
    }

    return option;
}

static void star_info(char *catalog_number) {
    char *table_name = (char *) check_and_get_option(STAR_TABLE);
    if (table_name == NULL) {
        return;
    }

    SpiceChar filter[FILTER_MAX_LEN];
    snprintf(filter, FILTER_MAX_LEN, "WHERE CATALOG_NUMBER = %s", catalog_number);

    SpiceInt rows;
    gate_load_stars(table_name, filter, &rows);
    if (rows == 0) {
        printf("No stars found in table '%s' with catalog number '%s'\n", table_name, catalog_number);
        return;
    }

    printf("Showing %d results for catalog number %s from table '%s':\n\n",
           rows, catalog_number, table_name);

    gate_star_info_spice1 parsed_stars[rows];
    gate_parse_stars(rows, parsed_stars);
    for (int i = 0; i < rows; ++i) {
        gate_star_info_spice1 info = parsed_stars[i];

        char *name = NULL;
        if (strcmp("HIPPARCOS", table_name) == 0) {
            for (int j = 0; j < csn_data_len; ++j) {
                csn_data data = csn_data_array[j];
                if (data.hip == info.catalog_number) {
                    name = data.name;
                    break;
                }
            }

            if (name == NULL) {
                name = "(Unnamed star)";
            }
        } else {
            name = "(Not supported for non-HIPPARCOS catalog)";
        }

        printf("Catalog number: %d\n"
               "Declination: %f degrees\n"
               "Declination epoch: %f years since 1950\n"
               "Declination proper motion: %f degrees per year\n"
               "Declination proper motion sigma: %f degrees per year\n"
               "Declination sigma: %f degrees\n"
               "Durchmusterung identifier: %d\n"
               "Parallax: %f degrees\n"
               "Right ascension: %f degrees\n"
               "Right ascension epoch: %f years since 1950\n"
               "Right ascension proper motion: %f degrees per year\n"
               "Right ascension proper motion sigma: %f degrees per year\n"
               "Right ascension sigma: %f degrees\n"
               "Spectral type: %s\n"
               "Visual magnitude: %f\n"
               "Name: %s\n",
               info.catalog_number,
               info.dec, info.dec_epoch, info.dec_pm, info.dec_pm_sigma, info.dec_sigma,
               info.dm_number, info.parallax,
               info.ra, info.ra_epoch, info.ra_pm, info.ra_pm_sigma, info.ra_sigma,
               info.spectral_type, info.visual_magnitude,
               name);

        if (i != rows - 1) {
            puts("");
        }
    }
}

static void star_azel(char **argv, volatile int *is_running) {
    SpiceChar filter[FILTER_MAX_LEN];
    snprintf(filter, FILTER_MAX_LEN, "WHERE CATALOG_NUMBER = %s", argv[2]);

    char *table_name = (char *) check_and_get_option(STAR_TABLE);
    if (table_name == NULL) {
        return;
    }

    SpiceInt rows;
    gate_load_stars(table_name, filter, &rows);
    if (rows == 0) {
        printf("No stars found in table '%s' with catalog number '%s'\n", table_name, argv[2]);
        return;
    }

    SpiceBoolean is_cont = SPICEFALSE;
    SpiceInt count;
    if (eq_ignore_case("CONT", argv[3])) {
        is_cont = SPICETRUE;
    } else {
        char *end;
        count = strtol(argv[3], &end, 10);
        if (argv[3] == end) {
            printf("Not a valid number: %s\n", argv[3]);
            return;
        }
    }

    SpiceDouble calc_et;
    if (eq_ignore_case("NOW", argv[4])) {
        gate_et_now(&calc_et);
    } else {
        str2et_c(argv[4], &calc_et);
    }

    char *observer_body = (char *) check_and_get_option(OBSERVER_BODY);
    if (observer_body == NULL) {
        return;
    }

    SpiceInt body_id;
    SpiceBoolean body_id_found;
    bodn2c_c(observer_body, &body_id, &body_id_found);
    if (!body_id_found) {
        printf("No NAIF ID was found for body '%s'! Try LOAD KERNEL?\n", observer_body);
        return;
    }

    SpiceDouble *observer_latitude_opt = (double *) check_and_get_option(OBSERVER_LATITUDE);
    SpiceDouble *observer_longitude_opt = (double *) check_and_get_option(OBSERVER_LONGITUDE);
    if (observer_latitude_opt == NULL || observer_longitude_opt == NULL) {
        return;
    }

    SpiceDouble observer_latitude = *observer_latitude_opt;
    SpiceDouble observer_longitude = *observer_longitude_opt;

    gate_topo_frame observer_frame;
    gate_load_topo_frame("STAR_AZEL_TOPO", body_id, observer_latitude, observer_longitude, 0, &observer_frame);

    gate_star_info_spice1 stars[rows];
    gate_parse_stars(rows, stars);

    printf("Printing azimuth/elevation for star '%s' in table '%s'\n\n", argv[2], table_name);

    SpiceDouble loop_start_et;
    gate_et_now(&loop_start_et);

    int rounds = 0;
    while (SPICETRUE) {
        SpiceChar calc_time_out[TIME_OUT_MAX_LEN];
        timout_c(calc_et, "YYYY-MM-DD HR:MN:SC.#### UTC ::UTC", TIME_OUT_MAX_LEN, calc_time_out);

        printf("%s:\n", calc_time_out);

        for (int i = 0; i < rows; ++i) {
            gate_star_info_spice1 info = stars[i];

            SpiceDouble azimuth;
            SpiceDouble elevation;
            gate_calc_star_topo(observer_frame, info, calc_et, NULL, &azimuth, &elevation);
            printf("Azimuth=%f Elevation=%f\n", azimuth, elevation);
        }

        if (!is_cont) {
            rounds++;
            if (rounds == count) {
                break;
            }
        } else {
            if (!*is_running) {
                *is_running = SPICETRUE;
                break;
            }
        }

        puts("");
        sleep(1);

        SpiceDouble current_et;
        gate_et_now(&current_et);

        SpiceDouble elapsed = current_et - loop_start_et;
        loop_start_et = current_et;

        calc_et += elapsed;
    }

    gate_unload_topo_frame(observer_frame);
}

void star(int argc, char **argv, volatile int *is_running) {
    if (argc < 2) {
        puts("This command requires at least 1 argument");
        return;
    }

    if (eq_ignore_case("INFO", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return star_info(argv[2]);
    }

    if (eq_ignore_case("AZEL", argv[1])) {
        if (argc != 5) {
            puts("This command requires 3 arguments");
            return;
        }
        return star_azel(argv, is_running);
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

static void body_info(char *naif_id_string) {
    char *naif_id_string_end;
    SpiceInt naif_id = strtol(naif_id_string, &naif_id_string_end, 10);
    if (naif_id_string == naif_id_string_end) {
        printf("'%s' is not a valid NAIF ID\n", naif_id_string);
        return;
    }

    SpiceChar body_name[BODY_NAME_MAX_LEN];
    SpiceBoolean found;
    bodc2n_c(naif_id, BODY_NAME_MAX_LEN, body_name, &found);

    if (!found) {
        printf("No body with NAIF ID '%s'. Try LOAD KERNEL?\n", naif_id_string);
        return;
    }

    printf("Info for body with NAIF ID '%s':\n\n", naif_id_string);
    printf("Body name: %s\n", body_name);
}

static void body_azel(char **argv, volatile int *is_running) {
    char *naif_id_string_end;
    SpiceInt naif_id = strtol(argv[2], &naif_id_string_end, 10);
    if (argv[2] == naif_id_string_end) {
        printf("'%s' is not a valid NAIF ID\n", argv[2]);
        return;
    }

    SpiceChar body_name[BODY_NAME_MAX_LEN];
    SpiceBoolean found;
    bodc2n_c(naif_id, BODY_NAME_MAX_LEN, body_name, &found);

    if (!found) {
        printf("No body with NAIF ID '%s'. Try LOAD KERNEL?\n", argv[2]);
        return;
    }

    SpiceBoolean is_cont = SPICEFALSE;
    SpiceInt count;
    if (eq_ignore_case("CONT", argv[3])) {
        is_cont = SPICETRUE;
    } else {
        char *end;
        count = strtol(argv[3], &end, 10);
        if (argv[3] == end) {
            printf("Not a valid number: %s\n", argv[3]);
            return;
        }
    }

    SpiceDouble calc_et;
    if (eq_ignore_case("NOW", argv[4])) {
        gate_et_now(&calc_et);
    } else {
        str2et_c(argv[4], &calc_et);
    }

    char *observer_body = (char *) check_and_get_option(OBSERVER_BODY);
    if (observer_body == NULL) {
        return;
    }

    SpiceInt observer_body_id;
    SpiceBoolean observer_body_id_found;
    bodn2c_c(observer_body, &observer_body_id, &observer_body_id_found);
    if (!observer_body_id_found) {
        printf("No NAIF ID was found for body '%s'! Try LOAD KERNEL?\n", observer_body);
        return;
    }

    SpiceDouble *observer_latitude_opt = (double *) check_and_get_option(OBSERVER_LATITUDE);
    SpiceDouble *observer_longitude_opt = (double *) check_and_get_option(OBSERVER_LONGITUDE);
    if (observer_latitude_opt == NULL || observer_longitude_opt == NULL) {
        return;
    }

    SpiceDouble observer_latitude = *observer_latitude_opt;
    SpiceDouble observer_longitude = *observer_longitude_opt;

    gate_topo_frame observer_frame;
    gate_load_topo_frame("BODY_AZEL_TOPO", observer_body_id, observer_latitude, observer_longitude, 0,
                         &observer_frame);

    printf("Printing azimuth/elevation for body '%s' (%s)\n\n", argv[2], body_name);

    SpiceDouble loop_start_et;
    gate_et_now(&loop_start_et);

    int rounds = 0;
    while (SPICETRUE) {
        SpiceChar calc_time_out[TIME_OUT_MAX_LEN];
        timout_c(calc_et, "YYYY-MM-DD HR:MN:SC.#### UTC ::UTC", TIME_OUT_MAX_LEN, calc_time_out);

        printf("%s:\n", calc_time_out);

        SpiceDouble state_vector[6];
        SpiceDouble lt;
        spkezr_c(body_name, calc_et, observer_frame.frame_name, "CN+S", observer_body,
                 state_vector, &lt);

        SpiceDouble azimuth;
        SpiceDouble elevation;
        gate_conv_rec_azel(state_vector, NULL, &azimuth, &elevation);
        printf("Azimuth=%f Elevation=%f\n", azimuth, elevation);

        if (!is_cont) {
            rounds++;
            if (rounds == count) {
                break;
            }
        } else {
            if (!*is_running) {
                *is_running = SPICETRUE;
                break;
            }
        }

        puts("");
        sleep(1);

        SpiceDouble current_et;
        gate_et_now(&current_et);

        SpiceDouble elapsed = current_et - loop_start_et;
        loop_start_et = current_et;

        calc_et += elapsed;
    }

    gate_unload_topo_frame(observer_frame);
}

void body(int argc, char **argv, volatile int *is_running) {
    if (argc < 2) {
        puts("This command requires at least 1 argument");
        return;
    }

    if (eq_ignore_case("INFO", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return body_info(argv[2]);
    }

    if (eq_ignore_case("AZEL", argv[1])) {
        if (argc != 5) {
            puts("This command requires 3 arguments");
            return;
        }
        return body_azel(argv, is_running);
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

static char *read_line(int line_len, SpiceChar *line) {
    for (int i = 0; i < line_len; ++i) {
        line[i] = fgetc(stdin);
    }

    // Overwrite the new line char with NULL
    line[line_len - 1] = '\0';

    return line;
}

static void sat_add(char *arg) {
    printf("Paste TLE data below:\n");

    SpiceChar lines[2][TLE_LINE_LEN];
    read_line(TLE_LINE_LEN, lines[0]);
    read_line(TLE_LINE_LEN, lines[1]);
    if (lines[0] == NULL || lines[1] == NULL) {
        printf("Invalid input\n");
        return;
    }

    char *id = strdup(arg);
    sat_data *data = malloc(sizeof(*data));

    data->lines[0] = strdup(lines[0]);
    data->lines[1] = strdup(lines[1]);
    getelm_c(TLE_MIN_YEAR, TLE_LINE_LEN, lines, &data->epoch, data->elements);

    char orbit_freq_str[12];
    memcpy(orbit_freq_str, lines[1] + 52, 11 * sizeof(*orbit_freq_str));
    orbit_freq_str[11] = '\0';

    char *end;
    double orbit_freq = strtod(orbit_freq_str, &end);
    if (end == orbit_freq_str) {
        printf("Error parsing orbit frequency '%s'\n", orbit_freq_str);
        return;
    }

    double orbit_period = 24 * 60 / orbit_freq;
    data->is_deep_space = orbit_period >= 225;

    gatecli_table_put(&sat_data_array, id, data);
    printf("Added satellite '%s' to the database\n", arg);
}

static void sat_rem(char *arg) {
    sat_data *data = gatecli_table_rem(&sat_data_array, arg);
    if (data != NULL) {
        free(data->lines[0]);
        free(data->lines[1]);
        free(data);

        printf("Successfully removed satellite '%s'\n", arg);
    } else {
        printf("No satellite in database called '%s'\n", arg);
    }
}

static void sat_info(char *arg) {
    sat_data *data = gatecli_table_get(&sat_data_array, arg);
    if (data == NULL) {
        printf("No object in database called '%s'\n", arg);
        return;
    }

    printf("ID: %s\n"
           "Epoch: %f seconds\n"
           "Deep space: %s\n"
           "\n"
           "TLE:\n"
           "%s\n"
           "%s\n",
           arg, data->epoch, data->is_deep_space ? "yes" : "no", data->lines[0], data->lines[1]);
}

static void sat_azel(char **argv, volatile int *is_running) {
    sat_data *sat = gatecli_table_get(&sat_data_array, argv[2]);
    if (sat == NULL) {
        printf("No satellite with ID '%s'. Try SAT ADD?\n", argv[2]);
        return;
    }

    SpiceBoolean is_cont = SPICEFALSE;
    SpiceInt count;
    if (eq_ignore_case("CONT", argv[3])) {
        is_cont = SPICETRUE;
    } else {
        char *end;
        count = strtol(argv[3], &end, 10);
        if (argv[3] == end) {
            printf("Not a valid number: %s\n", argv[3]);
            return;
        }
    }

    SpiceDouble calc_et;
    if (eq_ignore_case("NOW", argv[4])) {
        gate_et_now(&calc_et);
    } else {
        str2et_c(argv[4], &calc_et);
    }

    char *observer_body = (char *) check_and_get_option(OBSERVER_BODY);
    if (observer_body == NULL) {
        return;
    }

    SpiceInt observer_body_id;
    SpiceBoolean observer_body_id_found;
    bodn2c_c(observer_body, &observer_body_id, &observer_body_id_found);
    if (!observer_body_id_found) {
        printf("No NAIF ID was found for body '%s'! Try LOAD KERNEL?\n", observer_body);
        return;
    }

    SpiceDouble *observer_latitude_opt = (double *) check_and_get_option(OBSERVER_LATITUDE);
    SpiceDouble *observer_longitude_opt = (double *) check_and_get_option(OBSERVER_LONGITUDE);
    if (observer_latitude_opt == NULL || observer_longitude_opt == NULL) {
        return;
    }

    SpiceDouble observer_latitude = *observer_latitude_opt;
    SpiceDouble observer_longitude = *observer_longitude_opt;

    gate_topo_frame observer_frame;
    gate_load_topo_frame("SAT_AZEL_TOPO", observer_body_id, observer_latitude, observer_longitude, 0,
                         &observer_frame);

    printf("Printing azimuth/elevation for custom ID '%s' (%s)\n\n", argv[2], argv[2]);

    SpiceDouble loop_start_et;
    gate_et_now(&loop_start_et);

    int rounds = 0;
    while (SPICETRUE) {
        SpiceChar calc_time_out[TIME_OUT_MAX_LEN];
        timout_c(calc_et, "YYYY-MM-DD HR:MN:SC.#### UTC ::UTC", TIME_OUT_MAX_LEN, calc_time_out);

        printf("%s:\n", calc_time_out);

        SpiceDouble frame_transform_matrix[3][3];
        pxform_c("J2000", observer_frame.frame_name, calc_et, frame_transform_matrix);

        SpiceDouble cur_rec_j2000[6];
        if (!sat->is_deep_space) {
            ev2lin_(&calc_et, GEO_CONSTANTS, sat->elements, cur_rec_j2000);
        } else {
            dpspce_(&calc_et, GEO_CONSTANTS, sat->elements, cur_rec_j2000);
        }

        SpiceDouble rec[3];
        mxv_c(frame_transform_matrix, cur_rec_j2000, rec);
        gate_adjust_topo_rec(observer_frame, rec);

        SpiceDouble azimuth;
        SpiceDouble elevation;
        gate_conv_rec_azel(rec, NULL, &azimuth, &elevation);
        printf("Azimuth=%f Elevation=%f\n", azimuth, elevation);

        if (!is_cont) {
            rounds++;
            if (rounds == count) {
                break;
            }
        } else {
            if (!*is_running) {
                *is_running = SPICETRUE;
                break;
            }
        }

        puts("");
        sleep(1);

        SpiceDouble current_et;
        gate_et_now(&current_et);

        SpiceDouble elapsed = current_et - loop_start_et;
        loop_start_et = current_et;

        calc_et += elapsed;
    }

    gate_unload_topo_frame(observer_frame);
}

void sat(int argc, char **argv, volatile int *is_running) {
    if (argc < 2) {
        puts("This command requires at least 1 argument");
        return;
    }

    if (eq_ignore_case("ADD", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return sat_add(argv[2]);
    }

    if (eq_ignore_case("REM", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return sat_rem(argv[2]);
    }

    if (eq_ignore_case("INFO", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return sat_info(argv[2]);
    }

    if (eq_ignore_case("AZEL", argv[1])) {
        if (argc != 5) {
            puts("This command requires 3 arguments");
            return;
        }
        return sat_azel(argv, is_running);
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

static void calc_add(int argc, char **argv) {
    if (gatecli_table_get(&calc_data_array, argv[2]) != NULL) {
        printf("WARNING: Replacing existing data entry '%s'\n", argv[2]);
    }

    char *end;
    double r;
    double ra;
    double dec;
    double ra_pm = 0;
    double dec_pm = 0;
    if (argc >= 6) {
        r = strtod(argv[3], &end);
        if (end == argv[3]) {
            printf("Range '%s' is not a number\n", argv[3]);
            return;
        }

        ra = strtod(argv[4], &end);
        if (end == argv[4]) {
            printf("Right ascension '%s' is not a number\n", argv[4]);
            return;
        }

        dec = strtod(argv[5], &end);
        if (end == argv[5]) {
            printf("Declination '%s' is not a number\n", argv[5]);
            return;
        }
    } else {
        printf("Unrecognized command format\n");
        return;
    }

    if (argc == 8) {
        ra_pm = strtod(argv[6], &end);
        if (end == argv[6]) {
            printf("Right ascension '%s' is not a number\n", argv[6]);
            return;
        }

        dec_pm = strtod(argv[7], &end);
        if (end == argv[7]) {
            printf("Declination '%s' is not a number\n", argv[7]);
            return;
        }
    } else if (argc != 6) {
        printf("Unrecognized command format\n");
        return;
    }

    char *id = strdup(argv[2]);
    calc_data *calc = malloc(sizeof(*calc));
    calc->r = r;
    calc->ra = ra;
    calc->dec = dec;
    calc->ra_pm = ra_pm;
    calc->dec_pm = dec_pm;
    gatecli_table_put(&calc_data_array, id, calc);

    printf("Added '%s' to the custom object database\n", argv[2]);
}

static void calc_rem(char *arg) {
    calc_data *data = gatecli_table_rem(&calc_data_array, arg);
    if (data != NULL) {
        free(data);
        printf("Successfully removed custom object '%s'\n", arg);
    } else {
        printf("No object in database called '%s'\n", arg);
    }
}

static void calc_info(char *arg) {
    calc_data *data = gatecli_table_get(&calc_data_array, arg);
    if (data == NULL) {
        printf("No object in database called '%s'\n", arg);
        return;
    }

    printf("ID: %s\n"
           "Range: %f units\n"
           "Right ascension: %f deg\n"
           "Declination: %f deg\n"
           "Right ascension proper motion: %f deg/yr\n"
           "Declination proper motion: %f deg/yr\n",
           arg, data->r, data->ra, data->dec, data->ra_pm, data->dec_pm);
}

void calc_cur_pos(calc_data data, SpiceDouble et, SpiceDouble *ra, SpiceDouble *dec) {
    SpiceDouble dt = et / jyear_c();

    if (ra != NULL) {
        *ra = data.ra + (dt * data.ra_pm);
    }

    if (dec != NULL) {
        *dec = data.dec + (dt * data.dec_pm);
    }
}

static void calc_azel(char **argv, volatile int *is_running) {
    calc_data *body = gatecli_table_get(&calc_data_array, argv[2]);
    if (body == NULL) {
        printf("No custom body with ID '%s'. Try CALC ADD?\n", argv[2]);
        return;
    }

    SpiceBoolean is_cont = SPICEFALSE;
    SpiceInt count;
    if (eq_ignore_case("CONT", argv[3])) {
        is_cont = SPICETRUE;
    } else {
        char *end;
        count = strtol(argv[3], &end, 10);
        if (argv[3] == end) {
            printf("Not a valid number: %s\n", argv[3]);
            return;
        }
    }

    SpiceDouble calc_et;
    if (eq_ignore_case("NOW", argv[4])) {
        gate_et_now(&calc_et);
    } else {
        str2et_c(argv[4], &calc_et);
    }

    char *observer_body = (char *) check_and_get_option(OBSERVER_BODY);
    if (observer_body == NULL) {
        return;
    }

    SpiceInt observer_body_id;
    SpiceBoolean observer_body_id_found;
    bodn2c_c(observer_body, &observer_body_id, &observer_body_id_found);
    if (!observer_body_id_found) {
        printf("No NAIF ID was found for body '%s'! Try LOAD KERNEL?\n", observer_body);
        return;
    }

    SpiceDouble *observer_latitude_opt = (double *) check_and_get_option(OBSERVER_LATITUDE);
    SpiceDouble *observer_longitude_opt = (double *) check_and_get_option(OBSERVER_LONGITUDE);
    if (observer_latitude_opt == NULL || observer_longitude_opt == NULL) {
        return;
    }

    SpiceDouble observer_latitude = *observer_latitude_opt;
    SpiceDouble observer_longitude = *observer_longitude_opt;

    gate_topo_frame observer_frame;
    gate_load_topo_frame("CALC_AZEL_TOPO", observer_body_id, observer_latitude, observer_longitude, 0,
                         &observer_frame);

    printf("Printing azimuth/elevation for custom ID '%s' (%s)\n\n", argv[2], argv[2]);

    SpiceDouble loop_start_et;
    gate_et_now(&loop_start_et);

    int rounds = 0;
    while (SPICETRUE) {
        SpiceChar calc_time_out[TIME_OUT_MAX_LEN];
        timout_c(calc_et, "YYYY-MM-DD HR:MN:SC.#### UTC ::UTC", TIME_OUT_MAX_LEN, calc_time_out);

        printf("%s:\n", calc_time_out);

        SpiceDouble ra;
        SpiceDouble dec;
        calc_cur_pos(*body, calc_et, &ra, &dec);

        SpiceDouble cur_rec_j2000[3];
        SpiceDouble ra_rad = ra * rpd_c();
        SpiceDouble dec_rad = dec * rpd_c();
        radrec_c(body->r, ra_rad, dec_rad, cur_rec_j2000);

        SpiceDouble frame_transform_matrix[3][3];
        pxform_c("J2000", observer_frame.frame_name, calc_et, frame_transform_matrix);

        SpiceDouble rec[3];
        mxv_c(frame_transform_matrix, cur_rec_j2000, rec);
        gate_adjust_topo_rec(observer_frame, rec);

        SpiceDouble azimuth;
        SpiceDouble elevation;
        gate_conv_rec_azel(rec, NULL, &azimuth, &elevation);
        printf("Azimuth=%f Elevation=%f\n", azimuth, elevation);

        if (!is_cont) {
            rounds++;
            if (rounds == count) {
                break;
            }
        } else {
            if (!*is_running) {
                *is_running = SPICETRUE;
                break;
            }
        }

        puts("");
        sleep(1);

        SpiceDouble current_et;
        gate_et_now(&current_et);

        SpiceDouble elapsed = current_et - loop_start_et;
        loop_start_et = current_et;

        calc_et += elapsed;
    }

    gate_unload_topo_frame(observer_frame);
}


void calc(int argc, char **argv, volatile int *is_running) {
    if (argc < 2) {
        puts("This command requires at least 1 argument");
        return;
    }

    if (eq_ignore_case("ADD", argv[1])) {
        if (argc < 6) {
            puts("This command at least 4 arguments");
            return;
        }
        return calc_add(argc, argv);
    }

    if (eq_ignore_case("REM", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return calc_rem(argv[2]);
    }

    if (eq_ignore_case("INFO", argv[1])) {
        if (argc != 3) {
            puts("This command requires 1 argument");
            return;
        }
        return calc_info(argv[2]);
    }

    if (eq_ignore_case("AZEL", argv[1])) {
        if (argc != 5) {
            puts("This command requires 3 arguments");
            return;
        }
        return calc_azel(argv, is_running);
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}
