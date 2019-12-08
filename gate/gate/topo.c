#include "topo.h"
#include <math.h>
#include <stdio.h>

#define FRAME_ID_BEGIN 1400000
#define FRAME_ID_END 2000000

#define BUFFER_LINE_COUNT 10
#define BUFFER_MAX_LINE_LEN 100

#define KERNEL_MAX_VAR_LEN 33
#define MAX_FRAME_NAME_LEN 26

static void find_free_frame_id(SpiceInt *frame_id) {
    SPICEINT_CELL(used_ids, TOPO_MAX_FRAME_IDS);
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

void gate_load_topo_frame(ConstSpiceChar *frame_name, SpiceInt body_id,
                          SpiceDouble latitude, SpiceDouble longitude, SpiceDouble radius,
                          gate_topo_frame *topo_frame) {
    SpiceInt frame_id_lookup;
    namfrm_c(frame_name, &frame_id_lookup);
    if (frame_id_lookup != 0) {
        setmsg_c("Duplicate frame name: %s");
        errch_c("%s", frame_name);
        sigerr_c("dup_name");
        return;
    }

    SpiceInt body_fixed_frame_id;
    SpiceChar body_fixed_frame_name[MAX_FRAME_NAME_LEN];
    SpiceBoolean body_fixed_frame_found;
    cidfrm_c(body_id, MAX_FRAME_NAME_LEN, &body_fixed_frame_id, body_fixed_frame_name, &body_fixed_frame_found);
    if (!body_fixed_frame_found) {
        setmsg_c("Body fixed frame for %d cannot be found");
        errint_c("%d", body_id);
        sigerr_c("resolve_rel_frame");
        return;
    }

    SpiceInt frame_id;
    find_free_frame_id(&frame_id);

    SpiceDouble lon_adjusted = longitude;
    if (longitude < 0) {
        lon_adjusted = 360 + longitude;
    }
    SpiceDouble lat_adjusted = 90 - latitude;

    // Reference: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/frames.html#TK%20frame%20---%20Topographic
    SpiceChar kernel_buffer[BUFFER_LINE_COUNT][BUFFER_MAX_LINE_LEN];
    snprintf(kernel_buffer[0], BUFFER_MAX_LINE_LEN, "FRAME_%s = %d", frame_name, frame_id);
    snprintf(kernel_buffer[1], BUFFER_MAX_LINE_LEN, "FRAME_%d_NAME = '%s'", frame_id, frame_name);
    snprintf(kernel_buffer[2], BUFFER_MAX_LINE_LEN, "FRAME_%d_CLASS = 4", frame_id);
    snprintf(kernel_buffer[3], BUFFER_MAX_LINE_LEN, "FRAME_%d_CENTER = %d", frame_id, body_id);
    snprintf(kernel_buffer[4], BUFFER_MAX_LINE_LEN, "FRAME_%d_CLASS_ID = %d", frame_id, frame_id);
    snprintf(kernel_buffer[5], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_RELATIVE = '%s'", frame_id, body_fixed_frame_name);
    snprintf(kernel_buffer[6], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_SPEC = 'ANGLES'", frame_id);
    snprintf(kernel_buffer[7], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_UNITS = 'DEGREES'", frame_id);
    snprintf(kernel_buffer[8], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_AXES = (3, 2, 3)", frame_id);
    snprintf(kernel_buffer[9], BUFFER_MAX_LINE_LEN, "TKFRAME_%d_ANGLES = (-%.10f, -%.10f, 180)", frame_id, lon_adjusted,
             lat_adjusted);
    lmpool_c(kernel_buffer, BUFFER_MAX_LINE_LEN, BUFFER_LINE_COUNT);

    gate_topo_frame new_frame = {frame_name, body_id, latitude, longitude, radius};
    *topo_frame = new_frame;
}

void gate_load_earth_topo_frame(ConstSpiceChar *frame_name,
                                SpiceDouble latitude, SpiceDouble longitude, SpiceDouble height,
                                gate_topo_frame *topo_frame) {
    SpiceDouble observer_radius = 0;
    if (height != NAN) {
        SpiceInt returned_count;
        SpiceDouble radii[3];
        bodvrd_c("EARTH", "RADII", 3, &returned_count, radii);

        SpiceDouble lon_radians = longitude * rpd_c();
        SpiceDouble lat_radians = latitude * rpd_c();
        SpiceDouble f = (radii[0] - radii[2]) / radii[0];
        SpiceDouble observer_rec[3];
        georec_c(lon_radians, lat_radians, 0, radii[0], f, observer_rec);

        SpiceDouble earth_radius = sqrt(pow(observer_rec[0], 2) +
                                        pow(observer_rec[1], 2) +
                                        pow(observer_rec[2], 2));
        observer_radius = height + earth_radius;
    }

    gate_load_topo_frame(frame_name, 399, latitude, longitude, observer_radius, topo_frame);
}

// TODO: I'm not actually sure this gets me to the actual topographic
// point - is the Z axis orthogonal to the goedetic observation plane
// or geocentric plane?
void gate_adjust_topo_rec(gate_topo_frame topo_frame, SpiceDouble *rec) {
    rec[2] -= topo_frame.radius;
}

void gate_conv_adjust_topo_rec(gate_topo_frame topo_frame, ConstSpiceChar *units, SpiceDouble *rec) {
    SpiceDouble converted_radius;
    convrt_c(topo_frame.radius, "KM", units, &converted_radius);

    rec[2] -= converted_radius;
}

void gate_conv_rec_azel(SpiceDouble *rec, SpiceDouble *rg, SpiceDouble *az, SpiceDouble *el) {
    SpiceDouble range;
    SpiceDouble azimuth;
    SpiceDouble elevation;
    recrad_c(rec, &range, &azimuth, &elevation);

    if (rg != NULL) {
        *rg = range;
    }

    if (az != NULL) {
        *az = 360 - (azimuth * dpr_c());
    }

    if (el != NULL) {
        *el = elevation * dpr_c();
    }
}

void gate_unload_topo_frame(gate_topo_frame topo_frame) {
    ConstSpiceChar *frame_name = topo_frame.frame_name;

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
