#include "commands.h"
#include <stdio.h>
#include <cspice/SpiceUsr.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <timeconv.h>
#include <unistd.h>

#include "stars.h"
#include "topo.h"
#include "options.h"
#include "util.h"
#include "dispatcher.h"

#define TAB_NAME_MAX_LEN 100
#define FILTER_MAX_LEN 100
#define TIME_OUT_MAX_LEN 30

void help() {
    puts("--- HELP ---");
    puts("HELP - prints this message");
    puts("LOAD <CMD | KERNEL> <filename> - loads a set of commands or a kernel from file");
    puts("SET <option> <value> - sets the value of a particular option");
    puts("GET <option> - prints the value of a particular option");
    puts("SHOW <TABLES> - prints the available table names");
    puts("STAR INFO <catalog number> - prints information for a star with the given catalog number");
    puts("STAR AZEL <CONT | count> <catalog number> <ISO time | NOW> - prints the observation position the star with the given catalog number");
    puts("");
    puts("--- OPTIONS ---");
    for (int i = 0; i < OPTION_KEY_LENGTH; ++i) {
        printf("%s\n", key_to_string(i));
    }
}

static char *set_option_string(option_key key, char **argv) {
    size_t body_len = strlen(argv[2]) + 1;
    char *value_copy = malloc(body_len * sizeof(*value_copy));
    strcpy(value_copy, argv[2]);
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
                    fclose(cmd_file);
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

        return;
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

void show(int argc, char **argv) {
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

            printf("%d: %s\n", i, table_name);
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
               "Visual magnitude: %f\n",
               info.catalog_number,
               info.dec, info.dec_epoch, info.dec_pm, info.dec_pm_sigma, info.dec_sigma,
               info.dm_number, info.parallax,
               info.ra, info.ra_epoch, info.ra_pm, info.ra_pm_sigma, info.ra_sigma,
               info.spectral_type, info.visual_magnitude);

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
    gate_load_stars(table_name, "WHERE CATALOG_NUMBER = 70890", &rows);
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
        count = strtod(argv[3], &end);
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

    SpiceDouble loop_start_et;
    gate_et_now(&loop_start_et);

    int rounds = 0;
    while (SPICETRUE) {
        SpiceChar calc_time_out[TIME_OUT_MAX_LEN];
        timout_c(calc_et, "YYYY-MM-DD HR:MN:SC.#### UTC ::UTC", TIME_OUT_MAX_LEN, calc_time_out);

        printf("Azimuth/elevation for star with catalog number %s in table '%s' at %s:\n",
               argv[2], table_name, calc_time_out);

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

            puts("");
        }

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
