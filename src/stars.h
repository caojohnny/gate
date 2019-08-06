/**
 * Support for manipulating star data.
 *
 * NAIF undertook efforts to support stars in the SPICE
 * Toolkit long ago [1], however, these efforts stopped
 * and it appears that the only elements of those efforts
 * that remain in the modern SPICE Toolkit appears to be
 * perhaps aberration corrections and the leftover binary
 * star catalogs.
 *
 * This is not an attempt to integrate full functionality
 * because it would be difficult for me to do so as a
 * non-expert in this field. That being said, I can provide
 * a few utilities that can make working with stars that
 * little bit more bearable until NAIF decides to possibly
 * add better support into SPICE Toolkit itself.
 *
 * Because stars do not have NAIF IDs, it is not possible
 * to use pre-existing procedures such as spkcpo_c() to
 * determine state vectors.
 *
 * [1]: https://naif.jpl.nasa.gov/pub/naif/generic_kernels/aareadme.txt
 */

#ifndef GATE_STARS_H
#define GATE_STARS_H

#include <cspice/SpiceUsr.h>

#define STARS_QUERY_MAX_LEN 500

/**
 * Represents star data laid out in a SPICE TYPE 1 star
 * catalog.
 *
 * Currently, the only catalogs available are HIPPARCOS,
 * PPM, and TYCHO2.
 *
 * Information about each particular catalog and what each
 * column means and their units can be found from using the
 * COMMT utility and from searching for the parameters for
 * each catalog online.
 *
 * https://naif.jpl.nasa.gov/pub/naif/generic_kernels/stars/
 */
typedef struct {
    SpiceInt catalog_number;
    SpiceDouble dec;
    SpiceDouble dec_epoch;
    SpiceDouble dec_pm;
    SpiceDouble dec_pm_sigma;
    SpiceDouble dec_sigma;
    SpiceInt dm_number;
    SpiceDouble parallax;
    SpiceDouble ra;
    SpiceDouble ra_epoch;
    SpiceDouble ra_pm;
    SpiceDouble ra_pm_sigma;
    SpiceDouble ra_sigma;
    SpiceChar spectral_type[5];
    SpiceDouble visual_magnitude;
} gate_star_info_spice1;

/**
 * Loads stars from a stars table from a loaded EK to be
 * later parsed by gate_parse_stars().
 *
 * Requires a stars generic Events Kernel (EK) providing
 * the given table to be loaded.
 *
 * @param table the name of the table which to load the
 * star data into memory (input)
 * @param filter a filter string, which starts with either
 * {@code WHERE} and/or {@code ORDER} if a filter is
 * desired, otherwise {@code NULL} (input)
 * @param rows the number of rows that match the query
 * (output)
 *
 * @throws query if the query string was somehow mangled
 * or an error occurred executing that query
 */
void gate_load_stars(ConstSpiceChar *table, ConstSpiceChar *filter, SpiceInt *rows);

/**
 * Parses stars loaded by the gate_load_stars() procedure
 * into gate_star_info_spice1 data structs.
 *
 * Requires a stars generic Events Kernel (EK) providing
 * the given table to be loaded.
 *
 * Behavior undefined if gate_load_stars() was not called
 * prior to running this procedure.
 *
 * Behavior undefined if the given max_size exceeds the
 * number of rows loaded by gate_load_stars().
 *
 * @param max_size the maximum number of stars to parse
 * (not the actual number of stars to load) (input)
 * @param array the array of parsed star information to
 * parse into (output)
 */
void gate_parse_stars(SpiceInt max_size, gate_star_info_spice1 *array);

/**
 * Calculates the new position of the star at the given
 * epoch time, {@code et}, which accounts for the proper
 * motion of stars over the length of time from which the
 * position was first determined.
 *
 * @param info the information representing the star which
 * to determine the new star position (input)
 * @param et the epoch time after J2000 at which to
 * determine the new star position (input)
 * @param ra set by this procedure to represent the right
 * ascension of the star at the epoch time in the J2000
 * frame, or NULL if not desired (output)
 * @param dec set by this procedure to represent the
 * declination of the star at the epoch time in the J2000
 * frame, or NULL if not desired (output)
 * @param ra_u set by this procedure to represent the
 * uncertainty in the output right ascension, or NULL if
 * not desired (output)
 * @param dec_u set by this procedure to represent the
 * uncertainty in the output declination, or NULL if not
 * desired (output)
 */
void gate_calc_star_pos(gate_star_info_spice1 info, SpiceDouble et,
        SpiceDouble *ra, SpiceDouble *dec, SpiceDouble *ra_u, SpiceDouble *dec_u);



#endif // GATE_STARS_H
