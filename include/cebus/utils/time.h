#pragma once

#include "cebus/utils/timespan.h"

#include <stddef.h>

/// Representation of a date and time. This type holds an underlying `int64_t` that stores
/// the date and time as a number of 100-nanoseconds interval since 1 January 12:00 AM 1 A.D
/// in the Gregorian Calendar
typedef struct cb_date_time
{
    uint64_t ticks;
} cb_date_time;

typedef struct cb_ymd
{
    uint32_t year;
    uint32_t month;
    uint32_t day;
} cb_ymd;

typedef struct cb_hms
{
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
} cb_hms;

/// Creates a new `cb_date_time` from a given number of ticks
cb_date_time cb_date_time_from_ticks(uint64_t ticks);

/// Creates a new `cb_date_time` from a year, month and day
cb_date_time cb_date_time_from_ymd(uint32_t year, uint32_t month, uint32_t day);

/// Creates a new `cb_date_time` from a date year, month and day and time hours, minutes and seconds
cb_date_time cb_date_time_from_ymd_hms(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds);

/// Creates a new `cb_date_time` representing the current UTC date and time
cb_date_time cb_date_time_utc_now();

cb_date_time cb_date_time_since_epoch(cb_date_time dt);

/// Get the corresponding year for the given date time
uint32_t cb_date_time_year(cb_date_time dt);

/// Get the corresponding month for the given date time
uint32_t cb_date_time_month(cb_date_time dt);

/// Get the corresponding day for the given date time
uint32_t cb_date_time_day(cb_date_time dt);

/// Get the year, month and day part of the given date time
cb_ymd cb_date_time_ymd(cb_date_time dt);

/// Get the hours, minutes and seconds part of the given date time
cb_hms cb_date_time_hms(cb_date_time dt);

/// Get the corresponding hours for the given date time
uint32_t cb_date_time_hours(cb_date_time dt);

/// Get the corresponding minutes for the given date time
uint32_t cb_date_time_minutes(cb_date_time dt);

/// Get the corresponding seconds for the given date time
uint32_t cb_date_time_seconds(cb_date_time dt);

/// Compares two date and returns `cebus_true` if equal
cebus_bool cb_date_time_eq(cb_date_time lhs, cb_date_time rhs);

/// Returns `cebus_true` if the given date time is valid
cebus_bool cb_date_time_valid(cb_date_time dt);

/// Returns the tick count for the given date time
uint64_t cb_date_time_ticks(cb_date_time dt);

/// Write a simple string representation of the given date time to `s` and return
/// the number of characters written
size_t cb_date_time_print(cb_date_time dt, char* s, size_t size);

typedef struct time_instant
{
} time_instant;

time_instant time_instant_now();
timespan time_instant_elapsed(const time_instant* instant);
