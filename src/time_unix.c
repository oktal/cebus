#include "cebus/utils/time.h"
#include "cebus/utils/time_internal.h"

#include "cebus/utils/unimplemented.h"

#include <time.h>

/// Number of days from 1/1/0001 to unix-epoch (1/1/1970)
#define CB_DAYS_TO_1970 (CB_DAYS_PER_400_YEARS * 4 + CB_DAYS_PER_100_YEARS * 3 + CB_DAYS_PER_4_YEARS * 17 + CB_DAYS_PER_YEAR)

/// Number of ticks for unix-epoch
#define CB_EPOCH_TICKS (CB_DAYS_TO_1970 * CB_TICKS_PER_DAY)

// XXX: Unimplemented
time_instant time_instant_now()
{
    cebus_unimplemented();
}

timespan time_instant_elapsed(const time_instant* instant)
{
    cebus_unimplemented();
}

static uint64_t cb_date_time_utc_now_ticks()
{
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) < 0)
        return CB_TICKS_INVALID;

    return now.tv_sec * CB_TICKS_PER_SECOND + (now.tv_nsec / CB_NANOSECONDS_PER_TICK);
}

cb_date_time cb_date_time_utc_now()
{
    const uint64_t ticks = cb_date_time_utc_now_ticks();
    if (ticks == CB_TICKS_INVALID)
    {
        const cb_date_time invalid_dt = { ticks };
        return invalid_dt;
    }

    return cb_date_time_from_ticks(ticks + CB_EPOCH_TICKS);
}
