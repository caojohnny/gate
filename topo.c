#include "topo.h"
#include <stdio.h>

#define FRAME_ID_BEGIN 1400000
#define FRAME_ID_END 2000000
#define MAX_FRAME_IDS 100

#define BUFFER_LINE_COUNT 10
#define BUFFER_MAX_LINE_LEN 100
#define EARTH_FIXED_FRAME_ID "EARTH_FIXED"
#define KERNEL_MAX_VAR_LEN 33

static void find_free_frame_id(SpiceInt *frame_id) {
    SPICEINT_CELL(used_ids, MAX_FRAME_IDS);
    kplfrm_c(SPICE_FRMTYP_TK, &used_ids);

    *frame_id = FRAME_ID_BEGIN;

    SpiceInt cell_count = card_c(&used_ids);
    if (cell_count == 0) {
        return;
    }

    for (int i = 0; i < cell_count; ++i) {
        SpiceInt used_id = *((SpiceInt *) (&used_ids + i)->data);
        if (*frame_id == used_id) {
            *frame_id += 1;
        }
    }

    if (*frame_id > FRAME_ID_END) {
        sigerr_c("find_frame_id");
    }
}

void load_topo_frame(SpiceChar *frame_name, SpiceDouble lon, SpiceDouble lat) {
    SpiceInt frame_id_test;
    namfrm_c(frame_name, &frame_id_test);
    if (frame_id_test != 0) {
        setmsg_c("Duplicate frame name: %s");
        errch_c("%s", frame_name);
        sigerr_c("dup_name");
        return;
    }

    SpiceInt frame_id;
    find_free_frame_id(&frame_id);

    SpiceDouble lon_adjusted = lon;
    if (lon < 0) {
        lon_adjusted = 360 + lon;
    }
    SpiceDouble lat_adjusted = 90 - lat;

    // Reference: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/frames.html#TK%20frame%20---%20Topographic
    SpiceChar kernel_buffer[BUFFER_LINE_COUNT][BUFFER_MAX_LINE_LEN];
    snprintf(kernel_buffer[0], BUFFER_MAX_LINE_LEN, "FRAME_%s = %d", frame_name, frame_id);
    snprintf(kernel_buffer[1], BUFFER_MAX_LINE_LEN, "FRAME_%d_NAME = '%s'", frame_id, frame_name);
    snprintf(kernel_buffer[2], BUFFER_MAX_LINE_LEN, "FRAME_%d_CLASS = 4", frame_id);
    snprintf(kernel_buffer[3], BUFFER_MAX_LINE_LEN, "FRAME_%d_CENTER = 399", frame_id);
    snprintf(kernel_buffer[4], BUFFER_MAX_LINE_LEN, "FRAME_%d_CLASS_ID = %d", frame_id, frame_id);
    snprintf(kernel_buffer[5], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_RELATIVE = '"EARTH_FIXED_FRAME_ID"'", frame_id);
    snprintf(kernel_buffer[6], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_SPEC = 'ANGLES'", frame_id);
    snprintf(kernel_buffer[7], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_UNITS = 'DEGREES'", frame_id);
    snprintf(kernel_buffer[8], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_AXES = (3, 2, 3)", frame_id);
    snprintf(kernel_buffer[9], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_ANGLES = (-%.10f, -%.10f, 180)", frame_id, lon_adjusted, lat_adjusted);
    lmpool_c(kernel_buffer, BUFFER_MAX_LINE_LEN, BUFFER_LINE_COUNT);
}

void unload_topo_frame(SpiceChar *frame_name) {
    SpiceInt frame_id;
    namfrm_c(frame_name, &frame_id);
    if (frame_id == 0) {
        setmsg_c("Frame name cannot be resolved: %s");
        errch_c("%s", frame_name);
        sigerr_c("no_name");
        return;
    }

    SpiceChar kernel_buffer[BUFFER_LINE_COUNT][KERNEL_MAX_VAR_LEN];
    snprintf(kernel_buffer[0], KERNEL_MAX_VAR_LEN, "FRAME_%s", frame_name);
    snprintf(kernel_buffer[1], KERNEL_MAX_VAR_LEN, "FRAME_%d_NAME", frame_id);
    snprintf(kernel_buffer[2], KERNEL_MAX_VAR_LEN, "FRAME_%d_CLASS", frame_id);
    snprintf(kernel_buffer[3], KERNEL_MAX_VAR_LEN, "FRAME_%d_CENTER", frame_id);
    snprintf(kernel_buffer[4], KERNEL_MAX_VAR_LEN, "FRAME_%d_CLASS_ID", frame_id);
    snprintf(kernel_buffer[5], KERNEL_MAX_VAR_LEN, "TKFRAME_%d_RELATIVE", frame_id);
    snprintf(kernel_buffer[6], KERNEL_MAX_VAR_LEN, "TKFRAME_%d_SPEC", frame_id);
    snprintf(kernel_buffer[7], KERNEL_MAX_VAR_LEN, "TKFRAME_%d_UNITS", frame_id);
    snprintf(kernel_buffer[8], KERNEL_MAX_VAR_LEN, "TKFRAME_%d_AXES", frame_id);
    snprintf(kernel_buffer[9], KERNEL_MAX_VAR_LEN, "TKFRAME_%d_ANGLES", frame_id);

    for (int i = 0; i < BUFFER_LINE_COUNT; ++i) {
        dvpool_c(kernel_buffer[i]);
    }
}