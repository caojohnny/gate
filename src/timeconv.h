/**
 * Time conversion helpers for Unix systems.
 */

#ifndef GATE_TIMECONV_H
#define GATE_TIMECONV_H

#include <cspice/SpiceUsr.h>
#include <time.h>

void gate_unix_date_to_et(struct tm date, SpiceDouble *et);

void gate_unix_date_ns_to_et(struct tm date, time_t additional_ns, SpiceDouble *et);

void gate_unix_epoch_to_et(time_t unix_epoch, SpiceDouble *et);

void gate_unix_clock_to_et(struct timespec clock, SpiceDouble *et);

void gate_unix_ns_to_et(time_t unix_epoch, time_t additional_ns, SpiceDouble *et);

void gate_et_to_unix_ns(SpiceDouble et, time_t *unix_epoch, time_t *additional_ns);

void gate_et_to_unix_date_ns(SpiceDouble et, struct tm *unix_date, time_t *additional_ns);

#endif // GATE_TIMECONV_H
