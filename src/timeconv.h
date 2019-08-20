/**
 * Time conversion helpers for Unix systems.
 *
 * Only wrapper procedures are provided for the purpose
 * of quality-of-life improvements to working with Unix
 * time (e.g. clock_gettime(), clock()) and converting to
 * ephemeris time (ET).
 *
 * Whenever "Unix epoch" is referred, it means that an
 * offset from the Unix epoch (1 Jan 1970 00:00:00 UTC) in
 * milliseconds is desired. Whenever "ephemeris time" is
 * referred, it means that a Barycentric Dynamical Time
 * (TDB) offset in seconds is desired.
 */

#ifndef GATE_TIMECONV_H
#define GATE_TIMECONV_H

#include <cspice/SpiceUsr.h>
#include <time.h>

#define NS_PER_SEC 1000000000

/**
 * Obtains the current ephemeris time with nanosecond
 * accuracy.
 *
 * @param et the current ephemeris time (output)
 */
void gate_et_now(SpiceDouble *et);

/**
 * Converts a broken-down date in UTC to an ephemeris time
 * value.
 *
 * Do not convert from a Unix epoch to a broken-down date
 * in order to use this procedure, pass the epoch to
 * gate_unix_epoch_to_et() instead.
 *
 * This is equivalent to performing
 * gate_unix_utc_ns_to_et() with additional_ns set to 0.
 *
 * @param date the broken-down date in UTC (input)
 * @param et the equivalent ephemeris time (output)
 */
void gate_unix_utc_to_et(struct tm date, SpiceDouble *et);

/**
 * Converts a broken-down date in UTC plus an additional
 * nanoseconds component to an ephemeris time.
 *
 * Do not convert from a Unix epoch to a broken-down date
 * in order to use this procedure, pass the epoch to
 * gate_unix_epoch_to_et() instead.
 *
 * @param date the broken down date (input)
 * @param additional_ns additional nanoseconds to tack on
 * to the ephemeris time (input)
 * @param et the equivalent ephemeris time (output)
 */
void gate_unix_utc_ns_to_et(struct tm date, time_t additional_ns, SpiceDouble *et);

/**
 * Converts a Unix epoch value to an ephemeris time value.
 *
 * Do not convert from a broken-down struct tm date into an
 * epoch value in order to use this method. Use
 * gate_unix_utc_to_et() instead.
 *
 * @param unix_epoch the Unix epoch (input)
 * @param et the equivalent ephemeris time (output)
 */
void gate_unix_epoch_to_et(time_t unix_epoch, SpiceDouble *et);

/**
 * Converts a Unix clock struct containing the current Unix
 * epoch plus nanoseconds to an ephemeris time value.
 *
 * By default, the CLOCK_REALTIME spec should be used in
 * conjunction with clock_gettime() because it would not
 * make sense to use a non-wall-clock output to describe
 * a point in time.
 *
 * @param clock the CLOCK_REALTIME output from
 * clock_gettime() (input)
 * @param et the equivalent ephemeris time (output)
 */
void gate_unix_clock_to_et(struct timespec clock, SpiceDouble *et);

/**
 * Individually converts the components of a Unix epoch
 * plus additional nanoseconds into an ephemeris time
 * value.
 *
 * @param unix_epoch the Unix epoch time (input)
 * @param additional_ns the additional number of
 * nanoseconds to tack onto the ephemeris time (input)
 * @param et the equivalent ephemeris time (output)
 */
void gate_unix_ns_to_et(time_t unix_epoch, time_t additional_ns, SpiceDouble *et);

/**
 * Converts the given ephemeris time to a Unix epoch time
 * plus the extra nanoseconds.
 *
 * @param et the ephemeris time which to convert (input)
 * @param unix_epoch the Unix epoch time equivalent to the
 * ephemeris time, or NULL if not desired (output)
 * @param additional_ns the additional nanoseconds that
 * cannot be carried by the Unix epoch, or NULL if not
 * desired (output)
 */
void gate_et_to_unix_ns(SpiceDouble et, time_t *unix_epoch, time_t *additional_ns);

/**
 * Converts the given ephemeris time to a broken-down UTC
 * date.
 *
 * @param et the ephemeris time which to convert (input)
 * @param unix_date the equivalent broken-down UTC date, or
 * NULL if not desired (output)
 * @param additional_ns the extra nanoseconds that cannot
 * be carried by the broken-down date struct, or NULL if
 * not desired (output)
 */
void gate_et_to_unix_utc_ns(SpiceDouble et, struct tm *unix_date, time_t *additional_ns);

#endif // GATE_TIMECONV_H
