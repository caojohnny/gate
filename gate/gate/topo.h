/**
 * @file
 * Support for topocentric/topographic frames.
 *
 * The SPICE "built-in" reference frames do not bundle
 * out-of-the-box support for topocentric frames, which
 * are reference frames that are centered around a point
 * on the surface of a planet. While "body-fixed" frames,
 * which are fixed on the center of a planet but are non-
 * inertial (meaning that they rotate with the body, and
 * the orientation is therefore fixed to the crust), such
 * as `IAU_EARTH` and `ITRF93` exist, they are difficult to
 * use when the observation plane is centered around an
 * observer located on the surface of that body.
 *
 * A topographic frame is defined by an offset from a
 * standard body-fixed frame in order to describe the
 * position of the observer with respect to the rotation
 * of that body. This helper is able to describe the
 * topographic frame by loading a few kernel pool constants
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
 * A topocentric frame has the subtle difference of being
 * centered on a point on the surface of a body rather than
 * the center of the body itself. This difference doesn't
 * matter much for objects that are extremely far away, but
 * near-body objects such as satellites suffer from
 * relatively significant error. Adjustment procedures are
 * provided to modify coordinates in a topographic frame to
 * account for the difference with a topocentric frame.
 *
 * Two procedures are offered, one to load a new reference
 * frame for the given geodetic (i.e. the standard
 * system of locating an object) coordinates on Earth, and
 * one procedure to unload the kernel pool variables that
 * were loaded by the previous procedure.
 *
 * While generally the orientation of a topographic frame
 * with respect to the observer isn't that important, for
 * reference, the topographic frame produced by the
 * procedures in this file follow that outlined by the
 * SPICE Toolkit frames required reading
 * (https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/frames.html#Defining%20a%20TK%20Frame%20Using%20Euler%20Angles):
 * X points to true north, Y points west and Z points
 * outward normal to the observation plane.
 */

#ifndef GATE_TOPO_H
#define GATE_TOPO_H

#include <cspice/SpiceUsr.h>

#define TOPO_MAX_FRAME_IDS 100

/**
 * Represents a topocentric frame.
 *
 * This contains information about a loaded topographic
 * frame, but also has an extra `radius` field to define
 * the entire position of the observer relative to the
 * center of the provided body. While this is not
 * incorporated into the loaded topographic frame, it can
 * be used to make adjustments to a rectangular topographic
 * coordinate  vector in order to transform it into a
 * rectangular topocentric coordinate vector through the
 * use of procedures such as gate_adjust_topo_rec().
 */
typedef struct {
    ConstSpiceChar *frame_name;
    SpiceInt body_id;
    SpiceDouble latitude;
    SpiceDouble longitude;

    /**
     * Radius of the observer away from from the center of
     * the body in kilometers.
     */
    SpiceDouble radius;
} gate_topo_frame;

/**
 * Loads a new topographic reference frame for an observer
 * on a given body at the given coordinates and distance
 * away from the body center.
 *
 * Requires a kernel specifying a body-fixed frame for the
 * given body, usually a generic PCK.
 *
 * Because the offset only performs angular
 * transformations, the radius of the body is not
 * accounted for by the loaded reference frame. The
 * provided radius parameter is not used to load the
 * reference frame. However, the output `topo_frame can be
 * used in conjunction with provided procedures such as
 * gate_adjust_topo_rec() in order to take into account
 * the difference between the center positions of
 * topographic and topocentric frames.
 *
 * Frame names are limited to 26 characters to avoid
 * overflowing the kernel pool name limit (32).
 *
 * @param frame_name the name under which to configure a
 * new topographic frame. This is arbitrary and should be
 * unique (input)
 * @param body_id the NAIF ID of the body on which to
 * create a topographic frame (input)
 * @param latitude the geodetic latitude of the observer on
 * the surface of the body, -90 to 90 to represent 90S and
 * 90N (input)
 * @param longitude the geodetic longitude of the observer
 * on the surface of the body, -180 to 180 to represent
 * 180W and 180E (input)
 * @param radius the distance of the observer from the
 * center of the body. This is not incorporated into the
 * frame, but it will be available in the output
 * gate_topo_frame (input)
 * @param topo_frame a holder struct containing the
 * information for the loaded topographic frame and
 * additional radius data to define the topocentric frame
 * (output)
 *
 * @throws dup_name if the frame_name resolves to an
 * existing loaded frame (i.e. the provided name is a
 * duplicate)
 * @throws find_frame_id if the frame ID namespace has
 * been exhausted
 * @throws resolve_rel_frame if the body-fixed frame for
 * the specified body ID could not be resolved (i.e. none
 * have been assigned to that body)
 */
void gate_load_topo_frame(ConstSpiceChar *frame_name, SpiceInt body_id,
                          SpiceDouble latitude, SpiceDouble longitude, SpiceDouble radius,
                          gate_topo_frame *topo_frame);

/**
 * Loads a new topographic reference frame for an observer
 * on Earth by inserting a set of variables for a TK that
 * defines the offset from an EARTH_FIXED frame into the
 * kernel variable pool.
 *
 * Requires a kernel specifying an Earth-fixed frame, such
 * as a generic solar system PCK or a generic high-accuracy
 * kernel such as ITRF93.
 *
 * Because the offset only performs angular
 * transformations, the radius of the body is not
 * accounted for by the loaded reference frame. The
 * provided radius parameter is not used to load the
 * reference frame. However, the output `topo_frame can be
 * used in conjunction with provided procedures such as
 * gate_adjust_topo_rec() in order to take into account the
 * difference between the center positions of topographic
 * and topocentric frames.
 *
 * Frame names are limited to 26 characters to avoid
 * overflowing the kernel pool name limit (32).
 *
 * @param frame_name the name under which to configure a
 * new topographic frame. This is arbitrary and should be
 * unique.
 * @param latitude the geodetic latitude of the observer on
 * the surface of the Earth, -90 to 90 to represent 90S and
 * 90N (input)
 * @param longitude the geodetic longitude of the observer
 * on the surface of the Earth, -180 to 180 to represent
 * 180W and 180E (input)
 * @param height the height of the observer off of the
 * Earth's geodetic spheroid in kilometers at that
 * location, or 0 to use the geodetic radius, or NAN to use
 * the center of the Earth (whereby radius would therefore
 * be 0) (input)
 * @param topo_frame a holder struct containing the
 * information for the loaded topographic frame and
 * additional radius data to define the topocentric frame
 * (output)
 *
 * @throws dup_name if the frame_name resolves to an
 * existing loaded frame (i.e. the provided name is a
 * duplicate)
 * @throws find_frame_id if the frame ID namespace has
 * been exhausted
 * @throws resolve_rel_frame if the body-fixed frame for
 * Earth could not be resolved (e.g. because no loaded
 * kernel specifies one)
 */
void gate_load_earth_topo_frame(ConstSpiceChar *frame_name,
                                SpiceDouble latitude, SpiceDouble longitude, SpiceDouble height,
                                gate_topo_frame *topo_frame);

/**
 * This can be used to make modifications to the given
 * array of rectangular coordinates in the given topographic
 * frame to adjust for the position of the observer in a
 * topocentric frame.
 *
 * @param topo_frame the topocentric frame specification
 * which to use in order to adjust the given coordinate
 * array (input)
 * @param rec a set of rectangular topographic coordinates
 * in kilometers that will be modified in order to adjust
 * for the position of a topocentric frame on the surface
 * of the body instead of the center (input/output)
 */
void gate_adjust_topo_rec(gate_topo_frame topo_frame, SpiceDouble *rec);

/**
 * This can be used to make modifications to the given
 * array of rectangular coordinates in the given topographic
 * frame to adjust for the position of the observer in a
 * topocentric frame.
 *
 * @param topo_frame the topocentric frame specification
 * which to use in order to adjust the given coordinate
 * array (input)
 * @param units the of the values in the given rectangular
 * coordinate vector. Values recognized by convrt_c() are
 * accepted. If the units are in kilometers, prefer the
 * gate_adjust_topo_rec() procedure instead (input)
 * @param rec a set of rectangular topographic coordinates
 * in the given units that will be modified in order to
 * adjust for the position of a topocentric frame on the
 * surface of the body instead of the center (input/output)
 */
void gate_conv_adjust_topo_rec(gate_topo_frame topo_frame, ConstSpiceChar *units, SpiceDouble *rec);

/**
 * Helper procedure to convert topographic or topocentric
 * rectangular coordinates into range, azimuth, and
 * elevation.
 *
 * @param rec the topographic rectangular coordinates to
 * convert (input)
 * @param rg the range, or distance from the origin of the
 * topographic reference frame to the target object, in
 * the units of the rectangular coordinates passed in
 * (output)
 * @param az the azimuth of the target body in degrees
 * clockwise true north, or NULL if not desired (output)
 * @param el the elevation of the target body in degrees
 * above the observation plane, or NULL if not desired
 * (output)
 */
void gate_conv_rec_azel(SpiceDouble *rec, SpiceDouble *rg, SpiceDouble *az, SpiceDouble *el);

/**
 * Unloads a given topographic frame from the kernel
 * variable pool by deleting the constants loaded by the
 * `load_topo_frame()` function.
 *
 * Method behavior not defined if the given reference frame
 * is not configured or if it was not loaded using the
 * `load_topo_frame()` function.
 *
 * @param topo_frame the topographic frame
 *
 * @throws no_name if the given reference frame cannot
 * be resolved (i.e. it has never been loaded)
 */
void gate_unload_topo_frame(gate_topo_frame topo_frame);

#endif // GATE_TOPO_H
