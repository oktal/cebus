#define CB_NANOSECONDS_PER_TICK 100UL

#define CB_TICKS_INVALID (uint64_t) -1

/// Number of seconds per minute
#define CB_SECONDS_PER_MINUTE 60

/// Number of minutes per hour
#define CB_MINUTES_PER_HOUR 60

/// Number of seconds per hour
#define CB_SECONDS_PER_HOUR (CB_SECONDS_PER_MINUTE * CB_MINUTES_PER_HOUR)

/// Number of hours per day
#define CB_HOURS_PER_DAY 24UL

/// Number of days per non-leap year
#define CB_DAYS_PER_YEAR 365UL

/// Number of days in 4 years (+1 for leap year)
#define CB_DAYS_PER_4_YEARS (CB_DAYS_PER_YEAR * 4 + 1)

/// Number of days in 100 years
#define CB_DAYS_PER_100_YEARS (CB_DAYS_PER_4_YEARS * 25 - 1)

/// Number of days in 400 years
#define CB_DAYS_PER_400_YEARS (CB_DAYS_PER_100_YEARS * 4 + 1)

/// Number of milliseconds per second
#define CB_MILLI_SECONDS_PER_SECOND 1000UL

/// Number of 100-nanoseconds ticks per millisecond
#define CB_TICKS_PER_MILLISECOND 10000UL

/// Number of 100-nanoseconds ticks per second
#define CB_TICKS_PER_SECOND (CB_TICKS_PER_MILLISECOND * 1000)

/// Number of 100-nanoseconds ticks per minute
#define CB_TICKS_PER_MINUTE (CB_TICKS_PER_SECOND * CB_SECONDS_PER_MINUTE)
//
/// Number of 100-nanoseconds ticks per hour
#define CB_TICKS_PER_HOUR (CB_TICKS_PER_MINUTE * CB_MINUTES_PER_HOUR)
//
/// Number of 100-nanoseconds ticks per day
#define CB_TICKS_PER_DAY (CB_TICKS_PER_HOUR * CB_HOURS_PER_DAY)

/// Minimum supported year
#define CB_DATE_MIN_YEAR 1

/// Maximum supported year
#define CB_DATE_MAX_YEAR 9999

/// Number of days from 1/1/0001 to unix-epoch (1/1/1970)
#define CB_DAYS_TO_1970 (CB_DAYS_PER_400_YEARS * 4 + CB_DAYS_PER_100_YEARS * 3 + CB_DAYS_PER_4_YEARS * 17 + CB_DAYS_PER_YEAR)

/// Number of ticks for unix-epoch
#define CB_EPOCH_TICKS (CB_DAYS_TO_1970 * CB_TICKS_PER_DAY)
