/**
 * Support for topocentric frames.
 *
 * Requires earth_fixed.tf and a kernel specifying either
 * IAU_EARTH (usually a generic PCK) or ITRF93 if specified
 * by the EARTH_FIXED reference frame (usually a generic
 * high-accuracy kernel).
 *
 * The SPICE "built-in" reference frames do not bundle
 * out-of-the-box support for topocentric frames, which
 * are reference frames that are centered around a point
 * on the surface of a planet. While "body-fixed" frames,
 * which are fixed on the center of a planet but are non-
 * inertial (meaning that they rotate with the body), such
 * as {@code IAU_EARTH} and {@code ITRF93} exist, they are
 * difficult to use when the observation plane is centered
 * around an observer standing on the surface of that body.
 *
 * A topocentric frame is defined by an offset from a
 * standard body-fixed frame in order to describe the
 * position of the observer with respect to the rotation
 * of that body. This helper is able to describe the
 * topocentric frame by loading a few kernel pool constants
 * into the pool. Later procedures can then use the
 * specified reference frame in order to manipulate
 * vectors. A common use for topographic frames is to
 * determine the azimuth and elevation of a particular
 * object in the sky. Because right ascension refers to the
 * position of a body with respect to a reference frame
 * rather than with respect to an observer's hemisphere, it
 * is not able to an observer where to look or to point a
 * camera in order to observe that particular body.
 *
 * Two procedures are offered, one to load a new reference
 * frame for the given geodetic (i.e. the standard
 * system of locating an object) coordinates on Earth, and
 * one procedure to unload the kernel pool variables that
 * were loaded by the previous procedure.
 *
 * Only Earth-based topocentric frames are currently
 * supported.
 */

#ifndef GATE_TOPO_H
#define GATE_TOPO_H

#include <cspice/SpiceUsr.h>

#define TOPO_MAX_FRAME_IDS 100
#define TOPO_EARTH_FIXED_FRAME "EARTH_FIXED"

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
 * Frame names are limited to 26 characters to avoid
 * overflowing the kernel pool name limit (32).
 *
 * @param frame_name the name under which to configure a
 * new topocentric frame. This is arbitrary and should be
 * unique.
 * @param lat the latitude of the observer on the surface
 * of the Earth, -90 to 90 to represent 90S and 90N
 * @param lon the longitude of the observer on the surface
 * of the Earth, -180 to 180 to represent 180W and 180E
 *
 * @throws find_frame_id if the frame ID namespace has
 * been exhausted
 * @throws dup_name if the frame_name resolves to an
 * existing loaded frame (i.e. the provided name is a
 * duplicate)
 */
void gate_load_topo_frame(ConstSpiceChar *frame_name, SpiceDouble lat, SpiceDouble lon);

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
void gate_unload_topo_frame(ConstSpiceChar *frame_name);

#endif // GATE_TOPO_H
