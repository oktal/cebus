#include "cebus/utils/time.h"
#include "cebus/utils/time_internal.h"

#include "cebus/utils/unimplemented.h"

#include <time.h>

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
