#include "commands.h"
#include <stdio.h>
#include <cspice/SpiceUsr.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <timeconv.h>

#include "stars.h"
#include "topo.h"
#include "options.h"
#include "util.h"

#define TAB_NAME_MAX_LEN 100

void help() {
    puts("--- HELP ---");
    puts("HELP - prints this message");
    puts("LOAD <OPTION | KERNEL> <filename> - loads a pre-set options or a kernel from file");
    puts("SET <option> <value> - sets the value of a particular option");
    puts("GET <option> - prints the value of a particular option");
    puts("SHOW <TABLES> - prints the available table names");
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

        if (eq_ignore_case(valid_name, table_name)) {
            return SPICETRUE;
        }
    }

    return SPICEFALSE;
}

void set(char **argv) {
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

void get(char **argv) {
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

void load(char **argv) {
    if (eq_ignore_case("KERNEL", argv[1])) {
        furnsh_c(argv[2]);
        printf("Loaded kernel for file '%s'\n", argv[2]);
        return;
    }

    if (eq_ignore_case("OPTIONS", argv[1])) {
        puts("Not implemented.");
        // TODO: Load option pre-set
        return;
    }

    printf("Unrecognized option: '%s'\n", argv[1]);
}

void show(char **argv) {
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

// TODO: Register as command
void handle_star() {
    char *observer_body = (char *) check_and_get_option(OBSERVER_BODY);
    SpiceDouble observer_latitude = *(double *) check_and_get_option(OBSERVER_LATITUDE);
    SpiceDouble observer_longitude = *(double *) check_and_get_option(OBSERVER_LONGITUDE);
    char *table_name = (char *) check_and_get_option(STAR_TABLE);

    SpiceInt body_id;
    SpiceBoolean body_id_found;
    bodn2c_c(observer_body, &body_id, &body_id_found);
    if (!body_id_found) {
        printf("No NAIF ID was found for body '%s'! Try LOAD KERNEL?\n", observer_body);
        return;
    }
    
    gate_topo_frame observer_frame;
    gate_load_topo_frame("STAR_CMD_TOPO", body_id, observer_latitude, observer_longitude, 0, &observer_frame);

    SpiceInt rows;
    gate_load_stars(table_name, "WHERE CATALOG_NUMBER = 70890", &rows);

    gate_star_info_spice1 stars[rows];
    gate_parse_stars(rows, stars);

    struct timespec cur_time;
    clock_gettime(CLOCK_REALTIME, &cur_time);

    SpiceDouble et;
    gate_unix_clock_to_et(cur_time, &et);

    for (int i = 0; i < rows; ++i) {
        gate_star_info_spice1 info = stars[i];

        SpiceDouble azimuth;
        SpiceDouble elevation;
        gate_calc_star_topo(observer_frame, info, et, NULL, &azimuth, &elevation);
        printf("Look angle: az=%f el=%f\n", azimuth, elevation);
    }

    gate_unload_topo_frame(observer_frame);
}
