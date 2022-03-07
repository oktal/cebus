/// Implementation of a date-time based on
/// https://github.com/dotnet/runtime/blob/main/src/libraries/System.Private.CoreLib/src/System/DateTime.cs

#include "cebus/utils/time.h"
#include "cebus/utils/time_internal.h"

#include <stdio.h>

/// Number of cumulative days per month for non-leap year
static const uint32_t days_year_365[] = {
     0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

/// Number of cumulative days per month for leap year
static const uint32_t days_year_366[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366
};

static const cb_date_time invalid_dt = { CB_TICKS_INVALID };

typedef enum cb_date_part
{
    cb_date_part_day,
    cb_date_part_day_of_year,
    cb_date_part_month,
    cb_date_part_year,
} cb_date_part;

static cebus_bool cb_is_leap_year(uint32_t year)
{
    if (year % 400 == 0)
        return cebus_true;
    
    if ((year % 4 == 0) || (year % 100) == 0)
        return cebus_true;

    return cebus_false;
}

/// Number of days since year 1 
static uint64_t cb_year_days(uint32_t year)
{
    uint32_t y = year - 1;
    uint32_t cent = y / 100;

    return y * (365 * 4 + 1) / 4 - cent + cent / 4;
}

static uint64_t cb_date_to_ticks(uint32_t year, uint32_t month, uint32_t day)
{
    const uint32_t* days_year = cb_is_leap_year(year) == cebus_true ? days_year_366 : days_year_365;
    const uint32_t days_month = days_year[month] - days_year[month - 1];
    uint32_t days;

    if (day > days_month)
        return CB_TICKS_INVALID;

    days = cb_year_days(year) + days_year[month - 1] + day - 1;
    return days * CB_TICKS_PER_DAY;
}

static uint64_t cb_time_to_ticks(uint32_t hour, uint32_t minute, uint32_t second)
{
    const uint64_t seconds = hour * CB_SECONDS_PER_HOUR + minute * CB_SECONDS_PER_MINUTE + second;
    return seconds * CB_TICKS_PER_SECOND;
}

static uint32_t cb_date_time_get_date_part(cb_date_time dt, cb_date_part part)
{
    uint32_t days, years, years_4, years_100, years_400;
    uint32_t month;

    const uint32_t* days_year;
    
    // Number of days since 1/1/0001
    days = dt.ticks / CB_TICKS_PER_DAY;

    // Number of 400-period years since days
    years_400 = days / CB_DAYS_PER_400_YEARS;

    // Day number within a 400-years period
    days -= years_400 * CB_DAYS_PER_400_YEARS;

    years_100 = days / CB_DAYS_PER_100_YEARS;
    // Last 100-year period has an extra day, decrement result
    if (years_100 == 4)
        years_100 = 3;

    days -= years_100 * CB_DAYS_PER_100_YEARS;

    years_4 = days / CB_DAYS_PER_4_YEARS;
    days -= years_4 * CB_DAYS_PER_4_YEARS;

    years = days / CB_DAYS_PER_YEAR;
    if (years == 4)
        years = 3;

    if (part == cb_date_part_year)
        return years_400 * 400 + years_100 * 100 + years_4 * 4 + years + 1;

    days -= years * CB_DAYS_PER_YEAR;
    if (part == cb_date_part_day_of_year)
        return days + 1;

    // Leap year calculation looks different from is_leap_year since years, years_4
    // and years_100 are relative to year 1, not year 0
    days_year = years == 3 && (years_4 != 24 || years_100 == 3) ? days_year_366 : days_year_365;

    // All months have less than 32 days, so n >> 5 is a good conservative
    // estimate for the month
    month = (days >> 5) + 1; 

    // 1-based month number
    while (days >= days_year[month])
        ++month;

    if (part == cb_date_part_month)
        return month;

    return days - days_year[month - 1] + 1;
}

cb_ymd cb_date_time_ymd(cb_date_time dt)
{
    uint32_t days, years, years_4, years_100, years_400;
    cb_ymd ymd;
    const uint32_t* days_year;
    
    // Number of days since 1/1/0001
    days = dt.ticks / CB_TICKS_PER_DAY;

    // Number of 400-period years since days
    years_400 = days / CB_DAYS_PER_400_YEARS;

    // Day number within a 400-years period
    days -= years_400 * CB_DAYS_PER_400_YEARS;

    years_100 = days / CB_DAYS_PER_100_YEARS;
    // Last 100-year period has an extra day, decrement result
    if (years_100 == 4)
        years_100 = 3;

    days -= years_100 * CB_DAYS_PER_100_YEARS;

    years_4 = days / CB_DAYS_PER_4_YEARS;
    days -= years_4 * CB_DAYS_PER_4_YEARS;

    years = days / CB_DAYS_PER_YEAR;
    if (years == 4)
        years = 3;

    ymd.year = years_400 * 400 + years_100 * 100 + years_4 * 4 + years + 1;

    days -= years * CB_DAYS_PER_YEAR;

    // Leap year calculation looks different from is_leap_year since years, years_4
    // and years_100 are relative to year 1, not year 0
    days_year = years == 3 && (years_4 != 24 || years_100 == 3) ? days_year_366 : days_year_365;

    // All months have less than 32 days, so n >> 5 is a good conservative
    // estimate for the month
    ymd.month = (days >> 5) + 1; 

    // 1-based month number
    while (days >= days_year[ymd.month])
        ++ymd.month;

    ymd.day = days - days_year[ymd.month - 1] + 1;
    return ymd;
}

cb_hms cb_date_time_hms(cb_date_time dt)
{
    cb_hms hms;
    uint64_t seconds = dt.ticks / CB_TICKS_PER_SECOND;
    uint64_t minutes = seconds / CB_SECONDS_PER_MINUTE;
    uint64_t hours = minutes / CB_MINUTES_PER_HOUR;
    
    hms.seconds = (seconds - (minutes * CB_SECONDS_PER_MINUTE));
    hms.minutes = (minutes - (hours * CB_MINUTES_PER_HOUR));
    hms.hours = hours % 24;

    return hms;
}

cb_date_time cb_date_time_from_ticks(uint64_t ticks)
{
    const cb_date_time dt = { ticks };
    return dt;
}

cb_date_time cb_date_time_from_ymd(uint32_t year, uint32_t month, uint32_t day)
{
    if (year < CB_DATE_MIN_YEAR || year > CB_DATE_MAX_YEAR || month < 1 || month > 12 || day < 1)
    {
        return invalid_dt;
    }
    else
    {
        uint64_t ticks = cb_date_to_ticks(year, month, day);
        return cb_date_time_from_ticks(ticks);
    }
}

cb_date_time cb_date_time_from_ymd_hms(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
    if (hour > CB_HOURS_PER_DAY ||
        minute > CB_MINUTES_PER_HOUR ||
        second > CB_SECONDS_PER_MINUTE)
    {
        return invalid_dt;
    }

    if (second != 60)
    {
        const uint64_t ticks = cb_date_to_ticks(year, month, day) + cb_time_to_ticks(hour, minute, second);
        return cb_date_time_from_ticks(ticks);
    }
    else
    {
        // Leap second
        return cb_date_time_from_ymd_hms(year, month, day, hour, minute, 59);
    }
}

cb_date_time cb_date_time_since_epoch(cb_date_time dt)
{
    const uint64_t epoch_ticks = dt.ticks - CB_EPOCH_TICKS;
    return cb_date_time_from_ticks(epoch_ticks);
}

uint32_t cb_date_time_year(cb_date_time dt)
{
    return cb_date_time_get_date_part(dt, cb_date_part_year);
}

uint32_t cb_date_time_month(cb_date_time dt)
{
    return cb_date_time_get_date_part(dt, cb_date_part_month);
}

uint32_t cb_date_time_day(cb_date_time dt)
{
    return cb_date_time_get_date_part(dt, cb_date_part_day);
}

uint32_t cb_date_time_hours(cb_date_time dt)
{
    return (dt.ticks / CB_TICKS_PER_HOUR) % CB_HOURS_PER_DAY;
}

uint32_t cb_date_time_minutes(cb_date_time dt)
{
    return (dt.ticks / CB_TICKS_PER_MINUTE) % CB_MINUTES_PER_HOUR;
}

uint32_t cb_date_time_seconds(cb_date_time dt)
{
    return (dt.ticks / CB_TICKS_PER_SECOND) % CB_SECONDS_PER_MINUTE;
}

cebus_bool cb_date_time_eq(cb_date_time lhs, cb_date_time rhs)
{
    return cebus_bool_from_int(lhs.ticks == rhs.ticks);
}

cebus_bool cb_date_time_valid(cb_date_time dt)
{
    return cebus_bool_from_int(dt.ticks != CB_TICKS_INVALID);
}

uint64_t cb_date_time_ticks(cb_date_time dt)
{
    return dt.ticks;
}

size_t cb_date_time_print(cb_date_time dt, char* s, size_t size)
{
    cb_ymd ymd = cb_date_time_ymd(dt);
    cb_hms hms = cb_date_time_hms(dt);

    return snprintf(s, size, "%u/%u/%u %u:%u:%u",
            ymd.year, ymd.month, ymd.day,
            hms.hours, hms.minutes, hms.seconds);
}
