#include "timeconv.h"
#include <stdio.h>

#define NS_PER_SEC 1000000000
#define TIMECONV_BUFFER_LEN 100

void gate_unix_utc_to_et(struct tm date, SpiceDouble *et) {
    gate_unix_utc_ns_to_et(date, 0, et);
}

void gate_unix_utc_ns_to_et(struct tm date, time_t additional_ns, SpiceDouble *et) {
    time_t epoch = mktime(&date);
    gate_unix_ns_to_et(epoch, additional_ns, et);
}

void gate_unix_epoch_to_et(time_t unix_epoch, SpiceDouble *et) {
    gate_unix_ns_to_et(unix_epoch, 0, et);
}

void gate_unix_clock_to_et(struct timespec clock, SpiceDouble *et) {
    gate_unix_ns_to_et(clock.tv_sec, clock.tv_nsec, et);
}

void gate_unix_ns_to_et(time_t unix_epoch, time_t additional_ns, SpiceDouble *et) {
    struct tm *tm_utc = gmtime(&unix_epoch);

    SpiceDouble additional_sec = (SpiceDouble) additional_ns / NS_PER_SEC;

    SpiceInt yr = tm_utc->tm_year + 1900;
    SpiceInt mon = tm_utc->tm_mon + 1;
    SpiceInt day = tm_utc->tm_mday;
    SpiceInt hr = tm_utc->tm_hour;
    SpiceInt min = tm_utc->tm_min;
    SpiceDouble sec = tm_utc->tm_sec + additional_sec;

    SpiceChar buffer[TIMECONV_BUFFER_LEN];
    snprintf(buffer, TIMECONV_BUFFER_LEN, "%d-%d-%dT%d:%d:%.20f",
             yr, mon, day, hr, min, sec);

    str2et_c(buffer, et);
}

void gate_et_to_unix_ns(SpiceDouble et, time_t *unix_epoch, time_t *additional_ns) {
    struct tm unix_date;
    gate_et_to_unix_utc_ns(et, &unix_date, additional_ns);

    if (unix_epoch != NULL) {
        *unix_epoch = mktime(&unix_date);
    }
}

void gate_et_to_unix_utc_ns(SpiceDouble et, struct tm *unix_date, time_t *additional_ns) {
    SpiceChar buffer[TIMECONV_BUFFER_LEN];
    timout_c(et, "YYYY-MM-DDTHR:MN:SC ::UTC", TIMECONV_BUFFER_LEN, buffer);

    struct tm output;
    SpiceDouble sec;
    sscanf(buffer, "%d-%d-%dT%d:%d:%lf",
           &output.tm_year, &output.tm_mon, &output.tm_mday, &output.tm_hour, &output.tm_min, &sec);

    output.tm_sec = sec;

    if (unix_date != NULL) {
        *unix_date = output;
    }

    if (additional_ns != NULL) {
        *additional_ns = sec - output.tm_sec;
    }
}
