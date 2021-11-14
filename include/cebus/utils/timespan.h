#pragma once

#include "cebus/cebus_bool.h"

#include <stdint.h>

typedef struct timespan
{
    uint64_t secs;
    uint32_t nsecs;
} timespan;

timespan timespan_from_secs(uint64_t secs);
timespan timespan_from_millis(uint64_t msecs);
timespan timespan_from_micros(uint64_t usecs);
timespan timespan_from_nanos(uint64_t nsecs);

timespan timespan_from_minutes(uint32_t minutes);
timespan timespan_from_hours(uint32_t hours);

uint64_t timespan_as_secs(timespan ts);
uint64_t timespan_as_millis(timespan ts);
uint64_t timespan_as_micros(timespan ts);
uint64_t timespan_as_nanos(timespan ts);

void timespan_zero(timespan* ts);
cebus_bool timespan_is_zero(timespan ts);
