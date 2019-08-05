#ifndef GATE_TOPO_H
#define GATE_TOPO_H

#include <SpiceUsr.h>

// TODO: Possible support for topographic frames on other planets?

/**
 * Loads a new topocentric reference frame by inserting a
 * set of variables for a TK that defines the offset from
 * an EARTH_FIXED frame into the kernel variable pool.
 *
 * This is useful for determining the azimuth and elevation
 * of bodies through the use of position transformations
 * from a known reference frame into the topocentric
 * reference frame of an observer through functions such as
 * pxform_c().
 *
 * Requires earth_fixed.tf and a kernel specifying either
 * IAU_EARTH (usually a generic PCK) or ITRF93 if specified
 * by the EARTH_FIXED reference frame (usually a generic
 * high-accuracy kernel)
 *
 * Frame names are limited to 26 characters to avoid
 * overflowing the kernel pool name limit (32).
 *
 * @param frame_name the name under which to configure a
 * new topocentric frame. This is arbitrary and should be
 * unique.
 * @param lon the longitude of the observer on the surface
 * of the Earth, -180 to 180 to represent 180W and 180E
 * @param lat the latitude of the observer on the surface
 * of the Earth, -90 to 90 to represent 90S and 90N
 *
 * @throws find_frame_id if the frame ID namespace has
 * been exhausted
 * @throws dup_name if the frame_name resolves to an
 * existing loaded frame (i.e. the provided name is a
 * duplicate)
 */
void load_topo_frame(SpiceChar *frame_name, SpiceDouble lon, SpiceDouble lat);

/**
 * Unloads a given topographic frame from the kernel
 * variable pool by deleting the constants loaded by the
 * {@code load_topo_frame()} function.
 *
 * Method behavior not defined if the given reference frame
 * is not configured or if it was not loaded using the
 * {@code load_topo_frame()} function.
 *
 * @param frame_name the name of the reference frame to
 * unload
 *
 * @throws no_name if the given reference frame cannot
 * be resolved (i.e. it has never been loaded)
 */
void unload_topo_frame(SpiceChar *frame_name);

#endif // GATE_TOPO_H
