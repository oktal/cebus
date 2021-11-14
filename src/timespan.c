#include "cebus/utils/timespan.h"

#define SECS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define NANOS_PER_SEC 1000000000
#define NANOS_PER_MILLI 1000000
#define NANOS_PER_MICRO 1000
#define MILLIS_PER_SEC 1000
#define MICROS_PER_SEC 1000000

timespan timespan_from_secs(uint64_t secs)
{
    timespan ts;
    ts.secs = secs;
    ts.nsecs = 0;

    return ts;
}

timespan timespan_from_millis(uint64_t millis)
{
    timespan ts;
    ts.secs = millis / MILLIS_PER_SEC;
    ts.nsecs = (uint32_t)(millis % MILLIS_PER_SEC) * NANOS_PER_MILLI;

    return ts;
}

timespan timespan_from_micros(uint64_t usecs)
{
    timespan ts;
    ts.secs = usecs / MICROS_PER_SEC;
    ts.nsecs = (uint32_t)(usecs % MICROS_PER_SEC) * NANOS_PER_MICRO;

    return ts;
}

timespan timespan_from_nanos(uint64_t nsecs)
{
    timespan ts;
    ts.secs = nsecs / NANOS_PER_SEC;
    ts.nsecs = (uint32_t)(nsecs % NANOS_PER_SEC);

    return ts;
}

timespan timespan_from_minutes(uint32_t minutes)
{
    return timespan_from_secs(minutes * SECS_PER_MINUTE);
}

timespan timespan_from_hours(uint32_t hours)
{
    return timespan_from_minutes(hours * MINUTES_PER_HOUR);
}

uint64_t timespan_as_secs(timespan ts)
{
    return ts.secs;
}

uint64_t timespan_as_millis(timespan ts)
{
    return (ts.secs * MILLIS_PER_SEC) + (ts.nsecs / NANOS_PER_MILLI);
}

uint64_t timespan_as_micros(timespan ts)
{
    return (ts.secs * MICROS_PER_SEC) + (ts.nsecs / NANOS_PER_MICRO);
}

uint64_t timespan_as_nanos(timespan ts)
{
    return (ts.secs * NANOS_PER_SEC) + ts.nsecs;
}

void timespan_zero(timespan* ts)
{
    ts->secs = 0;
    ts->nsecs = 0;
}

cebus_bool timespan_is_zero(timespan ts)
{
    if (ts.secs == 0 && ts.nsecs == 0)
        return cebus_true;

    return cebus_false;
}
